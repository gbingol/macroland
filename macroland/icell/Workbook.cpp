#include "workbook.h"

#include <codecvt>
#include <locale>

#include <wx/artprov.h>
#include <wx/fontenum.h>

#include <script/scripting_funcs.h>

#include "../macrolandapp.h"
#include "../mainfrm/frmmacroland.h"
#include "../util_funcs.h"

#include "worksheet.h"


#include <grid/ws_xml.h>
#include <grid/worksheetbase.h>
#include <grid/ws_cell.h>
#include <grid/workbookbase.h>
#include <grid/events.h>
#include <grid/undoredo.h>
#include "../lua/lua_guielements.h"

#include "../icons/sz16/icons16.h"
#include "../icons/sz32/icons32.h"
#include "../icons/sz32/menu.xpm"



 
namespace ICELL
{
	CPopupAlignment::CPopupAlignment(CWorkbook* parent, const wxPoint& Position) :
		wxPopupTransientWindow(parent)
	{
		m_Parent = parent;


		m_btnHorLeft = new wxBitmapToggleButton(this, wxID_JUSTIFY_LEFT, align_left_xpm);
		m_btnHorCenter = new wxBitmapToggleButton(this, wxID_JUSTIFY_CENTER, align_horizontalcenter_xpm);
		m_btnHorRight = new wxBitmapToggleButton(this, wxID_JUSTIFY_RIGHT, align_right_xpm);

		m_btnVertBtm = new wxBitmapToggleButton(this, wxALIGN_BOTTOM, align_verticalbottom_xpm);
		m_btnVertCenter = new wxBitmapToggleButton(this, wxALIGN_CENTRE, align_verticalcenter_xpm);
		m_btnVertTop = new wxBitmapToggleButton(this, wxALIGN_TOP, align_verticaltop_xpm);


		auto ws = parent->GetActiveWS();

		int row = ws->GetGridCursorRow();
		int col = ws->GetGridCursorCol();
		int hor = 0, ver = 0;

		ws->GetCellAlignment(row, col, &hor, &ver);

		m_btnHorLeft->SetValue(hor == wxALIGN_LEFT);
		m_btnHorCenter->SetValue(hor == wxALIGN_CENTRE);
		m_btnHorRight->SetValue(hor == wxALIGN_RIGHT);

		auto VertBtns = { m_btnVertBtm , m_btnVertCenter , m_btnVertTop };
		for (auto btn : VertBtns)
			btn->SetValue(btn->GetId() == ver);

		auto sbSzrHoriz = new wxBoxSizer(wxHORIZONTAL);
		sbSzrHoriz->Add(m_btnHorLeft, 0, 0, 5);
		sbSzrHoriz->Add(m_btnHorCenter, 0, 0, 5);
		sbSzrHoriz->Add(m_btnHorRight, 0, 0, 5);

		auto sbSzrVert = new wxBoxSizer(wxHORIZONTAL);
		sbSzrVert->Add(m_btnVertBtm, 0, 0, 5);
		sbSzrVert->Add(m_btnVertCenter, 0, 0, 5);
		sbSzrVert->Add(m_btnVertTop, 0, 0, 5);

		auto szrMain = new wxBoxSizer(wxVERTICAL);
		szrMain->Add(sbSzrHoriz, 0, 0, 5);
		szrMain->Add(sbSzrVert, 0, 0, 5);

		SetSizerAndFit(szrMain);
		Layout();

		SetPosition(Position);

		m_btnHorLeft->Bind(wxEVT_TOGGLEBUTTON, &CPopupAlignment::OnToggleHoriz, this);
		m_btnHorCenter->Bind(wxEVT_TOGGLEBUTTON, &CPopupAlignment::OnToggleHoriz, this);
		m_btnHorRight->Bind(wxEVT_TOGGLEBUTTON, &CPopupAlignment::OnToggleHoriz, this);
		m_btnVertBtm->Bind(wxEVT_TOGGLEBUTTON, &CPopupAlignment::OnToggleVert, this);
		m_btnVertCenter->Bind(wxEVT_TOGGLEBUTTON, &CPopupAlignment::OnToggleVert, this);
		m_btnVertTop->Bind(wxEVT_TOGGLEBUTTON, &CPopupAlignment::OnToggleVert, this);
	}

	void CPopupAlignment::OnToggleHoriz(wxCommandEvent& event)
	{
		m_Parent->ChangeCellAlignment(event.GetId());

		auto BtnHors = { m_btnHorLeft , m_btnHorCenter , m_btnHorRight };
		for (auto btn : BtnHors)
			btn->SetValue(btn->GetId() == event.GetId());
	}

	void CPopupAlignment::OnToggleVert(wxCommandEvent& event)
	{
		m_Parent->ChangeCellAlignment(event.GetId());

		auto VertBtns = { m_btnVertBtm , m_btnVertCenter , m_btnVertTop };
		for (auto btn : VertBtns)
			btn->SetValue(btn->GetId() == event.GetId());
	}

	void CPopupAlignment::OnDismiss()
	{
		m_Parent->GetActiveWS()->SetFocus();
	}




	/******************************************************************/



	CWorkbook::CWorkbook(wxWindow* parent): grid::CWorkbookBase(parent)
	{
		//start with yellow color (changes as user selects a new one)
		m_FillColor = wxColor(255, 255, 0); 

		//start with red color
		m_FontColor = wxColor(255, 0, 0); 

		m_PagedTB = new lua::CToolBarNtbk(this);
		Init_TB_Home();
		m_WSNtbk = new CWorksheetNtbk(this);


		auto szrMain = new wxBoxSizer(wxVERTICAL);
		szrMain->Add(m_PagedTB, 0, wxEXPAND, 0);
		szrMain->Add(m_WSNtbk, 1, wxEXPAND, 0);
		SetSizerAndFit(szrMain);
		Layout();

		m_WSNtbk->Bind(ssEVT_WB_PAGECHANGED, &CWorkbook::OnWorkbookPageChanged, this);
		Bind(ssEVT_WORKBOOK_UNDOREDOSTACKCHANGED, &CWorkbook::OnUndoRedoStackChanged, this);

		Bind(ssEVT_WORKBOOK_DIRTY, [&](wxCommandEvent& event) { m_TB_Home->EnableTool(ID_COMMIT, true); });
		Bind(ssEVT_WORKBOOK_CLEAN, [&](wxCommandEvent& event) { m_TB_Home->EnableTool(ID_COMMIT, false);});

		m_PagedTB->Bind(wxEVT_NOTEBOOK_PAGE_CHANGED, [this](wxNotebookEvent& event)
		{
			GetActiveWS()->SetFocus();
		});


		m_EvtCallBack["pagechanged"] = std::list<Python::CEventCallbackFunc*>();
	}


	CWorkbook::~CWorkbook() = default;


	void CWorkbook::Init_TB_Home()
	{
		m_TB_Home = m_PagedTB->AddPage(L"Home")->GetToolBar();
		auto Menu = m_TB_Home->AddTool(ID_COMMIT, "Commit", wxArtProvider::GetBitmap(wxART_GO_UP), "Commit changes");

		m_TB_Home->AddSeparator();

		m_TB_Home->AddTool(ID_COPY, "Copy", wxArtProvider::GetBitmap(wxART_COPY), "Copy the selection to clipboard");
		m_TB_Home->AddTool(ID_CUT, "Cut", wxArtProvider::GetBitmap(wxART_CUT), "Remove the selection and put on the clipboard");

		m_TB_Home->AddTool(ID_PASTE, "Paste", wxArtProvider::GetBitmap(wxART_PASTE), "Paste content from clipboard");
		m_TB_Home->SetToolDropDown(ID_PASTE, true);

		m_TB_Home->AddSeparator();

		auto Bold = m_TB_Home->AddTool(ID_FONTBOLD, "Bold", font_bold_xpm, "Make bold", wxITEM_CHECK);
		auto Italic = m_TB_Home->AddTool(ID_FONTITALIC, "Italic", font_italic_xpm, "Make italic", wxITEM_CHECK);
		
		//horizontal and vertical alignments
		m_TB_Home->AddTool(ID_ALIGN, "Align", align_horizontalcenter_xpm, "Align cell content in vertical/horizontal directions");

		m_TB_Home->AddSeparator();

		m_TB_Home->AddTool(ID_FILLCOLOR, "Fill", wxBitmap(format_fillcolor_xpm), "Fill Color");
		m_TB_Home->AddTool(ID_FONTCOLOR, "Font", wxBitmap(format_fontcolor_xpm), "Font Color");
		m_TB_Home->SetToolDropDown(ID_FILLCOLOR, true);
		m_TB_Home->SetToolDropDown(ID_FONTCOLOR, true);


		m_TB_Home->AddSeparator();

		m_ComboFontFace = new wxComboBox(m_TB_Home, ID_FONTFACE, "Calibri", 
			wxDefaultPosition, wxDefaultSize, util::GetFontFaces(true));

		wxString choices[] = { "8", "9", "10", "11", "12", "14", "16", "18", "20", "24", "28" };
		m_ComboFontSize = new wxComboBox(m_TB_Home, ID_FONTSIZE, "11", 
			wxDefaultPosition, wxDefaultSize, 
			sizeof(choices)/sizeof(wxString), choices);

		m_TB_Home->AddControl(m_ComboFontFace);
		m_TB_Home->AddControl(m_ComboFontSize);

		m_TB_Home->AddSeparator();

		m_TB_Home->AddTool(ID_UNDO, "Undo", wxArtProvider::GetBitmap(wxART_UNDO), "Undo");
		m_TB_Home->AddTool(ID_REDO, "Redo", wxArtProvider::GetBitmap(wxART_REDO), "Redo");

		m_TB_Home->EnableTool(ID_UNDO, false);
		m_TB_Home->EnableTool(ID_REDO, false);

		m_TB_Home->AddSeparator();

		m_TB_Home->Realize();

		//events
		m_TB_Home->Bind(wxEVT_TOOL, [this](wxCommandEvent& event)
		{
			auto frmSciSuit = (frmMacroLand*)wxTheApp->GetTopWindow();
			Write(frmSciSuit->getProjSnapshotDir());
			frmSciSuit->MarkDirty();
			MarkClean();
		}, ID_COMMIT);
		
		
		m_TB_Home->Bind(wxEVT_AUITOOLBAR_TOOL_DROPDOWN, &CWorkbook::OnPaste, this, ID_PASTE);

		m_TB_Home->Bind(wxEVT_TOOL, [this](wxCommandEvent& ) { ((CWorksheet*)GetActiveWS())->Copy();},ID_COPY);
		m_TB_Home->Bind(wxEVT_TOOL, [this](wxCommandEvent& ) { ((CWorksheet*)GetActiveWS())->Cut(); }, ID_CUT);

		m_TB_Home->Bind(wxEVT_TOOL,[this](wxCommandEvent& ){ToggleFontWeight(); }, ID_FONTBOLD);
		m_TB_Home->Bind(wxEVT_TOOL, [this](wxCommandEvent& ) {ToggleFontStyle(); }, ID_FONTITALIC);

		
		m_TB_Home->Bind(wxEVT_AUITOOLBAR_TOOL_DROPDOWN, &CWorkbook::OnFillFontColor, this, ID_FILLCOLOR);
		m_TB_Home->Bind(wxEVT_AUITOOLBAR_TOOL_DROPDOWN, &CWorkbook::OnFillFontColor, this, ID_FONTCOLOR);

		m_ComboFontFace->Bind(wxEVT_COMBOBOX, &CWorkbook::OnFontChanged, this, ID_FONTFACE);
		m_ComboFontSize->Bind(wxEVT_COMBOBOX, &CWorkbook::OnFontChanged, this, ID_FONTSIZE);

		m_ComboFontFace->Bind(wxEVT_COMBOBOX_CLOSEUP, [this](wxCommandEvent& ) { GetActiveWS()->SetFocus(); });
		m_ComboFontSize->Bind(wxEVT_COMBOBOX_CLOSEUP, [this](wxCommandEvent& ) { GetActiveWS()->SetFocus(); });

		m_TB_Home->Bind(wxEVT_TOOL, [this](wxCommandEvent& event)
			{
				auto PopAlign = new CPopupAlignment(this, wxGetMousePosition());
				PopAlign->Popup();
			}, ID_ALIGN);


		m_TB_Home->Bind(wxEVT_TOOL, [this](wxCommandEvent& ) { ProcessUndoEvent(); }, ID_UNDO);
		m_TB_Home->Bind(wxEVT_TOOL, [this](wxCommandEvent& ) { ProcessRedoEvent(); }, ID_REDO);

		m_TB_Home->Bind(wxEVT_UPDATE_UI, &CWorkbook::OnUpdateUI, this);
		m_TB_Home->Bind(wxEVT_IDLE, [this](wxIdleEvent& event)
		{
			m_TB_Home->EnableTool(ID_PASTE, grid::xml::SupportsXML() || util::ClipbrdSupportsText());
			event.Skip();
		});
	}



	void CWorkbook::OnUpdateUI(wxUpdateUIEvent & evt)
	{
		auto ws = GetActiveWS();

		int row = ws->GetGridCursorRow();
		int col = ws->GetGridCursorCol();

		wxFont font = ws->GetCellFont(row, col);

		int evtID = evt.GetId();

		if (evtID == ID_FONTBOLD)
			evt.Check(font.GetWeight() == wxFONTWEIGHT_BOLD);

		else if (evtID == ID_FONTITALIC)
			evt.Check(font.GetStyle() == wxFONTSTYLE_ITALIC);

		else if (evtID == ID_FONTSIZE)
			m_ComboFontSize->SetValue(std::to_string(font.GetPointSize()));
		
		else if (evtID == ID_FONTFACE)
			m_ComboFontFace->SetValue(font.GetFaceName());
	}


	void CWorkbook::OnPaste(wxAuiToolBarEvent & event)
	{
		if (!event.IsDropDownClicked())
		{
			GetActiveWS()->Paste();
			return;
		}


		if (!wxTheClipboard->Open()) 
			return;

		wxMenu menu;

		if (grid::xml::SupportsXML())
		{
			menu.Append(ID_PASTE_VALUES, "Paste Values");
			menu.Append(ID_PASTE_FORMAT, "Paste Format");

			menu.Bind(wxEVT_MENU, [this](wxCommandEvent& ) { PasteValues(wxDF_TEXT); }, ID_PASTE_VALUES);
			menu.Bind(wxEVT_MENU, [this](wxCommandEvent& ) { PasteFormat(grid::xml::XMLDataFormat()); }, ID_PASTE_FORMAT);
		}

		else if (wxTheClipboard->IsSupported(wxDF_TEXT))
		{
			menu.Append(ID_PASTE_VALUES, "Paste Values");
			menu.Bind(wxEVT_MENU, [this](wxCommandEvent& ) { PasteValues(wxDF_TEXT); }, ID_PASTE_VALUES);
		}

		wxTheClipboard->Close();

		PopupMenu(&menu);
	}



	void CWorkbook::OnFontChanged(wxCommandEvent & event) //Font size or face
	{
		int evtID = event.GetId();

		if (evtID == ID_FONTSIZE)
		{
			long fontSize = 11;

			if (!m_ComboFontSize->GetStringSelection().ToLong(&fontSize)){
				wxMessageBox("ERROR: Not a valid font size!");
				return;
			}

			ChangeFontSize(fontSize);
		}

		else if (evtID == ID_FONTFACE)
		{
			wxString fontFace = m_ComboFontFace->GetStringSelection();
			wxFont fnt;
			fnt.SetFaceName(fontFace);

			if (!fnt.IsOk())
			{
				wxMessageBox("Font face is not valid!");
				return;
			}

			ChangeFontFace(fontFace);
		}
	}


	void CWorkbook::OnFillFontColor(wxAuiToolBarEvent& event)
	{
		if (!event.IsDropDownClicked())
		{
			if (event.GetId() == ID_FILLCOLOR)
				ChangeCellBGColor(m_FillColor);
			else
				ChangeTextColor(m_FontColor);

			return;
		}

		wxColourDialog dlg(wxTheApp->GetTopWindow());
		dlg.SetExtraStyle(wxTAB_TRAVERSAL);

		if (dlg.ShowModal() != wxID_OK)
			return;
		
		const auto& Color = dlg.GetColourData().GetColour();
		if (event.GetId() == ID_FILLCOLOR)
		{
			m_FillColor = Color;
			ChangeCellBGColor(m_FillColor);
			m_TB_Home->FindTool(ID_FILLCOLOR)->SetShortHelp("Fill Color: " + Color.GetAsString());
		}
		else
		{
			m_FontColor = Color;
			ChangeTextColor(m_FontColor);
			m_TB_Home->FindTool(ID_FONTCOLOR)->SetShortHelp("Font Color: " + Color.GetAsString());
		}
	}


	void CWorkbook::SetStatusText(const wxString& text, int number)
	{
		auto frmSciSuit = (frmMacroLand*)wxTheApp->GetTopWindow();
		auto StatBar = frmSciSuit->GetStatusBar();

		assert(StatBar != nullptr);
		StatBar->SetStatusText(text, number);
	}


	void CWorkbook::OnUndoRedoStackChanged(wxCommandEvent& event)
	{
		if (!GetUndoStack().empty())
		{
			auto top = GetUndoStack().top().get();
			m_TB_Home->SetToolShortHelp(ID_UNDO, top ? top->GetToolTip(true) : L"");
		}

		if (!GetRedoStack().empty())
		{
			auto top = GetRedoStack().top().get();
			m_TB_Home->SetToolShortHelp(ID_REDO, top ? top->GetToolTip(false) : L"");
		}

		m_TB_Home->EnableTool(ID_UNDO, !GetUndoStack().empty());
		m_TB_Home->EnableTool(ID_REDO, !GetRedoStack().empty());
	}


	void CWorkbook::BindPythonFunction(std::string EventName, Python::CEventCallbackFunc* Callbackfunc)
	{
		if (m_EvtCallBack.find(EventName) == m_EvtCallBack.end())
		{
			std::string s = "Event names: ";
			for (const auto& Pair : m_EvtCallBack)
				s += (Pair.first + " ");

			throw std::runtime_error(s);
		}

		m_EvtCallBack[EventName].push_back(Callbackfunc);
	}



	void CWorkbook::UnbindPythonFunction(std::string EventName, PyObject* FunctionObj)
	{
		if (m_EvtCallBack.find(EventName) != m_EvtCallBack.end())
		{
			auto& List = m_EvtCallBack[EventName];
			std::erase_if(List, [=](Python::CEventCallbackFunc* elem)
			{
				return elem->m_FuncObj == FunctionObj;
			});
		}
	}



	void CWorkbook::OnWorkbookPageChanged(wxAuiNotebookEvent& event)
	{
		if (m_EvtCallBack["pagechanged"].size() > 0)
		{
			auto gstate = PyGILState_Ensure();

			const auto& List = m_EvtCallBack["pagechanged"];
			for (const auto& CallBackFunc : List)
				CallBackFunc->call(CallBackFunc->m_FuncObj, CallBackFunc->m_FuncArgs, nullptr);

			PyGILState_Release(gstate);
		}
	}

}
