#include "ws_cell.h"

#include <sstream>

#include "ws_funcs.h"


namespace grid
{

	Cell::Cell(const wxGrid* worksheet, int row, int col)
	{
		m_WSBase = worksheet;

		m_Row = row;
		m_Column = col;

		m_Value = worksheet->GetCellValue(row, col);
		m_Font = worksheet->GetCellFont(row, col);
		m_BGColor = worksheet->GetCellBackgroundColour(row, col);
		m_TextColor = worksheet->GetCellTextColour(row, col);

		int horizontal = 0, vertical = 0;
		worksheet->GetCellAlignment(row, col, &horizontal, &vertical);

		m_HorAlign = horizontal;
		m_VerAlign = vertical;
	}


	bool Cell::operator==(const Cell& other) const
	{
		return m_Value == other.m_Value;
	}

	void Cell::SetAlignment(int horiz, int vert)
	{
		m_HorAlign = horiz >= 0 ? horiz : wxALIGN_LEFT;
		m_VerAlign = vert >= 0 ? vert : wxALIGN_BOTTOM;
	}


	Cell Cell::FromXMLNode(const wxGrid* const ws, const wxXmlNode* CellNode)
	{
		/*
		VAL: Cell value (content of cell)
		BGC: Cell's background color
		FGC: Color of the text in the cell
		HORALIGN: Horizontal alignment of cell's content: LEFT, CENTER, RIGHT
		*/

		long int rowpos = 0, colpos = 0;

		CellNode->GetAttribute("R").ToLong(&rowpos);
		CellNode->GetAttribute("C").ToLong(&colpos);

		Cell cell(ws, rowpos, colpos);
		
		wxXmlNode* node = CellNode->GetChildren();
		while (node)
		{
			std::wstring ChName = wxString::FromUTF8(node->GetName()).ToStdWstring();
			std::wstring NdCntnt = wxString::FromUTF8(node->GetNodeContent()).ToStdWstring();

			if (ChName == L"VAL")
				cell.SetValue(NdCntnt);

			else if (ChName == L"BGC")
				cell.SetBackgroundColor(wxColor(NdCntnt));

			else if (ChName == L"FGC")
				cell.SetTextColor(wxColor(NdCntnt));

			else if (ChName == L"HALIGN" || ChName == L"VALIGN")
			{
				long HAlign = 0, VAlign = 0;
				long Algn = !NdCntnt.empty() ? _wtoi(NdCntnt.c_str()) : 0;

				(ChName == L"HALIGN") ? HAlign = Algn : VAlign = Algn;
				cell.SetAlignment(HAlign, VAlign);
			}

			else if (ChName == "FONT")
				cell.SetFont(StringtoFont(NdCntnt));

			node = node->GetNext();
		}

		return cell;
	}


	std::pair<wxGridCellCoords, wxGridCellCoords>
		Cell::Get_TLBR(const std::vector<Cell>& vecCells)
	{
		//Given a vector of cells, it finds the TopLeft and BottomRight coordinates
		int T = vecCells[0].GetRow(); //top
		int L = vecCells[0].GetCol(); //left
		int B = vecCells[0].GetRow(); //bottom
		int R = vecCells[0].GetCol();

		for (const auto& c : vecCells)
		{
			T = std::min(c.GetRow(), T);
			L = std::min(c.GetCol(), L);

			B = std::max(c.GetRow(), B);
			R = std::max(c.GetCol(), R);
		}

		return { wxGridCellCoords(T, L), wxGridCellCoords(B, R) };
	}



	std::wstring Cell::ToXMLString() const
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

		std::wstringstream XML;

		std::wstring cellVal = GetValue();

		//Check cell value for special characters such as >, <, & which are not allowed in XML
		//Note that when reading back from XML document, wxWidgets automatically recognizes special characters, such as &lt; to be equal to <
		//However, when writing to XML we must substitute some of the special characters
		std::wstringstream modStr;
		for (const auto& c: cellVal)
		{
			if (c == '<') modStr << "&lt;";
			else if (c == '>') modStr << "&gt;";
			else if (c == '&') modStr << "&amp;";
			else modStr << c;
		}

		cellVal = modStr.str();

		XML << "<CELL R=" << "\"" << GetRow() << "\"" << " C=" << "\"" << GetCol() << "\"" << " >";

		if (!cellVal.empty())
			XML << "<VAL>" << cellVal << "</VAL>";

		XML << "<BGC>" << GetBackgroundColor().GetAsString().ToStdWstring() << "</BGC>";
		XML << "<FGC>" << GetTextColor().GetAsString().ToStdWstring() << "</FGC>";

		if (GetHAlign() != 0)
			XML << "<HALIGN>" << GetHAlign() << "</HALIGN>";

		if (GetVAlign() != 0)
			XML << "<VALIGN>" << GetVAlign() << "</VALIGN>";

		XML << "<FONT>" << FonttoString(GetFont()) << "</FONT>";

		XML << "</CELL>";

		return XML.str();
	}

	std::string Cell::ToJSON() const
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

		std::stringstream jsn;

		auto cellVal = m_WSBase->GetCellValue(m_Row, m_Column).utf8_string();

		jsn << "{";
		jsn << "\"row\":" << GetRow() << ",\"col\":" << GetCol() << ",";

		jsn << "\"val\":" << cellVal << ",";

		jsn << "\"bgc\":" << GetBackgroundColor().GetAsString().ToStdWstring() << ",";
		jsn << "\"fgc\":" << GetTextColor().GetAsString().ToStdWstring() << ",";

		jsn << "\"halign\":" << GetHAlign() << ",";

		jsn << "\"valign\":" << GetVAlign() << ",";

		jsn << "\"font\":" << FonttoString(GetFont());

		jsn << "}";

		return jsn.str();
	}
}