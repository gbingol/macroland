#pragma once

#include <wx/wx.h>
#include <wx/popupwin.h>
#include <wx/html/htmlwin.h>

#include "dllimpexp.h"

namespace script
{
	class CPopupHTML : public wxPopupTransientWindow
	{
	public:

		DLLSCRIPT CPopupHTML(wxWindow* parent, const wxPoint& Position, const wxSize& Size);
		DLLSCRIPT ~CPopupHTML() {}

		DLLSCRIPT void SetHTMLPage(const wxString& HTMLText);

		DLLSCRIPT auto GetMainSizer() const {
			return m_MainSizer;
		}

	protected:
		wxHtmlWindow* m_HelpWnd;

	private:
		wxSizer* m_MainSizer{ nullptr };
	};
}