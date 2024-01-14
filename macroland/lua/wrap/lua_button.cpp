#include <string>
#include <any>

#include "../luautil.h"
#include "../widgets/buttons.h"


using namespace lua;


static int Button_new(lua_State* L)
{
	int nargs = lua_gettop(L);
	CButton** btn = (CButton**)lua_newuserdata(L, sizeof(CButton*)); //userdata
	
	if (nargs == 1 && lua_type(L, 1) == LUA_TTABLE)
	{
		lua_pushvalue(L, 1); //userdata table
		
		auto Tbl = lua::ParseLuaTable(L);
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
				(*btn)->SetParam(Param);
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
			std::string errMsg = "Button does not have a method named: " + std::string(lua_tostring(L, 2));
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
	std::string regName = "Button";

	luaL_newmetatable(L, regName.c_str()); // metatable1
	luaL_setfuncs(L, Button_metatable, 0);
	
	lua_getglobal(L, "std"); //std
	lua_pushstring(L, regName.c_str());// std string	
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

		auto Tbl = lua::ParseLuaTable(L);
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

	int type = lua_type(L, 2);

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
	auto btn = (CToolBarHybridButton**)lua_newuserdata(L, sizeof(CToolBarHybridButton*)); //userdata

	CButton* MainBtn = nullptr;

	if (nargs != 1)
		luaL_error(L, "Hybrid button requires single param: Button");

	if (luaL_testudata(L, 1, "Button"))
		MainBtn = *(CButton**)lua_touserdata(L, 1);
	else
		luaL_error(L, "Hybrid button's param must be of type Button");


	*btn = new CToolBarHybridButton(MainBtn);

	luaL_getmetatable(L, "HybridButton"); //userdata metatable
	lua_setmetatable(L, -2); //userdata metatable1

	return 1;
}



static int HybridButton_Add(lua_State* L)
{
	auto HybridButton = *(CToolBarHybridButton**)lua_touserdata(L, 1);

	int type = lua_type(L, 2);

	if (!luaL_testudata(L, 2, "Button"))
		luaL_error(L, "Button expected.");

	CButton* btn = *(CButton**)lua_touserdata(L, 2);
	HybridButton->AddButton(btn);

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
			std::string errMsg = "HybridButton does not have a method named: " + std::string(lua_tostring(L, 2));
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
