#pragma once

#include <string>

#include <lua.hpp> 
#include <map>
#include <any>
#include <filesystem>

namespace lua
{
	template <typename T>
	void pushObject(lua_State* L, T* obj, std::string name)
	{
		T** Arr = (T**)lua_newuserdata(L, sizeof(T*));
		*Arr = obj;

		luaL_getmetatable(L, name.c_str());
		lua_setmetatable(L, -2);
	}


	//Table is on top of the stack
	std::map<std::string, std::any> ParseLuaTable(lua_State* L);

	void l_register(
		lua_State* L, 
		const std::string regName, 
		const luaL_Reg* metatable);

	bool RunLuaFile(
		lua_State* L, 
		const std::filesystem::path& path, 
		const std::string& ErrFileName);

	//run FileName (a lua file) in all sub-folders in the extensions folder
	void RunExtensions(lua_State* L,
		const std::string& FileName //File name and extension, ribbonbar.lua
	);
}