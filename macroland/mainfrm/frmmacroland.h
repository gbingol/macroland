#pragma once

#include <filesystem>
#include <fstream>
#include <thread>
#include <future>
#include <chrono>
#include <map>
#include <list>

#include <wx/wx.h>
#include <wx/notebook.h>
#include <wx/webrequest.h>

#include "../util/json.h"
#include "../python/PythonWrapper.h"

#include "interactstatbar.h"



namespace cmdedit { 
	class pnlCommandWindow; 
}

namespace ICELL { 
	class CWorkbook; 
}




class frmMacroLand: public wxFrame
{
public: 
	enum class MODE 
	{
		NEWPROJ = 0, //new project (project file does not exist)
		OPENPROJ, //open an existing project
		RECOVERY // recover project from snapshot (not implemented yet)
	};

public:
	frmMacroLand(const std::filesystem::path& ProjectPath);
	~frmMacroLand();

	void Save();

	auto getNotebook() const {
		return m_Notebook;
	}

	void MarkDirty() {
		m_IsDirty = true;
	}

	void MarkClean(); 

	bool isDirty() const {
		return m_IsDirty;
	}

	auto getProjSnapshotDir() const {
		return m_SnapshotDir;
	}

	auto getStatBarMenu() const {
		return m_StatBarMenu.get();
	}
	 
	auto getStBarRectField() const {
		return m_StBar_RectField;
	}

	void BindPyFunc(std::string EventName, 
					std::unique_ptr<Python::CEventCallbackFunc> Callbackfunc);

	void UnbindPyFunc(std::string EventName, PyObject *FunctionObj);

	void CallRegisteredPyFunc(const std::string& event);

protected:		
	void OnFileMenuOpen(wxMenuEvent& event);
	void OnOpenProject(wxCommandEvent& event);
	void OnClose( wxCloseEvent& event );
	
	//status bar related
	void StBar_OnRightUp(StatBarMouseEvent& event);
	void OnCheckNewVersion(wxWebRequestEvent& event);

private:
	bool CreateLockFile();
	bool CreateSnapshotDir();
	
	//Create sproj from snapshot directory
	void WriteProjFile();

	//Using terminal command, opens the project file in a separate instance
	void ExecuteProjFile(const std::filesystem::path& ProjPath);

	void AppendToRecentFilesArray(const std::filesystem::path &ProjPath);

private:	
	MODE m_Mode;

	wxNotebook* m_Notebook{ nullptr };
	InteractiveStatusBar* m_StBar{ nullptr };
	cmdedit::pnlCommandWindow* m_CmdWnd{ nullptr };
	ICELL::CWorkbook* m_Workbook{ nullptr };

	std::unique_ptr<wxMenu> m_StatBarMenu;
	int m_StBar_RectField = -1;
	
	//Has there been any changes to any saveable part (such as Workbook) of the project
	bool m_IsDirty;

	//Project File (.sproj)
	std::filesystem::path m_ProjFile{};

	//Snapshot directory path
	std::filesystem::path m_SnapshotDir{};

	//Lock file related declarations
	std::filesystem::path m_LockFile{};
	std::ofstream m_LockFileStream;

	//Project is either opened or a new project created
	std::string m_ProjDate;
	JSON::Array m_RecentFilesArr;

	wxMenuBar* m_menubar;
	wxMenu* m_FileMenu;
	wxMenu* m_WindowsMenu;
	wxMenu* m_RecentProjMenu;

	const int ID_PROJ_SAVE = wxNewId();
	const int ID_PROJ_OPEN = wxNewId();
	const int ID_RECENTPROJ{ wxNewId() };
	const int ID_FULLSCREEN{ wxNewId() };


	std::map<std::string, std::list<std::unique_ptr<Python::CEventCallbackFunc>>> m_EvtCallBack;
};


