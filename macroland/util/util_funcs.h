#pragma once

#include <filesystem>
#include <string>
#include <time.h>
#include <list>
#include <unordered_map>

#include <wx/wx.h>
#include <wx/dir.h>
#include <wx/wfstream.h>
#include <wx/zipstrm.h>





namespace util
{
	void ltrim(std::string& s);
	void rtrim(std::string& s);

	inline std::string trim(std::string& s)
	{
		ltrim(s);
		rtrim(s);
		return s;
	}

	/*
		txt = "abc.def", delim = "."   -> {"abc", "def"}
		txt = "abc.def.", delim = "."  -> {"abc", "def", ""}
	*/
	std::vector<std::string> split(
			std::string_view txt, 
			std::string_view delim);


	/*
		Arr={"abc", "def"}, delim="." ->"abc.def"
		Arr={"abc"}, delim="." ->"abc"
	*/
	std::string join(
		const std::vector<std::string>& Arr, 
		std::string_view delim);




	/**************************************************** */


	wxArrayString GetFontFaces(bool Sorted = true);

	void UnpackArchiveFile(
		const std::filesystem::path& ArchiveFile,
		const std::filesystem::path& Directory);

	bool ArchiveFolder(
		const std::filesystem::path& SrcFolderPath,
		const std::filesystem::path& ArchiveFile);


	class DirTraverserZIP : public wxDirTraverser
	{
	public:
		DirTraverserZIP(
			wxZipOutputStream& stream,
			const std::filesystem::path& baseDir) :
			m_Stream{ stream }, m_BaseDir{ baseDir } { }

		virtual wxDirTraverseResult OnFile(const wxString& filename);

		virtual wxDirTraverseResult OnDir(const wxString& dirname)
		{
			return wxDIR_CONTINUE;
		}

	private:
		wxZipOutputStream& m_Stream;
		std::filesystem::path m_BaseDir;
	};

}