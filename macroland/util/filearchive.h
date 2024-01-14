#pragma once

#include <filesystem>

#include <wx/wx.h>
#include <wx/dir.h>
#include <wx/wfstream.h>
#include <wx/zipstrm.h>

namespace util
{
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