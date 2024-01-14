#include "recentfiles.h"

#include <iterator>
#include <fstream>
#include <string>
#include <locale>
#include <codecvt>



namespace util
{
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
}