#include "ws_cell.h"

#include <sstream>

#include "ws_funcs.h"


namespace grid
{
	CellFormat::CellFormat(Cell* cell)
	{
		auto worksheet = cell->GetWorksheetBase();
		int row = cell->GetRow();
		int col = cell->GetCol();

		m_Font = worksheet->GetCellFont(row, col);
		m_BGColor = worksheet->GetCellBackgroundColour(row, col);
		m_TextColor = worksheet->GetCellTextColour(row, col);

		int horizontal = 0, vertical = 0;
		worksheet->GetCellAlignment(row, col, &horizontal, &vertical);

		m_HorAlign = horizontal;
		m_VerAlign = vertical;
	}


	bool CellFormat::operator==(const CellFormat& other) const
	{
		return m_Font == other.m_Font &&
			m_BGColor == other.m_BGColor &&
			m_TextColor == other.m_TextColor &&
			m_HorAlign == other.m_HorAlign &&
			m_VerAlign == other.m_VerAlign;
	}


	void CellFormat::SetAlignment(int horiz, int vert)
	{
		m_HorAlign = horiz >= 0 ? horiz : wxALIGN_LEFT;
		m_VerAlign = vert >= 0 ? vert : wxALIGN_BOTTOM;
	}




	/****************************  Cell  ********************************************/


	Cell::Cell(const wxGrid* worksheet, int row, int col)
	{
		m_WSBase = worksheet;

		m_Row = row;
		m_Column = col;

		m_Value = worksheet->GetCellValue(row, col);
		m_Format = CellFormat(this);
	}


	bool Cell::operator==(const Cell& other) const
	{
		return m_Value == other.m_Value;
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
		CellFormat format(&cell);

		wxXmlNode* node = CellNode->GetChildren();
		while (node)
		{
			std::wstring ChName = wxString::FromUTF8(node->GetName()).ToStdWstring();
			std::wstring NdCntnt = wxString::FromUTF8(node->GetNodeContent()).ToStdWstring();

			if (ChName == L"VAL")
				cell.SetValue(NdCntnt);

			else if (ChName == L"BGC")
				format.SetBackgroundColor(wxColor(NdCntnt));

			else if (ChName == L"FGC")
				format.SetTextColor(wxColor(NdCntnt));

			else if (ChName == L"HALIGN" || ChName == L"VALIGN")
			{
				long HAlign = 0, VAlign = 0;
				long Algn = !NdCntnt.empty() ? _wtoi(NdCntnt.c_str()) : 0;

				(ChName == L"HALIGN") ? HAlign = Algn : VAlign = Algn;
				format.SetAlignment(HAlign, VAlign);
			}

			else if (ChName == "FONT")
				format.SetFont(StringtoFont(NdCntnt));

			node = node->GetNext();
		}

		cell.SetFormat(format);

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



	CellFormat Cell::GetDefaultFormat() const
	{
		int horizontal = 0, vertical = 0;
		m_WSBase->GetDefaultCellAlignment(&horizontal, &vertical);

		CellFormat format;

		format.SetAlignment(horizontal, vertical);
		format.SetFont(m_WSBase->GetDefaultCellFont());
		format.SetTextColor(m_WSBase->GetDefaultCellTextColour());
		format.SetBackgroundColor(m_WSBase->GetDefaultCellBackgroundColour());

		return format;
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

		XML << "<BGC>" << m_Format.GetBackgroundColor().GetAsString().ToStdWstring() << "</BGC>";
		XML << "<FGC>" << m_Format.GetTextColor().GetAsString().ToStdWstring() << "</FGC>";

		if (m_Format.GetHAlign() != 0)
			XML << "<HALIGN>" << m_Format.GetHAlign() << "</HALIGN>";

		if (m_Format.GetVAlign() != 0)
			XML << "<VALIGN>" << m_Format.GetVAlign() << "</VALIGN>";

		XML << "<FONT>" << FonttoString(m_Format.GetFont()) << "</FONT>";

		XML << "</CELL>";

		return XML.str();
	}


}