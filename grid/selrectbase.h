#pragma once


#include <wx/wx.h>
#include <wx/grid.h>

#include "dllimpexp.h"

//events by selection rectangle
DLLGRID wxDECLARE_EVENT(EVT_DATA_MOVING, wxMouseEvent);
DLLGRID wxDECLARE_EVENT(EVT_DATA_MOVED, wxMouseEvent);


namespace grid
{
	class CWorksheetBase;

	class CSelRect : public wxEvtHandler
	{
	protected:
		virtual DLLGRID ~CSelRect();

	public:
		DLLGRID CSelRect(CWorksheetBase* ws,
			const wxPoint& start,
			const wxPoint& end,
			const wxPen& pen,
			const wxBrush& brush);

		DLLGRID void SetCoords(const wxGridCellCoords& tl, const wxGridCellCoords& br)
		{
			m_TL = tl;
			m_BR = br;
		}

		DLLGRID void SetInitCoords(const wxGridCellCoords& tl, const wxGridCellCoords& br)
		{
			m_InitTL = tl;
			m_InitBR = br;
		}

		DLLGRID void SetLButtonCoords(int row, int col)
		{
			m_LBtnDown.SetRow(row);
			m_LBtnDown.SetCol(col);
		}

		DLLGRID wxRect GetInflatedBoundingRect() const;

		DLLGRID auto GetBoundRect() const
		{
			return m_BoundRect;
		}

		DLLGRID void SetStart(const wxPoint& pt)
		{
			m_Start = pt;
			m_BoundRect = wxRect(m_Start, m_End);
		}

		DLLGRID void SetEnd(const wxPoint& pt)
		{
			m_End = pt;
			m_BoundRect = wxRect(m_Start, m_End);
		}

		DLLGRID bool CopyMoveBlock(
			const wxGridCellCoordsArray& from,
			const wxGridCellCoordsArray& to,
			bool IsMoving = false);

		DLLGRID void Draw(wxDC* dc);


	protected:
		DLLGRID void OnKeyDown(wxKeyEvent& event);
		DLLGRID void OnLeftUp(wxMouseEvent& event);
		DLLGRID void OnLeftDown(wxMouseEvent& event);

		DLLGRID void OnMouseMove(wxMouseEvent& event);

		DLLGRID void OnDataMoving(wxMouseEvent& event);
		DLLGRID void OnDataMoved(wxMouseEvent& event);

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