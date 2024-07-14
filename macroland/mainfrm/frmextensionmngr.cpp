#include "frmExtensionMngr.h"

#include <numeric>
#include <codecvt>
#include <locale>

#include <wx/artprov.h>

#include "../util_funcs.h"

#include "../lua/luautil.h"

#include "../consts.h"
#include "../icons/mainframeicon.xpm"




extern std::filesystem::path glbExeDir;



namespace extension
{

	CExtension::CExtension(
		lua_State* L, 
		const std::filesystem::path& DirEntry)
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;

		m_Enabled = IsEnabled(DirEntry);

		m_DirPath = DirEntry;

		int type = lua_getglobal(L, "guid");

		if (type == LUA_TSTRING)
		{
			m_GUID = lua_tostring(L, -1);
			lua_pop(L, 1);
		}

		type = lua_getglobal(L, "extname");
		if (type == LUA_TSTRING)
		{
			m_Name = converter.from_bytes(lua_tostring(L, -1));
			lua_pop(L, 1);
		}

		type = lua_getglobal(L, "developer");
		if (type == LUA_TSTRING)
		{
			m_Developer = converter.from_bytes(lua_tostring(L, -1));
			lua_pop(L, 1);
		}

		type = lua_getglobal(L, "readme");
		if (type == LUA_TSTRING)
		{
			m_ReadMe = std::filesystem::path(converter.from_bytes(lua_tostring(L, -1)));
			lua_pop(L, 1);
		}

		type = lua_getglobal(L, "short_desc");
		if (type == LUA_TSTRING)
		{
			m_Short_Desc = converter.from_bytes(lua_tostring(L, -1));
			lua_pop(L, 1);
		}

		type = lua_getglobal(L, "version");
		if (type == LUA_TSTRING)
		{
			m_Version = lua_tostring(L, -1);
			lua_pop(L, 1);
		}

		if (m_Version == "")
			m_Version = "1.0";
	}



	std::wstring CExtension::GetShortenedName() const
	{
		if (m_Name.length() > 75)
			return m_Name.substr(0, 75) + L"...";

		return m_Name;
	}



	int compareversions(
		const CExtension& A, 
		const CExtension& B)
	{
		std::vector<int> Version_A, Version_B;

		std::istringstream ss(A.m_Version);
		std::string token;

		while (std::getline(ss, token, '.'))
			Version_A.push_back(std::atoi(token.c_str()));

		std::istringstream ss2(B.m_Version);
		std::string token2;

		while (std::getline(ss2, token2, '.'))
			Version_B.push_back(std::atoi(token2.c_str()));

		for (size_t i = 0, j = 0; i < Version_A.size() && j < Version_B.size(); ++i, ++j)
		{
			if (Version_A[i] > Version_B[j])
				return 1;
			else if (Version_A[i] < Version_B[j])
				return -1;
		}

		return 0;
	}

}









pnlExtensionMngr::pnlExtensionMngr(wxWindow* parent) :
	wxPanel(parent)
{
	m_Parent = parent->GetParent();
	SetSizeHints(wxDefaultSize, wxDefaultSize);
	
	m_split = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D | wxSP_LIVE_UPDATE);

	m_LWExt = new wxListView(m_split);
	m_LWExt->SetFont(wxFont(11, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, "Arial"));
	wxItemAttr ColumnHeader;
	ColumnHeader.SetFont(wxFontInfo(FromDIP(14)).FaceName("Consolas").Italic());
	m_LWExt->SetHeaderAttr(ColumnHeader);

	m_HTMLWin = new wxHtmlWindow(m_split, wxID_ANY);

	m_split->SplitHorizontally(m_LWExt, m_HTMLWin, 0);
	m_split->SetSashGravity(0.30);

	auto szrMain = new wxBoxSizer(wxVERTICAL);
	szrMain->Add(m_split, 1, wxEXPAND | wxALL, 5);

	SetSizer(szrMain); 
	Layout();


	SetupListView();

	m_LWExt->Bind(wxEVT_LIST_ITEM_SELECTED, &pnlExtensionMngr::OnListEntrySelected, this);
	m_LWExt->Bind(wxEVT_RIGHT_DOWN, &pnlExtensionMngr::OnRightDown, this);

	//FileMenu->Bind(wxEVT_MENU, &pnlExtensionMngr::OnInstall, this);
}



void pnlExtensionMngr::SetupListView()
{ 
	auto List = new wxImageList(32, 32, true);
	 
	wxSize ImgSize = wxSize(32, 32);
	wxColor BG_True = wxColor(76, 134, 67);
	wxColor BG_False = wxColor(255, 0, 0);

	auto func = util::CreateBitmap;
	List->Add(func("", ImgSize, wxNullFont, BG_False, wxNullColour, nullptr)); // extension disabled
	List->Add(func("", ImgSize, wxNullFont, BG_True, wxNullColour, nullptr)); // extension enabled
	m_LWExt->SetImageList(List, wxIMAGE_LIST_SMALL);

	std::vector<int> Widths = { 2,1,2,5 };
	int Sum = std::accumulate(Widths.begin(), Widths.end(), 0);

	m_LWExt->InsertColumn(0, "Name", wxLIST_FORMAT_LEFT);
	m_LWExt->InsertColumn(1, "Version", wxLIST_FORMAT_LEFT);
	m_LWExt->InsertColumn(2, "Developer", wxLIST_FORMAT_LEFT);
	m_LWExt->InsertColumn(3, "Description", wxLIST_FORMAT_LEFT);

	auto W = m_Parent->GetClientRect().width * 2;
	for(size_t i=0; i<Widths.size(); ++i)
		m_LWExt->SetColumnWidth(i, (W*Widths[i])/(float)Sum);
	
	LoadExtensions();
}


void pnlExtensionMngr::LoadExtensions()
{
	m_Extensions.clear();
	m_LWExt->DeleteAllItems();

	lua_State* L = luaL_newstate();
	luaL_openlibs(L);

	namespace fs = std::filesystem;
	for (int index = 0; const auto & DirEntry : fs::directory_iterator(glbExeDir / consts::EXTENSIONS))
	{
		if (!DirEntry.is_directory())
			continue;

		lua::RunLuaFile(L, DirEntry / fs::path("manifest.lua"), "manifest.lua");

		extension::CExtension ext(L, DirEntry);
		m_Extensions.push_back(ext);

		m_LWExt->InsertItem(index, ext.m_Name, ext.m_Enabled);
		m_LWExt->SetItem(index, 1, ext.m_Version);
		m_LWExt->SetItem(index, 2, ext.m_Developer);
		m_LWExt->SetItem(index, 3, ext.m_Short_Desc);

		index++;
	}

	lua_close(L);
}


void pnlExtensionMngr::OnListEntrySelected(wxListEvent& event)
{
	long item = m_LWExt->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item < 0)
	{
		event.Skip();
		return;
	}

	m_SelIndex = item;

	auto extend = m_Extensions[item];
	auto Path = extend.GetReadMePath();
	if (std::filesystem::exists(Path))
		m_HTMLWin->LoadPage(Path.wstring());
	else
	{
		wxString html = "<html><body>";
		html << "<p><b>" << Path.wstring() << "</b> does not exist.</p>";
		html << "</body></html>";

		m_HTMLWin->SetPage(html);
	}

	event.Skip();
}


void pnlExtensionMngr::OnExtensionMenu(wxCommandEvent& event)
{
	int evtID = event.GetId();

	auto Extension = m_Extensions[m_SelIndex];

	if (evtID == ID_SHOWINEXPLORER)
		util::ShowInExplorer(Extension.m_DirPath);

	else if (evtID == ID_DISABLE)
	{
		auto P = extension::Disable(Extension.m_DirPath);

		m_Extensions[m_SelIndex].m_Enabled = false;
		m_Extensions[m_SelIndex].m_DirPath = P;

		m_LWExt->SetItemImage(m_SelIndex, m_Extensions[m_SelIndex].m_Enabled);
		m_LWExt->RefreshItem(m_SelIndex);
	}

	else if (evtID == ID_ENABLE)
	{
		auto P = extension::Enable(Extension.m_DirPath);

		m_Extensions[m_SelIndex].m_Enabled = true;
		m_Extensions[m_SelIndex].m_DirPath = P;

		m_LWExt->SetItemImage(m_SelIndex, m_Extensions[m_SelIndex].m_Enabled);
		m_LWExt->RefreshItem(m_SelIndex);
	}

	else if (evtID == ID_UNINSTALL)
	{
		int ans = wxMessageBox("Are you sure to uninstall the extension?", "Please Confirm", wxYES_NO);

			std::error_code ErrCode;
			auto NRemoved = std::filesystem::remove_all(Extension.m_DirPath, ErrCode);
			if (ErrCode)
				wxMessageBox(ErrCode.message());
			else
			{
				wxMessageBox(std::to_string(NRemoved) + " nodes removed.");
				m_LWExt->DeleteItem(m_SelIndex);
				m_LWExt->Refresh();

				m_Extensions.erase(m_Extensions.begin() + m_SelIndex);
			}
		}
	}


void pnlExtensionMngr::OnRightDown(wxMouseEvent& event)
{
	wxClientDC dc(m_LWExt);
	wxPoint pos = event.GetLogicalPosition(dc);

	int Row = -1;
	for (int i = 0; i < m_LWExt->GetItemCount(); ++i)
	{
		wxRect Rect;
		auto Item = m_LWExt->GetItemRect(i, Rect);

		if (Rect.Contains(pos)) {
			Row = i;
			break;
		}
	}

	if (Row < 0)
	{
		event.Skip();
		return;
	}
	
	if (m_SelIndex < 0)
	{
		wxMessageBox("Make a selection and then right-click");
		event.Skip();
		return;
	}
	wxMenu menu;

	auto extend = m_Extensions[m_SelIndex]; 
	wxString ExtName = extend.GetShortenedName();

	if (!extend.m_Enabled)
	{
		auto Enable = menu.Append(ID_ENABLE, "Enable \"" + ExtName + "\"");
		Enable->SetBitmap(wxArtProvider::GetBitmap(wxART_GO_UP));

		menu.Bind(wxEVT_MENU, &pnlExtensionMngr::OnExtensionMenu, this, ID_ENABLE);
	}
	else
	{
		auto ShowInExp = menu.Append(ID_SHOWINEXPLORER, "Show \"" + ExtName + "\" in File Explorer"); 
		ShowInExp->SetBitmap(wxArtProvider::GetBitmap(wxART_FIND)); 

		auto Disable = menu.Append(ID_DISABLE, "Disable \"" + ExtName + "\"");
		Disable->SetBitmap(wxArtProvider::GetBitmap(wxART_GO_DOWN));

		menu.AppendSeparator();

		auto Uninstall = menu.Append(ID_UNINSTALL, "Uninstall \"" + ExtName + "\"");
		Uninstall->SetBackgroundColour(wxColour(255, 0, 0));
		Uninstall->SetBitmap(wxArtProvider::GetBitmap(wxART_DELETE));

		menu.Bind(wxEVT_MENU, &pnlExtensionMngr::OnExtensionMenu, this, ID_SHOWINEXPLORER);
		menu.Bind(wxEVT_MENU, &pnlExtensionMngr::OnExtensionMenu, this, ID_DISABLE);
		menu.Bind(wxEVT_MENU, &pnlExtensionMngr::OnExtensionMenu, this, ID_UNINSTALL);
	}

	PopupMenu(&menu);
}


void pnlExtensionMngr::OnInstall(wxCommandEvent& event)
{
	namespace fs = std::filesystem;

	wxFileDialog dlg(this, "Open Extension", "", "", "zip file (*.zip)|*.zip", wxFD_OPEN);

	if (dlg.ShowModal() == wxID_CANCEL) 
		return;

	fs::path ZipPath = dlg.GetPath().ToStdWstring();
	auto FileName = ZipPath.stem();

	/*
		We are going to unpack it to SCISUIT/temp directory
		Let's check if any folder already with that name exists (highly unlikely)
		If so, remove it
	*/
	auto TempDir = glbExeDir / consts::TEMPDIR;
	if (fs::exists(TempDir / FileName))
		fs::remove_all(TempDir / FileName);

	//Unpack the zip to SCISUIT/temp/myextension
	util::UnpackArchiveFile(ZipPath, TempDir);

	//Does the new extension have manifest.lua file?
	if (!extension::IsValid(TempDir / FileName))
	{
		wxMessageBox("Extension does not have manifest file, aborting...", "Invalid Extension");

		//Operation cancelled, remove the folder from SCISUIT/temp
		fs::remove_all(TempDir / FileName);

		return;
	}

	/*
		We are about to copy the unpacked contents from SCISUIT/temp/myextension
		to SCISUIT/extensions/myextension

		if already SCISUIT/extensions/myextension exists
		prompt user that it will be overwritten
	*/
	auto ExtensionLoc = glbExeDir / consts::EXTENSIONS / FileName; 

	lua_State* L = luaL_newstate();
	luaL_openlibs(L);

	lua::RunLuaFile(L, ExtensionLoc / "manifest.lua", "manifest_err"); 
	extension::CExtension existingExt(L, ExtensionLoc);

	lua::RunLuaFile(L, TempDir / FileName / "manifest.lua", "manifest.err");
	extension::CExtension newExt(L, TempDir / FileName);

	lua_close(L);

	wxString Comparison;
	Comparison << "Existing   |    To be Installed" <<"\n";
	Comparison << "- - - - - - - - - - - - - - - - - -" << "\n";
	Comparison <<existingExt.m_Name << "  |  " << newExt.m_Name << "\n";
	Comparison << existingExt.m_Developer << "  |  " << newExt.m_Developer << "\n";
	Comparison << existingExt.m_Version << "  |  " << newExt.m_Version;

	if (fs::exists(ExtensionLoc))
	{		
		wxString msg;
		msg << "Directory with the same name already exists. Please see the comparison." << "\n \n";
		msg << Comparison << "\n \n";
		msg << "Continuing will replace the contents. Continue?";

		int RemoveDir = wxMessageBox(msg, "Please Confirm", wxYES_NO);
		if (RemoveDir == wxYES)
			fs::remove_all(ExtensionLoc);
		else
		{
			//User cancelled, remove the folder from SCISUIT/temp
			fs::remove_all(TempDir / FileName);
			return;
		}
	}
	else
	{
		for (const auto& ext: m_Extensions)
		{
			if (newExt == ext)
			{
				wxString msg;
				msg << "Extension at: " + ext.m_DirPath.wstring() << "\n";
				msg << "conflicts with the extension to be installed. \n \n";
				msg << "Continuing might break integrity of extensions. \n \n";
				msg << "Please contact the developer of the extension: " << newExt.m_Developer << "\n \n";
				msg << "Aborting...";

				wxMessageBox(msg, "Extension Conflict");

				//operation cancelled, remove the folder from SCISUIT/temp
				fs::remove_all(TempDir / FileName);
				return;
			}
		}

		/*
			No conflict with existing extensions and the folder does not exist
			Let's inform user about the extension to be installed.
		*/
		wxString info = "The following extension is about to be installed: \n \n";
		info << "Name: "<< newExt.m_Name << "\n";
		info <<"Developer: " << newExt.m_Developer << "\n";
		info << "Version: " << newExt.m_Version << "\n";
		info << "Description: " << newExt.m_Short_Desc << "\n \n";

		info << "Continue?";

		int Confirm = wxMessageBox(info, "Please Confirm", wxYES_NO);
		if (Confirm == wxNO)
		{
			//operation cancelled, remove the folder from SCISUIT/temp
			fs::remove_all(TempDir / FileName);
			return;
		}
	}

	/*
		Create SCISUIT/extensions/myextension folder
		and copy contents from SCISUIT/temp/myextension
	*/
	fs::create_directory(ExtensionLoc);
	fs::copy(TempDir / FileName, ExtensionLoc, fs::copy_options::recursive);

	//we are all set, remove SCISUIT/temp/myextension
	fs::remove_all(TempDir / FileName);

	LoadExtensions();

	wxMessageBox("Extension installed. Some extensions might require ScienceSuit to be restarted to take effect.", "Installation Complete!");
}
