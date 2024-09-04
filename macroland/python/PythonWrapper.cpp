#include "PythonWrapper.h"

#include <filesystem>
#include <locale>
#include <codecvt>

#include "../consts.h"

extern std::filesystem::path glbExeDir;

namespace Python
{
    bool RunExtensions(std::wstring_view fileName)
	{
		namespace fs = std::filesystem;
		
		for (const auto& DirEntry : fs::directory_iterator(glbExeDir / Info::EXTENSIONS))
		{
			if (!DirEntry.is_directory())
				continue;

			//Is Extension disabled
			auto Stem = DirEntry.path().stem();
			if (Stem.string()[0] == '_')
				continue;

			auto Path = DirEntry / fs::path(fileName);
			if (!fs::exists(Path))
				continue;

			auto gstate = PyGILState_Ensure();
			std::wstring_convert<std::codecvt_utf8<wchar_t>> cvt;
			if (auto cp = _Py_wfopen(Path.c_str(), L"rb"))
				PyRun_SimpleFileExFlags(cp, cvt.to_bytes(Path).c_str(), true, 0);
			
			PyGILState_Release(gstate);
		}

        return true;
	}


	bool RunDirectoryContents(const std::filesystem::path &DirPath)
	{
		namespace fs = std::filesystem;
		
		for (const auto& DirEntry : fs::directory_iterator(DirPath))
		{
			if (DirEntry.is_directory())
				continue;

			auto Path = DirEntry.path();

			auto gstate = PyGILState_Ensure();
			std::wstring_convert<std::codecvt_utf8<wchar_t>> cvt;
			if (auto cp = _Py_wfopen(Path.c_str(), L"rb"))
				PyRun_SimpleFileExFlags(cp, cvt.to_bytes(Path).c_str(), true, 0);
			
			PyGILState_Release(gstate);
		}

		return true;
	}
}