#include "wsntbk.h"

#include <lua.hpp>
#include "../lua/luautil.h"

#include <script/scripting_funcs.h>

#include "../consts.h"

#include "workbook.h"
#include "worksheet.h"

#include <grid/ntbkbase.h>


extern std::filesystem::path glbExeDir;
extern lua_State* glbLuaState;



 
namespace ICELL
{

	wxDEFINE_EVENT(ssEVT_WB_PAGECHANGED, wxAuiNotebookEvent);

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
			script::RunPyFile(glbExeDir / consts::CONFIG_DIR / "ws_pagechanged.py");

		wxAuiNotebookEvent PageEvt(evt);
		PageEvt.SetEventType(ssEVT_WB_PAGECHANGED);
		ProcessWindowEvent(PageEvt);
	}


	void CWorksheetNtbk::OnTabRightDown(wxAuiNotebookEvent& evt)
	{
		lua_pushliteral(glbLuaState, "WS_TAB_MENU");
		lua_setglobal(glbLuaState, "ACTIVEWIDGET");

		lua::RunExtensions(glbLuaState, "ws_tab_menu.lua");

		lua_pushnil(glbLuaState);
		lua_setglobal(glbLuaState, "ACTIVEWIDGET");


		PopupMenu(m_ContextMenu);
	}

	grid::CWorksheetBase* CWorksheetNtbk::CreateWorksheet(wxWindow* wnd, const std::wstring& Label, int nrows, int ncols) const
	{
		return  new CWorksheet(wnd, m_Workbook, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, Label, nrows, ncols);;
	}


}
