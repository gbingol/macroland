#pragma once

#include <wx/wx.h>
#include <wx/grid.h>

#include "dllimpexp.h"


namespace core
{
	class CArray;
}

namespace grid
{
	class CWorksheetBase;
	class CWorkbookBase;

	class CRangeBase
	{
	public:
		enum class SELECT { ALLROWS = -1, ALLCOLS = -2 };

		//merging two ranges from very top-left and very bottom-right
		static DLLGRID CRangeBase* Merge(const CRangeBase* range1, const CRangeBase* range2);

		//returns as A1:B2
		static DLLGRID wxString CoordsToStr(const wxGridCellCoords& TL, const wxGridCellCoords& BR);

	public:
		DLLGRID CRangeBase() = default;
		
		DLLGRID CRangeBase(
			CWorksheetBase* ws, 
			const wxGridCellCoords& TL, 
			const wxGridCellCoords& BR);
		
		DLLGRID CRangeBase(
			const wxString& str, 
			CWorkbookBase* wb); //there is a selection text
		
		DLLGRID CRangeBase(const CRangeBase& rhs);
		DLLGRID CRangeBase& operator=(const CRangeBase& rhs);

		DLLGRID CRangeBase(CRangeBase&& rhs) noexcept;
		DLLGRID CRangeBase& operator=(CRangeBase&& rhs) noexcept;

		virtual DLLGRID ~CRangeBase();


		/*
			Split the range (made up of N cols) into individual column ranges
			If range contains only a single col, returns the range itself
		*/
		DLLGRID std::list<CRangeBase*> split() const;

		DLLGRID CRangeBase* col(int col) const;


		/*
			Topleft coordinate(starting from(0, 0)) must be relative to the range itself
			and must be within the boundaries of the range
		*/
		DLLGRID CRangeBase* GetSubRange(
			const wxGridCellCoords& TopLeft,
			int NRows = (int)SELECT::ALLROWS,
			int NCols = (int)SELECT::ALLCOLS) const;


		//replace all occurences of Value with new Value
		DLLGRID void replace(
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
		DLLGRID bool Contains(const wxGridCellCoords& Coord) const;

		DLLGRID void set(int row, int col, const wxString& value);

		DLLGRID wxString get(int row, int col) const;
		DLLGRID wxString get(int pos) const;

		DLLGRID void clear() const;


	protected:
		//AB15 to AB and 15
		DLLGRID std::pair<wxString, wxString>
			ParseGridCoordinates(const wxString& coordinates);

	protected:
		CWorksheetBase* m_WSheet{ nullptr };
		CWorkbookBase* m_WBook{ nullptr };
		wxGridCellCoords m_TL, m_BR;
		wxString m_Str;
	};



}
