#include "popupalign.h"

#include "workbook.h"
#include "worksheet.h"

#include "../icons/sz16/icons16.h"
#include "../icons/sz32/icons32.h"

namespace ICELL
{
	CPopupAlignment::CPopupAlignment(CWorkbook* parent, const wxPoint& Position) :
		wxPopupTransientWindow(parent)
	{
		m_Parent = parent;


		m_btnHorLeft = new wxBitmapToggleButton(this, wxID_JUSTIFY_LEFT, align_left_xpm);
		m_btnHorCenter = new wxBitmapToggleButton(this, wxID_JUSTIFY_CENTER, align_horizontalcenter_xpm);
		m_btnHorRight = new wxBitmapToggleButton(this, wxID_JUSTIFY_RIGHT, align_right_xpm);

		m_btnVertBtm = new wxBitmapToggleButton(this, wxALIGN_BOTTOM, align_verticalbottom_xpm);
		m_btnVertCenter = new wxBitmapToggleButton(this, wxALIGN_CENTRE, align_verticalcenter_xpm);
		m_btnVertTop = new wxBitmapToggleButton(this, wxALIGN_TOP, align_verticaltop_xpm);


		auto ws = parent->GetActiveWS();

		int row = ws->GetGridCursorRow();
		int col = ws->GetGridCursorCol();
		int hor = 0, ver = 0;

		ws->GetCellAlignment(row, col, &hor, &ver);

		m_btnHorLeft->SetValue(hor == wxALIGN_LEFT);
		m_btnHorCenter->SetValue(hor == wxALIGN_CENTRE);
		m_btnHorRight->SetValue(hor == wxALIGN_RIGHT);

		auto VertBtns = { m_btnVertBtm , m_btnVertCenter , m_btnVertTop };
		for (auto btn : VertBtns)
			btn->SetValue(btn->GetId() == ver);

		auto sbSzrHoriz = new wxBoxSizer(wxHORIZONTAL);
		sbSzrHoriz->Add(m_btnHorLeft, 0, 0, 5);
		sbSzrHoriz->Add(m_btnHorCenter, 0, 0, 5);
		sbSzrHoriz->Add(m_btnHorRight, 0, 0, 5);

		auto sbSzrVert = new wxBoxSizer(wxHORIZONTAL);
		sbSzrVert->Add(m_btnVertBtm, 0, 0, 5);
		sbSzrVert->Add(m_btnVertCenter, 0, 0, 5);
		sbSzrVert->Add(m_btnVertTop, 0, 0, 5);

		auto szrMain = new wxBoxSizer(wxVERTICAL);
		szrMain->Add(sbSzrHoriz, 0, 0, 5);
		szrMain->Add(sbSzrVert, 0, 0, 5);

		SetSizerAndFit(szrMain);
		Layout();

		SetPosition(Position);

		m_btnHorLeft->Bind(wxEVT_TOGGLEBUTTON, &CPopupAlignment::OnToggleHoriz, this);
		m_btnHorCenter->Bind(wxEVT_TOGGLEBUTTON, &CPopupAlignment::OnToggleHoriz, this);
		m_btnHorRight->Bind(wxEVT_TOGGLEBUTTON, &CPopupAlignment::OnToggleHoriz, this);
		m_btnVertBtm->Bind(wxEVT_TOGGLEBUTTON, &CPopupAlignment::OnToggleVert, this);
		m_btnVertCenter->Bind(wxEVT_TOGGLEBUTTON, &CPopupAlignment::OnToggleVert, this);
		m_btnVertTop->Bind(wxEVT_TOGGLEBUTTON, &CPopupAlignment::OnToggleVert, this);
	}

	void CPopupAlignment::OnToggleHoriz(wxCommandEvent& event)
	{
		m_Parent->ChangeCellAlignment(event.GetId());

		auto BtnHors = { m_btnHorLeft , m_btnHorCenter , m_btnHorRight };
		for (auto btn : BtnHors)
			btn->SetValue(btn->GetId() == event.GetId());
	}

	void CPopupAlignment::OnToggleVert(wxCommandEvent& event)
	{
		m_Parent->ChangeCellAlignment(event.GetId());

		auto VertBtns = { m_btnVertBtm , m_btnVertCenter , m_btnVertTop };
		for (auto btn : VertBtns)
			btn->SetValue(btn->GetId() == event.GetId());
	}

	void CPopupAlignment::OnDismiss()
	{
		m_Parent->GetActiveWS()->SetFocus();
	}
}