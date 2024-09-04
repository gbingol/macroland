#include "icell.h"

#include <string>
#include <locale>
#include <codecvt>

#include <wx/artprov.h>
#include <wx/colordlg.h>


#include "../python/guielements.h"

#include "../macrolandapp.h"
#include "frmmacroland.h"

#include "../util/util_wx.h"
#include "../util/json.h"

#include "../cmdwnd/scripting_funcs.h"

#include "../grid/ws_funcs.h"
#include "../grid/rangebase.h"
#include "../grid/worksheetbase.h"
#include "../grid/workbookbase.h"
#include "../grid/ws_cell.h"
#include "../grid/undoredo.h"
#include "../grid/events.h"


#include "../icons/sz16/icons16.h"
#include "../icons/sz32/icons32.h"

#include "../consts.h"






extern std::filesystem::path glbExeDir;
extern ICELL::CWorkbook* glbWorkbook;
extern JSON::Value glbSettings;


namespace ICELL
{
	wxDEFINE_EVENT(ssEVT_WB_PAGECHANGED, wxAuiNotebookEvent);



	CWorksheet::CWorksheet(wxWindow* parent,
		CWorkbook* workbook,
		wxWindowID id,
		const wxPoint& pos,
		const wxSize& size,
		long style,
		wxString WindowName,
		int nrows,
		int ncols)
		:grid::CWorksheetBase(parent, workbook, id, pos, size, style, WindowName, nrows, ncols)
	{
		m_Workbook = workbook;
		m_ParentWnd = parent;

		SetWSName(WindowName.ToStdWstring());

		m_EvtCallBack["selecting"] = std::list<std::unique_ptr<Python::CEventCallbackFunc>>();
		m_EvtCallBack["selected"] = std::list<std::unique_ptr<Python::CEventCallbackFunc>>();

		auto JSONObject = glbSettings.as_object();
		if(JSONObject.contains("EventsFolder_Fire") && JSONObject["EventsFolder_Fire"].is_object()) 
		{
			auto FireObj = JSONObject["EventsFolder_Fire"].as_object();
			if(FireObj.contains("selecting") && FireObj["selecting"].is_bool())
				m_FirePySelectingEvt = FireObj["selecting"].as_bool();
			
			if(FireObj.contains("selected") && FireObj["selected"].is_bool())
				m_FirePySelectedEvt = FireObj["selected"].as_bool();
		}

		Bind(wxEVT_GRID_RANGE_SELECTING, &CWorksheet::OnRangeSelecting, this);
		Bind(wxEVT_GRID_RANGE_SELECTED, &CWorksheet::OnRangeSelected, this);
		Bind(wxEVT_GRID_SELECT_CELL, &CWorksheet::OnSelectCell, this);

		Bind(wxEVT_GRID_CELL_RIGHT_CLICK, &CWorksheet::OnRightClick, this);
	}



	CWorksheet::~CWorksheet()
	{
		for (auto& elem : m_PyWS)
		{
			if (elem && elem->state)
			{
				elem->state = false;
				Py_XDECREF(elem);
			}
		}
	}


	void CWorksheet::BindPyFunc(std::string EventName, 
								std::unique_ptr<Python::CEventCallbackFunc> Callbackfunc)
	{
		if(m_EvtCallBack.contains(EventName))
			m_EvtCallBack[EventName].push_back(std::move(Callbackfunc));
	}


	void CWorksheet::UnbindPyFunc(std::string EventName, 
								  PyObject* FunctionObj)
	{
		auto& List = m_EvtCallBack[EventName];
		std::erase_if(List, [&](auto& Elem) {
			return Elem->m_Func == FunctionObj;
		});
	}


	void CWorksheet::OnRightClick(wxGridEvent& event)
	{
		if (GetNumSelRows() == 1 && GetNumSelCols() == 1)
			return;

		if (m_ContextMenu) {
			delete m_ContextMenu;
			m_ContextMenu = nullptr;
		}

		m_ContextMenu = new wxMenu();

		auto Menu_Copy = m_ContextMenu->Append(wxID_ANY, "Copy");
		Menu_Copy->SetBitmap(wxArtProvider::GetBitmap(wxART_COPY));
		m_ContextMenu->Bind(wxEVT_MENU, [this](wxCommandEvent& e) { Copy(); }, Menu_Copy->GetId());

		auto Menu_Cut = m_ContextMenu->Append(wxID_ANY, "Cut");
		Menu_Cut->SetBitmap(wxArtProvider::GetBitmap(wxART_CUT));
		m_ContextMenu->Bind(wxEVT_MENU, [this](wxCommandEvent& e) { Cut(); }, Menu_Cut->GetId());

		if (!IsSelection()) {
			m_ContextMenu->AppendSeparator();
			
			auto Menu_Paste = m_ContextMenu->Append(wxID_ANY, "Paste");
			Menu_Paste->SetBitmap(wxArtProvider::GetBitmap(wxART_PASTE));
			Menu_Paste->Enable(false);

			if (!wxTheClipboard->IsSupported(wxDF_INVALID))
			{
				Menu_Paste->Enable(true);
				m_ContextMenu->Bind(wxEVT_MENU, [this](wxCommandEvent &event) { 
					Paste(); 
				}, Menu_Paste->GetId());

				if (grid::SupportsXML()) {
					auto PasteVal = m_ContextMenu->Append(wxID_ANY, "Paste Values");
					auto PasteFrmt = m_ContextMenu->Append(wxID_ANY, "Paste Format");

					m_ContextMenu->Bind(wxEVT_MENU, [this](wxCommandEvent& ) { 
						GetWorkbook()->PasteValues(wxDF_TEXT); 
					}, PasteVal->GetId());

					m_ContextMenu->Bind(wxEVT_MENU, [this](wxCommandEvent& ) { 
						GetWorkbook()->PasteFormat(grid::XMLDataFormat()); 
					}, PasteFrmt->GetId());
				}
			}

			PopupMenu(m_ContextMenu);

			return;
		}


		auto Menu_Del = m_ContextMenu->Append(wxID_ANY, "Delete");
		Menu_Del->SetBitmap(wxArtProvider::GetBitmap(wxART_DELETE));
		m_ContextMenu->Bind(wxEVT_MENU, [&](wxCommandEvent& e) 
		{ 
			Delete(); 
		}, Menu_Del->GetId());

		Python::RunExtensions(L"_wb_menu.py");

		PopupMenu(m_ContextMenu);

		event.Skip();
	}


	void CWorksheet::CallRegisteredPyFuncs(const std::string& event)
	{
		if (m_EvtCallBack[event].size() > 0) {
			const auto& List = m_EvtCallBack[event];
			for (const auto& CallBk : List)
				CallBk->call(CallBk->m_Func, CallBk->m_Args);
		}
	}



	void CWorksheet::OnRangeSelecting(wxGridRangeSelectEvent& event)
	{
		if (IsSelection()) 
			CallRegisteredPyFuncs("selecting");

		event.Skip();
	}


	void CWorksheet::OnRangeSelected(wxGridRangeSelectEvent& event)
	{
		auto PySelected = [this]()
		{
			auto Path = glbExeDir / Info::EVENTS / "ws_selected.py";
			if(std::filesystem::exists(Path))
			{
				auto gstate = PyGILState_Ensure();
				std::wstring_convert<std::codecvt_utf8<wchar_t>> cvt;
				if (auto cp = _Py_wfopen(Path.c_str(), L"rb"))
					PyRun_SimpleFileExFlags(cp, cvt.to_bytes(Path).c_str(), true, 0);
				
				PyGILState_Release(gstate);
			}
		};

		if (event.ShiftDown()) 
		{
			OnRangeSelecting(event);
			if(m_FirePySelectedEvt)
				PySelected();
			
			event.Skip();
			return;
		}

		if (event.Selecting())
		{
			CallRegisteredPyFuncs("selected");

			if(m_FirePySelectedEvt)
				PySelected();
		}

		event.Skip();
	}


	void CWorksheet::OnSelectCell(wxGridEvent& event)
	{
		wxString StatBarInfo;
		StatBarInfo << grid::ColNumtoLetters(event.GetCol() + 1) << (event.GetRow() + 1);

		GetWorkbook()->SetStatusText(StatBarInfo, 1);

		event.Skip();
	}





	/******************************************************* */

	CWorksheetNtbk::CWorksheetNtbk(CWorkbook* parent) :
		grid::CWorksheetNtbkBase(parent), m_Workbook{ parent }
	{
		Bind(wxEVT_AUINOTEBOOK_PAGE_CHANGED, &CWorksheetNtbk::OnPageChanged, this);	
	}


	CWorksheetNtbk::~CWorksheetNtbk() = default;


	void CWorksheetNtbk::OnPageChanged(wxAuiNotebookEvent& evt) 
	{
		m_ActiveWS = FindWorksheet(evt.GetSelection());
		m_ActiveWS->ClearSelection();
		m_ActiveWS->SetFocus();

		if (m_ActiveWS)
			cmdedit::RunPyFile(glbExeDir / Info::EVENTS / "ws_pagechanged.py");

		wxAuiNotebookEvent PageEvt(evt);
		PageEvt.SetEventType(ssEVT_WB_PAGECHANGED);
		ProcessWindowEvent(PageEvt);
	}


	void CWorksheetNtbk::OnTabRightDown(wxAuiNotebookEvent& evt)
	{
		Python::RunExtensions(L"_wb_tab_menu.py");

		PopupMenu(m_ContextMenu);
	}

	grid::CWorksheetBase* CWorksheetNtbk::CreateWorksheet(
		wxWindow* wnd, 
		const std::wstring& Label, 
		int nrows, 
		int ncols) const
	{
		return  new CWorksheet(wnd, 
					m_Workbook, 
					wxID_ANY, 
					wxDefaultPosition, 
					wxDefaultSize, 0, Label, nrows, ncols);;
	}




	/******************************************************************************* */

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

		m_PagedTB = new extension::CToolBarNtbk(this);
		Init_TB_Home();
		m_WSNtbk = new CWorksheetNtbk(this);


		auto szrMain = new wxBoxSizer(wxVERTICAL);
		szrMain->Add(m_PagedTB, 0, wxEXPAND, 0);
		szrMain->Add(m_WSNtbk, 1, wxEXPAND, 0);
		SetSizerAndFit(szrMain);
		Layout();

		m_WSNtbk->Bind(ssEVT_WB_PAGECHANGED, &CWorkbook::OnWorkbookPageChanged, this);
		Bind(ssEVT_WB_UNDOREDO, &CWorkbook::OnUndoRedoStackChanged, this);

		Bind(ssEVT_WB_DIRTY, [&](wxCommandEvent& event) { m_TB_Home->EnableTool(ID_COMMIT, true); });
		Bind(ssEVT_WB_CLEAN, [&](wxCommandEvent& event) { m_TB_Home->EnableTool(ID_COMMIT, false);});

		m_PagedTB->Bind(wxEVT_NOTEBOOK_PAGE_CHANGED, [this](wxNotebookEvent& event)
		{
			GetActiveWS()->SetFocus();
		});


		m_EvtCallBack["pagechanged"] = std::list<std::unique_ptr<Python::CEventCallbackFunc>>();
	}


	CWorkbook::~CWorkbook() = default;


	void CWorkbook::Init_TB_Home()
	{
		m_TB_Home = m_PagedTB->AddPage(L"Home")->GetToolBar();
		auto Menu = m_TB_Home->AddTool(ID_COMMIT, "Commit", wxArtProvider::GetBitmap(wxART_GO_UP), "Commit changes");

		m_TB_Home->AddSeparator();

		auto Bold = m_TB_Home->AddTool(ID_FONTBOLD, "Bold", font_bold_xpm, "Make bold", wxITEM_CHECK);
		auto Italic = m_TB_Home->AddTool(ID_FONTITALIC, "Italic", font_italic_xpm, "Make italic", wxITEM_CHECK);
		
		//horizontal and vertical alignments
		m_TB_Home->AddTool(ID_ALIGN, "Align", align_horizontalcenter_xpm, "Align cell content in vertical/horizontal directions");

		m_TB_Home->AddTool(ID_FILLCOLOR, "Fill", wxBitmap(format_fillcolor_xpm), "Fill Color");
		m_TB_Home->AddTool(ID_FONTCOLOR, "Font", wxBitmap(format_fontcolor_xpm), "Font Color");
		m_TB_Home->SetToolDropDown(ID_FILLCOLOR, true);
		m_TB_Home->SetToolDropDown(ID_FONTCOLOR, true);

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
		if(frmSciSuit)
		{
			auto StatBar = frmSciSuit->GetStatusBar();
			if(StatBar)
				StatBar->SetStatusText(text, number);
		}
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


	void CWorkbook::BindPyFunc(std::string EventName, 
									   std::unique_ptr<Python::CEventCallbackFunc> Callbackfunc)
	{
		if (m_EvtCallBack.contains(EventName))
			m_EvtCallBack[EventName].push_back(std::move(Callbackfunc));
	}



	void CWorkbook::UnbindPyFunc(std::string EventName, PyObject* FunctionObj)
	{
		if (m_EvtCallBack.find(EventName) != m_EvtCallBack.end())
		{
			auto& List = m_EvtCallBack[EventName];
			std::erase_if(List, [&](auto& elem)
			{
				return elem->m_Func == FunctionObj;
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
				CallBackFunc->call(CallBackFunc->m_Func, CallBackFunc->m_Args);

			PyGILState_Release(gstate);
		}
	}

}