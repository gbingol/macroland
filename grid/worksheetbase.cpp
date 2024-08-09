#include "worksheetbase.h"

#include <algorithm>
#include <set>
#include <wx/wx.h>
#include <wx/file.h>
#include <wx/sstream.h>
#include <wx/grid.h>
#include <wx/ribbon/control.h>
#include <wx/xml/xml.h>
#include <wx/tokenzr.h>
#include <wx/clipbrd.h>

#include "ws_cell.h"
#include "ws_funcs.h"
#include "undoredo.h"
#include "workbookbase.h"
#include "selrectbase.h"

#include "events.h"


namespace grid
{

	CWorksheetBase::CWorksheetBase(wxWindow* parent,
		wxWindowID id,
		const wxPoint& pos,
		const wxSize& size,
		long style,
		wxString WindowName,
		int nrows,
		int ncols) : wxGrid(parent, id, pos, size, style, WindowName)
	{

		CreateGrid(nrows, ncols);

		m_WSName = WindowName;
		m_IsDirty = false;

		const int FontSize{ 11 };
		wxFont defaultFont(wxFontInfo(FontSize).FaceName("Calibri"));
		SetDefaultCellFont(defaultFont);
		SetDefaultColSize(FromDIP(7 * FontSize));

		EnableEditing(false);
		EnableDragGridSize(false);
		SetDefaultCellOverflow(true);

		Bind(wxEVT_GRID_EDITOR_CREATED, [&](wxGridEditorCreatedEvent& event)
		{
			event.GetWindow()->Bind(wxEVT_KEY_DOWN, &CWorksheetBase::GridEditorOnKeyDown, this);
		});

		Bind(wxEVT_GRID_EDITOR_HIDDEN, [&](wxGridEvent& event)
		{
			//we need to use call after as we cant disable editing before hiding the shown grideditor 
			CallAfter([&]
				{
					EnableEditing(false);
				});
			event.Skip();
		});

		Bind(wxEVT_KEY_DOWN, &CWorksheetBase::OnKeyDown, this);

		Bind(wxEVT_GRID_CELL_CHANGED, &CWorksheetBase::OnCellDataChanged, this);

		Bind(wxEVT_GRID_SELECT_CELL, &CWorksheetBase::OnSelectCell, this);
		Bind(wxEVT_GRID_RANGE_SELECTING, &CWorksheetBase::OnRangeSelecting, this);
		Bind(wxEVT_GRID_RANGE_SELECTED, &CWorksheetBase::OnRangeSelected, this);

		Bind(wxEVT_GRID_LABEL_RIGHT_CLICK, &CWorksheetBase::OnLabelRightClick, this);
		Bind(wxEVT_GRID_COL_SIZE, &CWorksheetBase::OnColSize, this);
		Bind(wxEVT_GRID_ROW_SIZE, &CWorksheetBase::OnRowSize, this);

		GetGridWindow()->Bind(wxEVT_LEFT_DOWN, &CWorksheetBase::OnGridWindowLeftDown, this);
	}


	CWorksheetBase::CWorksheetBase(
		wxWindow* parent, 
		CWorkbookBase* workbook, 
		wxWindowID id, 
		const wxPoint& pos, 
		const wxSize& size, 
		long style, 
		wxString WindowName, 
		int nrows, 
		int ncols):CWorksheetBase(parent, id, pos, size, style, WindowName, nrows, ncols)
	{
		m_WBase = workbook;
	}


	CWorksheetBase::~CWorksheetBase() = default;


	void CWorksheetBase::OnRangeSelecting(wxGridRangeSelectEvent& event)
	{
		int col = GetSelTopLeft().GetCol();
		wxRect ColRect(GetColLeft(col), 0, GetNumSelCols() * GetColWidth(col), GetColLabelSize());

		int Row = GetSelTopLeft().GetRow();
		wxRect RowRect(0, GetRowTop(Row), GetRowLabelSize(), GetNumSelRows()*GetRowHeight(Row));

		GetGridColLabelWindow()->RefreshRect(ColRect + m_ColWndRect);
		GetGridRowLabelWindow()->RefreshRect(RowRect + m_RowWndRect);

		//save the selected rectangles
		m_ColWndRect = ColRect;
		m_RowWndRect = RowRect;

		event.Skip();
	}


	void CWorksheetBase::OnRangeSelected(wxGridRangeSelectEvent& event)
	{
		if (event.Selecting() == false)
			return;

		const wxGridCellCoordsArray& btl(GetSelectionBlockTopLeft());

		//force only 1 block is selected by user
		if (btl.size() > 1) 
		{
			wxMessageBox("Only one block/region can be selected.");
			ClearSelection();

			//make sure to return here as ClearSelection will cause ncols/nrows to have std::nullopt
			return;
		}

		size_t ncols = GetNumSelCols(), nrows = GetNumSelRows();

		if (ncols == 1 && nrows == 1)
			ClearSelection();

		if (ncols > 1 || nrows > 1)
		{
			int col = GetSelTopLeft().GetCol();
			wxRect ColRect(GetColLeft(col), 0, ncols * GetColWidth(col), GetColLabelSize());

			int Row = GetSelTopLeft().GetRow();
			wxRect RowRect(0, GetRowTop(Row), GetRowLabelSize(), nrows * GetRowHeight(Row));

			GetGridColLabelWindow()->RefreshRect(ColRect + m_ColWndRect);
			GetGridRowLabelWindow()->RefreshRect(RowRect + m_RowWndRect);

			//save the selected rectangles
			m_ColWndRect = ColRect;
			m_RowWndRect = RowRect;
		}

		event.Skip();
	}


	void CWorksheetBase::OnSelectCell(wxGridEvent& event)
	{	
		if (!m_ColWndRect.IsEmpty())
		{
			GetGridColLabelWindow()->RefreshRect(m_ColWndRect);
			GetGridColLabelWindow()->Update();
		}

		if (!m_RowWndRect.IsEmpty())
		{
			GetGridRowLabelWindow()->RefreshRect(m_RowWndRect);
			GetGridRowLabelWindow()->Update();
		}

		if (m_CurCoords != wxGridCellCoords())
		{
			wxRect ColRect(GetColLeft(m_CurCoords.GetCol()), 0, GetColWidth(m_CurCoords.GetCol()), GetColLabelSize());
			wxRect RowRect(0, GetRowTop(m_CurCoords.GetRow()), GetRowLabelSize(), GetRowHeight(m_CurCoords.GetRow()));

			GetGridColLabelWindow()->RefreshRect(ColRect);
			GetGridRowLabelWindow()->RefreshRect(RowRect);
		}

		int col = event.GetCol();
		int Row = event.GetRow();
		wxRect ColRect(GetColLeft(col), 0, GetColWidth(col), GetColLabelSize());
		wxRect RowRect(0, GetRowTop(Row), GetRowLabelSize(), GetRowHeight(Row));

		GetGridColLabelWindow()->RefreshRect(ColRect);
		GetGridRowLabelWindow()->RefreshRect(RowRect);

		//save the coordinates
		m_CurCoords = wxGridCellCoords(Row, col);

		event.Skip();
	}


	void CWorksheetBase::OnLabelRightClick(wxGridEvent& event)
	{
		int NCols = GetSelectedCols().GetCount();
		int NRows = GetSelectedRows().GetCount();

		wxMenu Menu;

		if (NCols >= 1)
		{
			Menu.Append(ID_INSERTCOL, "&Insert Column(s)");
			Menu.Bind(wxEVT_MENU, &CWorksheetBase::OnInsertColumns, this, ID_INSERTCOL);

			int NofColumnsNotSelected = GetNumberCols() - GetSelectedCols().GetCount();

			if (NofColumnsNotSelected >= 1) 
			{
				Menu.Append(ID_DELCOL, "&Delete Column(s)");
				Menu.Bind(wxEVT_MENU, &CWorksheetBase::OnDeleteColumns, this, ID_DELCOL);
			}
		}

		if (NRows >= 1)
		{
			Menu.Append(ID_INSERTROW, "&Insert Row(s)");
			Menu.Bind(wxEVT_MENU, &CWorksheetBase::OnInsertRows, this, ID_INSERTROW);

			int NofRowsNotSelected = GetNumberRows() - NRows;
			if (NofRowsNotSelected >= 1)
			{
				Menu.Append(ID_DELROW, "&Delete Row(s)");
				Menu.Bind(wxEVT_MENU, &CWorksheetBase::OnDeleteRows, this, ID_DELROW);
			}
		}

		PopupMenu(&Menu);

		event.Skip();
	}



	void CWorksheetBase::OnDeleteRows(wxCommandEvent& event)
	{
		int PosRowStart = GetSelectedRows()[0];
		int NRows = GetSelectedRows().size();

		auto evt = std::make_unique<RowsDeleted>(this);
		evt->SetInfo(PosRowStart, NRows);

		auto CheckSet = [&](const GridSet& aSet)->void
		{
			std::vector<Cell> InitCells;

			for (const auto& Coord : aSet)
			{
				if (Coord.GetRow() >= PosRowStart && Coord.GetRow() < PosRowStart + NRows)
					InitCells.push_back(GetAsCellObject(Coord));
			}

			evt->SetInitialCells(InitCells);
		};

		CheckSet(m_Content);
		CheckSet(m_Format);

		//Deletion 
		DeleteRows(PosRowStart, NRows, true);

		if(m_WBase)
			m_WBase->PushUndoEvent(std::move(evt));
	}



	void CWorksheetBase::OnInsertRows(wxCommandEvent& event)
	{
		int PosRowStart = GetSelectedRows()[0], NRows = GetSelectedRows().size();

		auto evt = std::make_unique<RowsInserted>(this);
		evt->SetInfo(PosRowStart, NRows);

		InsertRows(PosRowStart, NRows, true);

		if (m_WBase)
			m_WBase->PushUndoEvent(std::move(evt));
	}



	void CWorksheetBase::OnDeleteColumns(wxCommandEvent& event)
	{
		int PosColStart = GetSelectedCols()[0];
		int NCols = GetSelectedCols().size();

		auto evt = std::make_unique<ColumnsDeleted>(this);
		evt->SetInfo(PosColStart, NCols);

		auto CheckSet = [&](const GridSet& aSet)
		{
			std::vector<Cell> InitCells;

			for (const auto& Coord : aSet)
			{
				if (Coord.GetCol() >= PosColStart && Coord.GetCol() < PosColStart + NCols)
					InitCells.push_back(GetAsCellObject(Coord));
			}

			evt->SetInitialCells(InitCells);
		};

		CheckSet(m_Content);
		CheckSet(m_Format);

		//Deletion 
		DeleteCols(PosColStart, NCols, true);

		if (m_WBase)
			m_WBase->PushUndoEvent(std::move(evt));
	}



	void CWorksheetBase::OnInsertColumns(wxCommandEvent& event)
	{
		//start position of column
		int PosSt = GetSelectedCols()[0];
		int NCols = GetSelectedCols().size();

		auto evt = std::make_unique<ColumnsInserted>(this);
		evt->SetInfo(PosSt, NCols);

		InsertCols(PosSt, NCols, true);

		if (m_WBase)
			m_WBase->PushUndoEvent(std::move(evt));
	}


	void CWorksheetBase::OnColSize(wxGridSizeEvent& event)
	{
		int col = event.GetRowOrCol();

		bool HasAlreadyBeenSized = false;

		//Let's see if it is already adjusted by user
		if (m_AdjustedCols.find(col) != m_AdjustedCols.end())
			HasAlreadyBeenSized = true;

		auto changedEvt = std::make_unique<RowColSizeChanged>(this, RowColSizeChanged::ENTITY::COL, col);

		//If already been sized, the previous size should be in the m_AdjustedCols
		changedEvt->m_PrevSize = HasAlreadyBeenSized ? m_AdjustedCols[col] : GetDefaultColSize();

		//Register the final size
		m_AdjustedCols[col] = GetColSize(col);

		//Register the final size to undo event
		changedEvt->m_FinalSize = GetColSize(col);

		if (m_WBase)
			m_WBase->PushUndoEvent(std::move(changedEvt));

		//Size changed
		MarkDirty();


		event.Skip();
	}


	void CWorksheetBase::OnRowSize(wxGridSizeEvent& event)
	{
		int row = event.GetRowOrCol();

		int PrevSize = GetDefaultRowSize();
		if (auto Adjusted = m_AdjustedRows.contains(row))
			PrevSize = m_AdjustedRows[row].m_Height;

		auto changedEvt = std::make_unique<RowColSizeChanged>(this, RowColSizeChanged::ENTITY::ROW, row);
		changedEvt->m_PrevSize = PrevSize;

		//Register the final size
		m_AdjustedRows[row] = Row(GetRowHeight(row), false);

		//Register the final size to undo event
		changedEvt->m_FinalSize = GetRowHeight(row);

		if (m_WBase)
			m_WBase->PushUndoEvent(std::move(changedEvt));

		//Size changed
		MarkDirty();

		event.Skip();
	}



	void CWorksheetBase::OnCellDataChanged(wxGridEvent& event)
	{
		int row = event.GetRow(), col = event.GetCol();

		m_Content.insert(wxGridCellCoords{ row, col });

		MarkDirty();

		//Undo redo
		auto Changed = std::make_unique<CellDataChanged>(this, row, col);
		Changed->m_InitVal = event.GetString();
		Changed->m_LastVal = GetCellValue(row, col);

		if (m_WBase)
			m_WBase->PushUndoEvent(std::move(Changed));

		event.Skip();
	}



	void CWorksheetBase::OnGridWindowLeftDown(wxMouseEvent& event)
	{
		if (IsSelection() &&
			GetSelectedRows().Count() == 0 &&
			GetSelectedCols().Count() == 0)
		{
			auto TL = GetSelTopLeft();
			auto BR = GetSelBtmRight();

			wxRect selRect = BlockToDeviceRect(TL, BR);

			if (selRect.Contains(event.GetPosition()))
			{
				wxClientDC dc(GetGridWindow());

				m_RectData = new CSelRect(
					this,
					selRect.GetTopLeft(),
					selRect.GetBottomRight(),
					wxPen(wxColour(0, 0, 0), 2),
					wxBrush(wxNullColour, wxBRUSHSTYLE_TRANSPARENT));

				m_RectData->SetLButtonCoords(YToRow(event.GetY()), XToCol(event.GetX()));

				m_RectData->SetCoords(TL, BR);
				m_RectData->SetInitCoords(TL, BR);

				m_RectData->Draw(&dc);

				return;
			}
		}

		event.Skip();
	}


	void CWorksheetBase::Cut()
	{
		auto Coords = AddSelToClipbrd(this);
		wxGridCellCoords TL = Coords.first, BR = Coords.second;

		//Backup cells before clearing
		auto data_cut = std::make_unique<DataCut>(this);

		data_cut->SetCells(GetBlock(TL, BR));
		data_cut->SetCoords(TL, BR);

		if(m_WBase)
			m_WBase->PushUndoEvent(std::move(data_cut));

		//Clear content and format
		ClearBlockContent(TL, BR);
		ClearBlockFormat(TL, BR);

		MarkDirty();
	}


	void CWorksheetBase::Delete()
	{
		wxGridCellCoords TL, BR;

		if (!IsSelection())
		{
			TL.SetRow(GetGridCursorRow());
			TL.SetCol(GetGridCursorCol());
			BR = TL;
		}
		else
		{
			TL = GetSelTopLeft();
			BR = GetSelBtmRight();
		}


		auto evt = std::make_unique<CellContentDeleted>(this);
		evt->SetCoords(TL, BR);
		evt->SetInitialCells(GetBlock(TL, BR));

		//If there is actually anything deleted then we should be able to undo
		if (ClearBlockContent(TL, BR))
		{
			if (m_WBase)
				m_WBase->PushUndoEvent(std::move(evt));
		}

		EnableEditing(false);

		MarkDirty();
	}


	void CWorksheetBase::Paste()
	{
		if (!wxTheClipboard->Open())
			return;

		std::pair<wxGridCellCoords, wxGridCellCoords> Coords;

		if (wxTheClipboard->IsSupported(XMLDataFormat()))
			Coords = Paste_XMLDataFormat(PASTE::ALL);

		else if (wxTheClipboard->IsSupported(wxDF_TEXT))
			Coords = Paste_TextValues();

		wxTheClipboard->Close();


		//Do we have valid coordinates
		assert(
			Coords.first.GetCol() >= 0 &&
			Coords.first.GetRow() >= 0 &&
			Coords.second.GetCol() >= 0 &&
			Coords.second.GetRow() >= 0);

		if (Coords.first.GetCol() < 0 || Coords.first.GetRow() < 0)
			return;

		MarkDirty();

		auto dp_evt = std::make_unique<DataPasted>(this);
		dp_evt->SetPaste((int)PASTE::ALL);
		dp_evt->SetCoords(Coords);

		if(m_WBase)
			m_WBase->PushUndoEvent(std::move(dp_evt));
	}


	void CWorksheetBase::OnKeyDown(wxKeyEvent& event)
	{
		int KC = event.GetKeyCode();

		if (KC == WXK_DELETE)
		{
			Delete();
			return; //if not, the cell goes into edit mode
		}

		if (event.GetModifiers() == wxMOD_CONTROL)
		{
			wxChar Key = event.GetUnicodeKey();

			if (Key == 'C') Copy();
			else if (Key == 'X') Cut();
			else if (Key == 'V') Paste();
		}

		EnableEditing(true);  //enable editing so that user can type

		event.Skip();
	}



	Cell CWorksheetBase::GetAsCellObject(const wxGridCellCoords& coord) const
	{
		return GetAsCellObject(coord.GetRow(), coord.GetCol());
	}


	void CWorksheetBase::SetCellValue(
		int row,
		int col,
		const wxString& value,
		bool MakeDirty)
	{
		wxGrid::SetCellValue(row, col, value);

		if (value.IsEmpty())
			m_Content.erase(wxGridCellCoords(row, col));
		else
			m_Content.insert(wxGridCellCoords(row, col));

		if (MakeDirty)
			MarkDirty();
	}


	void CWorksheetBase::SetValue(int row, int col, const wxString& value, bool MakeDirty)
	{
		auto Table = GetTable();
		Table->SetValue(row, col, value);

		if (value.IsEmpty())
			m_Content.erase(wxGridCellCoords(row, col));
		else
			m_Content.insert(wxGridCellCoords(row, col));

		if (MakeDirty)
			MarkDirty();
	}


	void CWorksheetBase::SetCellFont(int row, int col, const wxFont& font)
	{
		wxGrid::SetCellFont(row, col, font);

		m_Format.insert(wxGridCellCoords(row, col));
		AdjustRowHeight(row);
		MarkDirty();
	}


	void CWorksheetBase::SetCellAlignment(int row, int col, int horiz, int vertical)
	{
		wxGrid::SetCellAlignment(row, col, horiz, vertical);

		m_Format.insert(wxGridCellCoords{ row, col });
		MarkDirty();
	}


	void CWorksheetBase::SetCellBackgroundColour(int row, int col, const wxColour& color)
	{
		wxGrid::SetCellBackgroundColour(row, col, color);
		
		m_Format.insert(wxGridCellCoords{ row, col });
		MarkDirty();
	}


	void CWorksheetBase::SetCellTextColour(int row, int col, const wxColour& color)
	{
		wxGrid::SetCellTextColour(row, col, color);

		m_Format.insert(wxGridCellCoords{ row, col });
		MarkDirty();
	}


	void CWorksheetBase::SetRowSize(
		int row,
		int height)
	{
		SetCleanRowSize(row, height);
		MarkDirty();
	}


	void CWorksheetBase::SetCleanRowSize(int row, int height)
	{
		m_AdjustedRows[row] = Row(height);
		wxGrid::SetRowSize(row, height);
	}


	CWorksheetBase::GridSet CWorksheetBase::GetChangedCells() const
	{
		//TODO: Check if union works better here (union of m_Content and m_Format)
		auto ChangedCells = m_Content;
		for (const auto& Coord : m_Format)
			ChangedCells.insert(Coord);

		return ChangedCells;
	}


	wxGridCellCoords CWorksheetBase::GetSelTopLeft() const
	{
		if (IsSelection())
			return GetSelectionBlockTopLeft()[0];

		return wxGridCellCoords();
	}


	wxGridCellCoords CWorksheetBase::GetSelBtmRight() const
	{
		if (IsSelection())
			return GetSelectionBlockBottomRight()[0];

		return wxGridCellCoords();
	}


	size_t CWorksheetBase::GetNumSelCols() const
	{
		if (IsSelection())
			return GetSelBtmRight().GetCol() - GetSelTopLeft().GetCol() + 1;

		return 0;
	}


	size_t CWorksheetBase::GetNumSelRows() const
	{
		if (IsSelection())
			return GetSelBtmRight().GetRow() - GetSelTopLeft().GetRow() + 1;

		return 0;
	}


	bool CWorksheetBase::ClearBlockFormat(const wxGridCellCoords& TL, const wxGridCellCoords& BR)
	{
		for (int row = TL.GetRow(); row <= BR.GetRow(); ++row)
			for (int col = TL.GetCol(); col <= BR.GetCol(); ++col)
			{
				SetCellFormattoDefault(row, col);
				m_Format.erase({ row, col });
				AdjustRowHeight(row);
			}

		MarkDirty();

		return true;
	}


	bool CWorksheetBase::ClearBlockContent(const wxGridCellCoords& TL, const wxGridCellCoords& BR)
	{
		bool AnyDel = false; //anything deleted

		for (int row = TL.GetRow(); row <= BR.GetRow(); ++row)
		{
			bool AnyOnRowDel = false;
			for (int col = TL.GetCol(); col <= BR.GetCol(); ++col)
			{
				if (GetCellValue(row, col) != wxEmptyString) {
					AnyDel = true;
					AnyOnRowDel = true;
				}

				SetCellValue(row, col, wxEmptyString);

				if (AnyOnRowDel)
					AdjustRowHeight(row);
			}
		}

		MarkDirty();

		return AnyDel;
	}


	void CWorksheetBase::ApplyCellFormat(int row, int column, const Cell& cell, bool MakeDirty)
	{
		auto Format = cell.GetFormat();

		wxGrid::SetCellFont(row, column, Format.GetFont());
		wxGrid::SetCellBackgroundColour(row, column, Format.GetBackgroundColor());
		wxGrid::SetCellTextColour(row, column, Format.GetTextColor());
		wxGrid::SetCellAlignment(row, column, Format.GetHAlign(), Format.GetVAlign());

		m_Format.insert({ row, column });

		if (MakeDirty)
			MarkDirty();
	}


	void CWorksheetBase::SetCellFormattoDefault(int row, int column)
	{
		int columnnumber = GetNumberCols();

		SetCellBackgroundColour(row, column, GetDefaultCellBackgroundColour());
		SetCellTextColour(row, column, GetDefaultCellTextColour());
		SetCellFont(row, column, GetDefaultCellFont());

		int horizontal = 0, vertical = 0;
		GetDefaultCellAlignment(&horizontal, &vertical);
		SetCellAlignment(row, column, horizontal, vertical);

		m_Format.erase({ row, column });

		MarkDirty();
	}


	void CWorksheetBase::SetCelltoDefault(int row, int column)
	{
		SetCellFormattoDefault(row, column);
		SetCellValue(row, column, wxEmptyString);

		m_Content.erase(wxGridCellCoords{ row, column });
		m_Format.erase(wxGridCellCoords{ row, column });

		MarkDirty();
	}


	void CWorksheetBase::AdjustRowHeight(int CurRow)
	{
		int Height = GetDefaultRowSize();

		if (m_AdjustedRows.contains(CurRow))
		{
			auto RowInfo = m_AdjustedRows[CurRow];

			//It is adjusted by user, take no action
			if (!RowInfo.m_SysAdj)
				return;
		}

		//Check the columns in the particular row (CurRow) with changed format
		for (const auto& Coord : m_Format)
		{
			//sets are inherently ordered and in this case it is ordered by row
			if (Coord.GetRow() > CurRow)
				break;

			if (Coord.GetRow() < CurRow)
				continue;

			//required height
			int ReqRow_H = FromDIP(GetCellFont(Coord.GetRow(), Coord.GetCol()).GetPixelSize().GetHeight());

			if (ReqRow_H > Height)
				Height = ReqRow_H;
		}

		//Here the MinimumRowHeight is the row height that will accomodate the font with the largest point size
		SetRowSize(CurRow, Height);

		m_AdjustedRows[CurRow] = Row(Height);
	}


	Cell CWorksheetBase::GetAsCellObject(int row, int column) const
	{
		Cell cell(this, row, column);
		cell.SetValue(GetCellValue(row, column).ToStdWstring());

		CellFormat format;

		int horiz = 0, vert = 0;
		GetCellAlignment(row, column, &horiz, &vert);
		format.SetAlignment(horiz, vert);

		format.SetBackgroundColor(GetCellBackgroundColour(row, column));
		format.SetFont(GetCellFont(row, column));
		format.SetTextColor(GetCellTextColour(row, column));

		cell.SetFormat(format);

		return cell;
	}


	std::vector<Cell> CWorksheetBase::GetBlock(const wxGridCellCoords& TL, const wxGridCellCoords& BR) const
	{
		std::vector<Cell> Cells;

		for (int i = TL.GetRow(); i <= BR.GetRow(); i++)
			for (int j = TL.GetCol(); j <= BR.GetCol(); j++)
				Cells.push_back(GetAsCellObject(i, j));

		return Cells;
	}


	void CWorksheetBase::SetBlockBackgroundColor(
		const wxGridCellCoords TL,
		const wxGridCellCoords BR,
		const wxColour& color)
	{

		for (int i = TL.GetRow(); i <= BR.GetRow(); i++)
			for (int j = TL.GetCol(); j <= BR.GetCol(); j++)
				SetCellBackgroundColour(i, j, color);

		RefreshBlock(TL, BR);

		MarkDirty();
	}


	void CWorksheetBase::SetBlockTextColour(
		const wxGridCellCoords TL,
		const wxGridCellCoords BR,
		const wxColour& color)
	{
		for (int i = TL.GetRow(); i <= BR.GetRow(); i++)
			for (int j = TL.GetCol(); j <= BR.GetCol(); j++)
				SetCellTextColour(i, j, color);

		RefreshBlock(TL, BR);

		MarkDirty();
	}


	void CWorksheetBase::SetBlockFont(
		const wxGridCellCoords TL,
		const wxGridCellCoords BR,
		const wxFont& font)
	{
		for (int i = TL.GetRow(); i <= BR.GetRow(); i++)
			for (int j = TL.GetCol(); j <= BR.GetCol(); j++)
				SetCellFont(i, j, font);


		RefreshBlock(TL, BR);

		MarkDirty();
	}


	void CWorksheetBase::SetBlockCellAlignment(
		const wxGridCellCoords& TL,
		const wxGridCellCoords& BR,
		int Alignment)
	{
		int Horiz{}, Vert{};

		for (int i = TL.GetRow(); i <= BR.GetRow(); i++)
			for (int j = TL.GetCol(); j <= BR.GetCol(); j++)
			{
				GetCellAlignment(i, j, &Horiz, &Vert);

				switch (Alignment)
				{
				case wxID_JUSTIFY_LEFT:
					Horiz = wxALIGN_LEFT;
					break;
				case wxID_JUSTIFY_CENTER:
					Horiz = wxALIGN_CENTRE;
					break;
				case wxID_JUSTIFY_RIGHT:
					Horiz = wxALIGN_RIGHT;
					break;
				case wxALIGN_BOTTOM:
					Vert = wxALIGN_BOTTOM;
					break;
				case wxALIGN_CENTRE:
					Vert = wxALIGN_CENTRE;
					break;
				case wxALIGN_TOP:
					Vert = wxALIGN_TOP;
					break;
				}

				SetCellAlignment(i, j, Horiz, Vert);
			}

		RefreshBlock(TL, BR);

		MarkDirty();
	}



	//overridden
	void CWorksheetBase::DrawColLabel(wxDC& dc, int col)
	{
		if (col == GetGridCursorCol())
			DrawFormattedColLabel(dc, col);
		else
			wxGrid::DrawColLabel(dc, col);
	}

	//overridden
	void CWorksheetBase::DrawColLabels(wxDC& dc, const wxArrayInt& cols)
	{
		for (size_t i = 0; i < cols.size(); ++i)
		{
			if (SelectionContainsColumn(cols[i]) || cols[i] == GetGridCursorCol())
				DrawFormattedColLabel(dc, cols[i]);
			else
				wxGrid::DrawColLabel(dc, cols[i]);
		}
	}


	//overridden
	void CWorksheetBase::DrawRowLabel(wxDC& dc, int Row)
	{
		if (Row == GetGridCursorRow())
			DrawFormattedRowLabel(dc, Row);
		else
			wxGrid::DrawRowLabel(dc, Row);
	}


	//overridden
	void CWorksheetBase::DrawRowLabels(wxDC& dc, const wxArrayInt& Rows)
	{
		for (size_t i = 0; i < Rows.size(); ++i)
		{
			if (SelectionContainsRow(Rows[i]) || Rows[i] == GetGridCursorRow())
				DrawFormattedRowLabel(dc, Rows[i]);
			else
				wxGrid::DrawRowLabel(dc, Rows[i]);
		}
	}




	void CWorksheetBase::DrawFormattedColLabel(wxDC& dc, int col)
	{
		wxPen InitialPen = dc.GetPen();
		wxBrush InitialBrush = dc.GetBrush();

		wxRect Rect(GetColLeft(col), 0, GetColWidth(col), GetColLabelSize());

		wxBrush brush(wxColour(224, 224, 224), wxBRUSHSTYLE_SOLID);

		dc.SetBrush(brush);
		dc.DrawRectangle(Rect);

		dc.SetTextForeground(wxColour(0, 102, 0));
		dc.DrawLabel(GetColLabelValue(col), Rect, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL);

		dc.SetPen(wxPen(wxColour(0, 255, 0), 3));
		dc.DrawLine(Rect.GetBottomLeft(), Rect.GetBottomRight());

		dc.SetPen(InitialPen);
		dc.SetBrush(InitialBrush);
	}



	void CWorksheetBase::DrawFormattedRowLabel(wxDC& dc, int Row)
	{
		wxPen InitialPen = dc.GetPen();
		wxBrush InitialBrush = dc.GetBrush();

		wxBrush brush(wxColour(224, 224, 224), wxBRUSHSTYLE_SOLID);
		dc.SetBrush(brush);

		wxRect Rect(0, GetRowTop(Row), GetRowLabelSize(), GetRowHeight(Row));
		dc.DrawRectangle(Rect);

		dc.SetTextForeground(wxColour(0, 102, 0));
		dc.DrawLabel(GetRowLabelValue(Row), Rect, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL);

		dc.SetPen(wxPen(wxColour(0, 255, 0), 3));
		dc.DrawLine(Rect.GetTopRight(), Rect.GetBottomRight());

		dc.SetPen(InitialPen);
		dc.SetBrush(InitialBrush);
	}


	bool CWorksheetBase::SelectionContainsColumn(int Col)
	{
		if (IsSelection() == false)
			return false;

		const wxGridCellCoords TL = GetSelTopLeft();
		const wxGridCellCoords BR = GetSelBtmRight();;

		if (Col <= BR.GetCol() && Col >= TL.GetCol())
			return true;

		return false;
	}


	bool CWorksheetBase::SelectionContainsRow(int Row)
	{
		if (IsSelection() == false)
			return false;

		const wxGridCellCoords TL = GetSelTopLeft();
		const wxGridCellCoords BR = GetSelBtmRight();

		if (Row <= BR.GetRow() && Row >= TL.GetRow())
			return true;

		return false;
	}


	void CWorksheetBase::Copy()
	{
		AddSelToClipbrd(this);
	}

	//does not handle starting key event
	void CWorksheetBase::GridEditorOnKeyDown(wxKeyEvent& event)
	{
		int KCode = event.GetKeyCode();

		int currow = GetGridCursorRow(), 
			curcol = GetGridCursorCol();

		auto TextCtrl = (wxTextCtrl*)event.GetEventObject();
		wxString CurText = TextCtrl->GetValue();

		long Pos = TextCtrl->GetInsertionPoint();

		bool IsTextSelected = TextCtrl->GetStringSelection().length() > 0 ? true : false;
		bool IsInBetween = (Pos == 1 || Pos < CurText.length()) && Pos != 0;


		if (IsInBetween ||
			IsTextSelected ||
			Pos == 0 && KCode == WXK_RIGHT ||
			Pos == CurText.length() && KCode == WXK_LEFT)
		{
			event.Skip();
			return;
		}

		
		if (KCode == WXK_RIGHT)
			SetCurrentCell(currow, curcol + 1);

		else if (KCode == WXK_LEFT && curcol != 0)
			SetCurrentCell(currow, curcol - 1);

		else if (KCode == WXK_UP && currow != 0)
			SetCurrentCell(currow - 1, curcol);

		event.Skip();
	}


	bool CWorksheetBase::DeleteRows(int pos, int numRows, bool updateLabels)
	{
		wxGrid::DeleteRows(pos, numRows, updateLabels);

		auto Update = [=](GridSet& Set)
		{
			std::erase_if(Set, [&](wxGridCellCoords Coords) 
			{
				return Coords.GetRow() >= pos && Coords.GetRow() < (pos + numRows);
			});


			for (auto& Coord : Set)
				if (Coord.GetRow() > pos) 
				{
					auto nh = Set.extract(Coord);

					assert(Coord.GetRow() - numRows >= 0);

					nh.value() = { Coord.GetRow() - numRows,Coord.GetCol() };
					Set.insert(std::move(nh));
				}
		};

		Update(m_Content);
		Update(m_Format);

		MarkDirty();

		return true;
	}


	bool CWorksheetBase::InsertRows(int pos, int numRows, bool updateLabels)
	{
		wxGrid::InsertRows(pos, numRows, updateLabels);

		auto UpdateSet = [=](GridSet& m_Set)
		{
			GridSet ASet;

			for (auto it = m_Set.begin(); it != m_Set.end();) 
			{
				wxGridCellCoords Coord = *it;

				if (Coord.GetRow() >= pos) {
					m_Set.erase(it++);

					ASet.insert({ Coord.GetRow() + numRows, Coord.GetCol() });
				}
				else
					++it;
			}

			m_Set.insert(ASet.begin(), ASet.end());
		};


		UpdateSet(m_Content);
		UpdateSet(m_Format);

		MarkDirty();

		return true;
	}


	bool CWorksheetBase::DeleteCols(int pos, int numCols, bool updateLabels)
	{
		wxGrid::DeleteCols(pos, numCols, updateLabels);

		auto Update = [=](GridSet& Set)
		{
			std::erase_if(Set, [&](wxGridCellCoords Coords) 
			{
				return Coords.GetCol() >= pos && Coords.GetCol() < (pos + numCols);
			});


			for (const auto& Coord : Set)
				if (Coord.GetCol() > pos)
				{
					auto nh = Set.extract(Coord);

					assert(Coord.GetCol() - numCols >= 0);

					nh.value() = { Coord.GetRow(), Coord.GetCol() - numCols };
					Set.insert(std::move(nh));
				}
		};


		Update(m_Content);
		Update(m_Format);

		MarkDirty();

		return true;
	}


	bool CWorksheetBase::InsertCols(int pos, int numCols, bool updateLabels)
	{
		wxGrid::InsertCols(pos, numCols, updateLabels);

		auto UpdateSet = [=](GridSet& m_Set)
		{
			GridSet ASet;

			for (auto it = m_Set.begin(); it != m_Set.end();) 
			{
				wxGridCellCoords Coord = *it;

				if (Coord.GetCol() >= pos)
				{
					m_Set.erase(it++);
					ASet.insert(wxGridCellCoords{ Coord.GetRow(), Coord.GetCol() + numCols });
				}
				else
					++it;
			}

			m_Set.insert(ASet.begin(), ASet.end());
		};

		UpdateSet(m_Content);
		UpdateSet(m_Format);

		MarkDirty();

		return true;
	}


	

	void CWorksheetBase::MarkDirty()
	{
		m_IsDirty = true;
		
		wxCommandEvent evt(ssEVT_WS_DIRTY, GetId());
		evt.SetEventObject(this);
		ProcessWindowEvent(evt);
	}


	void CWorksheetBase::MarkClean()
	{
		m_IsDirty = false;
	}


	

	void CWorksheetBase::OnKillFocus(wxFocusEvent& event)
	{
		auto wnd = event.GetWindow();

		if (wnd && (dynamic_cast<wxRibbonControl*>(wnd)))
		{
			SetFocus();
			return;
		}

		auto row = GetGridCursorRow();
		auto col = GetGridCursorCol();

		m_FocusIndicCell = wxGridCellCoords(row, col);

		wxClientDC dc(GetGridWindow());
		wxPen pen(wxColour(0, 0, 0), 2); 

		dc.SetPen(pen);
		dc.DrawRectangle(CellToRect(row, col));
	
		event.Skip();
	}

	void CWorksheetBase::OnSetFocus(wxFocusEvent& event)
	{
		if (m_FocusIndicCell != wxGridCellCoords())
		{
			RefreshBlock(m_FocusIndicCell, m_FocusIndicCell);
			m_FocusIndicCell = wxGridCellCoords();
		}
		
		event.Skip();
	}


	void CWorksheetBase::DrawCellHighlight(wxDC& dc, const wxGridCellAttr* attr)
	{
		/*int row = GetGridCursorRow(), col = GetGridCursorCol();

		wxRect rect = CellToRect(row, col);

		dc.SetPen(*wxRED_PEN);
		dc.DrawRectangle(rect);*/

		wxGrid::DrawCellHighlight(dc, attr);
	}


	bool CWorksheetBase::ReadXMLDoc(
		wxZipInputStream& ZipInStream,
		wxZipEntry* ZipEntry)
	{
		assert(ZipEntry != nullptr);

		ZipInStream.OpenEntry(*ZipEntry);

		auto szZipEntry = ZipEntry->GetSize();
		assert(szZipEntry > 0);

		auto XMLData = std::unique_ptr<char[]>(new char[szZipEntry]);
		ZipInStream.Read((void*)XMLData.get(), szZipEntry);


		wxString XMLContent = wxString::FromUTF8(XMLData.get(), szZipEntry);
		if (XMLContent.IsEmpty())
			return false;


		//We will create and XMLDocument from the contents so that we can do XML Parsing
		wxStringInputStream iss(XMLContent);
		if (iss.IsOk() == false)
			return false;

		wxXmlDocument xmlDoc(iss);

		return ParseXMLDoc(this, xmlDoc);
	}



	bool CWorksheetBase::ReadXMLDoc(const std::filesystem::path& WSPath)
	{
		wxFile file;
		if (file.Open(WSPath.wstring()) == false)
			return false;

		wxString XML;
		if (file.ReadAll(&XML) == false)
			return false;

		//We will create and XMLDocument from the contents so that we can do XML Parsing
		wxStringInputStream iss(XML);
		wxXmlDocument xmlDoc(iss);

		return ParseXMLDoc(this, xmlDoc);
	}


	std::pair<wxGridCellCoords, wxGridCellCoords> CWorksheetBase::Paste_XMLDataFormat(PASTE PasteWhat)
	{
		//This is where the user currently placed the cursor on Worksheet and pasting the data as of
		int RowPos = GetGridCursorRow();
		int ColPos = GetGridCursorCol();

		wxString XMLStr = GetXMLData();
		assert(!XMLStr.IsEmpty());

		auto xmlDoc = CreateXMLDoc(XMLStr);
		assert(xmlDoc.has_value() && xmlDoc.value().IsOk());

		auto cellVec = XMLDocToCells(this, xmlDoc.value());
		if (cellVec.size() == 0)
			return { wxGridCellCoords(), wxGridCellCoords() };

		//This is the area where the data is coming from
		auto Corners = Cell::Get_TLBR(cellVec);

		//Calculate the translation of rows and cols between where the data is coming from and where it is about to be pasted
		int diffRow = RowPos - Corners.first.GetRow(); //first: topleft
		int diffCol = ColPos - Corners.first.GetCol();

		for (const auto& cell : cellVec) {
			int row = cell.GetRow(), col = cell.GetCol();

			if (PasteWhat == PASTE::ALL || PasteWhat == PASTE::VALUES)
				SetCellValue(row + diffRow, col + diffCol, cell.GetValue());

			if (PasteWhat == PASTE::ALL || PasteWhat == PASTE::FORMAT) {
				ApplyCellFormat(row + diffRow, col + diffCol, cell);
				AdjustRowHeight(row + diffRow);
			}
		}

		MarkDirty();

		//This is the area where the data is pasted
		wxGridCellCoords TL(RowPos, ColPos);
		wxGridCellCoords BR(
			Corners.second.GetRow() + diffRow,
			Corners.second.GetCol() + diffCol);

		return { TL, BR };
	}


	std::pair<wxGridCellCoords, wxGridCellCoords> CWorksheetBase::Paste_TextValues()
	{
		//This is where the user currently placed the cursor on Worksheet and pasting the data as of
		int RowPos = GetGridCursorRow();
		int ColPos = GetGridCursorCol();

		//Number of columns in the data
		int NCols = 0;

		//If the data consists of rows with unequal number of columns
		int MaxNCols = NCols;

		wxGridCellCoords TopLeft(RowPos, ColPos);

		if (!wxTheClipboard->IsSupported(wxDF_TEXT) || !wxTheClipboard->Open())
			return{ wxGridCellCoords(), wxGridCellCoords() };

		wxTextDataObject data;
		wxTheClipboard->GetData(data);
		wxTheClipboard->Close();

		wxString str = data.GetText();
		if (str.empty())
			return { wxGridCellCoords(), wxGridCellCoords() };


		wxStringTokenizer lines(str, "\n", wxTOKEN_RET_EMPTY);
		while (lines.HasMoreTokens())
		{
			wxStringTokenizer columns(lines.GetNextToken(), '\t', wxTOKEN_RET_EMPTY);

			int colvalue = ColPos;

			while (columns.HasMoreTokens())
				SetCellValue(RowPos, colvalue++, columns.GetNextToken());

			RowPos++;

			NCols = (colvalue - 1) - ColPos + 1;

			if (NCols > MaxNCols)
				MaxNCols = NCols;
		}

		MarkDirty();

		//Note that RosPos is changing in the loop
		wxGridCellCoords BottomRight(RowPos - 1, ColPos + MaxNCols - 1); //NCols=EndPos-ColPos+1;

		return { TopLeft, BottomRight };
	}
}