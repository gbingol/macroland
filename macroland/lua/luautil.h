#pragma once

#include <string>
#include <filesystem>

#include <lua.hpp> 

namespace lua
{

	bool RunLuaFile(
		lua_State* L, 
		const std::filesystem::path& path, 
		const std::string& ErrFileName);

	//run FileName (a lua file) in all sub-folders in the extensions folder
	void RunExtensions(lua_State* L,
		const std::string& FileName //File name and extension, ribbonbar.lua
	);
}