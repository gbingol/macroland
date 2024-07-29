#include "worksheet.h"

#include <string>
#include <sstream>

#include <wx/clipbrd.h>
#include <wx/xml/xml.h>
#include <wx/artprov.h>
#include <wx/tokenzr.h>

#include <lua.hpp>
#include "../lua/luautil.h"

#include "../consts.h"

#include "workbook.h"

#include <grid/ws_funcs.h>
#include <grid/rangebase.h>
#include <grid/worksheetbase.h>
#include <grid/workbookbase.h>

#include "../icons/sz32/python_logo.xpm"




wxDEFINE_EVENT(ssEVT_GRID_SELECTION_BEGUN, wxGridRangeSelectEvent);


extern std::filesystem::path glbExeDir;
extern lua_State* glbLuaState;

extern ICELL::CWorkbook* glbWorkbook;


namespace ICELL
{


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

		m_EvtCallBack["selecting"] = std::list<Python::CEventCallbackFunc*>();
		m_EvtCallBack["selected"] = std::list<Python::CEventCallbackFunc*>();

		Bind(ssEVT_GRID_SELECTION_BEGUN, &CWorksheet::OnRangeSelectionBegun, this);
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


	void CWorksheet::BindPythonFunction(
			std::string EventName, 
			Python::CEventCallbackFunc* Callbackfunc)
	{
		if (m_EvtCallBack.find(EventName) == m_EvtCallBack.end())
		{
			std::string s = "Event names: ";
			for (const auto& Pair : m_EvtCallBack)
				s += Pair.first + " ";

			throw std::exception(s.c_str());
		}

		m_EvtCallBack[EventName].push_back(Callbackfunc);
	}


	void CWorksheet::UnbindPythonFunction(
		std::string EventName, 
		PyObject* FunctionObj)
	{
		if (m_EvtCallBack.find(EventName) == m_EvtCallBack.end())	
		{
			std::string s = "Event names: ";
			for (const auto& Pair : m_EvtCallBack)
				s += Pair.first + " ";

			throw std::exception(s.c_str());
		}

		auto& List = m_EvtCallBack[EventName];
		std::erase_if(List, [&](Python::CEventCallbackFunc* Elem)
		{
			return Elem->m_FuncObj == FunctionObj;
		});
	}


	void CWorksheet::OnRightClick(wxGridEvent& event)
	{
		if (GetNumSelRows() == 1 && GetNumSelCols() == 1)
			return;

		if (m_ContextMenu)
		{
			delete m_ContextMenu;
			m_ContextMenu = nullptr;
		}

		m_ContextMenu = new wxMenu();

		if (!IsSelection())
		{
			auto Menu_Paste = m_ContextMenu->Append(wxID_ANY, "Paste");
			Menu_Paste->SetBitmap(wxArtProvider::GetBitmap(wxART_PASTE));

			if (!wxTheClipboard->IsSupported(wxDF_INVALID))
			{
				m_ContextMenu->Bind(wxEVT_MENU, [this](wxCommandEvent &event)
				{
					 Paste(); 
				});
			}
			else
				Menu_Paste->Enable(false);

			PopupMenu(m_ContextMenu);

			delete m_ContextMenu;
			m_ContextMenu = nullptr;

			return;
		}

		auto Menu_Copy = m_ContextMenu->Append(wxID_ANY, "Copy");
		Menu_Copy->SetBitmap(wxArtProvider::GetBitmap(wxART_COPY));

		auto Menu_Cut = m_ContextMenu->Append(wxID_ANY, "Cut");
		Menu_Cut->SetBitmap(wxArtProvider::GetBitmap(wxART_CUT));

		auto Menu_Del = m_ContextMenu->Append(wxID_ANY, "Delete");
		Menu_Del->SetBitmap(wxArtProvider::GetBitmap(wxART_DELETE));

		m_ContextMenu->Bind(wxEVT_MENU, [&](wxCommandEvent& e) { Copy(); }, Menu_Copy->GetId());
		m_ContextMenu->Bind(wxEVT_MENU, [&](wxCommandEvent& e) { Cut(); }, Menu_Cut->GetId());
		m_ContextMenu->Bind(wxEVT_MENU, [&](wxCommandEvent& e) { Delete(); }, Menu_Del->GetId());

		lua_pushliteral(glbLuaState, "WS_MENU");
		lua_setglobal(glbLuaState, "ACTIVEWIDGET");

		lua::RunExtensions(glbLuaState, "ws_menu.lua");

		lua_pushnil(glbLuaState);
		lua_setglobal(glbLuaState, "ACTIVEWIDGET");

		PopupMenu(m_ContextMenu);

		event.Skip();
	}


	void CWorksheet::CallRegisteredPyFuncs(const std::string& event)
	{
		if (m_EvtCallBack[event].size() > 0) 
		{
			const auto& List = m_EvtCallBack[event];

			for (const auto& CallBk : List)
				CallBk->call(CallBk->m_FuncObj, CallBk->m_FuncArgs, nullptr);
		}
	}


	void CWorksheet::OnRangeSelectionBegun(wxGridRangeSelectEvent& event)
	{
		wxFile file;
		if (!file.Open((glbExeDir / consts::EVENTS / "ws_selecting.py").wstring())) {
			event.Skip();
			return;
		}

		if (!file.ReadAll(&m_WS_Selecting_Py)) {
			event.Skip();
			return;
		}

		if (!m_WS_Selecting_Py.empty())
			PyRun_SimpleString(m_WS_Selecting_Py.mb_str(wxConvUTF8));

		event.Skip();
	}



	void CWorksheet::OnRangeSelecting(wxGridRangeSelectEvent& event)
	{
		if (IsSelection()) 
		{
			if (!m_SelectionBegun)
			{
				m_WS_Selecting_Py = wxEmptyString;

				m_SelectionBegun = true;
				wxGridRangeSelectEvent ssEvent = event;
				ssEvent.SetEventType(ssEVT_GRID_SELECTION_BEGUN);
				wxPostEvent(this, ssEvent);
			}

			if(!m_WS_Selecting_Py.empty())
				PyRun_SimpleString(m_WS_Selecting_Py.mb_str(wxConvUTF8));

			CallRegisteredPyFuncs("selecting");
		}

		event.Skip();
	}


	void CWorksheet::OnRangeSelected(wxGridRangeSelectEvent& event)
	{
		if (event.ShiftDown()) 
		{
			OnRangeSelecting(event);
			event.Skip();
			return;
		}

		if (event.Selecting())
		{
			CallRegisteredPyFuncs("selected");

			//reset the variables
			m_SelectionBegun = false;
			m_WS_Selecting_Py = wxEmptyString;
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



	std::unique_ptr<CRange> CWorksheet::GetSelection()
	{
		return std::make_unique<CRange>(
			this, 
			GetSelTopLeft(), 
			GetSelBtmRight());
	}

	

	/****************************** RANGE  ***************************************/


	CRange::CRange(
		grid::CWorksheetBase* ws, 
		const wxGridCellCoords& TL, 
		const wxGridCellCoords& BR): grid::CRangeBase(ws, TL, BR)
	{
		m_WBook = m_WSheet->GetWorkbook();
	}


	CRange::CRange(const wxString& str, grid::CWorkbookBase* wb):
		grid::CRangeBase(str, wb){}


	CRange::~CRange() = default;

}