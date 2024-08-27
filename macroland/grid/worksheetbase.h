#pragma once

#include <vector>
#include <list>
#include <map>
#include <set>
#include <filesystem>

#include <wx/wx.h>
#include <wx/grid.h>
#include <wx/xml/xml.h>
#include <wx/textfile.h>
#include <wx/wfstream.h>
#include <wx/zipstrm.h>





//events by selection rectangle
wxDECLARE_EVENT(EVT_DATA_MOVING, wxMouseEvent);
wxDECLARE_EVENT(EVT_DATA_MOVED, wxMouseEvent);


namespace grid
{
	class Cell;
	class CWorkbookBase;
	class CSelRect;

	class CWorksheetBase :public wxGrid
	{
	protected:

		struct GridCoordComp {
			bool operator() (wxGridCellCoords p1, wxGridCellCoords p2) const {
				return p1.GetRow() < p2.GetRow() ||
					(p1.GetRow() == p2.GetRow() && p1.GetCol() < p2.GetCol());
			}
		};

		using GridSet = std::set<wxGridCellCoords, GridCoordComp>;

		struct Row {
			int m_Height;
			bool m_SysAdj = true; // adjusted by system
		};

	public:
		enum class PASTE { ALL = 0, FORMAT, VALUES };

	public:
		CWorksheetBase(
			wxWindow* parent,
			wxWindowID id,
			const wxPoint& pos,
			const wxSize& size,
			long style, wxString WindowName,
			int nrows = 1,
			int ncols = 1);

		CWorksheetBase(
			wxWindow* parent,
			CWorkbookBase* workbook,
			wxWindowID id,
			const wxPoint& pos,
			const wxSize& size,
			long style, wxString WindowName,
			int nrows = 1,
			int ncols = 1);

		virtual ~CWorksheetBase();

		//Is not owned by a workbook (implemented for compability)
		CWorkbookBase* GetWorkbook() const
		{
			return m_WBase;
		}

		bool ReadXMLDoc(wxZipInputStream& InStream, wxZipEntry* Entry);

		//Read from snapshot directory, WorksheetFullPath is in snapshot directory 
		bool ReadXMLDoc(const std::filesystem::path& WSPath);

		// Return the TL and BR coordinates where the data is pasted
		std::pair<wxGridCellCoords, wxGridCellCoords> Paste_XMLDataFormat(PASTE paste = PASTE::ALL);

		//Return the TL and BR coordinates where the data is pasted
		std::pair<wxGridCellCoords, wxGridCellCoords> Paste_TextValues();


		void Cut();
		void Delete();
		void Paste();


		bool IsDirty() const {
			return m_IsDirty;
		}


		auto GetChangedCells_Format() const {
			return  m_Format;
		}

		auto GetChangedCells_Content() const {
			return m_Content;
		}

		Cell GetAsCellObject(const wxGridCellCoords& coord) const;

		Cell GetAsCellObject(int row, int column) const;

		void SetCellValue(
			int row,
			int col,
			const wxString& value,
			bool MakeDirty = true);


		void SetValue(
			int row,
			int col,
			const wxString& value,
			bool MakeDirty = true);


		void SetCellFont(
			int row,
			int col,
			const wxFont& font);


		void SetCellAlignment(
			int row,
			int col,
			int horiz,
			int vertical);


		void SetCellBackgroundColour(
			int row,
			int col,
			const wxColour& color);


		void SetCellTextColour(
			int row,
			int col,
			const wxColour& color);


		void SetRowSize(
			int row,
			int height);


		void SetCleanRowSize(
			int row,
			int height);


		bool DeleteRows(
			int pos = 0,
			int numRows = 1,
			bool updateLabels = true);


		bool InsertRows(
			int pos = 0,
			int numRows = 1,
			bool updateLabels = true);


		bool DeleteCols(
			int pos = 0,
			int numCols = 1,
			bool updateLabels = true);


		bool InsertCols(
			int pos = 0,
			int numCols = 1,
			bool updateLabels = true);


		void MarkDirty();
		void MarkClean();


		//Returns a union of Changed Format and Changed Content
		GridSet GetChangedCells() const;

		wxGridCellCoords GetSelTopLeft() const;
		wxGridCellCoords GetSelBtmRight() const;

		size_t GetNumSelCols() const;
		size_t GetNumSelRows() const;


		bool ClearBlockContent(
			const wxGridCellCoords& TL,
			const wxGridCellCoords& BR);

		bool ClearBlockFormat(
			const wxGridCellCoords& TL,
			const wxGridCellCoords& BR);


		void SetCellFormattoDefault(
			int row,
			int column);


		void SetCelltoDefault(
			int row,
			int column);


		//Marks the worksheet as dirty
		void ApplyCellFormat(
			int row,
			int column,
			const Cell& cell,
			bool MakeDirty = true);


		void SetBlockBackgroundColor(
			const wxGridCellCoords TL,
			const wxGridCellCoords BR,
			const wxColour& color);


		void SetBlockTextColour(
			const wxGridCellCoords TL,
			const wxGridCellCoords BR,
			const wxColour& color);


		void SetBlockFont(
			const wxGridCellCoords TL,
			const wxGridCellCoords BR,
			const wxFont& font);


		void SetBlockCellAlignment(
			const wxGridCellCoords& TL,
			const wxGridCellCoords& BR,
			int Alignment);

		void AdjustRowHeight(int row);

		std::vector<Cell> GetBlock(
			const wxGridCellCoords& TL,
			const wxGridCellCoords& BR) const;


		void DrawCellHighlight(wxDC& dc, const wxGridCellAttr* attr) override;


		std::map<int, Row> GetAdjustedRows() const
		{
			return m_AdjustedRows;
		}

		std::map<int, int> GetAdjustedCols() const
		{
			return m_AdjustedCols;
		}


		void Copy();

		std::wstring GetWSName() const {
			return m_WSName;
		}

		void SetWSName(const std::wstring& name) {
			m_WSName = name;
		}


	protected:
		void OnKeyDown(wxKeyEvent& event);
		void OnLabelRightClick(wxGridEvent& event);
		void OnDeleteRows(wxCommandEvent& event);
		void OnInsertRows(wxCommandEvent& event);
		void OnDeleteColumns(wxCommandEvent& event);
		void OnInsertColumns(wxCommandEvent& event);

		void OnColSize(wxGridSizeEvent& event);
		void OnRowSize(wxGridSizeEvent& event);

		void OnCellDataChanged(wxGridEvent& event);

		void OnGridWindowLeftDown(wxMouseEvent& event);
		void GridEditorOnKeyDown(wxKeyEvent& event);

		void OnKillFocus(wxFocusEvent& event);
		void OnSetFocus(wxFocusEvent& event);

		void OnRangeSelecting(wxGridRangeSelectEvent& event);
		void OnRangeSelected(wxGridRangeSelectEvent& event);
		void OnSelectCell(wxGridEvent& event);

		void DrawColLabel(wxDC& dc, int col) override;
		void DrawColLabels(wxDC& dc, const wxArrayInt& cols) override;
		void DrawRowLabel(wxDC& dc, int Row) override;
		void DrawRowLabels(wxDC& dc, const wxArrayInt& Rows) override;

	private:
		//helper func for DrawColLabel and DrawColLabels
		void DrawFormattedColLabel(wxDC& dc, int col);

		//Helper func for DrawRowLabel and DrawRowLabels
		void DrawFormattedRowLabel(wxDC& dc, int Row);

		bool SelectionContainsColumn(int Col);
		bool SelectionContainsRow(int Row);


	protected:
		bool m_IsDirty = false;

		//Variable to track changed content and format
		GridSet m_Format, m_Content;

		//Rows adjusted automatically or by user --> int is row number 
		//TRUE if adjusted by user and FALSE if adjusted by automatically
		std::map<int, Row> m_AdjustedRows; //<row number, height>
		std::map<int, int> m_AdjustedCols; //<col number, width>

	private:
		const int ID_DELCOL{ wxNewId() };
		const int ID_INSERTCOL{ wxNewId() };
		const int ID_DELROW{ wxNewId() };
		const int ID_INSERTROW{ wxNewId() };

		std::wstring m_WSName{};

		//Is grid data ctrl active and will make (or making) selection
		bool m_IsGridDataCtrlSelection{ false };

		/*
			When the focus is lost the gridcell cursor disappears.
			This is incovenient as user does not know which cell to return back to.
			The following keeps the coordinates of the cell when focus is lost.
		*/
		wxGridCellCoords m_FocusIndicCell;

		CWorkbookBase* m_WBase;

		CSelRect* m_RectData;

		//Selected Rectangle for GetGridColLabelWindow()
		wxRect m_ColWndRect;
		
		//Selected Rectangle for GetGridRowLabelWindow()
		wxRect m_RowWndRect;

		//Current coordinates of GridCell cursor
		wxGridCellCoords m_CurCoords{ 0, 0 };
	};



	/**************************************************************** */


	class CSelRect : public wxEvtHandler
	{
	protected:
		virtual ~CSelRect();

	public:
		CSelRect(CWorksheetBase* ws,
			const wxPoint& start,
			const wxPoint& end,
			const wxPen& pen,
			const wxBrush& brush);

		void SetCoords(const wxGridCellCoords& tl, const wxGridCellCoords& br)
		{
			m_TL = tl;
			m_BR = br;
		}

		void SetInitCoords(const wxGridCellCoords& tl, const wxGridCellCoords& br)
		{
			m_InitTL = tl;
			m_InitBR = br;
		}

		void SetLButtonCoords(int row, int col)
		{
			m_LBtnDown.SetRow(row);
			m_LBtnDown.SetCol(col);
		}

		wxRect GetInflatedBoundingRect() const;

		auto GetBoundRect() const
		{
			return m_BoundRect;
		}

		void SetStart(const wxPoint& pt)
		{
			m_Start = pt;
			m_BoundRect = wxRect(m_Start, m_End);
		}

		void SetEnd(const wxPoint& pt)
		{
			m_End = pt;
			m_BoundRect = wxRect(m_Start, m_End);
		}

		bool CopyMoveBlock(
			const wxGridCellCoordsArray& from,
			const wxGridCellCoordsArray& to,
			bool IsMoving = false);

		void Draw(wxDC* dc);


	protected:
		void OnKeyDown(wxKeyEvent& event);
		void OnLeftUp(wxMouseEvent& event);
		void OnLeftDown(wxMouseEvent& event);

		void OnMouseMove(wxMouseEvent& event);

		void OnDataMoving(wxMouseEvent& event);
		void OnDataMoved(wxMouseEvent& event);

	protected:
		wxGridCellCoords m_TL, m_BR;
		wxGridCellCoords m_InitTL, m_InitBR;
		wxGridCellCoords m_LBtnDown;

	private:
		CWorksheetBase* m_WS{ nullptr };

		wxRect m_BoundRect;
		wxPoint m_Start, m_End;
		wxPen m_Pen;
		wxBrush m_Brush;
	};

}