#include "macrolandapp.h"

#include <Python.h>
#include <lua.hpp>

#include <wx/stdpaths.h>
#include <wx/filename.h>

#include "consts.h"
#include "python/registertopython.h"
#include "lua/registerribbontoluavm.h"

#include "mainfrm/frmmacroland.h"

#include "exceptions.h"


std::filesystem::path glbExeDir;
lua_State* glbLuaState;


wxIMPLEMENT_APP(MacroLandApp);


bool MacroLandApp::OnInit()
{
	namespace fs = std::filesystem;

	//Find executable path
	wxFileName exePath(wxStandardPaths::Get().GetExecutablePath());
	glbExeDir = (exePath.GetPath() + wxFileName::GetPathSeparator()).ToStdWstring();


	//lua state
	glbLuaState = luaL_newstate();
	luaL_openlibs(glbLuaState);
	RegisterLuaFuncAndUserdata(glbLuaState);

	PyImport_AppendInittab("__SCISUIT", CreateSystemModule);

	m_PyHome = glbExeDir / "python3106";
	Py_SetPythonHome(m_PyHome.wstring().c_str());

	//if m_PyHome does not point to a valid directory, ScienceSuit will NOT start
	Py_Initialize();
	CreateSciSuitModules();
	

	//needed by ribbon images, therefore must be started before main frame
	::wxInitAllImageHandlers();

	fs::path ArgPath;
	if (argc > 1) 
	{
		wxCmdLineParser parser(cmdLineDesc, argc, argv);
		parser.Parse();
		ArgPath = argv[1].ToStdWstring();
	}

	while(true)
	{
		try
		{
			m_frmMacroLand = new frmMacroLand(ArgPath);
			m_frmMacroLand->Show();
			m_frmMacroLand->Maximize();
			break;
		}
		catch(exceptions::PyPkgMissingException& e)
		{
			/*
				Note that this exception occurs at the constructor of frmMacroLand,
				therefore all resources will be destroyed automatically.
			*/
			wxString msg;
			msg << "scisuit and wxPython (both installing numpy) are crucial Python packages ";
			msg << "for the MacroLand App. Therefore must be installed for the system to function properly. \n";
			msg << "\n";
			msg << "Should I install them for you to " << m_PyHome.wstring();
			msg << "\n \n";
			msg << "If Yes, package installation process will begin and at the end the app will start.\n";
			msg << "If No, application will exit.";
			
			int ans = wxMessageBox(msg, "Missing Crucial Packages!", wxYES_NO);

			if(ans == wxNO)
				return false;
			
			auto PythonExe = m_PyHome / "python";
			wxString Cmd = "\"" + PythonExe.wstring() + "\"";
			Cmd << " -m pip install scisuit==" << consts::SCISUITPKG << " wxpython";

			wxExecute(Cmd, wxEXEC_SYNC);
		}
		catch(std::exception& e)
		{
			wxMessageBox(e.what());
			return false;
		}
	}

	return true;
}



int MacroLandApp::FilterEvent(wxEvent &event)
{
	auto EvtType = event.GetEventType();
	auto KeyCode = ((wxKeyEvent &)event).GetKeyCode();
	
	if (EvtType == wxEVT_KEY_DOWN)
	{
		if(m_frmMacroLand->IsFullScreen() && KeyCode == WXK_ESCAPE)
		{
			m_frmMacroLand->ShowFullScreen(false);
			return true;
		}
	}

    return -1;
}



void MacroLandApp::CreateSciSuitModules()
{
	auto SCISUITSYSTEM = PyImport_ImportModule("__SCISUIT");

	//create the core module initially with fit module
	auto GUI_Module = PyModule_Create(&System_GUI_Module);
	auto CmdWnd_Module = PyModule_Create(&CommandEditorModuleDef);

	auto SCISUIT = PyModule_GetDict(SCISUITSYSTEM);
	PyDict_SetItemString(SCISUIT, "GUI", GUI_Module);
	PyDict_SetItemString(SCISUIT, "COMMANDWINDOW", CmdWnd_Module);

	extern int PyInit_Worksheet(PyObject * Module);
	PyInit_Worksheet(GUI_Module);
}