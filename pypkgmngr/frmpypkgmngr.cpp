#include "frmpypkgmngr.h"

#include <wx/artprov.h>

#include <Python.h>

#include "util_funcs.h"
#include "icons/python_logo.xpm"


static std::set<std::wstring> GetInstalledPackages()
{
	std::set<std::wstring> Packages;

	std::string PythonCmd = "import pkgutil \n"
		"x = pkgutil.iter_modules() \n"
		"ModuleList = [] \n"
		"for i in x: \n"
		"    if(i.ispkg==True): \n"
		"        ModuleList.append(i.name)";

	auto Module = PyModule_New("pkgmanager");
	auto Dict = PyModule_GetDict(Module);

	std::string VarName = "ModuleList";

	auto ResultObj = PyRun_String(PythonCmd.c_str(), Py_file_input, Dict, Dict);

	PyObject* ListObj = nullptr;
	if (ResultObj)
		ListObj = PyDict_GetItemString(Dict, VarName.c_str());
	else
		PyErr_Clear();

	if (ListObj)
	{
		size_t N = PyList_GET_SIZE(ListObj);
		for (size_t i = 0; i < N; ++i)
		{
			auto ListItem = PyUnicode_AsWideCharString(PyList_GetItem(ListObj, i), nullptr);
			Packages.insert(ListItem);
		}
	}

	Py_XDECREF(ListObj);
	Py_XDECREF(Module);

	return Packages;
}


dlgPipFreeze::dlgPipFreeze(
	wxWindow* parent,
	wxWindowID id,
	const wxString& title,
	const wxPoint& pos,
	const wxSize& size,
	long style) : wxDialog(parent, id, title, pos, size, style)
{
	SetSizeHints(wxDefaultSize, wxDefaultSize);

	auto sizerMain = new wxBoxSizer(wxVERTICAL);


	/******* Cmd Window ************/
	m_radioCmdWnd = new wxRadioButton(this, wxID_ANY, "Command window", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	m_radioCmdWnd->SetValue(true); //by default show at command window
	sizerMain->Add(m_radioCmdWnd, 0, wxALL, 5);


	/******** Reqirements ***********/
	auto sizerRequirements = new wxBoxSizer(wxHORIZONTAL);

	m_radioFile = new wxRadioButton(this, wxID_ANY, "Generate requirements.txt file");

	m_dirPicker = new wxDirPickerCtrl(this, wxID_ANY, wxEmptyString, "Select a folder");
	m_dirPicker->SetToolTip("Ensure that you have write access to the selected folder!");
	m_dirPicker->Enable(false);

	sizerRequirements->Add(m_radioFile, 0, wxALL, 5);
	sizerRequirements->Add(m_dirPicker, 1, wxALL, 5);

	//
	sizerMain->Add(sizerRequirements, 1, wxEXPAND, 5);

	/********* OK Cancel Buttons  **********/
	m_sdbSizer = new wxStdDialogButtonSizer();
	m_sdbSizerOK = new wxButton(this, wxID_OK);
	m_sdbSizer->AddButton(m_sdbSizerOK);
	m_sdbSizerCancel = new wxButton(this, wxID_CANCEL);
	m_sdbSizer->AddButton(m_sdbSizerCancel);
	m_sdbSizer->Realize();

	sizerMain->Add(m_sdbSizer, 1, wxEXPAND, 5);


	SetSizerAndFit(sizerMain);
	Centre(wxBOTH);



	m_radioCmdWnd->Bind(wxEVT_COMMAND_RADIOBUTTON_SELECTED, [this](wxCommandEvent& event)
		{
			m_Path = "";
			m_dirPicker->Enable(false);
		});


	m_radioFile->Bind(wxEVT_COMMAND_RADIOBUTTON_SELECTED, [this](wxCommandEvent& event)
		{
			m_dirPicker->Enable(true);
		});


	m_dirPicker->Bind(wxEVT_COMMAND_DIRPICKER_CHANGED, [this](wxFileDirPickerEvent& event)
		{
			m_Path = event.GetPath().ToStdWstring();
			event.Skip();
		});

	m_sdbSizerCancel->Bind(wxEVT_COMMAND_BUTTON_CLICKED, [this](wxCommandEvent& event)
		{
			EndModal(wxID_CANCEL);
		});


	m_sdbSizerOK->Bind(wxEVT_COMMAND_BUTTON_CLICKED, [this](wxCommandEvent& event)
		{
			EndModal(wxID_OK);
		});
}







/*****************************  dlgPipInstall  *****************************************/



dlgPipInstall::dlgPipInstall(
	wxWindow* parent,
	wxWindowID id,
	const wxString& title,
	const wxPoint& pos,
	const wxSize& size,
	long style) : wxDialog(parent, id, title, pos, size, style)
{
	SetSizeHints(wxDefaultSize, wxDefaultSize);

	//Includes all sizers 
	auto sizerMain = new wxBoxSizer(wxVERTICAL);

	//Except OK and Cancel button all controls
	auto fgMainSizer = new wxFlexGridSizer(0, 2, 20, 0);
	fgMainSizer->AddGrowableCol(1);
	fgMainSizer->SetFlexibleDirection(wxBOTH);
	fgMainSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);


	/******************* Package Name and Version ********************/
	m_radioName = new wxRadioButton(this, wxID_ANY, "Package Name:", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	m_radioName->SetValue(true);
	fgMainSizer->Add(m_radioName, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

	m_pnlName = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);

	auto sizerPkgVersion = new wxBoxSizer(wxVERTICAL);

	auto sizerPkgNameInput = new wxBoxSizer(wxHORIZONTAL);

	m_staticText2 = new wxStaticText(m_pnlName, wxID_ANY, "Package Name:");
	sizerPkgNameInput->Add(m_staticText2, 0, wxALL, 5);

	m_txtPkgName = new wxTextCtrl(m_pnlName, wxID_ANY);
	sizerPkgNameInput->Add(m_txtPkgName, 1, wxALL, 5);


	sizerPkgVersion->Add(sizerPkgNameInput, 1, wxEXPAND, 5);

	auto sizerPkgVersionOpts = new wxBoxSizer(wxHORIZONTAL);

	m_radiopkgLatest = new wxRadioButton(m_pnlName, wxID_ANY, "Latest", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	m_radiopkgLatest->SetValue(true);
	sizerPkgVersionOpts->Add(m_radiopkgLatest, 0, wxALL, 5);

	m_radiopkgExact = new wxRadioButton(m_pnlName, wxID_ANY, "Exact");
	m_radiopkgExact->SetToolTip("Find the exact specified version of the package");
	sizerPkgVersionOpts->Add(m_radiopkgExact, 0, wxALL, 5);

	m_radiopkgMinVer = new wxRadioButton(m_pnlName, wxID_ANY, "Min");
	m_radiopkgMinVer->SetToolTip("Find a version of the package greater or equal (>=) to the min version");
	sizerPkgVersionOpts->Add(m_radiopkgMinVer, 0, wxALL, 5);

	m_radioMaxVer = new wxRadioButton(m_pnlName, wxID_ANY, "Max");
	m_radioMaxVer->SetToolTip("Find a version of the package less than or equal (<=) to the max version");
	sizerPkgVersionOpts->Add(m_radioMaxVer, 0, wxALL, 5);


	sizerPkgVersion->Add(sizerPkgVersionOpts, 1, wxEXPAND, 5);


	m_pnlName->SetSizer(sizerPkgVersion);
	m_pnlName->Layout();
	sizerPkgVersion->Fit(m_pnlName);
	fgMainSizer->Add(m_pnlName, 1, wxEXPAND | wxALL, 5);


	/************************** REQ FILE *******************/
	m_radioReqFile = new wxRadioButton(this, wxID_ANY, "Requirements File:");
	m_radioReqFile->SetToolTip(wxT("requirements file location (recursively applied)"));

	fgMainSizer->Add(m_radioReqFile, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

	m_pnlReqFile = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);

	auto bSizer13 = new wxBoxSizer(wxHORIZONTAL);

	m_filePickerReqFile = new wxFilePickerCtrl(m_pnlReqFile, wxID_ANY, wxEmptyString, "Select requirements file");
	bSizer13->Add(m_filePickerReqFile, 1, wxALL, 5);


	m_pnlReqFile->SetSizer(bSizer13);
	m_pnlReqFile->Layout();
	bSizer13->Fit(m_pnlReqFile);
	fgMainSizer->Add(m_pnlReqFile, 1, wxEXPAND | wxALL, 5);


	/**************** ARCHIVE FILE **************************/
	m_radioArchive = new wxRadioButton(this, wxID_ANY, "Archive File:");
	fgMainSizer->Add(m_radioArchive, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

	m_pnlArchive = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);

	auto fgSizerArchive = new wxFlexGridSizer(0, 2, 0, 0);
	fgSizerArchive->AddGrowableCol(1);
	fgSizerArchive->SetFlexibleDirection(wxBOTH);
	fgSizerArchive->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	m_radioLocalArchive = new wxRadioButton(m_pnlArchive, wxID_ANY, "Local:", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	m_radioLocalArchive->SetValue(true);
	fgSizerArchive->Add(m_radioLocalArchive, 0, wxALL, 5);

	m_filePickerLocalArchive = new wxFilePickerCtrl(m_pnlArchive, wxID_ANY, wxEmptyString, "Select an archive file");
	fgSizerArchive->Add(m_filePickerLocalArchive, 0, wxALL | wxEXPAND, 5);

	m_radioURLArchive = new wxRadioButton(m_pnlArchive, wxID_ANY, "URL:");
	fgSizerArchive->Add(m_radioURLArchive, 0, wxALL, 5);

	m_txtURLArchive = new wxTextCtrl(m_pnlArchive, wxID_ANY);
	m_txtURLArchive->SetToolTip(wxT("Paste the internet URL address where archive file is"));

	fgSizerArchive->Add(m_txtURLArchive, 0, wxALL | wxEXPAND, 5);


	m_pnlArchive->SetSizer(fgSizerArchive);
	m_pnlArchive->Layout();
	fgSizerArchive->Fit(m_pnlArchive);
	fgMainSizer->Add(m_pnlArchive, 1, wxEXPAND | wxALL, 5);


	sizerMain->Add(fgMainSizer, 1, wxEXPAND, 5);


	/******* BUTTONS *****************/

	m_sdbSizer = new wxStdDialogButtonSizer();
	m_sdbSizerOK = new wxButton(this, wxID_OK);
	m_sdbSizer->AddButton(m_sdbSizerOK);
	m_sdbSizerCancel = new wxButton(this, wxID_CANCEL);
	m_sdbSizer->AddButton(m_sdbSizerCancel);
	m_sdbSizer->Realize();

	sizerMain->Add(m_sdbSizer, 0, wxEXPAND, 5);


	SetSizerAndFit(sizerMain);
	Centre(wxBOTH);


	Bind(wxEVT_INIT_DIALOG, &dlgPipInstall::OnInitDialog, this);

	m_radioName->Bind(wxEVT_COMMAND_RADIOBUTTON_SELECTED, &dlgPipInstall::radioName_OnBtn, this);
	m_radioReqFile->Bind(wxEVT_COMMAND_RADIOBUTTON_SELECTED, &dlgPipInstall::radioReqFile_OnBtn, this);
	m_radioArchive->Bind(wxEVT_COMMAND_RADIOBUTTON_SELECTED, &dlgPipInstall::radioArchive_OnBtn, this);

	m_sdbSizerOK->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &dlgPipInstall::OnOKBtn, this);


	m_radiopkgLatest->Bind(wxEVT_COMMAND_RADIOBUTTON_SELECTED, [this](wxCommandEvent& event)
		{
			m_PkgVersion = "";
		});


	m_radiopkgExact->Bind(wxEVT_COMMAND_RADIOBUTTON_SELECTED, [this](wxCommandEvent& event)
		{
			wxString Ver = wxGetTextFromUser("Enter the specific version number", "Version");

			m_PkgVersion = "==" + Ver;
		});


	m_radiopkgMinVer->Bind(wxEVT_COMMAND_RADIOBUTTON_SELECTED, [this](wxCommandEvent& event)
		{
			wxString Ver = wxGetTextFromUser("Enter the minimum version number", "Version");

			m_PkgVersion = ">=" + Ver;
		});


	m_radioMaxVer->Bind(wxEVT_COMMAND_RADIOBUTTON_SELECTED, [this](wxCommandEvent& event)
		{
			wxString Ver = wxGetTextFromUser("Enter the maximum version number", "Version");

			m_PkgVersion = "<=" + Ver;
		});


	m_filePickerReqFile->Bind(wxEVT_COMMAND_FILEPICKER_CHANGED, [this](wxFileDirPickerEvent& event)
		{
			m_ReqFilePath = event.GetPath().ToStdWstring();
			event.Skip();
		});


	m_radioLocalArchive->Bind(wxEVT_COMMAND_RADIOBUTTON_SELECTED, [this](wxCommandEvent& event)
		{
			m_txtURLArchive->Enable(false);
			m_filePickerLocalArchive->Enable(true);
		});


	m_filePickerLocalArchive->Bind(wxEVT_COMMAND_FILEPICKER_CHANGED, [this](wxFileDirPickerEvent& event)
		{
			m_ArchiveFilePath = event.GetPath().ToStdWstring();
			event.Skip();
		});


	m_radioURLArchive->Bind(wxEVT_COMMAND_RADIOBUTTON_SELECTED, [this](wxCommandEvent& event)
		{
			m_txtURLArchive->Enable(true);
			m_filePickerLocalArchive->Enable(false);

			m_ArchiveFilePath = m_txtURLArchive->GetValue().ToStdWstring();
		});


	m_sdbSizerCancel->Bind(wxEVT_COMMAND_BUTTON_CLICKED, [this](wxCommandEvent& event)
		{
			EndModal(wxID_CANCEL);
		});

}



void dlgPipInstall::OnInitDialog(wxInitDialogEvent& event)
{
	m_pnlReqFile->Enable(false);
	m_pnlArchive->Enable(false);
	m_txtURLArchive->Enable(false);

	event.Skip();
}



void dlgPipInstall::radioName_OnBtn(wxCommandEvent& event)
{
	m_pnlArchive->Enable(false);
	m_pnlReqFile->Enable(false);

	m_pnlName->Enable(true);
	m_radiopkgLatest->SetValue(true);

	m_Mode = MODE::NAME;
}


void dlgPipInstall::radioReqFile_OnBtn(wxCommandEvent& event)
{
	m_pnlName->Enable(false);
	m_pnlArchive->Enable(false);

	m_pnlReqFile->Enable(true);

	m_Mode = MODE::REQ;
}


void dlgPipInstall::radioArchive_OnBtn(wxCommandEvent& event)
{
	m_pnlName->Enable(false);
	m_pnlReqFile->Enable(false);

	m_pnlArchive->Enable(true);
	m_txtURLArchive->Enable(false);
	m_radioLocalArchive->SetValue(true);
	m_filePickerLocalArchive->Enable(true);

	m_Mode = MODE::ARCHIVE;
}


void dlgPipInstall::OnOKBtn(wxCommandEvent& event)
{
	if (m_Mode == MODE::NAME && m_txtPkgName->GetValue().IsEmpty())
	{
		wxMessageBox("Package name cannot be blank.");
		return;
	}

	else if (m_Mode == MODE::REQ)
	{
		if (!std::filesystem::exists(m_ReqFilePath))
		{
			wxMessageBox("Invalid file path for the requirements file.");
			return;
		}
	}

	else if (m_Mode == MODE::ARCHIVE)
	{
		if (m_ArchiveFilePath.empty())
		{
			wxMessageBox("Archive file path cannot be empty.");
			return;
		}
	}

	EndModal(wxID_OK);
}






/*******************************  dlgPipList  ******************************************/

dlgPipList::dlgPipList(
	wxWindow* parent,
	wxWindowID id,
	const wxString& title,
	const wxPoint& pos,
	const wxSize& size,
	long style) : wxDialog(parent, id, title, pos, size, style)
{
	SetSizeHints(wxDefaultSize, wxDefaultSize);


	auto sizerMain = new wxBoxSizer(wxVERTICAL);

	/************* Command Prefixes ***************/
	auto sizerPrefixs = new wxBoxSizer(wxVERTICAL);

	m_chkListAll = new wxCheckBox(this, wxID_ANY, "All");
	m_chkListAll->SetToolTip("List all packages");
	m_chkListAll->SetValue(true);
	sizerPrefixs->Add(m_chkListAll, 0, wxALL, 5);

	m_chkOutdated = new wxCheckBox(this, wxID_ANY, "Outdated (--outdated)");
	m_chkOutdated->SetToolTip("List outdated packages");
	sizerPrefixs->Add(m_chkOutdated, 0, wxALL, 5);

	m_chkUptoDate = new wxCheckBox(this, wxID_ANY, "Up to date (--uptodate)");
	m_chkUptoDate->SetToolTip("List uptodate packages");
	sizerPrefixs->Add(m_chkUptoDate, 0, wxALL, 5);

	m_chkPre = new wxCheckBox(this, wxID_ANY, "Pre (--pre)");
	m_chkPre->SetToolTip("Include pre-release and development versions. By default, pip only finds stable versions.");
	sizerPrefixs->Add(m_chkPre, 0, wxALL, 5);

	m_chkNotRequired = new wxCheckBox(this, wxID_ANY, "Not required (--not-required)");
	m_chkNotRequired->SetToolTip("List packages that are not dependencies of installed packages.");
	sizerPrefixs->Add(m_chkNotRequired, 0, wxALL, 5);

	m_chkOutdated->Enable(false);
	m_chkUptoDate->Enable(false);
	m_chkPre->Enable(false);
	m_chkNotRequired->Enable(false);

	sizerMain->Add(sizerPrefixs, 1, wxEXPAND, 5);


	/************ Output format ************************/
	auto sizerFormat = new wxBoxSizer(wxHORIZONTAL);

	m_statictxtFormat = new wxStaticText(this, wxID_ANY, "Format:");
	m_statictxtFormat->SetFont(wxFont(9, wxFONTFAMILY_SWISS, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_BOLD, false, wxT("Arial")));
	m_statictxtFormat->SetForegroundColour(wxColour(255, 0, 0));

	sizerFormat->Add(m_statictxtFormat, 0, wxALL, 5);

	m_radioColumns = new wxRadioButton(this, wxID_ANY, "Columns", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	m_radioColumns->SetValue(true);
	sizerFormat->Add(m_radioColumns, 0, wxALL, 5);

	m_radioFreeze = new wxRadioButton(this, wxID_ANY, "Freeze");
	sizerFormat->Add(m_radioFreeze, 0, wxALL, 5);

	m_radioJSON = new wxRadioButton(this, wxID_ANY, "JSON");
	sizerFormat->Add(m_radioJSON, 0, wxALL, 5);


	sizerMain->Add(sizerFormat, 0, wxEXPAND, 5);


	sizerMain->Add(0, 10, 0, wxEXPAND, 5);

	m_sdbSizer = new wxStdDialogButtonSizer();
	m_sdbSizerOK = new wxButton(this, wxID_OK);
	m_sdbSizer->AddButton(m_sdbSizerOK);
	m_sdbSizerCancel = new wxButton(this, wxID_CANCEL);
	m_sdbSizer->AddButton(m_sdbSizerCancel);
	m_sdbSizer->Realize();

	sizerMain->Add(m_sdbSizer, 0, wxEXPAND, 5);


	SetSizerAndFit(sizerMain);

	this->Centre(wxBOTH);

	m_chkListAll->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &dlgPipList::OnChkListAll, this);

	m_chkOutdated->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &dlgPipList::OnChkCmdPrefixOptions, this);
	m_chkUptoDate->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &dlgPipList::OnChkCmdPrefixOptions, this);
	m_chkPre->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &dlgPipList::OnChkCmdPrefixOptions, this);
	m_chkNotRequired->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &dlgPipList::OnChkCmdPrefixOptions, this);

	m_radioColumns->Bind(wxEVT_COMMAND_RADIOBUTTON_SELECTED, &dlgPipList::OnRadioFormatOptions, this);
	m_radioFreeze->Bind(wxEVT_COMMAND_RADIOBUTTON_SELECTED, &dlgPipList::OnRadioFormatOptions, this);
	m_radioJSON->Bind(wxEVT_COMMAND_RADIOBUTTON_SELECTED, &dlgPipList::OnRadioFormatOptions, this);


	m_sdbSizerCancel->Bind(wxEVT_COMMAND_BUTTON_CLICKED, [this](wxCommandEvent& event)
		{
			EndModal(wxID_CANCEL);
		});


	m_sdbSizerOK->Bind(wxEVT_COMMAND_BUTTON_CLICKED, [this](wxCommandEvent& event)
		{
			EndModal(wxID_OK);
		});
}



void dlgPipList::OnChkCmdPrefixOptions(wxCommandEvent& event)
{
	if (m_chkOutdated->GetValue())
		m_Prefix += " --outdated";

	else if (m_chkUptoDate->GetValue())
		m_Prefix += " --uptodate";

	else if (m_chkPre->GetValue())
		m_Prefix += " --pre";

	else if (m_chkNotRequired->GetValue())
		m_Prefix += " --not-required";
}



void dlgPipList::OnChkListAll(wxCommandEvent& event)
{
	bool IsOn = m_chkListAll->GetValue();
	if (IsOn)
	{
		m_Prefix = "";

		m_chkOutdated->SetValue(false);
		m_chkUptoDate->SetValue(false);
		m_chkPre->SetValue(false);
		m_chkNotRequired->SetValue(false);
	}

	m_chkOutdated->Enable(!IsOn);
	m_chkUptoDate->Enable(!IsOn);
	m_chkPre->Enable(!IsOn);
	m_chkNotRequired->Enable(!IsOn);
}


void dlgPipList::OnRadioFormatOptions(wxCommandEvent& event)
{
	if (m_radioColumns->GetValue())
		m_Format = "";

	else if (m_radioFreeze->GetValue())
		m_Format = "--format freeze";

	else if (m_radioJSON->GetValue())
		m_Format = "--format json";
}





frmPyPkgMngr::PyPkg::PyPkg()
{
	m_Installed = false;
	m_InstallName = wxEmptyString;
	m_Requirement = REQ::MUST;
	m_Description = wxEmptyString;
}



frmPyPkgMngr::PyPkg::PyPkg(
	const wxString& InstallName,
	REQ Requirement,
	const wxString& Description,
	bool IsInstalled)
{
	m_Installed = IsInstalled;
	m_InstallName = InstallName;
	m_Requirement = Requirement;
	m_Description = Description;
}



wxString frmPyPkgMngr::PyPkg::toStr(REQ req)
{
	return (req == PyPkg::REQ::MUST) ? wxString("Must !!!") : wxString();
}




/*************************   frmStartup   ****************************/

frmPyPkgMngr::frmPyPkgMngr(const std::filesystem::path& PyHome) :
	wxFrame(nullptr, wxID_ANY, "Python Package Manager"), m_PyHome{PyHome}
{

	if (m_PyHome.empty())
	{
		wxString Msg = "Things went wrong possibly due to: \n";
		Msg += "1) The default Python home under SCISUIT is not available or renamed, \n";
		Msg += "2) The py_home in SCISUIT/home/settings_system.json is empty string (not explicitly set), \n";
		Msg += "3) There is no default Python in OS, try typing python in a terminal \n";

		wxMessageBox(Msg, "Python Home Error!"); 
		return;
	}
	
	wxIcon AppIcon(python_logo_xpm);
	if (AppIcon.IsOk())
		SetIcon(AppIcon);

	SetSizeHints(wxDefaultSize, wxDefaultSize);
	SetTitle("Python Package Manager");
	SetBackgroundColour(*wxWHITE);


	/********** Status Bar ***************/
	m_StatusBar = new wxStatusBar(this, wxID_ANY, wxST_SIZEGRIP);
	m_StatusBar->SetFont(wxFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, "Arial"));

	m_StatusBar->SetFieldsCount(2);
	int widths[] = { -3,  -1 };
	m_StatusBar->SetStatusWidths(2, widths);

	SetStatusBar(m_StatusBar);


	/******************* Module Status *********************/
	m_LWModules = new wxListView(this);
	m_LWModules->SetFont(wxFont(11, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, "Arial"));
	wxItemAttr ColumnHeader;
	ColumnHeader.SetFont(wxFontInfo(FromDIP(14)).FaceName("Consolas").Italic());
	m_LWModules->SetHeaderAttr(ColumnHeader);

	/**********************************************************/
	auto szrMain = new wxBoxSizer(wxVERTICAL);
	szrMain->Add(m_LWModules, 1, wxEXPAND | wxALL, 5);
	
	Maximize();

	SetSizer(szrMain); 
	Layout();


	SetupModulesListView();
	
	m_StatusBar->SetStatusText(L"PY_HOME: " + m_PyHome.wstring(), 0);

	m_LWModules->Bind(wxEVT_LIST_ITEM_SELECTED, &frmPyPkgMngr::ListEntrySelected, this);
	m_LWModules->Bind(wxEVT_RIGHT_DOWN, &frmPyPkgMngr::OnRightDown, this);
}



void frmPyPkgMngr::SetupModulesListView()
{ 
	auto ImgList = new wxImageList(32, 32, true);
	 
	wxSize ImgSize = wxSize(32, 32);
	wxColor BG_True = wxColor(76, 134, 67);
	wxColor BG_False = wxColor(255, 0, 0);

	ImgList->Add(util::CreateBitmap(wxEmptyString, ImgSize, wxNullFont, BG_False, wxNullColour, nullptr)); // pkg does not exist
	ImgList->Add(util::CreateBitmap(wxEmptyString, ImgSize, wxNullFont, BG_True, wxNullColour, nullptr)); // pkg exists
	m_LWModules->SetImageList(ImgList, wxIMAGE_LIST_SMALL);

	m_LWModules->InsertColumn(0, "Package name", wxLIST_FORMAT_LEFT);
	m_LWModules->InsertColumn(1, "Requirement", wxLIST_FORMAT_LEFT);
	m_LWModules->InsertColumn(2, "Description", wxLIST_FORMAT_LEFT);
	m_LWModules->SetColumnWidth(0, m_LWModules->GetRect().width / 5);
	m_LWModules->SetColumnWidth(1, m_LWModules->GetRect().width / 5);
	m_LWModules->SetColumnWidth(2, 3 * m_LWModules->GetRect().width / 5);
	
	//sets up m_ListedModules
	CheckListedModules(GetInstalledPackages());

	for (int index = 0; const auto & Elem : m_ListedModules)
	{
		const auto& [Name, Pkg] = Elem;

		m_LWModules->InsertItem(index, Name, Pkg.m_Installed);
		m_LWModules->SetItem(index, 1, PyPkg::toStr(Pkg.m_Requirement));

		if (Pkg.m_Requirement == PyPkg::REQ::MUST && Pkg.m_Installed == false)
		{
			m_LWModules->SetItemTextColour(index, *wxRED);
			m_LWModules->SetItemFont(index, wxFont(13, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, "Arial"));
		}

		m_LWModules->SetItem(index, 2, Pkg.m_Description);

		index++; 
	}
}


void frmPyPkgMngr::ListEntrySelected(wxListEvent& event)
{
	long item = m_LWModules->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

	if (item < 0){
		event.Skip(); 
		return;
	}

	m_SelModule = m_LWModules->GetItemText(item);
	bool IsInstalled = m_ListedModules[m_SelModule].m_Installed;

	wxMenu menu;

	if (!IsInstalled)
	{	
		auto Install = menu.Append(ID_LISTED_MODULE_INSTALL, "Install latest version of " + m_SelModule);
		Install->SetBitmap(wxArtProvider::GetBitmap(wxART_GO_DOWN));

		menu.Bind(wxEVT_MENU, &frmPyPkgMngr::OnManageListedModules, this, ID_LISTED_MODULE_INSTALL);
	}
	else
	{
		auto Upgrade = menu.Append(ID_LISTED_MODULE_UPGRADE, "Upgrade " + m_SelModule + " to latest version");
		Upgrade->SetBitmap(wxArtProvider::GetBitmap(wxART_PLUS));

		auto Show = menu.Append(ID_LISTED_MODULE_SHOW_VERBOSE, "Show Info " + m_SelModule);
		Show->SetBitmap(wxArtProvider::GetBitmap(wxART_INFORMATION));

		menu.AppendSeparator();

		auto Uninstall = menu.Append(ID_LISTED_MODULE_DEL, "Uninstall " + m_SelModule);
		Uninstall->SetBackgroundColour(wxColour(255, 0, 0));
		Uninstall->SetBitmap(wxArtProvider::GetBitmap(wxART_DELETE));

		menu.Bind(wxEVT_MENU, &frmPyPkgMngr::OnManageListedModules, this, ID_LISTED_MODULE_UPGRADE);
		menu.Bind(wxEVT_MENU, &frmPyPkgMngr::OnManageListedModules, this, ID_LISTED_MODULE_SHOW_VERBOSE);
		menu.Bind(wxEVT_MENU, &frmPyPkgMngr::OnManageListedModules, this, ID_LISTED_MODULE_DEL);
	}

	PopupMenu(&menu);

	event.Skip();
}


void frmPyPkgMngr::OnRightDown(wxMouseEvent& event)
{
	wxMenu MainMenu;

	auto PIPUpgrade = MainMenu.Append(ID_PIP_UPGRADE, "Upgrade pip");
	PIPUpgrade->SetBitmap(wxArtProvider::GetBitmap(wxART_PLUS));

	MainMenu.AppendSeparator();


	auto pipMenu=new wxMenu();
	MainMenu.AppendSubMenu(pipMenu, "pip commands");

	auto PIPCheck = pipMenu->Append(ID_PIP_CHECK, "check");
	auto PIPFreeze = pipMenu->Append(ID_PIP_FREEZE, "freeze...");
	auto PIPInstall = pipMenu->Append(ID_PIP_INSTALL, "install...");
	auto PIPList = pipMenu->Append(ID_PIP_LIST, "list...");
	auto PIPShow = pipMenu->Append(ID_PIP_SHOW, "show... ");
	auto PIPModUpgrade = pipMenu->Append(ID_PIP_MODULE_UPGRADE, "upgrade...");

	MainMenu.Bind(wxEVT_MENU, &frmPyPkgMngr::OnManageAllModules, this, ID_PIP_UPGRADE);

	MainMenu.Bind(wxEVT_MENU, &frmPyPkgMngr::OnManageAllModules, this, ID_PIP_CHECK);
	MainMenu.Bind(wxEVT_MENU, &frmPyPkgMngr::OnManageAllModules, this, ID_PIP_FREEZE);
	MainMenu.Bind(wxEVT_MENU, &frmPyPkgMngr::OnManageAllModules, this, ID_PIP_INSTALL);
	MainMenu.Bind(wxEVT_MENU, &frmPyPkgMngr::OnManageAllModules, this, ID_PIP_LIST);
	MainMenu.Bind(wxEVT_MENU, &frmPyPkgMngr::OnManageAllModules, this, ID_PIP_SHOW);
	MainMenu.Bind(wxEVT_MENU, &frmPyPkgMngr::OnManageAllModules, this, ID_PIP_MODULE_UPGRADE);

	PopupMenu(&MainMenu);
}



void frmPyPkgMngr::OnManageListedModules(wxCommandEvent& event)
{
	int evtID = event.GetId();

	auto PythonExe = m_PyHome / "python";
	std::wstring Cmd = L"\"" + PythonExe.wstring() + L"\"";
	
	if(evtID == ID_LISTED_MODULE_INSTALL)
		Cmd +=  L" -m pip install " + m_ListedModules[m_SelModule].m_InstallName + " & PAUSE";

	else if(evtID == ID_LISTED_MODULE_UPGRADE)
		Cmd += L" -m pip install --upgrade " + m_ListedModules[m_SelModule].m_InstallName + " & PAUSE";

	else if (evtID == ID_LISTED_MODULE_SHOW_VERBOSE)
		Cmd += L" -m pip show " + m_ListedModules[m_SelModule].m_InstallName + " --verbose & PAUSE";

	else if (evtID == ID_LISTED_MODULE_DEL)
		Cmd += L" -m pip uninstall " + m_ListedModules[m_SelModule].m_InstallName + " & PAUSE";


	wxShell(Cmd);

	bool ResetModulesLW = !(evtID == ID_LISTED_MODULE_SHOW_VERBOSE || evtID == ID_LISTED_MODULE_UPGRADE);

	if (ResetModulesLW)
		RefreshListViewImages();
}



void frmPyPkgMngr::OnManageAllModules(wxCommandEvent& event)
{
	int evtID = event.GetId();

	auto PythonExe = m_PyHome / "python";
	wxString Cmd = "\"" + PythonExe.wstring() + "\"";


	//this is for upgrading PIP itself
	if (evtID == ID_PIP_UPGRADE)
		Cmd += " -m pip install --upgrade pip & PAUSE";


	/*****   PIP Commands affecting modules ****/

	if (evtID == ID_PIP_CHECK)
		Cmd += " -m pip check & PAUSE";

	else if (evtID == ID_PIP_FREEZE)
	{
		dlgPipFreeze dlg(this);
		int id = dlg.ShowModal();

		if (id == wxID_CANCEL) return;
		
		auto Path = dlg.GetPath();
		if (Path.empty())
			Cmd += " -m pip freeze &PAUSE";
		else 
		{
			if (std::filesystem::exists(Path))
				Cmd += " -m pip freeze > " + Path.wstring() + "\\requirements.txt &PAUSE";
			else {
				wxMessageBox(wxString(Path.wstring()) + " does not exist!");
				return;
			}
		}
	}

	else if (evtID == ID_PIP_INSTALL)
	{
		dlgPipInstall dlg(this);
		
		int id = dlg.ShowModal();

		if (id == wxID_CANCEL) return;

		auto Mode = dlg.GetMode();

		//existence of a package name is checked by dlgPipInstall when OK button clicked
		if (Mode == dlgPipInstall::MODE::NAME)
			Cmd += " -m pip install " + dlg.GetPkgNameVersion() + " & PAUSE";

		else if (Mode == dlgPipInstall::MODE::REQ)
			Cmd += " -m pip install -r " + dlg.GetReqFilePath().wstring() + " & PAUSE";

		else if(Mode == dlgPipInstall::MODE::ARCHIVE)
			Cmd += " -m pip install " + dlg.GetArchiveFilePath().wstring() + " & PAUSE";	
	}

	else if (evtID == ID_PIP_LIST)
	{
		dlgPipList dlg(this);
		int id = dlg.ShowModal();

		if (id == wxID_CANCEL) return;
		
		Cmd += " -m pip list " + dlg.GetCommand() + " &PAUSE";
	}

	else if (evtID == ID_PIP_SHOW)
	{
		auto PkgName = wxGetTextFromUser("Enter a valid package name.", "Package name requested!");

		if (PkgName.IsEmpty()) return;

		Cmd += " -m pip show " + PkgName + " --verbose & PAUSE";
	}

	else if (evtID == ID_PIP_MODULE_UPGRADE)
	{
		auto PkgName = wxGetTextFromUser("Enter a valid package name.", "Package name requested!");

		if (PkgName.IsEmpty()) return;

		Cmd += " -m pip install --upgrade " + PkgName + " & PAUSE";
	}

	wxShell(Cmd);


	bool ResetModuleView = (evtID == ID_PIP_MODULE_UPGRADE || evtID == ID_PIP_INSTALL || evtID == ID_PIP_MODULE_UPGRADE);

	if (ResetModuleView)
		RefreshListViewImages();
}


void frmPyPkgMngr::CheckListedModules(const std::set<std::wstring>& InstalledModules)
{
	for (auto& Module : m_ListedModules)
		Module.second.m_Installed = InstalledModules.contains(Module.first);

}


void frmPyPkgMngr::RefreshListViewImages()
{
	// sets up m_ListedModules
	CheckListedModules(GetInstalledPackages());

	for (int index = 0; const auto & Elem : m_ListedModules) 
	{
		const auto& Pkg = Elem.second;
		m_LWModules->SetItemImage(index, Pkg.m_Installed);

		index++;
	}
}

