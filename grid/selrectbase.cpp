#include "selrectbase.h"

#include <forward_list>
#include <wx/wx.h>
#include <wx/grid.h>

#include "ws_cell.h"
#include "worksheetbase.h"
#include "undoredo.h"
#include "workbookbase.h"



//events by selection rectangle
wxDEFINE_EVENT(EVT_DATA_MOVING, wxMouseEvent);
wxDEFINE_EVENT(EVT_DATA_MOVED, wxMouseEvent);


//Is there an overlap with the source location
static bool SourceOverlap(
	const wxGridCellCoords& SourceTL, //source
	const wxGridCellCoords& SourceBR,
	const wxGridCellCoords& Target) //target location
{
	int col = Target.GetCol();
	int row = Target.GetRow();

	return (col <= SourceBR.GetCol() && col >= SourceTL.GetCol() &&
		row <= SourceBR.GetRow() && row >= SourceTL.GetRow());
}



namespace grid
{

	CSelRect::CSelRect(CWorksheetBase* ws,
		const wxPoint& start,
		const wxPoint& end,
		const wxPen& pen,
		const wxBrush& brush)
	{
		m_WS = ws;
		m_Pen = pen;
		m_Brush = brush;
		m_Start = start;
		m_End = end;

		m_BoundRect = wxRect(start, end);

		m_WS->Bind(wxEVT_KEY_DOWN, &CSelRect::OnKeyDown, this);
		m_WS->GetGridWindow()->Bind(wxEVT_LEFT_UP, &CSelRect::OnLeftUp, this);
		m_WS->GetGridWindow()->Bind(wxEVT_MOTION, &CSelRect::OnMouseMove, this);
		m_WS->GetGridWindow()->Bind(wxEVT_LEFT_DOWN, &CSelRect::OnLeftDown, this);

		Bind(EVT_DATA_MOVING, &CSelRect::OnDataMoving, this);
		Bind(EVT_DATA_MOVED, &CSelRect::OnDataMoved, this);
	}


	CSelRect::~CSelRect()
	{
		m_WS->Unbind(wxEVT_KEY_DOWN, &CSelRect::OnKeyDown, this);
		m_WS->GetGridWindow()->Unbind(wxEVT_LEFT_UP, &CSelRect::OnLeftUp, this);
		m_WS->GetGridWindow()->Unbind(wxEVT_MOTION, &CSelRect::OnMouseMove, this);
		m_WS->GetGridWindow()->Unbind(wxEVT_LEFT_DOWN, &CSelRect::OnLeftDown, this);

		Unbind(EVT_DATA_MOVING, &CSelRect::OnDataMoving, this);
		Unbind(EVT_DATA_MOVED, &CSelRect::OnDataMoved, this);
	}


	void CSelRect::OnKeyDown(wxKeyEvent& event)
	{
		if (event.GetKeyCode() == WXK_ESCAPE)
		{
			wxRect BRect = GetBoundRect();
			wxCoord dx = m_WS->FromDIP(200), dy = m_WS->FromDIP(200);
			BRect.Inflate(dx, dy);
			wxRect rect_Init = m_WS->BlockToDeviceRect(m_InitTL, m_InitBR);

			m_WS->Refresh(true, &rect_Init);
			m_WS->Refresh(true, &BRect);

			delete this;
		}

		event.Skip();
	}


	void CSelRect::OnLeftUp(wxMouseEvent& event)
	{
		//Check if we moved the data at all
		if (m_TL == m_InitTL)
		{
			m_WS->ClearSelection(); //clear original selection

			m_WS->Refresh();
			m_WS->Update();

			delete this;

			return;
		}

		wxMouseEvent dataMoved(EVT_DATA_MOVED);
		dataMoved.SetPosition(event.GetPosition());
		dataMoved.SetX(event.GetX());
		dataMoved.SetY(event.GetY());

		if (event.ControlDown())
			dataMoved.SetControlDown(true);

		wxPostEvent(this, dataMoved);

		event.Skip();
	}


	void CSelRect::OnLeftDown(wxMouseEvent& event)
	{
		if (m_WS->IsSelection() &&
			m_WS->GetSelectedRows().Count() == 0 &&
			m_WS->GetSelectedCols().Count() == 0)
		{
			auto TL = m_WS->GetSelTopLeft();
			auto BR = m_WS->GetSelBtmRight();

			wxRect selRect = m_WS->BlockToDeviceRect(TL, BR);

			if (!selRect.Contains(event.GetPosition()))
				delete this;
		}

		event.Skip();
	}


	void CSelRect::OnMouseMove(wxMouseEvent& event)
	{
		if (event.Dragging())
		{
			wxMouseEvent dataMoving(EVT_DATA_MOVING);
			dataMoving.SetPosition(event.GetPosition());
			dataMoving.SetX(event.GetX());
			dataMoving.SetY(event.GetY());

			wxPostEvent(this, dataMoving);

			//dragging the mouse on the wxGrid by default selects the cells,
			//therefore we need to avoid event.skip
			return;
		}

		event.Skip();
	}


	void CSelRect::OnDataMoving(wxMouseEvent& event)
	{
		int MouseCoord_Row = m_WS->YToRow(event.GetY());
		int MouseCoord_Col = m_WS->XToCol(event.GetX());

		if (MouseCoord_Row == wxNOT_FOUND || MouseCoord_Col == wxNOT_FOUND)
			return;

		//on which cell the mouse currently is
		wxGridCellCoords evtCoords(MouseCoord_Row, MouseCoord_Col);

		//Dont move the rectangle unless user moved a length of a row or col
		//Otherwise the rectangle will move incredibly fast as move method is called everytime mouse moves
		if (evtCoords.GetRow() == m_LBtnDown.GetRow() &&
			evtCoords.GetCol() == m_LBtnDown.GetCol())
			return;


		//Calculate where to move the rectangle based on mouse location
		//If for example nextRow<0 then the rectangle is moving somewhere to left
		int nextRow = evtCoords.GetRow() - m_LBtnDown.GetRow();
		int nextCol = evtCoords.GetCol() - m_LBtnDown.GetCol();


		//Calculate new coordinates of the rectangle
		int TL_Row = m_TL.GetRow() + nextRow;
		int TL_Col = m_TL.GetCol() + nextCol;
		int BR_Row = m_BR.GetRow() + nextRow;
		int BR_Col = m_BR.GetCol() + nextCol;

		//Dont let the rectangle go out of client area
		if (TL_Row < 0 ||
			TL_Col < 0 ||
			BR_Row < 0 ||
			BR_Col < 0)
			return;


		//updating the location of the Rectangle
		wxGridCellCoords new_TL(TL_Row, TL_Col);
		wxGridCellCoords new_BR(BR_Row, BR_Col);

		wxRect r = m_WS->BlockToDeviceRect(new_TL, new_BR);

		SetStart(r.GetTopLeft());
		SetEnd(r.GetBottomRight());

		wxRect BoundingRect = GetInflatedBoundingRect();
		m_WS->RefreshRect(BoundingRect, true);

		//we need to update quickly to be able to draw the rectangle while moving
		m_WS->Update();

		wxClientDC dc(m_WS->GetGridWindow());
		Draw(&dc);

		//Update points, must be updated at the very end
		m_TL = new_TL;
		m_BR = new_BR;
		m_LBtnDown = evtCoords;
	}



	void CSelRect::OnDataMoved(wxMouseEvent& event)
	{
		wxGridCellCoordsArray from, to;
		from.push_back(m_InitTL);
		from.push_back(m_InitBR);

		to.push_back(m_TL);
		to.push_back(m_BR);


		for (int row = m_TL.GetRow(); row <= m_BR.GetRow(); ++row)
		{
			for (int col = m_TL.GetCol(); col <= m_BR.GetCol(); ++col)
			{
				wxGridCellCoords coord(row, col);

				if (SourceOverlap(m_InitTL, m_InitBR, coord))
					continue;

				if (!m_WS->GetCellValue(row, col).empty())
				{
					if (wxMessageBox("Target data will be overwritten. Continue?", "Confirm", wxYES_NO) == wxNO)
					{
						delete this;
						return;
					}
				}
			}//for (int col
		}


		//by default and mostly we move the block (CtrlDown copies)
		bool IsMoving = !event.ControlDown();

		auto dataMoved = std::make_unique<DataMovedEvent>(m_WS, IsMoving);

		dataMoved->SetInitCoords(m_InitTL, m_InitBR);
		dataMoved->SetCells(m_WS->GetBlock(m_InitTL, m_InitBR));

		//new block is in selected mode
		m_WS->SelectBlock(m_TL, m_BR);

		//move cursor to the top left of new block
		m_WS->GoToCell(m_TL);

		CopyMoveBlock(from, to, IsMoving);

		wxRect BoundingRect = GetInflatedBoundingRect();
		wxRect rect_Init = m_WS->BlockToDeviceRect(m_InitTL, m_InitBR);

		m_WS->Refresh();

		dataMoved->SetFinalCoords(m_TL, m_BR);

		if(m_WS->GetWorkbook())
			m_WS->GetWorkbook()->PushUndoEvent(std::move(dataMoved));

		delete this;
	}



	wxRect CSelRect::GetInflatedBoundingRect() const
	{
		wxRect BR = GetBoundRect();
		wxCoord dx = m_WS->FromDIP(200), dy = m_WS->FromDIP(200);
		BR.Inflate(dx, dy);

		return BR;
	}


	bool CSelRect::CopyMoveBlock(
		const wxGridCellCoordsArray& from,
		const wxGridCellCoordsArray& to,
		bool IsMoving)
	{

		const wxGridCellCoords tl_from = from[0], br_from = from[1];
		const wxGridCellCoords tl_to = to[0], br_to = to[1];

		if (tl_from == tl_to && br_from == br_to)
			return false;

		int distY = tl_to.GetRow() - tl_from.GetRow();
		int distX = tl_to.GetCol() - tl_from.GetCol();

		int tl_row = tl_from.GetRow(), br_row = br_from.GetRow();
		int tl_col = tl_from.GetCol(), br_col = br_from.GetCol();

		std::forward_list<Cell> Block;
		for (int i = tl_row; i <= br_row; i++)
		{
			for (int j = tl_col; j <= br_col; j++)
			{
				Block.emplace_front(m_WS, i, j);

				if (IsMoving)
					m_WS->SetCelltoDefault(i, j);
			}

			m_WS->AdjustRowHeight(i);
		}


		for (const auto& cell : Block)
		{
			int row = cell.GetRow() + distY;
			int col = cell.GetCol() + distX;

			m_WS->SetCellValue(row, col, cell.GetValue());

			m_WS->ApplyCellFormat(row, col, cell);
			m_WS->AdjustRowHeight(row);
		}

		m_WS->MarkDirty();

		return true;
	}

	void CSelRect::Draw(wxDC* dc)
	{
		wxPen oldPen = dc->GetPen();
		wxBrush oldBrush = dc->GetBrush();

		dc->SetPen(m_Pen);
		dc->SetBrush(m_Brush);

		dc->DrawRectangle(wxRect(m_Start, m_End));

		dc->SetPen(oldPen);
		dc->SetBrush(oldBrush);
	}

}//ICELL