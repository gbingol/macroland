#include "macrolandapp.h"

#include <Python.h>
#include <lua.hpp>
#include <codecvt>
#include <locale>

#include <wx/stdpaths.h>
#include <wx/filename.h>

#include "consts.h"
#include "python/registertopython.h"
#include "lua/registerlua.h"

#include "mainfrm/frmmacroland.h"


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

	std::wstring_convert<std::codecvt_utf8<wchar_t>> cvt;
	auto Path = glbExeDir / "_init.py";

	if (std::filesystem::exists(Path))
	{
		if (auto cp = _Py_wfopen(Path.wstring().c_str(), L"rb"))
			PyRun_SimpleFileExFlags(cp, cvt.to_bytes(Path.wstring()).c_str(), true, 0);
	}

	fs::path ArgPath;
	if (argc > 1) 
	{
		wxCmdLineParser parser(cmdLineDesc, argc, argv);
		parser.Parse();
		ArgPath = argv[1].ToStdWstring();
	}

	
	try
	{
		m_frmMacroLand = new frmMacroLand(ArgPath);
		m_frmMacroLand->Show();
		m_frmMacroLand->Maximize();
		
	}
	
	catch(std::exception& e)
	{
		wxMessageBox(e.what());
		return false;
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