#include "luautil.h"

#include <codecvt>
#include <locale>
#include <chrono>

#include <wx/wx.h>
#include <wx/file.h>

#include "../consts.h"



extern std::filesystem::path glbExeDir;



namespace lua
{
	bool RunLuaFile(
		lua_State* L, 
		const std::filesystem::path& path, 
		const std::string& ErrFileName)
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
			using namespace std::chrono;
			const auto now = time_point_cast<seconds>(zoned_time(current_zone(), system_clock::now()).get_local_time());
			auto FileName = std::format("{:%Y%m%d %H%M%S}", now)+ ".txt";
			auto ErrPath = path.parent_path() / FileName;

			wxFile errFile;
			errFile.Create(ErrPath.wstring());
			errFile.Write(Err);
			errFile.Close();
		};

		try {
			int Err = luaL_dostring(L, contents.mb_str(wxConvUTF8));

			if (Err != LUA_OK) {
				auto ErrMsg = wxString::FromUTF8(lua_tostring(L, -1));
				lua_pop(L, 1);
				ReportErr(ErrMsg);

				return false;
			}
		}
		catch (std::exception& e) {
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
		for (const auto& DirEntry : fs::directory_iterator(glbExeDir / Info::EXTENSIONS))
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
