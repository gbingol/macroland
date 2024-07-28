#include "ws_funcs.h"
#include <algorithm>
#include <assert.h>
#include <string>
#include <vector>
#include <sstream>

#include <wx/wx.h>
#include <wx/grid.h>
#include <wx/clipbrd.h>
#include <wx/tokenzr.h>

#include "ws_cell.h"
#include "ws_xml.h"
#include "worksheetbase.h"

namespace grid
{
	std::string ColNumtoLetters(size_t num)
	{
		//Takes a number say 772 and finds equivalent character set, which is ACR for 772
		//For example Z=26, AA=27, AB=28 ...

		std::string Str;

		assert(num > 0);

		int NChars = (int)(ceil(log(num) / log(26.0)));

		if (NChars == 1)
		{
			int modular = num % 26;

			if (modular == 0) modular = 26;

			Str = char(65 + modular - 1);

			return Str;
		}

		size_t val = num;
		std::vector<char> tbl;
		for (int i = 1; i <= (NChars - 1); i++)
		{
			val /= 26;
			tbl.insert(tbl.begin(), char(65 + int(val % 26) - 1));
		}

		for (const auto& c : tbl)
			Str += c;

		Str += char(65 + int(num % 26) - 1);

		return Str;
	}



	int LetterstoColumnNumber(const std::string& letters)
	{
		//str can be AA, BC, AAC ...
		std::string str = letters;
		std::transform(str.begin(), str.end(), str.begin(), ::toupper); //convert all to upper case

		int retNum = 0;
		int len = str.length();
		int n = 0;

		//str[len]='\0'
		for (int i = len - 1; i >= 0; i--)
		{
			int val = (int)str[i] - 65 + 1;
			retNum += pow(26, n) * val;

			n++;
		}

		return retNum;
	}


	std::wstring FonttoString(const wxFont& font)
	{
		if (!font.IsOk())
			return L"";

		std::wstringstream fntStr;

		if (font.GetWeight() == wxFONTWEIGHT_BOLD)
			fntStr << "BOLD|";

		if (font.GetStyle() == wxFONTSTYLE_ITALIC)
			fntStr << "ITALIC|";

		if (font.GetUnderlined())
			fntStr << "UNDERLINED|";

		fntStr << ";";
		fntStr << font.GetFaceName() << ";";
		fntStr << font.GetPointSize() << ";";

		return fntStr.str();
	}


	wxFont StringtoFont(const wxString& str)
	{
		wxFont retFont = wxFont();
		std::vector<wxString> fontProps;
		wxStringTokenizer tokens(str, ";", wxTOKEN_RET_EMPTY);

		while (tokens.HasMoreTokens())
			fontProps.push_back(wxString::FromUTF8(tokens.GetNextToken()));

		if (fontProps.size() < 3)
			return wxNullFont;

		if (!fontProps.at(0).IsEmpty())
		{
			wxString fontWeight = fontProps.at(0);

			if (fontWeight.Contains("BOLD"))
				retFont.MakeBold();

			if (fontWeight.Contains("ITALIC"))
				retFont.MakeItalic();

			if (fontWeight.Contains("UNDERLINED"))
				retFont.MakeUnderlined();
		}

		retFont.SetFaceName(fontProps.at(1));

		long PointSize;
		fontProps.at(2).ToLong(&PointSize);
		retFont.SetPointSize(PointSize);

		return retFont;
	}



	wxString GenerateTabString(
		const CWorksheetBase* ws,
		const wxGridCellCoords& TL,
		const wxGridCellCoords& BR)
	{
		wxString Str;

		for (int i = TL.GetRow(); i <= BR.GetRow(); ++i)
		{
			for (int j = TL.GetCol(); j <= BR.GetCol(); ++j)
				Str << ws->GetCellValue(i, j) << "\t";

			Str.RemoveLast();
			Str << "\n";
		}

		return Str;
	}


	std::pair<wxGridCellCoords, wxGridCellCoords> 
		AddSelToClipbrd(const CWorksheetBase* ws)
	{
		wxGridCellCoords TL, BR;

		if (ws->IsSelection()) 
		{
			TL = ws->GetSelTopLeft();
			BR = ws->GetSelBtmRight();
		}
		else 
		{
			TL.SetRow(ws->GetGridCursorRow());
			TL.SetCol(ws->GetGridCursorCol());

			BR = TL;
		}

		auto XMLStr = xml::GenerateXMLString(ws, TL, BR);
		wxString TabStr = GenerateTabString(ws, TL, BR);

		wxDataObjectComposite* dataobj = new wxDataObjectComposite();
		dataobj->Add(new xml::XMLDataObject(XMLStr), true);
		dataobj->Add(new wxTextDataObject(TabStr));

		if (wxTheClipboard->Open())
		{
			wxTheClipboard->SetData(dataobj);
			wxTheClipboard->Flush();
			wxTheClipboard->Close();
		}

		return { TL, BR };
	}

}