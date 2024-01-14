#include "workbookbase.h"

#include <codecvt>
#include <locale>
#include <wx/sstream.h>
#include <wx/txtstrm.h>

#include "ntbkbase.h"
#include "worksheetbase.h"
#include "undoredo.h"
#include "ws_cell.h"
#include "ws_xml.h"

#include "events.h"

namespace grid
{
	CWorkbookBase::CWorkbookBase(wxWindow* parent) : wxPanel(parent)
	{
	}

	CWorkbookBase::~CWorkbookBase() = default;

	CWorksheetBase* CWorkbookBase::GetActiveWS() const {
		return m_WSNtbk->GetActiveWorksheet();
	}

	CWorksheetBase* CWorkbookBase::GetWorksheet(const std::wstring& worksheetname) const {
		return m_WSNtbk->GetWorksheet(worksheetname);
	}

	CWorksheetBase* CWorkbookBase::GetWorksheet(const size_t PageNumber) const {
		return m_WSNtbk->FindWorksheet(PageNumber);
	}

	size_t CWorkbookBase::size() const {
		return m_WSNtbk->GetPageCount();
	}

	
	bool CWorkbookBase::AddNewWorksheet(const std::wstring& tblname, int nrows, int ncols)
	{
		return GetWorksheetNotebook()->AddNewWorksheet(tblname, nrows, ncols);
	}


	void CWorkbookBase::ShowWorksheet(const std::wstring& worksheetname) const
	{
		auto worksheet = GetWorksheet(worksheetname);
		int index = m_WSNtbk->GetPageIndex(worksheet->GetParent());

		m_WSNtbk->SetSelection(index);
	}


	void CWorkbookBase::ShowWorksheet(const grid::CWorksheetBase* worksheet) const
	{
		int index = m_WSNtbk->GetPageIndex(worksheet->GetParent());

		m_WSNtbk->SetSelection(index);
	}


	void CWorkbookBase::PushUndoEvent(std::unique_ptr<WorksheetUndoRedoEvent> event)
	{
		//Check if current event and event on the top of redo stack are the same
		if (!m_UndoStack.empty())
		{
			auto top = m_UndoStack.top().get();
			if (!(top == event.get()))
				m_UndoStack.push(std::move(event));
		}
		else
			m_UndoStack.push(std::move(event));

		//At any undoable event that is pushed onto stack, clear Redo stack
		std::stack<std::unique_ptr<grid::WorksheetUndoRedoEvent>>().swap(m_RedoStack);

		wxCommandEvent CmdEvt(ssEVT_WORKBOOK_UNDOREDOSTACKCHANGED, GetId());
		CmdEvt.SetEventObject(this);
		ProcessWindowEvent(CmdEvt);
	}


	void CWorkbookBase::ProcessUndoEvent()
	{
		if (m_UndoStack.empty())
			return;

		auto UndoRedoEvt = std::move(m_UndoStack.top());
		UndoRedoEvt->Undo();

		//If it is a redoable event then push it onto redo stack
		if (UndoRedoEvt->CanRedo())
			m_RedoStack.push(std::move(UndoRedoEvt));

		//We are done, pop it from Undo
		m_UndoStack.pop();

		wxCommandEvent CmdEvt(ssEVT_WORKBOOK_UNDOREDOSTACKCHANGED, GetId());
		CmdEvt.SetEventObject(this);
		ProcessWindowEvent(CmdEvt);
	}



	void CWorkbookBase::ProcessRedoEvent()
	{
		if (m_RedoStack.empty())
			return;

		auto UndoRedoEvt = std::move(m_RedoStack.top());
		UndoRedoEvt->Redo();

		//Every redoable event can be undone, therefore, push it onto Undo stack
		m_UndoStack.push(std::move(UndoRedoEvt));

		//We are done, pop it from Redo
		m_RedoStack.pop();

		wxCommandEvent CmdEvt(ssEVT_WORKBOOK_UNDOREDOSTACKCHANGED, GetId());
		CmdEvt.SetEventObject(this);
		ProcessWindowEvent(CmdEvt);
	}


	void CWorkbookBase::EnableEditing(bool Enable)
	{
		grid::CWorksheetBase* ws{ nullptr };
		for (size_t i = 0; i < m_WSNtbk->GetPageCount(); i++)
		{
			ws = m_WSNtbk->FindWorksheet(i);
			if (ws)
				ws->EnableCellEditControl(Enable);
		}
	}


	void CWorkbookBase::TurnOnGridSelectionMode(bool IsOn)
	{
		grid::CWorksheetBase* ws{ nullptr };
		for (size_t i = 0; i < m_WSNtbk->GetPageCount(); i++)
		{
			ws = m_WSNtbk->FindWorksheet(i);
			if (ws)
				ws->TurnOnGridSelectionMode(IsOn);
		}
	}


	bool CWorkbookBase::Write(const std::filesystem::path& SnapshotDir)
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;

		std::stringstream XML;
		XML << "<?xml version=\"1.0\" encoding=\"utf-8\" ?> \n";
		XML << "<WORKBOOK>\n"; //root tag

		int NWorksheet = 0;

		auto WS_DirPath = SnapshotDir / "worksheets";

		if (!std::filesystem::exists(WS_DirPath))
			std::filesystem::create_directory(WS_DirPath);

		for (size_t PgNum = 0; PgNum < m_WSNtbk->size(); ++PgNum)
		{
			//Save worksheets with names sheet1, sheet2 under worksheets folder
			auto WS = GetWorksheet(PgNum);

			auto WS_Path = WS_DirPath / ("sheet" + std::to_string(++NWorksheet) + ".xml");

			wxFile file;
			if (!file.Create(WS_Path.wstring(), true))
				return false;

			if (!file.Open(WS_Path.wstring(), wxFile::write))
				return false;

			std::string Contents = xml::GenerateXMLString(WS); //UTF8
			if (!file.Write(Contents))
				return false;

			file.Close();

			//Create info on each worksheet for workbook.xml, collect info from each worksheet 
			XML << "<WORKSHEET" << " ENAME=" << "\"" << "sheet" << NWorksheet << "\"";

			XML << " NROWS=" << "\"" << WS->GetNumberRows() << "\"";
			XML << " NCOLS=" << "\"" << WS->GetNumberCols() << "\"";

			XML << " NAME=" << "\"" << converter.to_bytes(WS->GetWSName()) << "\"";

			XML << "></WORKSHEET> \n";
		}
		XML << "<ACTIVE" << " NAME=" << "\"" << converter.to_bytes(GetActiveWS()->GetWSName()) << "\"" << "></ACTIVE> \n";

		XML << "</WORKBOOK>"; //end of XML

		std::filesystem::path WB_Path = SnapshotDir / "workbook.xml";

		wxFile file;
		file.Create(WB_Path.wstring(), true);
		file.Open(WB_Path.wstring(), wxFile::write);

		return file.Write(XML.str()) && file.Close();
	}


	bool CWorkbookBase::Read(const std::filesystem::path& SnapshotDir)
	{
		wxFile file;
		file.Open((SnapshotDir / "workbook.xml").wstring());

		wxString WB_XMLContent;
		file.ReadAll(&WB_XMLContent);

		wxStringInputStream stringstream(WB_XMLContent);
		wxXmlDocument xmlDoc(stringstream);

		wxXmlNode* root_node = xmlDoc.GetRoot();
		if (!root_node)
		{
			//workbook.xml is malformed
			wxMessageBox("The project file is malformed!", "ERROR");
			return false;
		}

		wxString ActiveWSName = wxEmptyString;

		wxXmlNode* Child = root_node->GetChildren();
		while (Child)
		{
			wxString NodeName = Child->GetName();
			if (NodeName == "WORKSHEET")
			{
				//sheet1.xml ...
				auto WSEntryName = Child->GetAttribute("ENAME");
				auto WSName = wxString::FromUTF8(Child->GetAttribute("NAME"));

				long NRows = 0, NCols = 0;
				Child->GetAttribute("NROWS").ToLong(&NRows);
				Child->GetAttribute("NCOLS").ToLong(&NCols);

				AddNewWorksheet(WSName.ToStdWstring(), NRows, NCols);

				GetActiveWS()->ReadXMLDoc(SnapshotDir / L"worksheets" / (WSEntryName + ".xml").ToStdWstring());
			}

			else if (NodeName == "ACTIVE")
				ActiveWSName = wxString::FromUTF8(Child->GetAttribute("NAME"));

			Child = Child->GetNext();
		}


		//Set the page to the last active worksheet
		if (!ActiveWSName.empty())
		{
			if (auto pos = m_WSNtbk->FindWorksheet(ActiveWSName.ToStdWstring()))
				m_WSNtbk->SetSelection(pos.value());
		}

		MarkClean();

		return true;
	}



	void CWorkbookBase::MarkDirty()
	{
		m_IsDirty = true;
		
		wxCommandEvent evt(ssEVT_WORKBOOK_DIRTY, GetId());
		evt.SetEventObject(this);
		ProcessWindowEvent(evt);
	}


	void CWorkbookBase::MarkClean()
	{
		m_IsDirty = false;
		wxCommandEvent evt(ssEVT_WORKBOOK_CLEAN, GetId());
		evt.SetEventObject(this);
		ProcessWindowEvent(evt);
	}



	std::pair<wxGridCellCoords, wxGridCellCoords> CWorkbookBase::GetSelectionCoords() const
	{
		auto ws = GetActiveWS();

		wxGridCellCoords TL, BR;

		if (!ws->IsSelection()) 
		{
			int row = ws->GetGridCursorRow();
			int col = ws->GetGridCursorCol();
			TL.SetRow(row);
			TL.SetCol(col);

			BR = TL;
		}
		else {
			//There is selection
			TL = ws->GetSelTopLeft();
			BR = ws->GetSelBtmRight();
		}

		return { TL, BR };
	}


	void CWorkbookBase::ChangeCellAlignment(int wxAlignID, bool RefreshBlock)
	{
		auto ws = GetActiveWS();
		const auto& [TL, BR] = GetSelectionCoords();

		auto alignEvt = std::make_unique<grid::CellAlignmentChangedEvent>(ws);
		alignEvt->m_TL = TL;
		alignEvt->m_BR = BR;
		alignEvt->m_InitVal = ws->GetBlock(TL, BR);

		//Change
		ws->SetBlockCellAlignment(TL, BR, wxAlignID);

		//After change
		alignEvt->m_LastVal = ws->GetBlock(TL, BR);

		PushUndoEvent(std::move(alignEvt));

		ws->MarkDirty();
		if (RefreshBlock)
			ws->RefreshBlock(TL, BR);
	}


	void CWorkbookBase::ChangeCellBGColor(const wxColor& BGColor, bool RefreshBlock)
	{
		auto ws = GetActiveWS();
		const auto& [TL, BR] = GetSelectionCoords();

		auto evt = std::make_unique<grid::CellBGColorChangedEvent>(ws);
		evt->m_TL = TL;
		evt->m_BR = BR;
		evt->m_InitVal = ws->GetBlock(TL, BR);

		//Change
		ws->SetBlockBackgroundColor(TL, BR, BGColor);

		//After change
		evt->m_LastVal = ws->GetBlock(TL, BR);

		PushUndoEvent(std::move(evt));

		ws->MarkDirty();
		if (RefreshBlock)
			ws->RefreshBlock(TL, BR);
	}


	void CWorkbookBase::ChangeTextColor(const wxColor& TxtColor, bool RefreshBlock)
	{
		auto ws = GetActiveWS();
		const auto& [TL, BR] = GetSelectionCoords();
		
		auto evt = std::make_unique<grid::TextColorChangedEvent>(ws);
		evt->m_TL = TL;
		evt->m_BR = BR;
		evt->m_InitVal = ws->GetBlock(TL, BR);

		//Change
		ws->SetBlockTextColour(TL, BR, TxtColor);

		//After change
		evt->m_LastVal = ws->GetBlock(TL, BR);

		PushUndoEvent(std::move(evt));
		
		ws->MarkDirty();
		if (RefreshBlock)
			ws->RefreshBlock(TL, BR);
	}


	void CWorkbookBase::ChangeFontSize(int FontPointSize, bool RefreshBlock)
	{
		auto ws = GetActiveWS();
		const auto& [TL, BR] = GetSelectionCoords();

		auto fntChanged = std::make_unique<grid::FontChangedEvent>(ws);
		fntChanged->m_InitVal = ws->GetBlock(TL, BR);
		fntChanged->m_TL = TL;
		fntChanged->m_BR = BR;
		fntChanged->m_FontProperty = "size";

		//Change the font
		for (int i = TL.GetRow(); i <= BR.GetRow(); i++)
		{
			for (int j = TL.GetCol(); j <= BR.GetCol(); j++)
			{
				wxFont CellFont = ws->GetCellFont(i, j);
				CellFont.SetPointSize(FontPointSize);

				ws->SetCellFont(i, j, CellFont);
			}
			
			ws->AdjustRowHeight(i);
		}

		//Font already changed
		fntChanged->m_LastVal = ws->GetBlock(TL, BR);

		PushUndoEvent(std::move(fntChanged));

		ws->MarkDirty();
		if (RefreshBlock)
			ws->RefreshBlock(TL, BR);
	}


	void CWorkbookBase::ToggleFontWeight(bool RefreshBlock)
	{
		auto ws = GetActiveWS();
		const auto& [TL, BR] = GetSelectionCoords();

		auto fntChanged = std::make_unique<grid::FontChangedEvent>(ws);
		fntChanged->m_InitVal = ws->GetBlock(TL, BR);
		fntChanged->m_TL = TL;
		fntChanged->m_BR = BR;
		fntChanged->m_FontProperty = "bold";


		//Font is changing
		for (int i = TL.GetRow(); i <= BR.GetRow(); i++) 
		{
			for (int j = TL.GetCol(); j <= BR.GetCol(); j++)
			{
				wxFont font = ws->GetCellFont(i, j);
				
				if (font.GetWeight() == wxFONTWEIGHT_BOLD)
					font.SetWeight(wxFONTWEIGHT_NORMAL);
				else
					font.MakeBold();

				ws->SetCellFont(i, j, font);
			}
		}

		//Font already changed
		fntChanged->m_LastVal = ws->GetBlock(TL, BR);

		PushUndoEvent(std::move(fntChanged));

		ws->MarkDirty();

		if (RefreshBlock)
			ws->RefreshBlock(TL, BR);
		
	}


	void CWorkbookBase::ToggleFontStyle(bool RefreshBlock)
	{
		auto ws = GetActiveWS();
		const auto& [TL, BR] = GetSelectionCoords();

		auto fntChanged = std::make_unique<grid::FontChangedEvent>(ws);
		fntChanged->m_InitVal = ws->GetBlock(TL, BR);
		fntChanged->m_TL = TL;
		fntChanged->m_BR = BR;
		fntChanged->m_FontProperty = "italic";


		//Font is changing
		for (int i = TL.GetRow(); i <= BR.GetRow(); i++) 
		{
			for (int j = TL.GetCol(); j <= BR.GetCol(); j++)
			{
				wxFont font = ws->GetCellFont(i, j);
				
				if (font.GetStyle() == wxFONTSTYLE_ITALIC)
					font.SetStyle(wxFONTSTYLE_NORMAL);
				else
					font.MakeItalic();

				ws->SetCellFont(i, j, font);
			}
		}

		//Font already changed
		fntChanged->m_LastVal = ws->GetBlock(TL, BR);

		PushUndoEvent(std::move(fntChanged));

		ws->MarkDirty();

		if (RefreshBlock)
			ws->RefreshBlock(TL, BR);
	}


	void CWorkbookBase::ToggleFontUnderlined(bool RefreshBlock)
	{
		auto ws = GetActiveWS();
		const auto& [TL, BR] = GetSelectionCoords();

		auto fntChanged = std::make_unique<grid::FontChangedEvent>(ws);
		fntChanged->m_InitVal = ws->GetBlock(TL, BR);
		fntChanged->m_TL = TL;
		fntChanged->m_BR = BR;
		fntChanged->m_FontProperty = "underlined";

		//Font is changing
		for (int i = TL.GetRow(); i <= BR.GetRow(); i++) 
		{
			for (int j = TL.GetCol(); j <= BR.GetCol(); j++)
			{
				wxFont font = ws->GetCellFont(i, j);
				
				if (font.GetUnderlined())
					font.SetUnderlined(false);
				else
					font.MakeUnderlined();

				ws->SetCellFont(i, j, font);
			}
		}

		//Font already changed
		fntChanged->m_LastVal = ws->GetBlock(TL, BR);

		PushUndoEvent(std::move(fntChanged));

		ws->MarkDirty();

		if (RefreshBlock)
			ws->RefreshBlock(TL, BR);
	}

	


	void CWorkbookBase::ChangeFontFace(const wxString& fontFaceName, bool RefreshBlock)
	{
		auto ws = GetActiveWS();
		const auto& [TL, BR] = GetSelectionCoords();

		auto fntChanged = std::make_unique<grid::FontChangedEvent>(ws);
		fntChanged->m_InitVal = ws->GetBlock(TL, BR);
		fntChanged->m_TL = TL;
		fntChanged->m_BR = BR;
		fntChanged->m_FontProperty = "face";

		//Change the font
		for (int i = TL.GetRow(); i <= BR.GetRow(); i++)
		{
			for (int j = TL.GetCol(); j <= BR.GetCol(); j++)
			{
				wxFont CellFont = ws->GetCellFont(i, j);
				CellFont.SetFaceName(fontFaceName);

				ws->SetCellFont(i, j, CellFont);
			}
		}

		//Font already changed
		fntChanged->m_LastVal = ws->GetBlock(TL, BR);

		PushUndoEvent(std::move(fntChanged));

		ws->MarkDirty();
		if (RefreshBlock)
			ws->RefreshBlock(TL, BR);
	}


	bool CWorkbookBase::PasteValues(const wxDataFormat& ClipbrdFormat)
	{
		auto ws = GetActiveWS();
		auto evt = std::make_unique<grid::DataPasted>(ws);

		std::pair<wxGridCellCoords, wxGridCellCoords> Coords;
		
		if (ClipbrdFormat == grid::xml::XMLDataFormat())
			Coords = ws->Paste_XMLDataFormat(CWorksheetBase::PASTE::VALUES);

		else if (ClipbrdFormat == wxDF_TEXT)
			Coords = ws->Paste_TextValues();
		
		evt->SetPaste((int)CWorksheetBase::PASTE::VALUES);
		evt->SetCoords(Coords.first, Coords.second);

		PushUndoEvent(std::move(evt));

		return true;
	}


	bool CWorkbookBase::PasteFormat(const wxDataFormat& ClipbrdFormat, bool RefreshBlock)
	{
		if (ClipbrdFormat != grid::xml::XMLDataFormat())
			return false;

		auto ws = GetActiveWS();

		auto evt = std::make_unique<grid::DataPasted>(ws);		
		auto Coords = ws->Paste_XMLDataFormat(CWorksheetBase::PASTE::FORMAT);
		
		evt->SetPaste((int)CWorksheetBase::PASTE::FORMAT);
		evt->SetCoords(Coords.first, Coords.second);

		PushUndoEvent(std::move(evt));

		if(RefreshBlock)
			ws->RefreshBlock(Coords.first, Coords.second);

		return true;
	}
}