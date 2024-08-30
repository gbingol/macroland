#include <map>
#include <string>
#include <any>
#include <algorithm>
#include <codecvt>
#include <locale>

#include <lua.hpp>

#include <wx/wx.h>

#include "../mainfrm/frmmacroland.h"
#include "../mainfrm/icell.h"
#include"../macrolandapp.h"


#include "lua_guielements.h"
#include "luautil.h"


extern ICELL::CWorkbook* glbWorkbook;
using namespace lua;


namespace
{
	std::map<std::string, std::any> ParseLuaTable(lua_State* L)
	{
		std::map<std::string, std::any> Tbl;

		lua_pushnil(L);
		while (lua_next(L, -2))
		{
			std::string key = lua_tostring(L, -2);
			std::transform(key.begin(), key.end(), key.begin(), ::tolower);

			int Type = lua_type(L, -1);
			if (Type == LUA_TNUMBER)
			{
				double num = lua_tonumber(L, -1);
				Tbl.insert({ key, int(num) == num ? int(num) : num });
			}

			else if (Type == LUA_TSTRING)
			{
				std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
				std::wstring str = converter.from_bytes(lua_tostring(L, -1));
				Tbl.insert({ key, str });
			}

			else if (Type == LUA_TTABLE)
			{
				auto SubTable = ParseLuaTable(L);
				Tbl.insert({ key, SubTable });
			}

			lua_pop(L, 1);
		}

		return Tbl;
	}


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

		if (lua_type(L, 1) == LUA_TNONE) {
			ContextMenu->AppendSeparator();
			return 0;
		}

		try {
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
		catch (const std::exception& e) {
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


	return 0;
}



/********************************************* */

static int Button_new(lua_State* L)
{
	int nargs = lua_gettop(L);
	CButton** btn = (CButton**)lua_newuserdata(L, sizeof(CButton*)); //userdata
	
	if (nargs == 1 && lua_type(L, 1) == LUA_TTABLE)
	{
		lua_pushvalue(L, 1); //userdata table
		
		auto Tbl = ParseLuaTable(L);
		std::wstring title;
		std::filesystem::path PyPath, ImagePath;
		std::wstring ModulePath, funcName;
		std::any Param;
		try
		{
			if(Tbl.find("title") != Tbl.end())
				title = std::any_cast<std::wstring>(Tbl.at("title"));

			if (Tbl.find("py") != Tbl.end())
				PyPath = std::any_cast<std::wstring>(Tbl.at("py"));

			if (Tbl.find("img") != Tbl.end())
				ImagePath = std::any_cast<std::wstring>(Tbl.at("img"));

			if (Tbl.find("module") != Tbl.end())
				ModulePath = std::any_cast<std::wstring>(Tbl.at("module"));

			if (Tbl.find("call") != Tbl.end())
				funcName = std::any_cast<std::wstring>(Tbl.at("call"));

			if (Tbl.find("param") != Tbl.end())
				Param = Tbl.at("param");

			if (title.empty())
				luaL_error(L, "title must be defined for Button");

			if (PyPath.empty() && ModulePath.empty())
				luaL_error(L, "py or module must be defined for Button");

			if(!PyPath.empty() && !ModulePath.empty())
				luaL_error(L, "Either py or module must be defined (not both) for a Button");

			if (PyPath.empty())
			{
				if(!ModulePath.empty() && funcName.empty())
					luaL_error(L, "if module defined, call must be defined for Button");
			}

			*btn = new CButton(title);

			if(!PyPath.empty())
				(*btn)->SetScriptPath(PyPath);

			if (!ModulePath.empty())
			{
				(*btn)->SetModulePath(ModulePath);
				(*btn)->SetFuncName(funcName);
				//(*btn)->SetParam(Param);
			}

			if (!ImagePath.empty())
				(*btn)->SetImgPath(ImagePath);
		}
		catch (const std::exception& e)
		{
			luaL_error(L, e.what());
		}

		lua_pop(L, 1); //userdata
	}
	else
		luaL_error(L, "Argument must be of type Lua table");


	luaL_getmetatable(L, "Button"); //userdata metatable
	lua_setmetatable(L, -2); //userdata metatable1

	return 1;
}



static int Button_type(lua_State* L)
{
	lua_pushstring(L, "Button");
	return 1;
}


static int Button_index(lua_State* L)
{
	if (lua_type(L, 2) == LUA_TSTRING)
	{
		lua_getmetatable(L, 1);

		int FieldType = lua_getfield(L, -1, lua_tostring(L, 2));

		if (FieldType == LUA_TNIL)
		{
			auto errMsg = "Button does not have a method named: " + std::string(lua_tostring(L, 2));
			luaL_error(L, errMsg.c_str());
		}

		else if (FieldType == LUA_TFUNCTION)
			lua_pushcfunction(L, lua_tocfunction(L, -1));
		else
			luaL_error(L, "Unknown type");

		return 1;
	}

	return 0;
}



static const luaL_Reg Button_globalstable[] = 
{
	{ "new", Button_new },
	{ NULL, NULL },
};


static const luaL_Reg Button_metatable[] = 
{
	{"__index", Button_index},
	{"type", Button_type},

	{ NULL, NULL }
};




int luaopen_Button(lua_State* L)
{
	auto regName = "Button";

	luaL_newmetatable(L, regName); // metatable1
	luaL_setfuncs(L, Button_metatable, 0);
	
	lua_getglobal(L, "std"); //std
	lua_pushstring(L, regName);// std string	
	luaL_newlib(L, Button_globalstable); //std string library
	lua_settable(L, -3); //std
	lua_pop(L, 1);

	return 0;
}



/*******************  DROP BUTTON ************************/

static int DropButton_new(lua_State* L)
{
	int nargs = lua_gettop(L);
	auto btn = (CToolBarDropButton**)lua_newuserdata(L, sizeof(CToolBarDropButton*)); //userdata

	if (nargs == 1 && lua_type(L, 1) == LUA_TTABLE)
	{
		lua_pushvalue(L, 1); //userdata table

		auto Tbl = ParseLuaTable(L);
		std::wstring title;
		std::filesystem::path ImagePath;
		try
		{
			if (Tbl.find("title") != Tbl.end())
				title = std::any_cast<std::wstring>(Tbl.at("title"));

			if (Tbl.find("img") != Tbl.end())
				ImagePath = std::any_cast<std::wstring>(Tbl.at("img"));
		}
		catch (const std::exception& e)
		{
			luaL_error(L, e.what());
		}

		if (title.empty())
			luaL_error(L, "title must be defined for DropButton");

		*btn = new CToolBarDropButton(title);

		if (!ImagePath.empty())
			(*btn)->SetImgPath(ImagePath);

		lua_pop(L, 1); //userdata
	}
	else
		luaL_error(L, "Argument must be of type Lua table");


	luaL_getmetatable(L, "DropButton"); //userdata metatable
	lua_setmetatable(L, -2); //userdata metatable1

	return 1;
}



static int DropButton_type(lua_State* L)
{
	lua_pushstring(L, "DropButton");
	return 1;
}


static int DropButton_Add(lua_State* L)
{
	auto DropBtn = *(CToolBarDropButton**)lua_touserdata(L, 1);
	
	if (!luaL_testudata(L, 2, "Button"))
		luaL_error(L, "Button expected.");

	CButton* btn = *(CButton**)lua_touserdata(L, 2);
	DropBtn->AddButton(btn);

	return 0;
}


static int DropButton_index(lua_State* L)
{
	if (lua_type(L, 2) == LUA_TSTRING)
	{
		lua_getmetatable(L, 1);

		int FieldType = lua_getfield(L, -1, lua_tostring(L, 2));


		if (FieldType == LUA_TNIL)
		{
			std::string errMsg = "DropButton does not have a method named: " + std::string(lua_tostring(L, 2));
			luaL_error(L, errMsg.c_str());
		}

		else if (FieldType == LUA_TFUNCTION)
			lua_pushcfunction(L, lua_tocfunction(L, -1));
		else
			luaL_error(L, "Unknown type");

		return 1;
	}

	return 0;
}


static const luaL_Reg DropButton_globalstable[] =
{
	{ "new", DropButton_new },
	{ NULL, NULL },
};


static const luaL_Reg DropButton_metatable[] =
{
	{"__index", DropButton_index},

	{"add", DropButton_Add},
	{"type", DropButton_type},

	{ NULL, NULL }
};




int luaopen_DropButton(lua_State* L)
{
	auto NAME = "DropButton";

	luaL_newmetatable(L, NAME);
	luaL_setfuncs(L, DropButton_metatable, 0);

	lua_getglobal(L, "std"); //std
	lua_pushstring(L, NAME);// std string	
	luaL_newlib(L, DropButton_globalstable); //std string library
	lua_settable(L, -3); //std
	lua_pop(L, 1);

	return 0;
}




/**********************   HYBRID BUTTON ****************************/

static int HybridButton_new(lua_State* L)
{
	int nargs = lua_gettop(L);
	auto btn = (CHybridButton**)lua_newuserdata(L, sizeof(CHybridButton*)); //userdata

	CButton* MainBtn = nullptr;

	if (nargs != 1)
		luaL_error(L, "Hybrid button requires single param: Button");

	if (luaL_testudata(L, 1, "Button"))
		MainBtn = *(CButton**)lua_touserdata(L, 1);
	else
		luaL_error(L, "Hybrid button's param must be of type Button");


	*btn = new CHybridButton(MainBtn);

	luaL_getmetatable(L, "HybridButton"); //userdata metatable
	lua_setmetatable(L, -2); //userdata metatable1

	return 1;
}



static int HybridButton_Add(lua_State* L)
{
	auto HybridBtn = *(CHybridButton**)lua_touserdata(L, 1);
	int type = lua_type(L, 2);

	if (!luaL_testudata(L, 2, "Button"))
		luaL_error(L, "Button expected.");

	CButton* btn = *(CButton**)lua_touserdata(L, 2);
	HybridBtn->AddButton(btn);

	return 0;
}


static int HybridButton_index(lua_State* L)
{
	if (lua_type(L, 2) == LUA_TSTRING)
	{
		lua_getmetatable(L, 1);

		int FieldType = lua_getfield(L, -1, lua_tostring(L, 2));

		if (FieldType == LUA_TNIL)
		{
			auto errMsg = "HybridButton does not have a method named: " + std::string(lua_tostring(L, 2));
			luaL_error(L, errMsg.c_str());
		}

		else if (FieldType == LUA_TFUNCTION)
			lua_pushcfunction(L, lua_tocfunction(L, -1));
		else
			luaL_error(L, "Unknown type");

		return 1;
	}

	return 0;
}



static const luaL_Reg HybridButton_globalstable[] =
{
	{ "new", HybridButton_new },
	{ NULL, NULL },
};



static const luaL_Reg HybridButton_metatable[] =
{
	{"__index", HybridButton_index},
	{"add", HybridButton_Add},

	{ NULL, NULL }
};




int luaopen_HybridButton(lua_State* L)
{
	auto NAME = "HybridButton";

	luaL_newmetatable(L, NAME);
	luaL_setfuncs(L, HybridButton_metatable, 0);

	lua_getglobal(L, "std"); //std
	lua_pushstring(L, NAME);// std string	
	luaL_newlib(L, HybridButton_globalstable); //std string library
	lua_settable(L, -3); //std
	lua_pop(L, 1);

	return 0;
}



/***********************************************************************/

static int Menu_new(lua_State* L)
{
	int nargs = lua_gettop(L);
	auto btn = (CMenu**)lua_newuserdata(L, sizeof(CMenu*)); //userdata

	if (nargs == 1 && lua_type(L, 1) == LUA_TTABLE)
	{
		lua_pushvalue(L, 1); //userdata table

		auto Tbl = ParseLuaTable(L);
		std::wstring title;
		std::filesystem::path ImagePath;
		try
		{
			if(Tbl.find("title") !=Tbl.end())
				title = std::any_cast<std::wstring>(Tbl.at("title"));

			if (Tbl.find("img") != Tbl.end())
				ImagePath = std::any_cast<std::wstring>(Tbl.at("img"));
		}
		catch (const std::exception& e) {
			luaL_error(L, e.what());
		}

		*btn = new CMenu(title);

		if(!ImagePath.empty())
			(*btn)->SetImgPath(ImagePath);

		lua_pop(L, 1); //userdata
	}
	else
		luaL_error(L, "Argument must be of type Lua table");


	luaL_getmetatable(L, "Menu"); //userdata metatable
	lua_setmetatable(L, -2); //userdata metatable1

	return 1;
}



static int Menu_type(lua_State* L)
{
	lua_pushstring(L, "Menu");
	return 1;
}


static int Menu_Add(lua_State* L)
{
	CMenu* Menu = *(CMenu**)lua_touserdata(L, 1);
	int type = lua_type(L, 2);
	
	if (!luaL_testudata(L, 2, "Button"))
		luaL_error(L, "Button expected.");

	CButton* btn = *(CButton**)lua_touserdata(L, 2);
	Menu->AddButton(btn);

	return 0;
}


static int Menu_index(lua_State* L)
{
	if (lua_type(L, 2) == LUA_TSTRING)
	{
		lua_getmetatable(L, 1);

		int FieldType = lua_getfield(L, -1, lua_tostring(L, 2));


		if (FieldType == LUA_TNIL)
		{
			std::string errMsg = "Menu does not have a method named: " + std::string(lua_tostring(L, 2));
			luaL_error(L, errMsg.c_str());
		}

		else if (FieldType == LUA_TFUNCTION)
			lua_pushcfunction(L, lua_tocfunction(L, -1));
		else
			luaL_error(L, "Unknown type");

		return 1;
	}

	return 0;
}



static const luaL_Reg Menu_globalstable[] =
{
	{ "new", Menu_new },
	{ NULL, NULL },
};


static const luaL_Reg Menu_metatable[] =
{
	{"__index", Menu_index},

	{"add", Menu_Add},
	{"type", Menu_type},

	{ NULL, NULL }
};




int luaopen_Menu(lua_State* L)
{
	auto regName = "Menu";

	luaL_newmetatable(L, regName); // metatable1
	luaL_setfuncs(L, Menu_metatable, 0);

	lua_getglobal(L, "std"); //std
	lua_pushstring(L, regName);// std string	
	luaL_newlib(L, Menu_globalstable); //std string library
	lua_settable(L, -3); //std
	lua_pop(L, 1);

	return 0;
}






/*********************************************************************** */


static int Page_new(lua_State* L)
{
	int nargs = lua_gettop(L);
	auto page = (CToolBarPage**)lua_newuserdata(L, sizeof(CToolBarPage*)); //userdata

	if (nargs == 1 && lua_type(L, 1) == LUA_TSTRING)
	{
		lua_pushvalue(L, 1); //userdata table

		wxString title = wxString::FromUTF8(lua_tostring(L, 1));
		auto Ntbk = glbWorkbook->GetToolBarNtbk();
		auto Page = Ntbk->FindPage(title.ToStdWstring());

		if(!Page)
			*page = new CToolBarPage(Ntbk, title.ToStdWstring());
		else
			*page = (CToolBarPage *)Page;

		lua_pop(L, 1); //userdata
	}
	else
		luaL_error(L, "Argument must be of type Lua table");


	luaL_getmetatable(L, "ToolBarPage"); //userdata metatable
	lua_setmetatable(L, -2); //userdata metatable1

	return 1;
}



static int Page_type(lua_State* L)
{
	lua_pushstring(L, "ToolBarPage");
	return 1;
}


static int Page_Add(lua_State* L)
{
	auto Page = *(CToolBarPage**)lua_touserdata(L, 1);
	int type = lua_type(L, 2);

	CElement* Elem = nullptr;
	if (luaL_testudata(L, 2, "Button"))
		Elem = *(CButton**)lua_touserdata(L, 2);
	else if (luaL_testudata(L, 2, "DropButton"))
		Elem = *(CToolBarDropButton**)lua_touserdata(L, 2);
	else if (luaL_testudata(L, 2, "HybridButton"))
		Elem = *(CHybridButton**)lua_touserdata(L, 2);

	else
		luaL_error(L, "ToolBarPage requires Button or DropButton.");

	Page->AddElement(Elem);

	return 0;
}


static int Page_Start(lua_State* L)
{
	auto Page = *(CToolBarPage**)lua_touserdata(L, 1);

	auto Ntbk = glbWorkbook->GetToolBarNtbk();
	Ntbk->AddPage(Page);

	return 0;
}





static int Page_index(lua_State* L)
{
	if (lua_type(L, 2) == LUA_TSTRING)
	{
		lua_getmetatable(L, 1);
		int FieldType = lua_getfield(L, -1, lua_tostring(L, 2));

		if (FieldType == LUA_TNIL)
		{
			auto errMsg = "Page does not have a method named: " + std::string(lua_tostring(L, 2));
			luaL_error(L, errMsg.c_str());
		}

		else if (FieldType == LUA_TFUNCTION)
			lua_pushcfunction(L, lua_tocfunction(L, -1));
		else
			luaL_error(L, "Unknown type");

		return 1;
	}

	return 0;
}



static const luaL_Reg Page_globalstable[] =
{
	{ "new", Page_new },
	{ NULL, NULL },
};


static const luaL_Reg Page_metatable[] =
{
	{"__index", Page_index},

	{"add", Page_Add},
	{"start", Page_Start},
	{"type", Page_type},

	{ NULL, NULL }
};




int luaopen_ToolBarPage(lua_State* L)
{
	auto NAME = "ToolBarPage";

	luaL_newmetatable(L, NAME);
	luaL_setfuncs(L, Page_metatable, 0);

	lua_getglobal(L, "std"); //std
	lua_pushstring(L, NAME);// std string	
	luaL_newlib(L, Page_globalstable); //std string library
	lua_settable(L, -3); //std
	lua_pop(L, 1);

	return 0;
}