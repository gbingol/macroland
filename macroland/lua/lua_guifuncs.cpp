#include <lua.hpp>

#include <wx/wx.h>
#include "buttons.h"
#include "menuelem.h"
#include "panel.h"

#include "../scripting/pnlcmdwnd.h"
#include "../mainfrm/frmmacroland.h"
#include"../macrolandapp.h"


#include "../icell/workbook.h"
#include "../icell/worksheet.h"


extern ICELL::CWorkbook* glbWorkbook;
using namespace lua;


namespace
{

	void wxMenu_AddButton(wxMenu* Menu, CButtonBase* btn)
	{
		int btnID = btn->GetId();
		wxString Title = btn->GetTitle();
		wxBitmap bmp = btn->GetBitmap(btn->GetImagePath());

		auto Item = Menu->Append(btnID, Title);
		Item->SetBitmap(bmp);

		Menu->Bind(wxEVT_MENU, &CButtonBase::OnClick, (CButtonBase*)btn, btnID);
	}


	int contextmenu(lua_State* L, wxMenu* ContextMenu)
	{
		if (!ContextMenu)
			luaL_error(L, "It is highly likely the extension file and function do not match.");

		if (lua_type(L, 1) == LUA_TNONE)
		{
			ContextMenu->AppendSeparator();
			return 0;
		}

		try
		{
			if (luaL_testudata(L, 1, "Button"))
			{
				CButton* btn = *(CButton**)lua_touserdata(L, 1);
				if (btn->IsOK())
					wxMenu_AddButton(ContextMenu, btn);
			}
			else if (luaL_testudata(L, 1, "Menu"))
			{
				auto menu = *(CMenu**)lua_touserdata(L, 1);
				wxMenu* SubMenu = new wxMenu();

				for (auto btn : menu->GetList())
				{
					if (btn->IsOK())
						wxMenu_AddButton(SubMenu, btn);
				}

				auto MenuItem = ContextMenu->AppendSubMenu(SubMenu, menu->GetTitle());
				MenuItem->SetBitmap(menu->GetBitmap(menu->GetImagePath()));
			}
			else
				luaL_error(L, "Button/Menu expected.");
		}
		catch (const std::exception& e)
		{
			luaL_error(L, e.what());
		}

		return 0;
	}


	int ws_stbar_menu(lua_State* L)
	{
		auto frmSciSuit = (frmMacroLand*)wxTheApp->GetTopWindow();
		auto ContextMenu = frmSciSuit->getStatBarMenu();

		int nargs = lua_gettop(L);
		if (nargs == 1)
		{
			int Field = lua_tointeger(L, 1) - 1;
			if (Field == frmSciSuit->getStBarRectField())
				ContextMenu->AppendSeparator();

			return 0;
		}

		if (nargs != 2)
			luaL_error(L, "statbar_contextmenu expect two parameters: button/menu and integer");

		int type = lua_type(L, 2);
		if (type == LUA_TNUMBER)
		{
			int Field = lua_tointeger(L, 2) - 1;
			if (Field == frmSciSuit->getStBarRectField())
				return contextmenu(L, ContextMenu);
		}

		return 0;
	}


	int sci_apps(lua_State* L)
	{
		if (luaL_testudata(L, 1, "Panel"))
		{
			CPanel* pnl = *(CPanel**)lua_touserdata(L, 1);
			auto frmSciSuit = (frmMacroLand*)wxTheApp->GetTopWindow();
			frmSciSuit->getTopBar()->GetAppsWnd()->Append(*pnl);
		}
		else
			luaL_error(L, "Internal error: Panel expected");

		return 0;
	}


	int ws_menu(lua_State* L)
	{
		auto ActiveWS = (ICELL::CWorksheet*)glbWorkbook->GetActiveWS();
		auto ContextMenu = ActiveWS->GetContextMenu();
		return contextmenu(L, ContextMenu);
	}



	int ws_tab_menu(lua_State* L)
	{
		auto ContextMenu = glbWorkbook->GetWorksheetNotebook()->GetContextMenu();
		return contextmenu(L, ContextMenu);
	}


	int cmdwnd_contextmenu(lua_State* L)
	{
		auto ContextMenu = scripting::cmdedit::pnlCommandWindow::GetInputWndContextMenu();
		return contextmenu(L, ContextMenu);
	}

}


int l_append(lua_State* L)
{
	lua_getglobal(L, "ACTIVEWIDGET"); 
	std::string widget = lua_tostring(L, -1);

	//pop ACTIVEWIDGET from stack as it misleads int nargs = lua_gettop(L);
	lua_pop(L, 1);

	if (widget == "WS_STBAR_MENU")
		return ws_stbar_menu(L);

	else if (widget == "WS_MENU")
		return ws_menu(L);

	else if (widget == "WS_TAB_MENU")
		return ws_tab_menu(L);

	else if (widget == "SCI_APPS")
		return sci_apps(L);

	return 0;
}
