#include <string>
#include <wx/wx.h>

#include "luautil.h"
#include "toolbarntbk.h"
#include "buttons.h"

#include "../icell/workbook.h"


extern ICELL::CWorkbook* glbWorkbook;

using namespace lua;




static int Page_new(lua_State* L)
{
	int nargs = lua_gettop(L);
	auto page = (CToolBarPage**)lua_newuserdata(L, sizeof(CToolBarPage*)); //userdata

	if (nargs == 1 && lua_type(L, 1) == LUA_TSTRING)
	{
		lua_pushvalue(L, 1); //userdata table

		wxString title = wxString::FromUTF8(lua_tostring(L, 1));
		*page = new CToolBarPage(glbWorkbook->GetToolBarNtbk(), title.ToStdWstring());

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
		Elem = *(CToolBarHybridButton**)lua_touserdata(L, 2);

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

	int type = lua_getglobal(L, "ISEXTENSION");
	if (type == LUA_TBOOLEAN)
	{
		Page->SetPageType(CToolBarPage::PAGETYPE::EXTENSION);
		lua_pop(L, 1);
	}

	type = lua_getglobal(L, "ACTIVEDIR");
	if (type == LUA_TSTRING)
	{
		//TODO: Use codecvt
		wxString path = wxString::FromUTF8(lua_tostring(L, -1));
		if (path != "")
			Page->SetExtensionPath(path.ToStdWstring());
			
		lua_pop(L, 1);
	}

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
			std::string errMsg = "Page does not have a method named: " + std::string(lua_tostring(L, 2));
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