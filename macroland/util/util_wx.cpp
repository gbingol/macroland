#include "util_wx.h"

#include <iterator>
#include <fstream>

#include <wx/tokenzr.h>
#include <wx/fontenum.h>


namespace util
{
	
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


}