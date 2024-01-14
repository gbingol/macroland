#include "ws_xml.h"
#include <optional>
#include <tuple>
#include <string>
#include <sstream>
#include <codecvt>
#include <locale>
#include <wx/wx.h>
#include <wx/xml/xml.h>
#include <wx/sstream.h>
#include <wx/clipbrd.h>

#include "worksheetbase.h"
#include "ws_cell.h"


namespace grid::xml
{
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