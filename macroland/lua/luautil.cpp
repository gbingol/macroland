#include "luautil.h"

#include <algorithm>
#include <codecvt>
#include <locale>

#include <wx/wx.h>
#include <wx/file.h>

#include "../util_funcs.h"

#include "../consts.h"



extern std::filesystem::path glbExeDir;



namespace lua
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



	void l_register(lua_State* L, const std::string regName, const luaL_Reg* metatable)
	{
		//The object must have a Metatable name appended to its registration name
		//i.e., if the registration name is Array, then metatable name must be ArrayMetatable

		std::string metatablename = regName;

		luaL_newmetatable(L, metatablename.c_str()); // metatable1
		luaL_setfuncs(L, metatable, 0);

		lua_pushstring(L, "__index");
		lua_pushvalue(L, -2);  //  metatable1 __index metatable1
		lua_settable(L, -3); // metatable1[__index]=metatable1

		lua_setglobal(L, regName.c_str());
	}


	bool RunLuaFile(lua_State* L, const std::filesystem::path& path, const std::string& ErrFileName)
	{
		wxString contents;
		wxFile file;

		if (!std::filesystem::exists(path))
			return false;
		
		if (!file.Open(path.wstring(), wxFile::read))
			return false;
		
		if (!file.ReadAll(&contents))
			return false;

		auto ReportErr = [&](const wxString& Err)
		{
			util::CDate date;
			std::string FileName = ErrFileName + date.GetDate("", true) + " " + date.GetTime("") + ".txt";
			auto ErrPath = path.parent_path() / FileName;

			wxFile errFile;
			errFile.Create(ErrPath.wstring());
			errFile.Write(Err);
			errFile.Close();
		};

		try
		{
			int Err = luaL_dostring(L, contents.mb_str(wxConvUTF8));

			if (Err != LUA_OK)
			{
				wxString ErrMsg = wxString::FromUTF8(lua_tostring(L, -1));
				lua_pop(L, 1);

				ReportErr(ErrMsg);

				return false;
			}
		}
		catch (std::exception& e)
		{
			ReportErr(e.what());
			return false;
		}

		return true;
	}


	void RunExtensions(lua_State* L, const std::string& FileName)
	{
		namespace fs = std::filesystem;
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;

		//We are telling lua that we are now loading extensions
		lua_pushboolean(L, true);
		lua_setglobal(L, "ISEXTENSION");

		//extensions
		for (const auto& DirEntry : fs::directory_iterator(glbExeDir / consts::EXTENSIONS))
		{
			if (!DirEntry.is_directory())
				continue;

			//Is Extension disabled
			auto Stem = DirEntry.path().stem();
			if (Stem.string()[0] == '_')
				continue;

			
			auto StemDir = DirEntry.path().stem().wstring();
			lua_pushstring(L, converter.to_bytes(StemDir).c_str());
			lua_setglobal(L, "ACTIVEDIR_STEM");

			auto ActiveDir = DirEntry.path().wstring();
			lua_pushstring(L, converter.to_bytes(ActiveDir).c_str());
			lua_setglobal(L, "ACTIVEDIR");

			auto Path = DirEntry / fs::path(FileName.c_str());
			if (!fs::exists(Path))
				continue;

			std::string ErrFile = fs::path(FileName).stem().string() + "_err";
			lua::RunLuaFile(L, Path, ErrFile);
		}

		//reset global variables
		lua_pushnil(L);
		lua_setglobal(L, "ISEXTENSION");

		lua_pushnil(L);
		lua_setglobal(L, "ACTIVEDIR");

		lua_pushnil(L);
		lua_setglobal(L, "ACTIVEDIR_STEM");
	}
}
