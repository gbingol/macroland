#pragma once

#include <wx/wx.h>
#include <wx/grid.h>





namespace grid
{
	class CWorksheetBase;
	class CWorkbookBase;

	class CRangeBase
	{
	public:
		enum class SELECT { ALLROWS = -1, ALLCOLS = -2 };

	public:
		CRangeBase() = default;
		
		CRangeBase(
			CWorksheetBase* ws, 
			const wxGridCellCoords& TL, 
			const wxGridCellCoords& BR);
		
		CRangeBase(
			const wxString& str, 
			CWorkbookBase* wb); //there is a selection text
		
		CRangeBase(const CRangeBase& rhs);
		CRangeBase& operator=(const CRangeBase& rhs);

		CRangeBase(CRangeBase&& rhs) noexcept;
		CRangeBase& operator=(CRangeBase&& rhs) noexcept;

		virtual ~CRangeBase();


		/*
			Split the range (made up of N cols) into individual column ranges
			If range contains only a single col, returns the range itself
		*/
		std::list<CRangeBase*> split() const;

		CRangeBase* col(int col) const;


		/*
			Topleft coordinate(starting from(0, 0)) must be relative to the range itself
			and must be within the boundaries of the range
		*/
		CRangeBase* GetSubRange(
			const wxGridCellCoords& TopLeft,
			int NRows = (int)SELECT::ALLROWS,
			int NCols = (int)SELECT::ALLCOLS) const;


		//replace all occurences of Value with new Value
		void replace(
			const wxString& Value,
			const wxString& newValue);


		wxGridCellCoords topleft() const {
			return m_TL;
		}

		wxGridCellCoords bottomright() const {
			return m_BR;
		}

		size_t nrows() const {
			return (size_t)bottomright().GetRow() - (size_t)topleft().GetRow() + 1;
		}


		size_t ncols() const {
			return (size_t)bottomright().GetCol() - topleft().GetCol() + 1;
		}

		virtual CWorksheetBase* GetWorksheet() const {
			return m_WSheet;
		}

		wxString toString() const {
			return m_Str;
		}

		//Coord is relative to the worksheet
		bool Contains(const wxGridCellCoords& Coord) const;

		void set(int row, int col, const wxString& value);

		wxString get(int row, int col) const;
		wxString get(int pos) const;

		void clear() const;


	protected:
		//AB15 to AB and 15
		std::pair<wxString, wxString>
			ParseGridCoordinates(const wxString& coordinates);

	protected:
		CWorksheetBase* m_WSheet{ nullptr };
		CWorkbookBase* m_WBook{ nullptr };
		wxGridCellCoords m_TL, m_BR;
		wxString m_Str;
	};
}
