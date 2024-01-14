#include "extension.h"

#include <codecvt>
#include <locale>



extern std::filesystem::path glbExeDir;



namespace extension
{

	CExtension::CExtension(
		lua_State* L, 
		const std::filesystem::path& DirEntry)
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;

		m_Enabled = IsEnabled(DirEntry);

		m_DirPath = DirEntry;

		int type = lua_getglobal(L, "guid");

		if (type == LUA_TSTRING)
		{
			m_GUID = lua_tostring(L, -1);
			lua_pop(L, 1);
		}

		type = lua_getglobal(L, "extname");
		if (type == LUA_TSTRING)
		{
			m_Name = converter.from_bytes(lua_tostring(L, -1));
			lua_pop(L, 1);
		}

		type = lua_getglobal(L, "developer");
		if (type == LUA_TSTRING)
		{
			m_Developer = converter.from_bytes(lua_tostring(L, -1));
			lua_pop(L, 1);
		}

		type = lua_getglobal(L, "readme");
		if (type == LUA_TSTRING)
		{
			m_ReadMe = std::filesystem::path(converter.from_bytes(lua_tostring(L, -1)));
			lua_pop(L, 1);
		}

		type = lua_getglobal(L, "short_desc");
		if (type == LUA_TSTRING)
		{
			m_Short_Desc = converter.from_bytes(lua_tostring(L, -1));
			lua_pop(L, 1);
		}

		type = lua_getglobal(L, "version");
		if (type == LUA_TSTRING)
		{
			m_Version = lua_tostring(L, -1);
			lua_pop(L, 1);
		}

		if (m_Version == "")
			m_Version = "1.0";
	}



	std::wstring CExtension::GetShortenedName() const
	{
		if (m_Name.length() > 75)
			return m_Name.substr(0, 75) + L"...";

		return m_Name;
	}



	int compareversions(
		const CExtension& A, 
		const CExtension& B)
	{
		std::vector<int> Version_A, Version_B;

		std::istringstream ss(A.m_Version);
		std::string token;

		while (std::getline(ss, token, '.'))
			Version_A.push_back(std::atoi(token.c_str()));

		std::istringstream ss2(B.m_Version);
		std::string token2;

		while (std::getline(ss2, token2, '.'))
			Version_B.push_back(std::atoi(token2.c_str()));

		for (size_t i = 0, j = 0; i < Version_A.size() && j < Version_B.size(); ++i, ++j)
		{
			if (Version_A[i] > Version_B[j])
				return 1;
			else if (Version_A[i] < Version_B[j])
				return -1;
		}

		return 0;
	}

}
