#pragma once

#include <filesystem>
#include <set>
#include <map>

#include <wx/wx.h>
#include <wx/filepicker.h>
#include <wx/listctrl.h>



class dlgPipFreeze : public wxDialog
{
public:

	dlgPipFreeze(
		wxWindow* parent,
		wxWindowID id = wxID_ANY,
		const wxString& title = "pip freeze command",
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxCAPTION | wxRESIZE_BORDER);

	~dlgPipFreeze() = default;


	auto GetPath() const {
		return m_Path;
	}

protected:
	wxRadioButton* m_radioCmdWnd;
	wxRadioButton* m_radioFile;
	wxDirPickerCtrl* m_dirPicker;
	wxStdDialogButtonSizer* m_sdbSizer;
	wxButton* m_sdbSizerOK;
	wxButton* m_sdbSizerCancel;


private:
	/*
		If path is empty, then freeze command shows at command window
		If there is a selected path, then creates a requirements.txt file at
		the selected path
	*/
	std::filesystem::path m_Path;
};




/******************************** dlgPipInstall *************************************************/


class dlgPipInstall : public wxDialog
{
public:
	enum class MODE {
		NAME = 0, //name specified
		REQ, //requirement file
		ARCHIVE //archive file
	};

public:

	dlgPipInstall(
		wxWindow* parent,
		wxWindowID id = wxID_ANY, const wxString& title = "pip install command",
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxCAPTION | wxRESIZE_BORDER);

	~dlgPipInstall() = default;


	auto GetMode() const {
		return m_Mode;
	}


	//pkg name and version, if empty latest or min (>=), exact (==), max (<=)
	auto GetPkgNameVersion() const {
		return m_txtPkgName->GetValue() + m_PkgVersion;
	}


	//requirement file path
	auto GetReqFilePath() const {
		return m_ReqFilePath;
	}

	//archive path: Local or internet
	auto GetArchiveFilePath() const {
		return m_ArchiveFilePath;
	}

protected:
	wxRadioButton* m_radioName;
	wxPanel* m_pnlName;
	wxStaticText* m_staticText2;
	wxTextCtrl* m_txtPkgName;
	wxRadioButton* m_radiopkgLatest;
	wxRadioButton* m_radiopkgExact;
	wxRadioButton* m_radiopkgMinVer;
	wxRadioButton* m_radioMaxVer;
	wxRadioButton* m_radioReqFile;

	wxPanel* m_pnlReqFile;
	wxFilePickerCtrl* m_filePickerReqFile;
	wxRadioButton* m_radioArchive;

	wxPanel* m_pnlArchive;
	wxRadioButton* m_radioLocalArchive;
	wxFilePickerCtrl* m_filePickerLocalArchive;
	wxRadioButton* m_radioURLArchive;
	wxTextCtrl* m_txtURLArchive;

	wxStdDialogButtonSizer* m_sdbSizer;
	wxButton* m_sdbSizerOK;
	wxButton* m_sdbSizerCancel;


	void OnInitDialog(wxInitDialogEvent& event);

	void radioName_OnBtn(wxCommandEvent& event);
	void radioReqFile_OnBtn(wxCommandEvent& event);
	void radioArchive_OnBtn(wxCommandEvent& event);

	void OnOKBtn(wxCommandEvent& event);


private:
	MODE m_Mode = MODE::NAME;

	wxString m_PkgVersion{};//

	std::filesystem::path m_ReqFilePath{};
	std::filesystem::path m_ArchiveFilePath{};
};






/**************************** dlgPipList ***************************************/


class dlgPipList : public wxDialog
{
public:
	dlgPipList(
		wxWindow* parent,
		wxWindowID id = wxID_ANY,
		const wxString& title = "pip list command",
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize, long style = wxCAPTION | wxRESIZE_BORDER | wxSTAY_ON_TOP);

	~dlgPipList() {}

	auto GetCommand() const {
		return m_Prefix + " " + m_Format;
	}

protected:
	wxCheckBox* m_chkListAll;
	wxCheckBox* m_chkOutdated;
	wxCheckBox* m_chkUptoDate;
	wxCheckBox* m_chkPre;
	wxCheckBox* m_chkNotRequired;

	wxStaticText* m_statictxtFormat;
	wxRadioButton* m_radioColumns;
	wxRadioButton* m_radioFreeze;
	wxRadioButton* m_radioJSON;

	wxStdDialogButtonSizer* m_sdbSizer;
	wxButton* m_sdbSizerOK;
	wxButton* m_sdbSizerCancel;

	void OnChkCmdPrefixOptions(wxCommandEvent& event);
	void OnChkListAll(wxCommandEvent& event);

	void OnRadioFormatOptions(wxCommandEvent& event);

private:
	wxString m_Cmd{};
	wxString m_Prefix{};
	wxString m_Format{};
};






/*********************************************************************/

class frmPyPkgMngr:public wxFrame
{
public:
	struct PyPkg
	{
		//either must or recommended
		enum class REQ { MUST = 0, REC };

		REQ m_Requirement;

		//is it installed on the chosen python home
		bool m_Installed;

		//wx will be installed if wxPython is installed
		std::wstring m_InstallName;
		std::wstring m_Description;

		PyPkg();

		PyPkg(
			const wxString& InstallName, 
			REQ Requirement, 
			const wxString& Description = wxEmptyString,
			bool IsInstalled = false);

		static wxString toStr(REQ req);
	};

public:
	frmPyPkgMngr(const std::filesystem::path& PyHome);
	~frmPyPkgMngr() = default;

protected:

	void ListEntrySelected(wxListEvent& event);
	void OnRightDown(wxMouseEvent& event);

	void OnManageListedModules(wxCommandEvent& event);
	void OnManageAllModules(wxCommandEvent& event);

private:
	void SetupModulesListView();

	void CheckListedModules(const std::set<std::wstring>& InstalledModules);

	void RefreshListViewImages();


protected:

	wxStatusBar* m_StatusBar{ nullptr };

	wxListView* m_LWModules{ nullptr };

private: 
	std::filesystem::path m_PyHome{};

	const int ID_LISTED_MODULE_INSTALL{ wxNewId() };
	const int ID_LISTED_MODULE_UPGRADE{ wxNewId() };
	const int ID_LISTED_MODULE_SHOW_VERBOSE{ wxNewId() };
	const int ID_LISTED_MODULE_DEL{ wxNewId() };

	const int ID_PIP_UPGRADE{ wxNewId() };

	const int ID_PIP_CHECK{ wxNewId() }; //check cmd
	const int ID_PIP_FREEZE{ wxNewId() }; //freeze cmd
	const int ID_PIP_INSTALL{ wxNewId() }; //install cmd
	const int ID_PIP_LIST{ wxNewId() }; //list cmd
	const int ID_PIP_SHOW{ wxNewId() }; //show cmd
	const int ID_PIP_MODULE_UPGRADE{ wxNewId() }; //upgrade cmd


	//selected module
	std::wstring m_SelModule;

	std::map<std::wstring, PyPkg> m_ListedModules =
	{
		{L"scisuit", PyPkg(L"scisuit", PyPkg::REQ::MUST, L"scisuit library (also installs wx).")},

		{L"wx", PyPkg(L"wxpython", PyPkg::REQ::REC, L"wxPython library for apps (also installs numpy).")},

		{L"numpy", PyPkg(L"numpy", PyPkg::REQ::REC, L"Support for large, multi-dimensional arrays and matrices.")},

		{L"scipy", PyPkg(L"scipy", PyPkg::REQ::REC, L"Modules for many tasks in science and engineering.")},

		{L"cv2", PyPkg(L"opencv-python", PyPkg::REQ::REC, L"OpenCV library for image processing.")},

		{L"pandas", PyPkg(L"pandas", PyPkg::REQ::REC, L"Fast and flexible data structures for \"relational\" or \"labeled\" data.")},

		{L"sklearn", PyPkg(L"scikit-learn", PyPkg::REQ::REC, L"Module for machine learning (also installs scipy).")}
	};

};

