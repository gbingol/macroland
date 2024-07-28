#pragma once

#include <string>
#include <tuple>

#include <wx/wx.h>
#include <wx/grid.h>

#include "dllimpexp.h"

namespace grid
{
	class CWorksheetBase;

	DLLGRID std::string ColNumtoLetters(size_t num);

	DLLGRID int LetterstoColumnNumber(const std::string& letters);

	DLLGRID wxFont StringtoFont(const wxString& str);
	DLLGRID std::wstring FonttoString(const wxFont& font);


	//tabs and newlines
	DLLGRID wxString GenerateTabString(
		const CWorksheetBase* ws,
		const wxGridCellCoords& TopLeft,
		const wxGridCellCoords& BottomRight);

	DLLGRID std::pair<wxGridCellCoords, wxGridCellCoords>
		AddSelToClipbrd(const CWorksheetBase* ws);
}