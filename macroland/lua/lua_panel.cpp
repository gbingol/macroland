#include <string>
#include <wx/wx.h>

#include "luautil.h"
#include "buttons.h"
#include "panel.h"



using namespace lua;


namespace
{

	int Construct(lua_State* L)
	{
		int nargs = lua_gettop(L);
		auto panel = (CPanel**)lua_newuserdata(L, sizeof(CPanel*)); //userdata

		if (nargs == 1 && lua_type(L, 1) == LUA_TSTRING)
		{
			lua_pushvalue(L, 1); //userdata table

			wxString title = wxString::FromUTF8(lua_tostring(L, 1));
			*panel = new CPanel(title.ToStdWstring());

			lua_pop(L, 1); //userdata
		}
		else
			luaL_error(L, "Argument must be of type Lua table");


		luaL_getmetatable(L, "Panel"); //userdata metatable
		lua_setmetatable(L, -2); //userdata metatable1

		return 1;
	}



	int Type(lua_State* L)
	{
		lua_pushstring(L, "Panel");
		return 1;
	}


	int Add(lua_State* L)
	{
		auto Section = *(CPanel**)lua_touserdata(L, 1);

		int type = lua_type(L, 2);

		CElement* Elem = nullptr;
		if (luaL_testudata(L, 2, "Button"))
			Elem = *(CButton**)lua_touserdata(L, 2);

		else
			luaL_error(L, "Panel requires Button.");

		Section->AddElement(Elem);

		return 0;
	}





	int Index(lua_State* L)
	{
		if (lua_type(L, 2) == LUA_TSTRING)
		{
			lua_getmetatable(L, 1);

			int FieldType = lua_getfield(L, -1, lua_tostring(L, 2));
			if (FieldType == LUA_TNIL)
			{
				std::string errMsg = "Panel does not have a method named: " + std::string(lua_tostring(L, 2));
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

}



static const luaL_Reg globalstable[] =
{
	{ "new", Construct },
	{ NULL, NULL },
};


static const luaL_Reg metatable[] =
{
	{"__index", Index},

	{"add", Add},
	{"type", Type},

	{ NULL, NULL }
};




int luaopen_Panel(lua_State* L)
{
	auto NAME = "Panel";

	luaL_newmetatable(L, NAME);
	luaL_setfuncs(L, metatable, 0);

	lua_getglobal(L, "std"); //std
	lua_pushstring(L, NAME);// std string	
	luaL_newlib(L, globalstable); //std string library
	lua_settable(L, -3); //std
	lua_pop(L, 1);

	return 0;
}