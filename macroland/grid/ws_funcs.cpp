#include "ws_funcs.h"

#include <algorithm>
#include <assert.h>
#include <string>
#include <vector>
#include <sstream>
#include <string>
#include <sstream>
#include <codecvt>
#include <locale>

#include <wx/wx.h>
#include <wx/grid.h>
#include <wx/clipbrd.h>
#include <wx/tokenzr.h>
#include <wx/sstream.h>


#include "ws_cell.h"
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


	std::string FonttoString(const wxFont& font)
	{
		if (!font.IsOk())
			return "";

		std::stringstream fntStr;

		if (font.GetWeight() == wxFONTWEIGHT_BOLD)
			fntStr << "BOLD|";

		if (font.GetStyle() == wxFONTSTYLE_ITALIC)
			fntStr << "ITALIC|";

		if (font.GetUnderlined())
			fntStr << "UNDERLINED|";

		fntStr << ";";
		fntStr << font.GetFaceName().utf8_str() << ";";
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

		auto XMLStr = GenerateXMLString(ws, TL, BR);
		wxString TabStr = GenerateTabString(ws, TL, BR);

		wxDataObjectComposite* dataobj = new wxDataObjectComposite();
		dataobj->Add(new XMLDataObject(XMLStr), true);
		dataobj->Add(new wxTextDataObject(TabStr));

		if (wxTheClipboard->Open())
		{
			wxTheClipboard->SetData(dataobj);
			wxTheClipboard->Flush();
			wxTheClipboard->Close();
		}

		return { TL, BR };
	}





	/*************************************************************** */


	std::optional<wxXmlDocument> CreateXMLDoc(const wxString& XMLString)
	{
		wxStringInputStream ss(XMLString);
		wxXmlDocument xmlDoc(ss);

		if (!xmlDoc.IsOk()) return std::nullopt;

		return xmlDoc;
	}


	wxString GetXMLData()
	{
		if (!wxTheClipboard->Open())
			return wxEmptyString;

		if (!wxTheClipboard->IsSupported(XMLDataFormat()))
			return wxEmptyString;
		
		XMLDataObject xmlObj;
		wxTheClipboard->GetData(xmlObj);
		wxTheClipboard->Close();

		return xmlObj.GetText();
	}


	bool SupportsXML()
	{
		if (!wxTheClipboard->Open())
			return false;

		bool Supports = wxTheClipboard->IsSupported(XMLDataFormat());
		wxTheClipboard->Close();

		return Supports;
	}


	std::vector<Cell> XMLDocToCells(
		const CWorksheetBase* ws,
		const wxXmlDocument& xmlDoc)
	{
		std::vector<Cell> Cells;

		wxXmlNode* worksheet_node = xmlDoc.GetRoot();

		if (!worksheet_node)
			return Cells;

		wxXmlNode* node = worksheet_node->GetChildren();
		while (node) {
			if (node->GetName() == "CELL")
				Cells.push_back(Cell::FromXMLNode(ws, node));

			node = node->GetNext();
		}

		return Cells;
	}


	std::tuple<long, long, bool> XMLNodeToRowsCols(const wxXmlNode* xmlNode)
	{
		wxString Name = xmlNode->GetName();

		if (!(Name == "ROW" || Name == "COL"))
			throw std::exception("XMLNode name must be ROW or COL");

		long int rowpos = 0, size = 22, usrAdj = 0;
		bool Adjusted = false; //user adjusted

		xmlNode->GetAttribute("LOC").ToLong(&rowpos);
		xmlNode->GetAttribute("SIZE").ToLong(&size);
		xmlNode->GetAttribute("USERADJ").ToLong(&usrAdj);

		Adjusted = usrAdj == 0 ? false : true;

		return std::make_tuple(rowpos, size, Adjusted);
	}


	bool ParseXMLDoc(
		CWorksheetBase* ws,
		const wxXmlDocument& xmlDoc)
	{
		if (xmlDoc.IsOk() == false)
			return false;

		wxXmlNode* ws_node = xmlDoc.GetRoot();
		if (!ws_node)
			return false;


		wxXmlNode* node = ws_node->GetChildren();
		while (node)
		{
			wxString NodeName = node->GetName();

			if (NodeName == "CELL")
			{
				Cell cell = Cell::FromXMLNode(ws, node);
				int Row = cell.GetRow(), Col = cell.GetCol();

				if (!cell.GetValue().empty())
					ws->SetValue(Row, Col, cell.GetValue(), false);

				ws->ApplyCellFormat(Row, Col, cell, false); //does not mark the worksheet as dirty
			}

			else if (NodeName == "ROW")
			{
				const auto [row, height, mybool] = XMLNodeToRowsCols(node);
				ws->SetCleanRowSize(row, height);
			}

			else if (NodeName == "COL")
			{
				const auto [col, width, mybool] = XMLNodeToRowsCols(node);
				ws->SetColSize(col, width);
			}

			node = node->GetNext();
		}

		return true;
	}


	std::string GenerateXMLString(
		const CWorksheetBase* ws,
		const wxGridCellCoords& TL,
		const wxGridCellCoords& BR)
	{
		std::stringstream Str;
		Str << "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n";

		Str << "<WORKSHEET> \n";

		for (int i = TL.GetRow(); i <= BR.GetRow(); i++)
			for (int j = TL.GetCol(); j <= BR.GetCol(); j++)
			{
				wxString s = ws->GetAsCellObject(i, j).ToXMLString();
				Str << s.mb_str(wxConvUTF8) << "\n";
			}

		Str << "</WORKSHEET>";

		return Str.str();
	}


	
	std::string GenerateXMLString(CWorksheetBase* ws)
	{
		/*
		VAL: Cell value (content of cell)
		BGC: Cell's background color
		FGC: Color of the text in the cell
		HALIGN: Horizontal alignment of cell's content: LEFT, CENTER, RIGHT
		VALIGN: Horizontal alignment of cell's content: LEFT, CENTER, RIGHT
		FONTSIZE: Font size
		FONTFACE:
		*/

		auto Changed = ws->GetChangedCells();

		std::stringstream XMLString;
		XMLString << "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n";
		XMLString << "<WORKSHEET> \n";

		for (const auto& elem : Changed)
		{
			std::wstring c = ws->GetAsCellObject(elem).ToXMLString();
			std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
			XMLString << converter.to_bytes(c) << "\n";
		}

		std::wstringstream RowXML;
		for (const auto& elem : ws->GetAdjustedRows())
		{
			RowXML << "<ROW";
			RowXML << " LOC=" << "\"" << elem.first << "\"";
			RowXML << " SIZE=" << "\"" << elem.second.m_Height << "\"";
			RowXML << " USERADJ=" << "\"" << !elem.second.m_SysAdj << "\"";
			RowXML << ">" << "</ROW>\n";
		}

		if (RowXML.rdbuf()->in_avail() > 0)
			XMLString << RowXML.str() << "\n";

		std::wstringstream ColXML;
		for (const auto& elem : ws->GetAdjustedCols())
		{
			ColXML << "<COL";
			ColXML << " LOC=" << "\"" << elem.first << "\"";
			ColXML << " SIZE=" << "\"" << elem.second << "\"";
			ColXML << " USERADJ=" << "\"" << true << "\"";
			ColXML << ">" << "</COL>\n";
		}

		if (ColXML.rdbuf()->in_avail() > 0)
			XMLString << ColXML.str() << "\n";

		XMLString << "</WORKSHEET>";

		return XMLString.str();
	}


}