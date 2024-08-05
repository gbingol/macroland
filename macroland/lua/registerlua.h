#pragma once

#include <lua.hpp>

extern int luaopen_Button(lua_State* L);
extern int luaopen_DropButton(lua_State* L);
extern int luaopen_HybridButton(lua_State* L);
extern int luaopen_ToolBarPage(lua_State* L);
extern int luaopen_Menu(lua_State* L);
extern int l_append(lua_State* L);



static bool RegisterLuaFuncAndUserdata(lua_State* L) 
{
	typedef int(*luafunction)(lua_State*);

	
	auto regFunc = [&](luafunction func, const char* funcname, const char* libName="std") 
	{
		lua_getglobal(L, libName); //stk: std
		lua_pushstring(L, funcname);// stk: std string	
		lua_pushcfunction(L, func);  // stk: std string func
		lua_settable(L, -3); //std
		lua_pop(L, 1);
	};


	//Create std library
	lua_createtable(L, 0, 0);
	lua_setglobal(L, "std");

	luaopen_Button(L);
	luaopen_Menu(L);

	luaopen_DropButton(L);
	luaopen_HybridButton(L);
	luaopen_ToolBarPage(L);


	regFunc(l_append, "append");

	return true;
}