#pragma once

#include <wx/wx.h>
#include <wx/aui/aui.h>

#include <Python.h>



namespace cmdedit
{
	class CCmdLine;

	class pnlCommandWindow : public wxPanel
	{
	public:
		pnlCommandWindow(wxWindow* parent, wxWindowID id = wxID_ANY);
		~pnlCommandWindow() = default;

	protected:
		CCmdLine* m_cmdLine;

	private:
		wxAuiManager* m_mgr;
	};




	/*************************************************** */

	class pnlHistory : public wxPanel
	{
	public:

		pnlHistory(wxWindow* parent, CCmdLine* cmdline);
		~pnlHistory() = default;

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