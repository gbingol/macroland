#include "frmmacroland.h" 

#include <codecvt>
#include <locale>


#include <wx/artprov.h>
#include <wx/sstream.h>
#include <wx/txtstrm.h>
#include <wx/dir.h>

#include <Python.h> 

#include "../lua/luautil.h"

#include "../cmdwnd/pnlcmdwnd.h"


#include "../icons/mainframeicon.xpm"
#include "../consts.h"
#include "../util_funcs.h"

#include "icell.h"


ICELL::CWorkbook* glbWorkbook{nullptr};
extern std::filesystem::path glbExeDir;
extern lua_State* glbLuaState;




frmMacroLand::frmMacroLand(const std::filesystem::path & ProjectPath):
	wxFrame(nullptr, wxID_ANY,"" ), m_IsDirty{false}
{
	wxTheApp->SetTopWindow(this);

	m_ProjFile = ProjectPath;
	m_Mode = m_ProjFile.empty() ? MODE::NEWPROJ : MODE::OPENPROJ;

	wxIcon AppIcon(mainframeicon_xpm);
	if (AppIcon.IsOk())
		SetIcon(AppIcon);

	
	if (m_Mode == MODE::OPENPROJ)
	{
		if (!CreateLockFile())
			throw std::runtime_error("Could not create .lock file.");

		SetTitle(std::string(Info::VERSION) + L" - " + m_ProjFile.wstring());
	}

	else if (m_Mode == MODE::NEWPROJ)
		SetTitle(Info::VERSION);


	m_Notebook = new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_LEFT);


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

	m_CmdWnd = new cmdedit::pnlCommandWindow(m_Notebook);


	/************* Create Menu Bar  ************/
	m_menubar = new wxMenuBar( 0 );

	m_RecentFiles = std::make_unique<util::CRecentFiles>(glbExeDir / Info::HOMEDIR / Info::RECENTPROJ);
	m_RecentFiles->ReadOrCreate();

	m_FileMenu = new wxMenu();
	
	auto Item = m_FileMenu->Append(ID_PROJ_SAVE, "Save Commits", "Save commits to project file");
	Item->SetBitmap(wxArtProvider::GetBitmap(wxART_FILE_SAVE));

	Item = m_FileMenu->Append(ID_PROJ_OPEN, "Open Project", "Open .proj file in a new instance");
	Item->SetBitmap(wxArtProvider::GetBitmap(wxART_FILE_OPEN));

	m_FileMenu->AppendSeparator(); //after this it is the recent files menu

	m_WindowsMenu = new wxMenu();
	bool IsFull = IsFullScreen();
	Item = m_WindowsMenu->Append(ID_FULLSCREEN, "Full Screen", "Turn on/off full screen", wxITEM_CHECK);
	Item->Check(IsFull);

	m_WindowsMenu->Bind(wxEVT_MENU, [this, IsFull](wxCommandEvent&)
	{
		ShowFullScreen(!IsFull);
	});

	m_FileMenu->Bind(wxEVT_MENU, [this](wxCommandEvent&) {Save(); }, ID_PROJ_SAVE);
	m_FileMenu->Bind(wxEVT_MENU_OPEN, &frmMacroLand::OnFileMenuOpen, this);
	m_FileMenu->Bind(wxEVT_MENU, &frmMacroLand::OnOpenProject, this, ID_PROJ_OPEN);


	m_menubar->Append( m_FileMenu, "File");
	m_menubar->Append( m_WindowsMenu, "Windows");
	SetMenuBar( m_menubar );

	/************************Create Status Bar******************/
	m_StBar = new InteractiveStatusBar(this, wxID_ANY, wxST_SIZEGRIP);
	m_StBar->SetFieldsCount(3);
	m_StBar->SetFont(wxFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, "Arial"));

	m_StBar->CatchStatBarMouseEvents(false, false, false, true);

	int widths[] = { -2,-4,-1 };
	m_StBar->SetStatusWidths(3, widths); 
	m_StBar->SetStatusText(Info::COPYRIGHT, 0);
	SetStatusBar(m_StBar);


	/*********************** Add to framework ***********************************/

	m_Notebook->AddPage(m_Workbook, "Workbook");
	m_Notebook->AddPage(m_CmdWnd, "Command");
	
	auto szrMain = new wxBoxSizer(wxVERTICAL);
	szrMain->Add(m_Notebook, 1, wxEXPAND);
	SetSizer(szrMain);
	Layout();


	Bind(wxEVT_CLOSE_WINDOW, &frmMacroLand::OnClose, this);
	m_StBar->Bind(ssEVT_STATBAR_RIGHT_UP, &frmMacroLand::StBar_OnRightUp, this);

	Maximize();

	auto thr = std::thread([this]()
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		CallAfter([this]
		{
			RunLuaExtensions();
		});	
	});
	thr.detach();
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

	/*
	We are using scisuit Python package for plotting
	When any plot window is shown (regardless of how, command line, app, charts toolbar...)
	and the mainframe is exited, macrolandapp.exe still is shown in Windows Task Manager as running

	The following command forcefully kills this process
*/
	auto PID = wxGetProcessId();
	wxExecute("taskkill /f /pid " + std::to_string(PID));

	/*
		wxExecute is async therefore, PyFinalize_Ex still have 
		time to execute.
		If it causes any crash (it might cause when wxPython shows a dialog and user want to exit MacroLand) 
		then the operating system will kill the task anyway
	*/
	Py_FinalizeEx();
}

 

void frmMacroLand::OnClose(wxCloseEvent &event)
{
	/*
		when wxPython shows a dialog and user want to exit MacroLand
		it prevents MacroLand from exiting until the dialog is closed. 
		However, at this stage the frame still is active and seems alive
		(which does not make sense as it is progressing towards a close)

		Therefore we disable all actions (Frame and glbWorkbook). 
		Now the MacroLand App looks in inactive state and there is not much 
		user can do except closing the dialog.
	*/

	if (!m_IsDirty) 
	{
		glbWorkbook->Enable(false);
		m_CmdWnd->Enable(false);
		Enable(false);	

		event.Skip();
		return;
	}

	int ans = wxMessageBox(
		"Save commits before exiting?",
		"Save",
		wxYES_NO | wxCANCEL);

	if (ans == wxNO || ans == wxCANCEL) {
		//closes mainframe (exits)
		if (ans == wxNO)
			event.Skip();
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
		wxFile file((glbExeDir / Info::HOMEDIR / Info::RECENTPROJ).wstring(), wxFile::write_append);
		file.Write(m_ProjFile.wstring(), wxConvUTF8);
		file.Close();
	}

	glbWorkbook->Enable(false);
	m_CmdWnd->Enable(false);
	Enable(false);

	event.Skip();
}


void frmMacroLand::OnFileMenuOpen(wxMenuEvent& event)
{
	/*
		Recent Project Files menu is part of File menu (file menu is the parent)
		therefore when Recent Project Files menu opens, OnFileMenu is called again
		and this causes problems (such as crash)

		The following code avoids this
	*/
	if(event.GetEventObject() == m_RecentProjMenu)
		return;

	if (m_RecentProjMenu)
		m_FileMenu->Destroy(ID_RECENTPROJ);


	m_RecentProjMenu = new wxMenu();

	for (const auto& path : m_RecentFiles->GetList())
	{
		if (!std::filesystem::exists(path))
			continue;

		int ID = wxNewId();

		m_RecentProjMenu->Append(ID, path.wstring());
		m_RecentProjMenu->Bind(wxEVT_MENU, [this, path](wxCommandEvent& CmdEvt)
		{
			try
			{
				ExecuteProjFile(path.wstring());
			}
			catch (const std::exception& e)
			{
				wxMessageBox(e.what());
			}
		}, ID);
	}

	m_FileMenu->Append(ID_RECENTPROJ, "Recent Projects", m_RecentProjMenu);

	//Enable/disable menu items
	m_FileMenu->Enable(ID_PROJ_SAVE, isDirty());
}



void frmMacroLand::OnOpenProject(wxCommandEvent &event)
{
	wxFileDialog dlg(this, "Open Project", "", "", "MacroLand Project (*.sproj)|*.sproj", wxFD_OPEN);

	if (dlg.ShowModal() != wxID_OK)
		return;

	try {
		wxString FilePath = dlg.GetPath();
		ExecuteProjFile(FilePath.ToStdWstring());
	}
	catch (const std::exception& e) {
		wxMessageBox(e.what());
	}
}



void frmMacroLand::ExecuteProjFile(const std::filesystem::path& ProjPath)
{
	const auto LockFile = ProjPath.parent_path() / (ProjPath.stem().wstring() + L".lock");
	if (std::filesystem::exists(LockFile))
		throw std::exception("Project is already open (.lock file exists)");

	auto Exe = glbExeDir / "macrolandapp.exe";
	if(!std::filesystem::exists(Exe))
	{
		wxMessageBox(Exe.wstring() + L" does not exist");
		return;
	}
	
	wxString Cmd = L"\"" + Exe.wstring() + L"\"" + L"  " + L"\"" + ProjPath.wstring() + L"\"";
	wxExecute(Cmd, wxEXEC_ASYNC);

	m_RecentFiles->Append(ProjPath);
	m_RecentFiles->Write();
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

		wxFile file((glbExeDir / Info::HOMEDIR / Info::RECENTPROJ).wstring(), wxFile::write_append);
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
	fs::path TempFile = glbExeDir / Info::TEMPDIR / m_ProjFile.filename().wstring();


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
		using namespace std::string_literals;

		auto TimeStamp = m_ProjDate.GetDate("", true) + m_ProjDate.GetTime("");
		auto ProjDir = glbExeDir / Info::TEMPDIR / m_ProjFile.stem().concat(" -- ").concat(TimeStamp);

		if (!fs::exists(m_SnapshotDir)) 
		{
			wxMessageBox("Contents of "s + Info::TEMPDIR + "directory missing. Restart the project!!!", "Important!");
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
	if (!fs::exists(glbExeDir / Info::TEMPDIR))
		fs::create_directory(glbExeDir / Info::TEMPDIR);
	
	
	auto TimeStamp = m_ProjDate.GetDate("", true) + m_ProjDate.GetTime("");

	m_SnapshotDir = glbExeDir / Info::TEMPDIR;
	if (!m_ProjFile.empty())
		m_SnapshotDir /= (m_ProjFile.stem()).concat(" -- ").concat(TimeStamp);
	else
		m_SnapshotDir /= (std::string("UnsavedProject -- ") + TimeStamp);


	return fs::create_directory(m_SnapshotDir);
}