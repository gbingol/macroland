#pragma once

#include <string>

#include <lua.hpp> 
#include <map>
#include <any>
#include <filesystem>

namespace lua
{

	//Table is on top of the stack
	std::map<std::string, std::any> ParseLuaTable(lua_State* L);

	bool RunLuaFile(
		lua_State* L, 
		const std::filesystem::path& path, 
		const std::string& ErrFileName);

	//run FileName (a lua file) in all sub-folders in the extensions folder
	void RunExtensions(lua_State* L,
		const std::string& FileName //File name and extension, ribbonbar.lua
	);
}