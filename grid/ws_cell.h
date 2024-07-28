#pragma once

#include <string>
#include <wx/wx.h>
#include <wx/grid.h>
#include <wx/xml/xml.h>

#include "dllimpexp.h"

namespace grid
{
	class Cell;

	class CellFormat
	{
	public:
		DLLGRID CellFormat() = default;
		DLLGRID CellFormat(Cell* cell);

		DLLGRID bool operator==(const CellFormat& other) const;

		DLLGRID void SetFont(const wxFont& font) {
			m_Font = font;
		}

		DLLGRID wxFont GetFont() const {
			return m_Font;
		}

		DLLGRID void SetBackgroundColor(const wxColor& color) {
			m_BGColor = color;
		}

		DLLGRID wxColor GetBackgroundColor() const {
			return m_BGColor;
		}

		DLLGRID void SetTextColor(const wxColor& color) {
			m_TextColor = color;
		}

		DLLGRID wxColor GetTextColor() const
		{
			return m_TextColor;
		}


		DLLGRID void SetAlignment(int horiz, int vert);

		DLLGRID int GetHAlign() const {
			return m_HorAlign;
		}

		DLLGRID int GetVAlign() const {
			return m_VerAlign;
		}

	private:
		wxFont m_Font{ wxNullFont };
		wxColor m_BGColor{ wxNullColour }, m_TextColor{ wxNullColour };
		int m_HorAlign{ 0 }, m_VerAlign{ 0 };

	};




	class Cell
	{

	public:
		static DLLGRID Cell FromXMLNode(const wxGrid* const ws, const wxXmlNode* CellNode);

		//Get topleft and bottom right coordinates
		static DLLGRID std::pair<wxGridCellCoords, wxGridCellCoords>
			Get_TLBR(const std::vector<Cell>& vecCells);

	public:
		DLLGRID Cell() = default;
		DLLGRID Cell(const wxGrid* ws, int row = -1, int col = -1);

		DLLGRID bool operator==(const Cell& other) const;

		DLLGRID const wxGrid* GetWorksheetBase() const
		{
			return m_WSBase;
		}

		DLLGRID int GetRow() const {
			return m_Row;
		}

		DLLGRID void SetRow(int row) {
			m_Row = row;
		}

		DLLGRID int GetCol() const {
			return m_Column;
		}

		DLLGRID void SetCol(int col) {
			m_Column = col;
		}

		DLLGRID std::wstring GetValue() const {
			return m_Value;
		}

		DLLGRID void SetValue(const std::wstring& val) {
			m_Value = val;
		}

		DLLGRID CellFormat GetFormat() const {
			return m_Format;
		}

		DLLGRID void SetFormat(const CellFormat& format) {
			m_Format = format;
		}


		DLLGRID CellFormat GetDefaultFormat() const;
		DLLGRID std::wstring ToXMLString() const;

	private:
		int m_Row{ -1 }, m_Column{ -1 };
		std::wstring m_Value{};

		const wxGrid* m_WSBase{ nullptr };
		CellFormat m_Format;
	};
}