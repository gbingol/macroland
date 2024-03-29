#pragma once

#include <filesystem>
#include <string>
#include <lua.hpp>



namespace extension
{

	struct CExtension
	{
		std::wstring m_Name{}; //application name
		std::wstring m_Developer{};
		std::wstring m_Short_Desc{}; //Brief description of the extension
		std::string m_Version{}; //Version of the extension
		std::filesystem::path m_ReadMe; //Full name of the readme file, readme.html

		bool m_Enabled{}; //is the extension enabled
		std::filesystem::path m_DirPath{};
		std::string m_GUID{}; //Globally Unique Identifier


		CExtension() = default;

		/*
			Make an extension from lua state which is populated from  manifest.lua
			DirEntry is the directory path of the extension
		*/
		CExtension(lua_State* L, const std::filesystem::path& DirEntry);

		bool operator==(const CExtension& rhs) const
		{
			return m_GUID == rhs.m_GUID;
		}

		//full path of readme file
		auto GetReadMePath() const
		{
			return m_DirPath / m_ReadMe;
		}

		std::wstring GetShortenedName() const;
	};



	//given a directory path, can it qualify as an extension
	static bool IsValid(
		const std::filesystem::path& p)
	{
		return std::filesystem::exists(p / "manifest.lua");
	}


	static bool IsEnabled(const std::filesystem::path& p)
	{
		auto Stem = p.stem();
		return Stem.string()[0] != '_';
	}


	//ExtensionPath: the path of the extension that is enabled
	static std::filesystem::path Disable(
		const std::filesystem::path& ExtensionPath)
	{
		auto p = ExtensionPath.parent_path();
		p /= std::filesystem::path("_");
		p += ExtensionPath.stem();

		std::filesystem::rename(ExtensionPath, p);

		return p;
	}


	//ExtensionPath: the path of the extension that is disabled
	static std::filesystem::path Enable(
		const std::filesystem::path& ExtensionPath)
	{
		auto parent = ExtensionPath.parent_path();

		auto Stem = ExtensionPath.stem().wstring();
		auto EnabledDir = std::filesystem::path(Stem.begin() + 1, Stem.end());

		auto EnabledPath = parent / EnabledDir;

		std::filesystem::rename(ExtensionPath, EnabledPath);

		return EnabledPath;
	}


	//+1:this is newer, 0: same, -1: this is older
	int compareversions(
		const CExtension& A, 
		const CExtension& B);
}