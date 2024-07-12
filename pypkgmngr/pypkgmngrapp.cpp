#include "pypkgmngrapp.h"

#include <Python.h>

#include <wx/stdpaths.h>
#include <wx/filename.h>


#include "frmpypkgmngr.h"


static const char* PYHOMEDIR = "python3106";


std::filesystem::path glbExeDir{};  


wxIMPLEMENT_APP(PyPkgMngrApp);


bool PyPkgMngrApp::OnInit()
{
	namespace fs = std::filesystem;

	//Find executable path
	wxFileName exePath(wxStandardPaths::Get().GetExecutablePath());
	glbExeDir = (exePath.GetPath() + wxFileName::GetPathSeparator()).ToStdWstring();

	auto PyPath = glbExeDir / "python311";
	Py_SetPythonHome(PyPath.wstring().c_str());

	//if m_PyHome does not point to a valid directory, ScienceSuit will NOT start
	Py_Initialize();
	
	wxFrame* frm = new frmPyPkgMngr(PyPath);
	frm->Show();
	frm->Maximize();

    return true;
}
