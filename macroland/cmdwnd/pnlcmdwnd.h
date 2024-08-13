#pragma once

#include <wx/wx.h>
#include <wx/aui/aui.h>

#include <Python.h>




namespace cmdedit
{
	class CCmdLine;

	class pnlRightPane : public wxPanel
	{
	public:
		pnlRightPane(wxWindow* parent, CCmdLine* cmdLine, PyObject* pymodule);
		~pnlRightPane();

	protected:
		wxNotebook* m_ntbk;
	};




	/*
		Has to be derived from wxPanel as 
		in main frame it is used as child of splitter window
	*/
	class pnlCommandWindow : public wxPanel
	{
	public:
		pnlCommandWindow(wxWindow* parent, wxWindowID id = wxID_ANY);
		~pnlCommandWindow() = default;

		static PyObject* GetCommandWndModule();

	protected:
		CCmdLine* m_cmdLine;
		static PyObject* m_Module;

	private:
		const int ID_TOOLMENU{ wxNewId() };
		const int ID_BROWSEPATH{ wxNewId() };
		const int ID_BROWSECMDHIST{ wxNewId() };

		wxAuiManager* m_mgr;
	};
}