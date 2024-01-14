#pragma once

#include <wx/wx.h>

#include "../styledtxtctrl.h"

#include "../dllimpexp.h"

namespace script
{
	class COutputWnd : public CStyledTextCtrl
	{
	public:
		DLLSCRIPT COutputWnd(
			wxWindow* parent,
			wxWindowID id = -1,
			CStyledTextCtrl* InputWnd = nullptr);

		DLLSCRIPT ~COutputWnd() = default;

		DLLSCRIPT wxSize DoGetBestSize() const;

		DLLSCRIPT void AppendOutput(const wxString& txt, bool PrependLine = true);

	protected:
		DLLSCRIPT void OnModified(wxStyledTextEvent& event);

		DLLSCRIPT void OnSetFocus(wxFocusEvent& event);
		DLLSCRIPT void OnKillFocus(wxFocusEvent& event);

		DLLSCRIPT void OnRightUp(wxMouseEvent& event);

		DLLSCRIPT void OnPopMenu(wxCommandEvent& event);

	private:
		wxFont m_Font;
		wxWindow* m_PrntWnd;
		CStyledTextCtrl* m_InputWnd;

		const int ID_SAVE{ wxNewId() };
		const int ID_COPY{ wxNewId() };
		const int ID_DELALL{ wxNewId() };
		const int ID_SHOWLINENO{ wxNewId() };

		bool m_ShowLineNo{ true };
	};
}

