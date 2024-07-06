#pragma once

#include <wx/wx.h>

#include <script/pnlpathbrowse.h>

#include <Python.h>

class pnlPythonPathBrowser : public wxPanel
{
public:
	pnlPythonPathBrowser(wxWindow* parent, PyObject* pymodule);
	~pnlPythonPathBrowser();

protected:
	wxBitmapButton* m_btn;;
	script::pnlPathBrowser* m_pnlBrowse;

	void OnButtonClick(wxCommandEvent& event);

	PyObject* m_Module;
};