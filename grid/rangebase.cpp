#include "rangebase.h"

#include "worksheetbase.h"
#include "workbookbase.h"
#include "ws_funcs.h"



namespace grid
{
	CRangeBase* CRangeBase::Merge(
		const CRangeBase* range1,
		const CRangeBase* range2)
	{
		auto ws1 = range1->GetWorksheet();
		auto ws2 = range2->GetWorksheet();

		//ranges must be on the same worksheet
		if (ws1 != ws2)
			return nullptr;


		auto TL1 = range1->topleft();
		auto TL2 = range2->topleft();

		auto BR1 = range1->bottomright();
		auto BR2 = range2->bottomright();


		int TL_Col = TL1.GetCol() < TL2.GetCol() ? TL1.GetCol() : TL2.GetCol();
		int TL_Row = TL1.GetRow() < TL2.GetRow() ? TL1.GetRow() : TL2.GetRow();

		int BR_Col = BR1.GetCol() > BR2.GetCol() ? BR1.GetCol() : BR2.GetCol();
		int BR_Row = BR1.GetRow() > BR2.GetRow() ? BR1.GetRow() : BR2.GetRow();


		wxGridCellCoords TL(TL_Row, TL_Col);
		wxGridCellCoords BR(BR_Row, BR_Col);


		return new CRangeBase(ws1, TL, BR);
	}

	wxString CRangeBase::CoordsToStr(const wxGridCellCoords& TL, const wxGridCellCoords& BR)
	{
		//Indexing starts from (0,0), Report to user starting from 1
		int tlRow = TL.GetRow() + 1, tlCol = TL.GetCol() + 1;
		int brRow = BR.GetRow() + 1, brCol = BR.GetCol() + 1;

		wxString Str;
		Str << ColNumtoLetters(tlCol) << tlRow << ":"
			<< ColNumtoLetters(brCol) << brRow;

		return Str;
	}





	CRangeBase::CRangeBase(grid::CWorksheetBase* ws, const wxGridCellCoords& TL, const wxGridCellCoords& BR)
	{
		m_WSheet = ws;
		m_WBook = nullptr;

		m_TL = TL;
		m_BR = BR;

		m_Str = m_WSheet->GetWSName() + "!" + CoordsToStr(m_TL, m_BR);
	}


	CRangeBase::CRangeBase(const wxString& str, CWorkbookBase* wb)
	{
		m_Str = str;
		m_WBook = wb;

		size_t row = 0, col = 0;

		int ExclamationPos = str.Find('!');
		if (ExclamationPos < 0)
			throw std::exception("Invalid format: Exclamation (!) character is missing.");

		//cant be the first character
		if (ExclamationPos == 0)
			throw std::exception("Invalid format: Position of exclamation (!) character is not valid.");

		wxString WorksheetName = str.SubString(0, (size_t)ExclamationPos - 1);

		m_WSheet = m_WBook->GetWorksheet(WorksheetName.ToStdWstring());
		if (m_WSheet == 0)
		{
			wxString Err = WorksheetName + " does not exist!";
			throw std::exception((const char*)Err.mb_str(wxConvUTF8));
		}

		int ColumnPos = str.Find(':', true);
		if (ColumnPos < 0)
			throw std::exception("Column character is missing.");

		if (ColumnPos <= ExclamationPos)
			throw std::exception("Column character not in valid position.");


		wxString strTL = str.SubString((size_t)ExclamationPos + 1, (size_t)ColumnPos - 1).Trim().Trim(false);
		wxString strBR = str.SubString((size_t)ColumnPos + 1, str.Length()).Trim().Trim(false);

		try {

			auto LetterNumber_TL = ParseGridCoordinates(strTL);
			row = wxAtoi(LetterNumber_TL.second);
			col = LetterstoColumnNumber(LetterNumber_TL.first.ToStdString());
		}
		catch (std::exception&) {
			throw std::exception("Error in parsing top-left coordinates");
		}

		if (row > m_WSheet->GetNumberRows())
			throw std::exception("Top left row number greater than number of rows in the worksheet");

		if (col > m_WSheet->GetNumberCols())
			throw std::exception("Top left column number greater than number of cols in the worksheet ");

		m_TL.SetCol(col - 1);
		m_TL.SetRow(row - 1);

		try {
			auto LetterNumber_BR = ParseGridCoordinates(strBR);
			row = wxAtoi(LetterNumber_BR.second);
			col = LetterstoColumnNumber(LetterNumber_BR.first.ToStdString());
		}
		catch (std::exception&)
		{
			throw std::exception("Error in parsing bottom-right coordinates");
		}

		if (row > m_WSheet->GetNumberRows())
			throw std::exception("Bottom right row number greater than number of worksheets");

		if (col > m_WSheet->GetNumberCols())
			throw std::exception("Bottom right column number greater than number of cols in the worksheet ");

		m_BR.SetCol(col - 1);
		m_BR.SetRow(row - 1); //First row=0

		int NRows = m_BR.GetRow() - m_TL.GetRow() + 1;
		int NCols = m_BR.GetCol() - m_TL.GetCol() + 1;

		if (NRows > m_WSheet->GetNumberRows())
			throw std::exception("Number of rows by coordinates exceeds number of actual rows.");

		if (NCols > m_WSheet->GetNumberCols())
			throw std::exception("Number of cols by coordinates exceeds number of actual columns.");
	}


	CRangeBase::~CRangeBase()
	{
		m_WSheet = nullptr;
		m_WBook = nullptr;
	}


	CRangeBase::CRangeBase(const CRangeBase& rhs)
	{
		m_WSheet = rhs.m_WSheet;
		m_WBook = rhs.m_WBook;

		m_TL = rhs.m_TL;
		m_BR = rhs.m_BR;

		m_Str = rhs.m_Str;
	}


	CRangeBase& CRangeBase::operator=(const CRangeBase& rhs)
	{
		m_WBook = rhs.m_WBook;
		m_WSheet = rhs.m_WSheet;

		m_TL = rhs.m_TL;
		m_BR = rhs.m_BR;

		m_Str = rhs.m_Str;

		return *this;
	}


	CRangeBase::CRangeBase(CRangeBase&& rhs) noexcept
	{
		m_WSheet = rhs.m_WSheet;
		rhs.m_WSheet = nullptr;

		m_WBook = rhs.m_WBook;
		rhs.m_WBook = nullptr;

		m_TL = std::move(rhs.m_TL);
		m_BR = std::move(rhs.m_BR);

		m_Str = std::move(rhs.m_Str);
	}


	CRangeBase& CRangeBase::operator=(CRangeBase&& rhs) noexcept
	{
		m_WSheet = rhs.m_WSheet;
		rhs.m_WSheet = nullptr;

		m_WBook = rhs.m_WBook;
		rhs.m_WBook = nullptr;

		m_TL = std::move(rhs.m_TL);
		m_BR = std::move(rhs.m_BR);

		m_Str = std::move(rhs.m_Str);

		return *this;
	}



	CRangeBase* CRangeBase::col(int col) const
	{
		if (col >= ncols())
			throw std::exception("Request is out of range boundaries.");

		wxGridCellCoords TL = topleft();
		TL.SetCol(TL.GetCol() + col);

		wxGridCellCoords BR = bottomright();
		BR.SetCol(topleft().GetCol() + col);

		return new CRangeBase(m_WSheet, TL, BR);
	}


	void CRangeBase::set(int row, int col, const wxString& value)
	{
		//row and column are relative to topleft position of selection
		int posRow = topleft().GetRow() + row;
		int posCol = topleft().GetCol() + col;

		if (posRow<topleft().GetRow() || posRow>bottomright().GetRow())
			throw std::exception("Requested row is out of boundaries of the range");

		if (posCol<topleft().GetCol() || posCol>bottomright().GetCol())
			throw std::exception("Requested column is out of boundaries of the range");

		m_WSheet->SetCellValue(posRow, posCol, value);
	}


	bool CRangeBase::Contains(const wxGridCellCoords& Coord) const
	{
		bool RowInRange = false, ColInRange = false;

		auto TL = topleft();
		auto BR = bottomright();

		if (Coord.GetRow() <= BR.GetRow() &&
			Coord.GetRow() >= TL.GetRow())
			RowInRange = true;

		if (Coord.GetCol() <= BR.GetCol() &&
			Coord.GetCol() >= TL.GetCol())
			ColInRange = true;

		if (RowInRange && ColInRange)
			return true;

		return false;
	}



	std::list<CRangeBase*> CRangeBase::split() const
	{
		std::list<CRangeBase*> retList;

		size_t NCols = ncols();

		for (size_t i = 0; i < NCols; ++i)
		{
			CRangeBase* rng = GetSubRange(wxGridCellCoords(0, i), (int)SELECT::ALLROWS, 1);
			retList.push_back(rng);
		}

		return retList;
	}



	CRangeBase* CRangeBase::GetSubRange(const wxGridCellCoords& TopLeft, int NRows, int NCols) const
	{
		if (NRows == 0 || NCols == 0)
			throw std::exception("nrows or ncols cannot be 0");

		int NumberOfRows = NRows, NumberOfCols = NCols;


		/*
			TopLeft: Relative to the range
			WS_TL is the coordinate relative to the worksheet
		*/
		wxGridCellCoords WS_TL = TopLeft;

		WS_TL.SetCol(TopLeft.GetCol() + topleft().GetCol());
		WS_TL.SetRow(TopLeft.GetRow() + topleft().GetRow());


		if (!Contains(WS_TL))
			throw std::exception("coordinates defined by row, col is not within the range");


		bool SelAllRows = NRows == (int)SELECT::ALLROWS || NRows < 0;
		bool SelAllCols = NCols == (int)SELECT::ALLCOLS || NCols < 0;

		if (SelAllRows)
			NumberOfRows = nrows() - TopLeft.GetRow();

		if (SelAllCols)
			NumberOfCols = ncols() - TopLeft.GetCol();

		wxGridCellCoords TL = WS_TL;
		wxGridCellCoords BR = TL;

		BR.SetCol(TL.GetCol() + NumberOfCols - 1);
		BR.SetRow(TL.GetRow() + NumberOfRows - 1);

		//BR might exceed
		if (!Contains(BR))
			throw std::exception("Boundaries of the current range exceeded");

		return new CRangeBase(m_WSheet, TL, BR);
	}


	void CRangeBase::clear() const
	{
		m_WSheet->ClearBlockContent(m_TL, m_BR);
		m_WSheet->ClearBlockFormat(m_TL, m_BR);
		m_WSheet->RefreshBlock(m_TL, m_BR);
	}


	wxString CRangeBase::get(
		int row,
		int col) const
	{
		//row and column are relative to topleft position of selection
		int posRow = topleft().GetRow() + row;
		int posCol = topleft().GetCol() + col;

		if (posRow<topleft().GetRow() ||
			posRow>bottomright().GetRow())
			throw std::exception("Row is out of range.");

		if (posCol<topleft().GetCol() ||
			posCol>bottomright().GetCol())
			throw std::exception("Column is out of range.");


		return m_WSheet->GetCellValue(posRow, posCol).Trim().Trim(false);
	}


	wxString CRangeBase::get(int pos) const
	{
		assert(pos >= 0);
		assert(pos < nrows() * ncols());

		int row = pos / ncols();
		int col = pos - row * ncols();

		return get(row, col);
	}





	void CRangeBase::replace(
		const wxString& Value,
		const wxString& newValue)
	{
		int NRows = nrows();
		int NCols = ncols();

		int k = 0;
		for (int i = 0; i < NRows; ++i)
			for (int j = 0; j < NCols; ++j)
			{
				if (get(i, j) == Value)
					set(i, j, newValue);
			}
	}


	//AB15 to AB and 15
	std::pair<wxString, wxString> 
		CRangeBase::ParseGridCoordinates(const wxString& coordinates)
	{
		wxString Number, Letter;
		for (const auto& c : coordinates)
		{
			if (c == ' ') continue;

			if (isdigit(c))
				Number += c;

			else if (isalpha(c))
				Letter += c;
			else
				throw std::exception(("Unexpected character found in " + coordinates).c_str());
		}

		if (Letter.empty())
			throw std::exception("Alphabetic characters are missing ");

		if (Number.empty())
			throw std::exception("Numeric characters are missing");

		return { Letter, Number };
	}



	

}