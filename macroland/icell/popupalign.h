#pragma once

#include <wx/wx.h>
#include <wx/popupwin.h>
#include <wx/tglbtn.h>


namespace ICELL
{
	class CWorkbook;

	class CPopupAlignment : public wxPopupTransientWindow
	{
	public:
		CPopupAlignment(CWorkbook* parent, const wxPoint& Position);

	protected:
		void OnToggleHoriz(wxCommandEvent& event);
		void OnToggleVert(wxCommandEvent& event);

		void OnDismiss() override;

	private:
		CWorkbook* m_Parent;
		wxBitmapToggleButton* m_btnHorLeft;
		wxBitmapToggleButton* m_btnHorCenter;
		wxBitmapToggleButton* m_btnHorRight;
		wxBitmapToggleButton* m_btnVertBtm;
		wxBitmapToggleButton* m_btnVertCenter;
		wxBitmapToggleButton* m_btnVertTop;
	};
}