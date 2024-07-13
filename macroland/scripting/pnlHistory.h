#pragma once

#include <vector>

#include <wx/wx.h>

#include <script/cmdwnd/inputwndbase.h>


namespace scripting::cmdedit
{
	class CCmdLine;

	class pnlBrowseHistory : public wxPanel
	{
	public:

		pnlBrowseHistory(wxWindow* parent, CCmdLine* cmdline);
		~pnlBrowseHistory() = default;

	protected:
		void OnCmdExecuted(wxCommandEvent& event);

		void OnCopyBtn(wxCommandEvent& event);

		void OnListBox(wxCommandEvent& event);
		void OnListBoxDClick(wxCommandEvent& event);

	protected:
		wxListBox* m_lstBox;
		wxBitmapButton* m_btn;
		wxCheckBox *m_chkExecuteCmd;

		CCmdLine* m_CmdLine;
	};
}
