#include "pnlcmdwnd.h"

#include "cmdline.h"
#include "pnlHistory.h"

#include "../icons/sz32/menu.xpm"






namespace cmdedit
{
	PyObject* pnlCommandWindow::m_Module = nullptr;
	CCmdLine* CmdLine = nullptr;




	pnlRightPane::pnlRightPane(wxWindow* parent, CCmdLine* cmdLine, PyObject* pymodule) : wxPanel(parent)
	{
		auto MainSizer = new wxBoxSizer(wxVERTICAL);

		//m_ntbk = new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_BOTTOM);

		auto History = new pnlBrowseHistory(this, cmdLine);

		//m_ntbk->AddPage(History, "history", true);;
		
		MainSizer->Add(History, 1, wxEXPAND | wxALL, 5);
		SetSizer(MainSizer);
		Layout();
	}

	pnlRightPane::~pnlRightPane() = default; 





	/****************************************************************************************/

	pnlCommandWindow::pnlCommandWindow(wxWindow* parent, wxWindowID id) :
		wxPanel(parent, id)
	{
		auto sci = PyImport_ImportModule("__SCISUIT");
		m_Module = PyObject_GetAttrString(sci, "COMMANDWINDOW");
		Py_XDECREF(sci);

		m_cmdLine = new CCmdLine(this, m_Module);
		CmdLine = m_cmdLine;

		
		auto RightPane = new pnlRightPane(this, m_cmdLine, m_Module);

		m_mgr = new wxAuiManager(this, wxAUI_MGR_LIVE_RESIZE | wxAUI_MGR_DEFAULT);
		
		m_mgr->AddPane(m_cmdLine, wxAuiPaneInfo().
			Name("CmdWnd").Caption("Command Window").
			Center().CloseButton(false).MaximizeButton(true).Layer(0).Position(0));

		m_mgr->AddPane(RightPane, wxAuiPaneInfo().
			Name("InfoPane").Caption("Info Pane").
			Right().MinSize(FromDIP(200), -1).CloseButton(false).Resizable(true).MaximizeButton(true).Layer(0).Position(0));

		m_mgr->Update();

		//Allow to be resized below minimum size
		m_mgr->GetPane("InfoPane").min_size = wxSize(-1, -1);
	}


	PyObject* pnlCommandWindow::GetCommandWndModule()
	{
		return m_Module;
	}

}