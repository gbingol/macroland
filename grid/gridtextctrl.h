#pragma once

#include <wx/wx.h>
#include <wx/grid.h>
#include <wx/minifram.h>

#include "dllimpexp.h"

namespace grid
{
	class GridTextFloatingFrame;
	class CWorkbookBase;

	class CGridTextCtrl :public wxControl
	{
		DECLARE_DYNAMIC_CLASS(CGridTextCtrl);

	public:

		DLLGRID CGridTextCtrl() = default;

		DLLGRID CGridTextCtrl(
			wxWindow* parent,
			CWorkbookBase* workbook,
			wxWindowID id = wxID_ANY,
			wxBitmap bmp = wxNullBitmap,
			const wxColor& ListeningSel = wxColor(167, 243, 248),
			const wxColor& NotListeningSel = wxColor(255, 255, 255));

		virtual DLLGRID ~CGridTextCtrl();

		DLLGRID wxString GetValue() const { return m_Txt->GetValue(); }

		DLLGRID auto GetButton() const { return m_Btn; }

		DLLGRID void SetFocus() { m_Txt->SetFocus(); }

		DLLGRID wxSize DoGetBestSize() const;

	protected:
		DLLGRID void OnButtonClicked(wxCommandEvent& event);

		DLLGRID void OnPaint(wxPaintEvent& event);

		DLLGRID void OnSetFocus(wxFocusEvent& evt);
		DLLGRID void OnKillFocus(wxFocusEvent& event);

		DLLGRID void OnGridRangeSelecting(wxGridRangeSelectEvent& event);
		DLLGRID void OnGridRangeSelected(wxGridRangeSelectEvent& event);

		DLLGRID void FloatFrm_OnClose(wxCloseEvent& event);

	private:
		wxString GetRangeText(wxGridRangeSelectEvent& event);

	private:
		wxTextCtrl* m_Txt;
		wxBitmapButton* m_Btn;

		GridTextFloatingFrame* m_FloatFrm;

		wxWindow* m_PrntWnd;
		wxWindow* m_TopLevelWnd{ nullptr };

		wxColor m_ListeningSel{};
		wxColor m_NOT_ListeningSel{};
		CWorkbookBase* m_Workbook;
	};


	/************************************************/

	class GridTextFloatingFrame : public wxMiniFrame
	{
	public:
		DLLGRID GridTextFloatingFrame(wxWindow* parent, CWorkbookBase* workbook);
		DLLGRID ~GridTextFloatingFrame() = default;

		DLLGRID auto GetValue() const
		{ 
			return m_Txt->GetValue(); 
		}

	protected:
		CGridTextCtrl* m_Txt;
	};
}
