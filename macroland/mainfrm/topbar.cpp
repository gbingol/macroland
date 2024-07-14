#include "topbar.h"

#include <wx/artprov.h>

#include "frmmacroland.h"
#include "frmextensionmngr.h"
#include "../consts.h"


namespace
{
	wxPoint MenuPos(const wxControl* control)
	{
		wxPoint BL = control->GetRect().GetBottomLeft();
		return control->ToPhys(BL);
	}
}


extern std::filesystem::path glbExeDir;


CTopBar::CTopBar(frmMacroLand* parent, wxWindowID id) : wxPanel(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL)
{
	m_ParentWnd = parent;

	m_File = new wxHyperlinkCtrl(this, wxID_ANY, "File", "");
	m_Tools = new wxHyperlinkCtrl(this, wxID_ANY, "Tools", "");
	m_Windows = new wxHyperlinkCtrl(this, wxID_ANY, "Windows", "");

	auto szrMain = new wxBoxSizer(wxHORIZONTAL);
	szrMain->Add(m_File, 0, wxRIGHT | wxLEFT, 10);
	szrMain->Add(m_Tools, 0, wxRIGHT | wxLEFT, 10);
	szrMain->Add(m_Windows, 0, wxRIGHT | wxLEFT, 10);
	SetSizer(szrMain);
	Layout();


	InitFileMenu();

	m_File->Bind(wxEVT_HYPERLINK, &CTopBar::OnFile, this);
	m_Tools->Bind(wxEVT_HYPERLINK, &CTopBar::OnTools, this);
	m_Windows->Bind(wxEVT_HYPERLINK, &CTopBar::OnWindows, this);
}


CTopBar::~CTopBar() = default;


void CTopBar::InitFileMenu()
{
	m_RecentFiles = std::make_unique<util::CRecentFiles>(glbExeDir / consts::HOME / consts::RECENTPROJ);
	m_RecentFiles->ReadOrCreate();

	m_FileMenu = new wxMenu();
	auto Item = m_FileMenu->Append(ID_PROJ_SAVE, "Save Commits", "Save commits to project file");
	Item->SetBitmap(wxArtProvider::GetBitmap(wxART_FILE_SAVE));

	Item = m_FileMenu->Append(ID_PROJ_OPEN, "Open Project", "Open .proj file in a new instance");
	Item->SetBitmap(wxArtProvider::GetBitmap(wxART_FILE_OPEN));

	m_FileMenu->AppendSeparator();

	m_FileMenu->Bind(wxEVT_MENU, [this](wxCommandEvent&) {m_ParentWnd->Save(); }, ID_PROJ_SAVE);
	m_FileMenu->Bind(wxEVT_MENU, &CTopBar::OnOpen, this, ID_PROJ_OPEN);
}


void CTopBar::OnFile(wxHyperlinkEvent& event)
{
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
	m_FileMenu->Enable(ID_PROJ_SAVE, m_ParentWnd->isDirty());
	
	//Show the menu
	PopupMenu(m_FileMenu, MenuPos(m_File));
}



void CTopBar::OnTools(wxHyperlinkEvent& event)
{
	wxMenu menu;
	auto Item = menu.Append(ID_EXTMNGR, "Extension Manager", "Manage extensions using extension manager");
	Item->SetBitmap(wxArtProvider::GetBitmap(wxART_HARDDISK));

	menu.Bind(wxEVT_MENU, &CTopBar::OnExtensionMngr, this, ID_EXTMNGR);

	//Show the menu
	PopupMenu(&menu, MenuPos(m_Tools));
}


void CTopBar::OnWindows(wxHyperlinkEvent& event)
{
	bool IsFull = m_ParentWnd->IsFullScreen();

	wxMenu menu;
	auto Item = menu.Append(ID_FULLSCREEN, "Full Screen", "Turn on/off full screen", wxITEM_CHECK);
	Item->Check(IsFull);

	menu.Bind(wxEVT_MENU, [this, IsFull](wxCommandEvent&)
		{
			m_ParentWnd->ShowFullScreen(!IsFull);
		});

	//Show the menu
	PopupMenu(&menu, MenuPos(m_Windows));
}



void CTopBar::OnOpen(wxCommandEvent& event)
{
	wxFileDialog dlg(this, "Open Project", "", "", "MacroLand Project (*.sproj)|*.sproj", wxFD_OPEN);

	if (dlg.ShowModal() != wxID_OK)
		return;

	try
	{
		wxString FilePath = dlg.GetPath();
		ExecuteProjFile(FilePath.ToStdWstring());
	}
	catch (const std::exception& e)
	{
		wxMessageBox(e.what());
	}
}



void CTopBar::OnExtensionMngr(wxCommandEvent& event)
{
	if (!m_frmExtMngr) {
		m_frmExtMngr = new frmExtensionMngr(this);

		m_frmExtMngr->Maximize();
		m_frmExtMngr->Show();

		m_frmExtMngr->Bind(wxEVT_CLOSE_WINDOW, [&](wxCloseEvent& closeEvent)
			{
				m_frmExtMngr = nullptr;
				closeEvent.Skip();
			});
	}
	else
		m_frmExtMngr->Raise();
}




void CTopBar::ExecuteProjFile(const std::filesystem::path& ProjPath)
{
	const auto LockFile = ProjPath.parent_path() / (ProjPath.stem().wstring() + L".lock");
	if (std::filesystem::exists(LockFile))
		throw std::exception("Project is already open (.lock file exists)");

	auto Exe = glbExeDir / "macrolandapp.exe";
	wxString Cmd = L"\"" + Exe.wstring() + L"\"" + L"  " + L"\"" + ProjPath.wstring() + L"\"";
	wxExecute(Cmd, wxEXEC_ASYNC);

	m_RecentFiles->Append(ProjPath);
	m_RecentFiles->Write();
}


