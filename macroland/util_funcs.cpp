#include "util_funcs.h"

#include <iterator>
#include <fstream>
#include <locale>
#include <codecvt>

#include <wx/clipbrd.h>
#include <wx/tokenzr.h>
#include <wx/fontenum.h>


namespace util
{
	void rtrim(std::string& s)
	{
		s.erase(std::find_if(s.rbegin(), s.rend(), [](const char ch) 
		{
			return !std::isspace(ch);
		}).base(), s.end());
	}


	void ltrim(std::string& s) 
	{
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](const char ch)
		{
			return !std::isspace(ch);
		}));
	}

	std::vector<std::string> split(std::string_view txt, std::string_view delim)
    {
        std::vector<std::string> retVec;
    	auto npos = txt.find(delim);
    	size_t start = 0;
		
		while(npos != std::string::npos)
		{
			retVec.push_back(std::string(txt.substr(start, npos-start)));
			start = npos + delim.length();
			npos = start;
			npos = txt.find(delim, npos);  
		}
		retVec.push_back(std::string(txt.substr(start, npos-start)));

    	return retVec;
    }


    std::string join(
		const std::vector<std::string> &Arr, 
		std::string_view delim)
    {
		size_t len = Arr.size();
		if(len == 1)
			return Arr[0];
		
		std::stringstream ss;
        for(size_t i=0; i<len - 1; ++i)
			ss<<Arr[i]<<delim;
		
		ss<<*Arr.rbegin();
		return ss.str();
    }

	void ShowInExplorer(const std::filesystem::path& FullPath)
	{
		wxString cmd = L"explorer /select, " + FullPath.wstring();
		wxExecute(cmd, wxEXEC_ASYNC, NULL);
	}


	bool ClipbrdSupportsText()
	{
		if (!wxTheClipboard->Open())
			return false;

		bool Supports = wxTheClipboard->IsSupported(wxDF_TEXT);
		wxTheClipboard->Close();

		return Supports;
	}


	wxBitmap CreateBitmap(
		const wxString& Text,
		const wxSize& szBMP,
		const wxFont& Font,
		const wxColour& BG,
		const wxColour& TxtFG,
		wxWindow* wnd)
	{
		int BMPWidth = szBMP.GetWidth();
		int BMPHeight = szBMP.GetHeight();

		wxBitmap bmp(BMPWidth, BMPHeight);

		wxMemoryDC memDC;
		memDC.SelectObject(bmp);
		if (BG.IsOk())
			memDC.SetBackground(BG);
		else
			memDC.SetBackground(*wxWHITE);
		memDC.Clear();

		if (TxtFG.IsOk())
			memDC.SetTextForeground(TxtFG);

		if (Font.IsOk())
			memDC.SetFont(Font);

		wxSize szTxt;
		if (wnd)
			szTxt = wnd->FromDIP(memDC.GetTextExtent(Text));
		else
			szTxt = memDC.GetTextExtent(Text);

		int y = BMPHeight / 2 - szTxt.GetHeight() / 2;
		int x = BMPWidth / 2 - szTxt.GetWidth() / 2;

		if (!Text.empty())
			memDC.DrawText(Text, wxPoint(x, y));

		return bmp;
	}


	wxBitmap GetAsBitmap(wxWindow* window, const wxPoint& TopLeft, const wxSize& Size, wxColor BGColor)
	{
		wxClientDC cdc(window);

		int Width = Size.GetWidth(), Height = Size.GetHeight();
		wxBitmap bmp(Width, Height);

		wxMemoryDC memDC;
		memDC.SelectObject(bmp);
		memDC.SetBackground(BGColor);
		memDC.Clear();

		memDC.Blit(wxPoint(0, 0), wxSize(Width, Height), &cdc, TopLeft);

		memDC.SelectObject(wxNullBitmap);

		return bmp;
	}



	wxBitmap GetAsBitmap(wxWindow* window, wxColor BGColor)
	{
		int Width, Height;
		wxPoint TopLeft;

		if (window->IsTopLevel())
		{
			Width = window->GetClientRect().width;
			Height = window->GetClientRect().height;

			TopLeft = window->GetClientRect().GetTopLeft();
		}
		else
		{
			Width = window->GetRect().width;
			Height = window->GetRect().height;

			TopLeft = window->GetRect().GetTopLeft();
		}

		return GetAsBitmap(window, TopLeft, wxSize(Width, Height), BGColor);
	}


	wxArrayString GetFontFaces(bool Sorted)
	{
		wxArrayString fontFaceList;
		wxFontEnumerator fe;
		fe.EnumerateFacenames();
		fontFaceList = fe.GetFacenames();

		for (size_t i = 0; i < fontFaceList.size(); i++)
		{
			wxString str = fontFaceList[i];
			if (str.Contains("@")) {
				fontFaceList.Remove(str);
				continue;
			}
		}

		if (Sorted)
			fontFaceList.Sort();

		return fontFaceList;
	}


		void UnpackArchiveFile(
		const std::filesystem::path& ArchiveFile,
		const std::filesystem::path& Directory)
	{

		wxFileInputStream FileInStream(ArchiveFile.wstring());
		if (!FileInStream.IsOk())
			throw std::runtime_error("File cannot be read");

		wxZipInputStream ZipInStream(FileInStream);
		if (!ZipInStream.IsOk())
			throw std::runtime_error("Archive file is corrupted.");

		if (!std::filesystem::exists(Directory))
			throw std::exception("Directory does not exist");

		wxZipEntry* ZipEntry;
		while ((ZipEntry = ZipInStream.GetNextEntry()) != NULL)
		{
			if (ZipEntry->IsDir())
				continue;

			auto EntryPath = ZipEntry->GetInternalName();
			wxFileName EntryFile(EntryPath);

			auto DirFullPath = Directory.wstring() + wxFileName::GetPathSeparator() + EntryFile.GetPath();

			auto SubDirectories = EntryFile.GetDirs();

			size_t Depth = SubDirectories.size();
			while (!wxDir::Exists(DirFullPath))
			{
				Depth--;
				EntryFile.RemoveLastDir();
				DirFullPath = Directory.wstring() + wxFileName::GetPathSeparator() + EntryFile.GetPath();
			}

			for (size_t i = Depth; i < SubDirectories.size(); i++) {
				DirFullPath = DirFullPath + wxFileName::GetPathSeparator() + SubDirectories[i];

				wxDir::Make(DirFullPath);
			}

			wxString FullPath = Directory.wstring() + wxFileName::GetPathSeparator() + ZipEntry->GetInternalName();

			ZipInStream.OpenEntry(*ZipEntry);

			auto ZipEntrySize = ZipEntry->GetSize();

			void* RawData = malloc(ZipEntrySize);
			ZipInStream.Read(RawData, ZipEntrySize);

			wxFile file;
			file.Create(FullPath);
			file.Open(FullPath, wxFile::write);

			file.Write(RawData, ZipEntrySize);
			file.Close();
		}
	}


	bool ArchiveFolder(
		const std::filesystem::path& SrcFolderPath,
		const std::filesystem::path& ArchiveFile)
	{
		do
		{
			wxFFileOutputStream out(ArchiveFile.wstring());

			if (!out.IsOk())
				break;

			wxZipOutputStream ZipOutStream(out);

			if (!ZipOutStream.IsOk())
				break;

			DirTraverserZIP traverser(ZipOutStream, SrcFolderPath);

			wxDir srcDir(SrcFolderPath.wstring());

			if (srcDir.Traverse(traverser, wxEmptyString) == (size_t)-1)
				break;

			return true;

		} while (false);

		return false;
	}


	wxDirTraverseResult DirTraverserZIP::OnFile(const wxString& filename)
	{
		do {
			wxFileName newFileName(filename);
			newFileName.MakeRelativeTo(m_BaseDir.wstring());

			if (!m_Stream.PutNextEntry(newFileName.GetFullPath()))
				break;

			wxFileInputStream in(filename);
			if (!in.IsOk()) break;

			m_Stream.Write(in);

			return wxDIR_CONTINUE;
		} while (false);

		return wxDIR_STOP;
	}


	/************************************************************** */

	CDate::CDate()
	{
		time_t Time = std::time(0);
		std::tm* TimeInfo = std::localtime(&Time);

		m_Year = 1900 + TimeInfo->tm_year;
		m_Month = TimeInfo->tm_mon + 1;
		m_Day = TimeInfo->tm_mday;
		m_Hour = TimeInfo->tm_hour;
		m_Minute = TimeInfo->tm_min;
		m_Second = TimeInfo->tm_sec;
	}

	std::string CDate::year() const {
		return std::to_string(m_Year);
	}

	std::string CDate::month() const
	{
		return (m_Month < 10) ?
			std::string("0") + std::to_string(m_Month) :
			std::to_string(m_Month);
	}

	std::string CDate::day() const
	{
		return (m_Day < 10) ?
			std::string("0") + std::to_string(m_Day) :
			std::to_string(m_Day);
	}

	std::string CDate::hour() const
	{
		return (m_Hour < 10) ?
			std::string("0") + std::to_string(m_Hour) :
			std::to_string(m_Hour);
	}

	std::string CDate::minute() const
	{
		return (m_Minute < 10) ?
			std::string("0") + std::to_string(m_Minute) :
			std::to_string(m_Minute);
	}

	std::string CDate::second() const
	{
		return (m_Second < 10) ?
			std::string("0") + std::to_string(m_Second) :
			std::to_string(m_Second);
	}


	bool CDate::operator<(const CDate& other) const
	{
		return  !(
			m_Year > other.m_Year ||
			m_Month > other.m_Month ||
			m_Day > other.m_Day ||
			m_Hour > other.m_Hour ||
			m_Minute > other.m_Minute ||
			m_Second >= other.m_Second);
	}

	bool CDate::operator==(const CDate& other) const
	{
		return (
			m_Year == other.m_Year &&
			m_Month == other.m_Month &&
			m_Day == other.m_Day &&
			m_Hour == other.m_Hour &&
			m_Minute == other.m_Minute &&
			m_Second == other.m_Second);
	}

	bool CDate::operator>(const CDate& other) const
	{
		return (
			!(*this < other) && 
			!(*this == other));
	}





	/**************************************************************** */

	CRecentFiles::CRecentFiles(const std::filesystem::path& filePath)
	{
		m_FilePath = filePath;
	}


	bool CRecentFiles::ReadOrCreate()
	{
		if (std::filesystem::exists(m_FilePath))
		{
			std::wifstream file(m_FilePath);
			file.imbue(std::locale(file.getloc(), new std::codecvt_utf8_utf16<wchar_t>));

			std::wstring Line;
			while (std::getline(file, Line))
				m_List.push_back(Line);

			file.close();
		}
		else
		{
			std::wofstream file(m_FilePath);
			file.close();
		}

		return true;
	}


	bool CRecentFiles::Write()
	{
		std::wofstream file(m_FilePath);
		file.imbue(std::locale(file.getloc(), new std::codecvt_utf8_utf16<wchar_t>));
		for (const auto& p : m_List)
		{
			if (std::filesystem::exists(p))
				file << p.wstring() << std::endl;
		}

		file.close();

		return true;
	}

	void CRecentFiles::Append(const std::filesystem::path& path)
	{
		for (const auto& p : m_List)
		{
			if (p == path)
			{
				m_List.remove(p);
				break;
			}
		}

		m_List.push_front(path);
	}




	Configuration::Configuration(std::filesystem::path path)
	{
		if (!std::filesystem::exists(path))
			throw std::exception("File does not exist");

		std::wifstream file(path);
		if (!file.is_open())  
			throw std::exception("Failed to open file");
		file.imbue(std::locale(file.getloc(), new std::codecvt_utf8<wchar_t>));
	
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		for (std::wstring Line; std::getline(file, Line);)
			m_Content.append(converter.to_bytes(Line));

		file.close();
	}


	std::unordered_map<std::string, std::string> Configuration::Parse()
	{
		std::unordered_map<std::string, std::string> Map;

		std::istringstream input(m_Content);
		std::string id, value;
		for (std::string line; std::getline(input, line);)
		{
			line = trim(line);

			if(line[0] == '#' || line.empty()) continue;

			bool Statement = line[0] != '-';
			if(Statement) {
				auto v = split(line, "=");
				if(v.size() < 2)
					continue;

				id = trim(v[0]);
				std::transform(id.begin(), id.end(), id.begin(), ::toupper);
				value = v[1];
				Map[id] = value;
			}
			else {
				//Get it from Map
				std::string str = Map[id];

				//remove '-' from the line
				line.erase(line.begin());
				str += "\n" + line;

				//update Map
				Map[id] = str;
			}
		}

		return Map;
	}
}