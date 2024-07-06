#include "pnlcmdwnd.h"

#include "cmdline.h"
#include "pnlpathbrowse.h"
#include "pnlHistory.h"
#include "pnlvariablebrowser.h"

#include "../icons/sz32/menu.xpm"



static struct PyMethodDef CommandEditorMethods[] = { 
	{ NULL, NULL, 0, NULL } 
};

static struct PyModuleDef CommandEditorModuleDef = {
	PyModuleDef_HEAD_INIT,
	"CommandWindow",
	"Command Window Module",
	-1,
	CommandEditorMethods,
};



namespace scripting::cmdedit
{
	PyObject* pnlCommandWindow::m_Module = nullptr;
	CCmdLine* CmdLine = nullptr;




	pnlRightPane::pnlRightPane(wxWindow* parent, CCmdLine* cmdLine, PyObject* pymodule) : wxPanel(parent)
	{
		auto MainSizer = new wxBoxSizer(wxVERTICAL);

		m_ntbk = new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_BOTTOM);

		auto Path = new pnlPythonPathBrowser(m_ntbk, pymodule);
		auto History = new pnlBrowseHistory(m_ntbk, cmdLine);
		auto Variables = new pnlVariableBrowser(m_ntbk, pymodule);

		m_ntbk->AddPage(History, "history", true);
		m_ntbk->AddPage(Variables, "vars", false);
		m_ntbk->AddPage(Path, "sys.path", false);
		
		MainSizer->Add(m_ntbk, 1, wxEXPAND | wxALL, 5);
		SetSizer(MainSizer);
		Layout();
	}

	pnlRightPane::~pnlRightPane() = default; 





	/****************************************************************************************/

	pnlCommandWindow::pnlCommandWindow(wxWindow* parent, wxWindowID id) :
		wxPanel(parent, id)
	{
		m_Module = PyModule_Create(&CommandEditorModuleDef);

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


	wxMenu* pnlCommandWindow::GetInputWndContextMenu()
	{
		return CmdLine->GetInputWnd()->GetContextMenu();
	}

}