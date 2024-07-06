#include <string>


#include <wx/wx.h>

#include <lua.hpp>

#include "luautil.h"
#include "menuelem.h"
#include "buttons.h"


using namespace lua;

/*******************  MENU ************************/

static int Menu_new(lua_State* L)
{
	int nargs = lua_gettop(L);
	auto btn = (CMenu**)lua_newuserdata(L, sizeof(CMenu*)); //userdata

	if (nargs == 1 && lua_type(L, 1) == LUA_TTABLE)
	{
		lua_pushvalue(L, 1); //userdata table

		auto Tbl = lua::ParseLuaTable(L);
		std::wstring title;
		std::filesystem::path ImagePath;
		try
		{
			if(Tbl.find("title") !=Tbl.end())
				title = std::any_cast<std::wstring>(Tbl.at("title"));

			if (Tbl.find("img") != Tbl.end())
				ImagePath = std::any_cast<std::wstring>(Tbl.at("img"));
		}
		catch (const std::exception& e) 
		{
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
	std::string regName = "Menu";

	luaL_newmetatable(L, regName.c_str()); // metatable1
	luaL_setfuncs(L, Menu_metatable, 0);

	lua_getglobal(L, "std"); //std
	lua_pushstring(L, regName.c_str());// std string	
	luaL_newlib(L, Menu_globalstable); //std string library
	lua_settable(L, -3); //std
	lua_pop(L, 1);

	return 0;
}