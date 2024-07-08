#pragma once

#include <filesystem>
#include <string>
#include <wx/wx.h>

namespace util
{
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
}