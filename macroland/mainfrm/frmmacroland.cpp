#include "frmmacroland.h" 

#include <codecvt>
#include <locale>

#include <wx/artprov.h>
#include <wx/sstream.h>
#include <wx/txtstrm.h>
#include <wx/dir.h>

#include <Python.h> 

#include "../lua/luautil.h"

#include "../scripting/pnlcmdwnd.h"


#include "../icons/mainframeicon.xpm"
#include "../consts.h"
#include "../icell/workbook.h"
#include "../util_funcs.h"

#include "frmextensionmngr.h"


ICELL::CWorkbook* glbWorkbook{nullptr};
extern std::filesystem::path glbExeDir;
extern lua_State* glbLuaState;




static bool Check_SciSuitPkg(std::string pkgName)
{
	std::string PythonCmd = "import pkgutil \n"
		"x = pkgutil.iter_modules() \n"
		"SCISUIT = False \n"
		"for i in x: \n"
		"    if (i.ispkg==True and i.name ==\""+pkgName + "\"):\n";
		
	PythonCmd +=
		"        SCISUIT=True \n"
		"        break";

	auto Module = PyModule_New("Check_SciSuitPkg");
	auto Dict = PyModule_GetDict(Module);

	bool IsInstalled = false;

	auto ResultObj = PyRun_String(PythonCmd.c_str(), Py_file_input, Dict, Dict);
	if (ResultObj)
	{
		PyObject* wxObj = PyDict_GetItemString(Dict, "SCISUIT");
		IsInstalled = Py_IsTrue(wxObj);
	}

	Py_XDECREF(Dict);
	Py_XDECREF(Module);

	return IsInstalled;
}



/**********************************************************************************/


frmMacroLand::frmMacroLand(const std::filesystem::path & ProjectPath):
	wxFrame(nullptr, wxID_ANY,"" ), m_IsDirty{false}
{
	
	for(std::string pkgName: {"scisuit", "wx"})
	{
		if (!Check_SciSuitPkg(pkgName))
		{
			std::string msg = pkgName + " is missing. \n"
			"Please simply run Python Package Manager App to install it.";

			throw std::exception(msg.c_str());
		}
	}


	wxTheApp->SetTopWindow(this);

	m_ProjFile = ProjectPath;
	m_Mode = m_ProjFile.empty() ? MODE::NEWPROJ : MODE::OPENPROJ;

	SetSizeHints(wxDefaultSize, wxDefaultSize); 

	wxIcon AppIcon(mainframeicon_xpm);
	if (AppIcon.IsOk())
		SetIcon(AppIcon);

	
	if (m_Mode == MODE::OPENPROJ)
	{
		if (!CreateLockFile())
			throw std::runtime_error("Could not create .lock file.");

		SetTitle(std::string(consts::VERSION) + L" - " + m_ProjFile.wstring());
	}

	else if (m_Mode == MODE::NEWPROJ)
		SetTitle(consts::VERSION);


	m_Notebook = new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_BOTTOM);
	m_Notebook->SetOwnFont(m_Notebook->GetFont().MakeBold());


	/********************      Create the snapshot directory        ***************************/

	m_ProjDate = util::CDate();

	if (!CreateSnapshotDir())
		throw std::runtime_error("Cannot write to SCISUIT/temp directory. Aborting...");


	m_Workbook = new ICELL::CWorkbook(m_Notebook);

	if (m_Mode == MODE::OPENPROJ)
	{
		util::UnpackArchiveFile(m_ProjFile, m_SnapshotDir);
		m_Workbook->Read(m_SnapshotDir);
	}
	else
		m_Workbook->AddNewWorksheet();

	m_Workbook->Show();
	glbWorkbook = m_Workbook;
	glbWorkbook->MarkClean();

	m_CmdWnd = new scripting::cmdedit::pnlCommandWindow(m_Notebook);

	/************************Create Status Bar******************/
	m_StBar = new InteractiveStatusBar(this, wxID_ANY, wxST_SIZEGRIP);
	m_StBar->SetFieldsCount(3);
	m_StBar->SetFont(wxFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, "Arial"));

	m_StBar->CatchStatBarMouseEvents(false, false, false, true);

	int widths[] = { -2,-4,-1 };
	m_StBar->SetStatusWidths(3, widths); 
	m_StBar->SetStatusText(consts::COPYRIGHT, 0);
	SetStatusBar(m_StBar);


	/*********************** Add to framework ***********************************/
	auto extMngr = new pnlExtensionMngr(m_Notebook);

	m_Notebook->AddPage(m_Workbook, "Workbook");
	m_Notebook->AddPage(m_CmdWnd, "Command Window");
	m_Notebook->AddPage(extMngr, "Extension Manager");
	m_TopBar = new CTopBar(this);
	
	auto szrMain = new wxBoxSizer(wxVERTICAL);
	szrMain->Add(m_TopBar, 0, wxEXPAND);
	szrMain->Add(m_Notebook, 1, wxEXPAND);
	SetSizer(szrMain);
	Layout();


	Bind(wxEVT_CLOSE_WINDOW, &frmMacroLand::OnClose, this);
	m_StBar->Bind(ssEVT_STATBAR_RIGHT_UP, &frmMacroLand::StBar_OnRightUp, this);

	RunInitPyFile();
	RunLuaExtensions();
	
	Maximize();
}




void frmMacroLand::RunInitPyFile()
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> cvt;
	auto Path = glbExeDir / "_init.py";

	if (std::filesystem::exists(Path))
	{
		if (auto cp = _Py_wfopen(Path.wstring().c_str(), L"rb"))
			PyRun_SimpleFileExFlags(cp, cvt.to_bytes(Path.wstring()).c_str(), true, 0);
	}
}


void frmMacroLand::RunLuaExtensions()
{
	lua::RunExtensions(glbLuaState, "ws_tbar.lua");

	lua_setglobal(glbLuaState, "ACTIVEWIDGET");

	lua_pushnil(glbLuaState);
	lua_setglobal(glbLuaState, "ACTIVEWIDGET");
}



frmMacroLand::~frmMacroLand()
{
	namespace fs = std::filesystem;

	if (m_LockFileStream.is_open())
	{
		m_LockFileStream.close();

		std::error_code ErrCode;
		bool IsRemoved = fs::remove(m_LockFile);

		if (!IsRemoved)
			wxMessageBox(ErrCode.message());
	}

	if (fs::exists(m_SnapshotDir))
	{
		std::error_code ErrCode;
		auto NFilesDirs = fs::remove_all(m_SnapshotDir, ErrCode);

		if (NFilesDirs == 0)
			wxMessageBox(ErrCode.message());
	}

	lua_close(glbLuaState);
	Py_Finalize();

	/*
	We are using scisuit Python package for plotting
	When any plot window is shown (regardless of how, command line, app, charts toolbar...)
	and the mainframe is exited, macroland.exe still is shown in Windows Task Manager as running

	The following command forcefully kills this process
*/
	auto PID = wxGetProcessId();
	wxExecute("taskkill /f /pid " + std::to_string(PID));
}

 

void frmMacroLand::OnClose(wxCloseEvent &event)
{
	if (m_IsDirty)
	{

		int ans = wxMessageBox(
			"Save commits before exiting?",
			"Save",
			wxYES_NO | wxCANCEL);


		if (ans == wxNO || ans == wxCANCEL)
		{
			//closes mainframe (exits)
			if (ans == wxNO)
			{
				wxExecute("taskkill /IM \"macroland.exe\" /F");
				event.Skip();
			}

			return;
		}


		//Project file exists - can proceed to save
		if (!m_ProjFile.empty())
			WriteProjFile();

		else
		{
			//NO project file. Ask to create one
			wxFileDialog dlgSave(this, "Save Project", "", "", "project file (*.sproj)|*.sproj", wxFD_SAVE);
			int ans = dlgSave.ShowModal();

			if (ans == wxID_CANCEL)
				return;

			m_ProjFile = dlgSave.GetPath().ToStdWstring();

			WriteProjFile();

			//register the project path to recent projects
			wxFile file((glbExeDir / consts::HOME / consts::RECENTPROJ).wstring(), wxFile::write_append);
			file.Write(m_ProjFile.wstring(), wxConvUTF8);
			file.Close();
		}
	}

	event.Skip();
}



void frmMacroLand::MarkClean()
{
	m_IsDirty = false;

	//Mark all saveable parts clean
	m_Workbook->MarkClean();
}



void frmMacroLand::StBar_OnRightUp(StatBarMouseEvent& event)
{
	m_StBar_RectField = event.GetRectFieldNumber();

	m_StatBarMenu = std::make_unique<wxMenu>();
	
	lua_pushliteral(glbLuaState, "WS_STBAR_MENU");
	lua_setglobal(glbLuaState, "ACTIVEWIDGET");

	lua::RunExtensions(glbLuaState, "ws_stbar_menu.lua");

	lua_pushnil(glbLuaState);
	lua_setglobal(glbLuaState, "ACTIVEWIDGET");

	if (m_StatBarMenu->GetMenuItemCount() > 0)
		m_StBar->PopupMenu(m_StatBarMenu.get());

	event.Skip();
}



void frmMacroLand::Save()
{	
	bool ProjExists = !m_ProjFile.empty();
	
	if (!ProjExists)
	{
		wxFileDialog dlg(this,
			_("Save Project As _?"), "", "",
			_("Science Suit Project (*.sproj)|*.sproj"), 
			wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

		if (dlg.ShowModal() == wxID_CANCEL)
			return;

		m_ProjFile = dlg.GetPath().ToStdWstring();

		SetTitle(GetTitle() + " - "+ m_ProjFile.wstring());

		CreateLockFile();

		wxFile file((glbExeDir / consts::HOME / consts::RECENTPROJ).wstring(), wxFile::write_append);
		file.Write(m_ProjFile.wstring(), wxConvUTF8);
		file.Close();
	}
		
	WriteProjFile();
	MarkClean();

	util::CDate timeInfo;
	m_StBar->SetStatusText("Last Saved: " + timeInfo.GetTime(":"));
}



void frmMacroLand::WriteProjFile()
{
	namespace fs = std::filesystem;

	//path to create the temporary project (.sproj) file
	fs::path TempFile = glbExeDir / consts::TEMPDIR / m_ProjFile.filename().wstring();


	//pack contents of snapshot directory as project file (.sproj)
	util::ArchiveFolder(m_SnapshotDir, TempFile);


	//Copy temporary project file to original project location
	std::error_code CopyErr;
	if (bool Copied = fs::copy_file(TempFile, m_ProjFile, fs::copy_options::overwrite_existing, CopyErr))
	{
		if (!Copied)
		{
			wxMessageBox("Could not save the project, due to:" + CopyErr.message());
			return;
		}

		std::error_code RmErr;
		if (bool Removed = fs::remove(TempFile, RmErr))
		{
			if (!Removed)
				wxMessageBox("Could not remove the temp .sproj file, due to: " + RmErr.message());
		}
	}


	/*
		The snapshot directory has a name of UnsavedProject -- timestamp
		Rename it so that it will be project files name, such as abc -- timestamp
	*/
	if (m_Mode == MODE::NEWPROJ) 
	{
		auto TimeStamp = m_ProjDate.GetDate("", true) + m_ProjDate.GetTime("");
		auto ProjDir = glbExeDir / consts::TEMPDIR / m_ProjFile.stem().concat(" -- ").concat(TimeStamp);

		if (!fs::exists(m_SnapshotDir)) 
		{
			wxMessageBox("Contents of " + std::string(consts::TEMPDIR) + "directory missing. Restart the project!!!", "Danger!");
			return;
		}

		//rename the snapshot directory starting with "UnsavedProject -- " with the project name itself
		fs::rename(m_SnapshotDir, ProjDir);
			
		//update snapshot path after renaming
		m_SnapshotDir = ProjDir;

		//update the status as if it is an opened project (has a project file now)
		m_Mode = MODE::OPENPROJ;	
	}
}


bool frmMacroLand::CreateLockFile()
{
	m_LockFile = m_ProjFile.parent_path() / m_ProjFile.stem().concat(".lock");

	//Project file in use
	if(std::filesystem::exists(m_LockFile))
		return false;

	//Just create the file (cant be done with std::filesystem)
	m_LockFileStream.open(m_LockFile);
	
	return true;
}



bool frmMacroLand::CreateSnapshotDir()
{
	namespace fs = std::filesystem;

	//if there is no temporary directory create one
	if (!fs::exists(glbExeDir / consts::TEMPDIR))
		fs::create_directory(glbExeDir / consts::TEMPDIR);
	
	
	auto TimeStamp = m_ProjDate.GetDate("", true) + m_ProjDate.GetTime("");

	m_SnapshotDir = glbExeDir / consts::TEMPDIR;
	if (!m_ProjFile.empty())
		m_SnapshotDir /= (m_ProjFile.stem()).concat(" -- ").concat(TimeStamp);
	else
		m_SnapshotDir /= (std::string("UnsavedProject -- ") + TimeStamp);


	return fs::create_directory(m_SnapshotDir);
}