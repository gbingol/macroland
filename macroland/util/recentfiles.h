#pragma once

#include <filesystem>
#include <list>

namespace util
{
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

		std::list<std::filesystem::path> GetList() const
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