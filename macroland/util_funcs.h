#pragma once

#include <filesystem>
#include <string>
#include <time.h>
#include <list>

#include <wx/wx.h>
#include <wx/dir.h>
#include <wx/wfstream.h>
#include <wx/zipstrm.h>





namespace util
{
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


	void ShowInExplorer(const std::filesystem::path& FullPath);

	bool ClipbrdSupportsText();

	wxBitmap CreateBitmap(
		const wxString& Text,
		const wxSize& BitmapSize,
		const wxFont& Font,
		const wxColour& BG,
		const wxColour& TxtFG,
		wxWindow* wnd = nullptr);

	//Get the area as image specified by TopLeft and size
	wxBitmap GetAsBitmap(
		wxWindow* window,
		const wxPoint& TopLeft,
		const wxSize& Size,
		wxColor BGColor = wxColor(255, 255, 255));


	//Get the client area (whatever drawn there) of a window as bitmap
	wxBitmap GetAsBitmap(
		wxWindow* window,
		wxColor BGColor = wxColor(255, 255, 255));

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


	/************************************************************************ */

	class CDate
	{
	public:
		CDate();

		CDate(const CDate& another) = default;
		CDate& operator=(const CDate& other) = default;

		bool operator<(const CDate& other) const;
		bool operator==(const CDate& other) const;
		bool operator>(const CDate& other) const;

		~CDate() = default;

		std::string year() const;

		//if Name=true then returns January, February..., otherwise appends 0 if month is smaller than 10
		std::string month() const;

		//if day is smaller than 10, appends zero
		std::string day() const;

		//if hour is smaller than 10, appends zero
		std::string hour() const;

		//if minute is smaller than 10, appends zero
		std::string minute() const;

		//if second is smaller than 10, appends zero
		std::string second() const;

		//Get the current time as hour:minute:second
		std::string GetTime(const char* sep = ":") const 
		{
			return hour() + sep + minute() + sep + second();
		}

		//Gets the date as: day/month/year, reversed=true -> year/month/day 
		std::string GetDate(const char* sep = "/", bool reversed = false) const
		{
			if (reversed)
				return year() + sep + month() + sep + day();

			return  day() + sep + month() + sep + year();
		}

	private:
		int m_Year, m_Month, m_Day;
		int m_Hour, m_Minute, m_Second;
	};




	/******************************************************************** */

	class CRecentFiles
	{
	public:
		//filepath is the file's path which contains/should contain entries
		CRecentFiles(const std::filesystem::path& filePath);

		//if file exists reads the contents otherwise creates the file
		bool ReadOrCreate();

		//if file exists, deletes and creates a new one and writes the contents and closes file
		bool Write();

		void Append(const std::filesystem::path& path);

		auto GetList() const
		{
			return m_List;
		}

		void ClearList()
		{
			m_List.clear();
		}

	private:
		std::filesystem::path m_FilePath;
		std::list<std::filesystem::path> m_List;
	};
}