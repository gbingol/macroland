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

	//Create Home Directories
	auto ScriptDir = glbExeDir / consts::HOME / ".script";
	auto UnSavedScriptDir = glbExeDir / consts::HOME / ".script/drafts";

	if (!fs::exists(ScriptDir))
		fs::create_directory(ScriptDir);

	if (!fs::exists(UnSavedScriptDir))
		fs::create_directory(UnSavedScriptDir);

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

	try
	{
		auto frm = new frmMacroLand(ArgPath);
		frm->Show();
		frm->Maximize();
	}
	catch(exceptions::PyPkgMissingException& e)
	{
		wxString msg;
		msg << "scisuit and wxPython (both installing numpy) are crucial Python packages.";
		msg << "Therefore must be installed for the MacroLand App to function properly. \n";
		msg << "\n";
		msg << "Should I install them for you to " << m_PyHome.wstring();
		msg << "\n \n";
		msg << "If you select No, then please run Python Package Manager App to install.";

		int ans = wxMessageBox(msg, "Missing Crucial Packages!", wxYES_NO);

		if(ans == wxNO)
			return false;
		
		auto PythonExe = m_PyHome / "python";
		std::wstring Cmd = L"\"" + PythonExe.wstring() + L"\"";
		Cmd +=  L" -m pip install scisuit wxpython";
		
		wxExecute(Cmd, wxEXEC_ASYNC);

		return false;
	}
	catch(std::exception& e)
	{
		wxMessageBox(e.what());
		return false;
	}

	return true;
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
	extern int PyInit_Workbook(PyObject * Module);
	extern int PyInit_Range(PyObject * Module);

	PyInit_Workbook(GUI_Module);
	PyInit_Worksheet(GUI_Module);
	PyInit_Range(GUI_Module);
}