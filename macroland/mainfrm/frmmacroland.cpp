#include "frmmacroland.h" 

#include <codecvt>
#include <locale>
#include <algorithm>

#include <wx/artprov.h>
#include <wx/sstream.h>
#include <wx/txtstrm.h>
#include <wx/dir.h>

#include <Python.h> 

#include "../cmdwnd/pnlcmdwnd.h"
#include "../icons/mainframeicon.xpm"
#include "../consts.h"

#include "../util/util_wx.h"
#include "../util/util_string.h"
#include "../util/util_python.h"

#include "icell.h"


ICELL::CWorkbook* glbWorkbook{nullptr};
extern std::filesystem::path glbExeDir;
extern JSON::Value glbSettings;



frmMacroLand::frmMacroLand(const std::filesystem::path & ProjectPath):
	wxFrame(nullptr, wxID_ANY,"" ), m_IsDirty{false}
{
	wxTheApp->SetTopWindow(this);

	m_ProjFile = ProjectPath;
	m_Mode = m_ProjFile.empty() ? MODE::NEWPROJ : MODE::OPENPROJ;

	wxIcon AppIcon(mainframeicon_xpm);
	if (AppIcon.IsOk())
		SetIcon(AppIcon);

	std::string Title = std::string("MacroLand v") + Info::VERSION + " " + Info::RELEASEDATE;
	if (m_Mode == MODE::OPENPROJ)
	{
		if (!CreateLockFile())
			throw std::runtime_error("Could not create .lock file.");

		SetTitle(Title + L" - " + m_ProjFile.wstring());
	}

	else if (m_Mode == MODE::NEWPROJ)
		SetTitle(Title);


	//Create Main Notebook based on preferences from settings.json	
	auto JSONObject = glbSettings.as_object();

	auto TabPos = wxNB_LEFT;
	if(JSONObject.contains("tabPosition") && JSONObject["tabPosition"].is_object()) 
	{
		auto TabPosObj = JSONObject["tabPosition"].as_object();
		if(TabPosObj.contains("pos") && TabPosObj["pos"].is_string())
		{
			auto Str = TabPosObj["pos"].as_string();
			if(Str == "right") TabPos=wxNB_RIGHT;
			else if(Str == "bottom") TabPos = wxNB_BOTTOM;
			else if(Str == "top") TabPos = wxNB_TOP;
		}
	}

	m_Notebook = new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, TabPos);


	/********************      Create the snapshot directory        ***************************/

	using namespace std::chrono;
	const auto now = time_point_cast<seconds>(zoned_time(current_zone(), system_clock::now()).get_local_time());
	m_ProjDate = std::format("{:%Y%m%d%H%M%S}", now);

	if (!CreateSnapshotDir())
		throw std::runtime_error("Cannot write to SCISUIT/temp directory. Aborting...");


	//Create Workbook and load project file (if any requested)
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

	//Create Command Window
	m_CmdWnd = new cmdedit::pnlCommandWindow(m_Notebook);


	//Load Recent files list
	if(std::filesystem::exists(glbExeDir / Info::HOMEDIR / Info::RECENTPROJ))
	{
		JSON::JSON json(glbExeDir / Info::HOMEDIR / Info::RECENTPROJ);

		JSON::Error err;
		auto Val = json.Parse(err);
		if(!err.failed)
			m_RecentFilesArr = Val.as_array();
	}

	//Start creating menus
	m_FileMenu = new wxMenu();
	
	auto Item = m_FileMenu->Append(ID_PROJ_SAVE, "Save Commits", "Save commits to project file");
	Item->SetBitmap(wxArtProvider::GetBitmap(wxART_FILE_SAVE));

	Item = m_FileMenu->Append(ID_PROJ_OPEN, "Open Project", "Open .proj file in a new instance");
	Item->SetBitmap(wxArtProvider::GetBitmap(wxART_FILE_OPEN));

	m_FileMenu->AppendSeparator(); //after this it is the recent files menu

	m_WindowsMenu = new wxMenu();
	Item = m_WindowsMenu->Append(ID_FULLSCREEN, "Full Screen", "Turn on/off full screen");
	m_WindowsMenu->Bind(wxEVT_MENU, [this](wxCommandEvent&)
	{
		ShowFullScreen(true);
	}, ID_FULLSCREEN);

	m_FileMenu->Bind(wxEVT_MENU, [this](wxCommandEvent&) {Save(); }, ID_PROJ_SAVE);
	m_FileMenu->Bind(wxEVT_MENU_OPEN, &frmMacroLand::OnFileMenuOpen, this);
	m_FileMenu->Bind(wxEVT_MENU, &frmMacroLand::OnOpenProject, this, ID_PROJ_OPEN);

	
	/************* Create Menu Bar  ************/
	m_menubar = new wxMenuBar( 0 );
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
	m_StBar->SetStatusText(wxString::FromUTF8(Info::COPYRIGHT), 0);
	SetStatusBar(m_StBar);


	/*********************** Add to framework ***********************************/

	m_Notebook->AddPage(m_Workbook, "Workbook");
	m_Notebook->AddPage(m_CmdWnd, "Command");
	
	auto szrMain = new wxBoxSizer(wxVERTICAL);
	szrMain->Add(m_Notebook, 1, wxEXPAND);
	SetSizer(szrMain);
	Layout();


	Maximize();

	

	//Create a web request to download and check if new version is available
	auto WebRequest = wxWebSession::GetDefault().CreateRequest(this,
			"https://www.pebytes.com/downloads/newversion.json");

	bool autoUpdate = true;
	if(JSONObject.contains("autoUpdate")) {
		if(auto UpdateObj = JSONObject["autoUpdate"].is_bool())
			autoUpdate = JSONObject["autoUpdate"].as_bool();
	}
	if(autoUpdate)
		WebRequest.Start();


	//Bind all events
	Bind(wxEVT_WEBREQUEST_STATE, &frmMacroLand::OnCheckNewVersion, this);
	Bind(wxEVT_CLOSE_WINDOW, &frmMacroLand::OnClose, this);
	m_StBar->Bind(ssEVT_STATBAR_RIGHT_UP, &frmMacroLand::StBar_OnRightUp, this);


	util::RunPythonFile(glbExeDir / "_frmwork_.py");
	util::RunExtensions();
	util::RunPythonFile(glbExeDir / Info::HOMEDIR / "_init_.py");
}



frmMacroLand::~frmMacroLand()
{
	namespace fs = std::filesystem;

	if (m_LockFileStream.is_open())
	{
		m_LockFileStream.close();

		std::error_code ErrCode;
		bool IsRemoved = fs::remove(m_LockFile, ErrCode);

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

	int ans = wxMessageBox("Save before exiting?", "Save?", wxYES_NO | wxCANCEL);
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

		AppendToRecentFilesArray(m_ProjFile);
		//write it so that the opening project can see it
		JSON::JSON::Write(m_RecentFilesArr, glbExeDir / Info::HOMEDIR / Info::RECENTPROJ);
	}

	glbWorkbook->Enable(false);
	m_CmdWnd->Enable(false);
	Enable(false);

	event.Skip();
}

void frmMacroLand::OnCheckNewVersion(wxWebRequestEvent &event)
{
	switch (event.GetState())
    {
        case wxWebRequest::State_Completed:
        {
           	wxInputStream* Input = event.GetResponse().GetStream();
			wxTextInputStream text(*Input );
		
			std::string str;
			while(Input->IsOk() && !Input->Eof())
				str += text.ReadLine().Trim().Trim(false).utf8_string() + "\n";
				
			auto json = JSON::JSON(str);
			JSON::Error ec;
			auto jsval = json.Parse(ec);
			if(ec.failed)
				return;
			auto Map = jsval.as_object();

			auto t = std::thread([Map]
			{
				bool AnyNewVersion = false; //
				std::string URL, Message, NewVersionInfo;
			for(const auto& s: Map) 
			{
				auto id = s.first;
				auto value = s.second;
				if(id == "VERSION") 
				{
					auto NewOne = value.as_string();
					NewVersionInfo = util::trim(NewOne);
					std::string CurVer = Info::VERSION;

					auto pos = std::find(std::begin(NewOne), std::end(NewOne), '.');
					if(pos != std::end(NewOne))
    					NewOne.erase(pos, pos+1);

					pos = std::find(std::begin(CurVer), std::end(CurVer), '.');
					if(pos != std::end(CurVer))
    					CurVer.erase(pos, pos+1);
					
					if(std::stoi(NewOne)>std::stoi(CurVer))
							AnyNewVersion = true;
			
					if(!AnyNewVersion) break;
				}
				if(id == "URL") 
				{
					auto strval = value.as_string();
					URL = util::trim(strval);
				}
				if(id == "INFO") 
				{
					auto arr = value.as_array();
					std::string s;
					for(const auto& v: arr)
						s += v.as_string() + "\n";
					Message = s;
				}
			}

			if(AnyNewVersion)
			{
				auto Prompt = "Version " + NewVersionInfo + " is available. \n \n";
				Prompt += Message + "\n \n";

				Prompt += "Would you like to download now?";

				auto Ans = wxMessageBox(wxString::FromUTF8(Prompt), "New Version Available!", wxYES_NO);
				if(Ans == wxYES)
					wxLaunchDefaultBrowser(URL);
				//glbWorkbook->Refresh();
			}
			});
			t.detach();
			break;
		}
       
        case wxWebRequest::State_Failed:
            break;
    }
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

	for (const auto& p : m_RecentFilesArr.data())
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		auto s = p.as_string();
		auto path = std::filesystem::path(converter.from_bytes(s));
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

	AppendToRecentFilesArray(ProjPath);

	//write it so that the opening project can see it
	JSON::JSON::Write(m_RecentFilesArr, glbExeDir / Info::HOMEDIR / Info::RECENTPROJ);
}


void frmMacroLand::AppendToRecentFilesArray(const std::filesystem::path &ProjPath)
{
	bool Exists = false;
	for(const auto& e: m_RecentFilesArr.data())
	{
		Exists = std::filesystem::u8path(e.as_string()) == ProjPath;
		if(Exists) break;
	}
	if(!Exists)
		m_RecentFilesArr.push_back((const char *)ProjPath.generic_u8string().data());
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

	CallRegisteredPyFunc("rightclick_" + std::to_string(m_StBar_RectField));

	if (m_StatBarMenu->GetMenuItemCount() > 0)
		m_StBar->PopupMenu(m_StatBarMenu.get());

	event.Skip();
}



void frmMacroLand::BindPyFunc(
			std::string EventName, 
			std::unique_ptr<Python::CEventCallbackFunc> Callbackfunc)
{
	
	if (!m_EvtCallBack.contains(EventName))
		m_EvtCallBack[EventName] = std::list<std::unique_ptr<Python::CEventCallbackFunc>>();
	
	m_EvtCallBack[EventName].push_back(std::move(Callbackfunc));
}



void frmMacroLand::UnbindPyFunc(
			std::string EventName, 
			PyObject* FunctionObj)
{
	if (m_EvtCallBack.find(EventName) != m_EvtCallBack.end())
	{
		auto& List = m_EvtCallBack[EventName];
		std::erase_if(List, [&](auto& elem)
		{
			return elem->m_Func == FunctionObj;
		});
	}
}


void frmMacroLand::CallRegisteredPyFunc(const std::string& event)
{
	if (m_EvtCallBack[event].size() > 0) {
		const auto& List = m_EvtCallBack[event];
		for (const auto& CallBk : List)
			CallBk->call(CallBk->m_Func, CallBk->m_Args);
	}
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

		AppendToRecentFilesArray(m_ProjFile);
		//write it so that the opening project can see it
		JSON::JSON::Write(m_RecentFilesArr, glbExeDir / Info::HOMEDIR / Info::RECENTPROJ);
	}
		
	WriteProjFile();
	MarkClean();

	using namespace std::chrono;
	const auto now = time_point_cast<seconds>(
			zoned_time(current_zone(), system_clock::now()).get_local_time()); 
	m_StBar->SetStatusText("Last Saved: " + std::format("{:%H:%M:%S}", now));
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

		auto ProjDir = glbExeDir / Info::TEMPDIR / m_ProjFile.stem().concat(" -- ").concat(m_ProjDate);

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

	m_SnapshotDir = glbExeDir / Info::TEMPDIR;
	if (!m_ProjFile.empty())
		m_SnapshotDir /= (m_ProjFile.stem()).concat(" -- ").concat(m_ProjDate);
	else
		m_SnapshotDir /= (std::string("UnsavedProject -- ") + m_ProjDate);


	return fs::create_directory(m_SnapshotDir);
}