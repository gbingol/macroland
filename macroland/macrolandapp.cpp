#include "macrolandapp.h"

#include <Python.h>
#include <codecvt>
#include <locale>

#include <wx/msgdlg.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>

#include "python/registertopython.h"

#include "mainfrm/frmmacroland.h"

#include "util/json.h"
#include "util/util_string.h"


std::filesystem::path glbExeDir;
JSON::Value glbSettings;


wxIMPLEMENT_APP(MacroLandApp);


bool MacroLandApp::OnInit()
{
	namespace fs = std::filesystem;

	//Find executable path
	wxFileName exePath(wxStandardPaths::Get().GetExecutablePath());
	glbExeDir = (exePath.GetPath() + wxFileName::GetPathSeparator()).ToStdWstring();

	auto json = JSON::JSON(glbExeDir/"_settings.json");
	JSON::Error err;
	glbSettings = json.Parse(err);

	
	if(!InitSciSuitModules())
	{
		wxMessageBox("Could not initialize Python interpreter and modules.");
		return false;
	}

	//needed by ribbon images, therefore must be started before main frame
	::wxInitAllImageHandlers();

	fs::path ArgPath;
	if (argc > 1) {
		wxCmdLineParser parser(cmdLineDesc, argc, argv);
		parser.Parse();
		ArgPath = argv[1].ToStdWstring();
	}

	
	try {
		m_frmMacroLand = new frmMacroLand(ArgPath);
		m_frmMacroLand->Show();
		m_frmMacroLand->Maximize();
		
	}	
	catch(std::exception& e) {
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



bool MacroLandApp::InitSciSuitModules()
{
	PyImport_AppendInittab("__SCISUIT", CreateSystemModule);
	
	auto JSONObject = glbSettings.as_object();
	if(JSONObject.contains("PythonExe") && JSONObject["PythonExe"].is_object())
	{
		auto PathObj = JSONObject["PythonExe"].as_object();
		if(PathObj.contains("path") && PathObj["path"].is_string())
		{
			auto PathStr = PathObj["path"].as_string();
			PathStr = util::trim(PathStr);

			m_PyHome = PathStr;
			if(!m_PyHome.empty())
			{
				if(m_PyHome.is_relative())
					m_PyHome = (glbExeDir / m_PyHome).lexically_normal();
				
				_Py_SetProgramFullPath(m_PyHome.wstring().c_str());
			}
		}
	}
	
	if(m_PyHome.empty() || !std::filesystem::exists(m_PyHome))
	{
		m_PyHome = glbExeDir / "python3106";
		if(!std::filesystem::exists(m_PyHome))
			return false;

		Py_SetPythonHome(m_PyHome.wstring().c_str());
	}


	//if m_PyHome does not point to a valid directory, ScienceSuit will NOT start
	Py_Initialize();


	auto SCISUITSYSTEM = PyImport_ImportModule("__SCISUIT");
	PyObject* sci_dict = PyModule_GetDict(SCISUITSYSTEM);

	//create the core module initially with fit module
	auto GUI = PyModule_Create(&System_GUI_Module);
	auto CmdWnd = PyModule_Create(&CommandEditorModuleDef);
	auto Extension = PyModule_Create(&System_Extension_Module);

	PyObject* gui_dict = PyModule_GetDict(GUI);
	PyObject* cmdwnd_dict = PyModule_GetDict(CmdWnd);
	PyObject* extension_dict = PyModule_GetDict(Extension);

	// Get the __builtins__ module
	PyObject* builtins = PyEval_GetBuiltins();

	// Set the __builtins__ attribute in the module dictionaries
	PyDict_SetItemString(gui_dict, "__builtins__", builtins);
	PyDict_SetItemString(cmdwnd_dict, "__builtins__", builtins);
	PyDict_SetItemString(extension_dict, "__builtins__", builtins);

	//auto SCISUIT = PyModule_GetDict(SCISUITSYSTEM);
	PyDict_SetItemString(sci_dict, "GUI", GUI);
	PyDict_SetItemString(sci_dict, "COMMANDWINDOW", CmdWnd);
	PyDict_SetItemString(sci_dict, "EXTENSION", Extension);

	extern int PyInit_Worksheet(PyObject * Module);
	PyInit_Worksheet(GUI);


	auto Path = (glbExeDir / "_appinit_.py").wstring();
	if (!std::filesystem::exists(Path))
		return false;
		
	std::wstring_convert<std::codecvt_utf8<wchar_t>> cvt;
	if (auto cp = _Py_wfopen(Path.c_str(), L"rb"))
		PyRun_SimpleFileExFlags(cp, cvt.to_bytes(Path).c_str(), true, 0);

	return true;
}