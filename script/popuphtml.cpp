#include "popuphtml.h"

namespace script
{
	CPopupHTML::CPopupHTML(wxWindow* prnt, const wxPoint& Pos, const wxSize& Size) :
		wxPopupTransientWindow(prnt)
	{
		SetSizeHints(wxDefaultSize, wxDefaultSize);

		m_HelpWnd = new wxHtmlWindow(this);
		SetPosition(Pos);
		SetSize(Size);

		m_MainSizer = new wxBoxSizer(wxVERTICAL);
		m_MainSizer->Add(m_HelpWnd, 1, wxEXPAND, 5);
		SetSizer(m_MainSizer);
		Layout();
	}


	void CPopupHTML::SetHTMLPage(const wxString& HTMLText)
	{
		m_HelpWnd->SetPage(HTMLText);
	}
}