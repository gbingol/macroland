#pragma once

#include <filesystem>
#include <fstream>
#include <thread>
#include <mutex>
#include <future>

#include <wx/wx.h>
#include <wx/notebook.h>
#include <wx/webrequest.h>

#include "../util_funcs.h"

#include "interactstatbar.h"



namespace cmdedit 
{ 
	class pnlCommandWindow; 
}


namespace ICELL 
{ 
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

	auto getNotebook() const
	{
		return m_Notebook;
	}

	void MarkDirty()
	{
		m_IsDirty = true;
	}

	void MarkClean();

	bool isDirty() const
	{
		return m_IsDirty;
	}

	auto getProjSnapshotDir() const
	{
		return m_SnapshotDir;
	}

	auto getProjFilePath() const
	{
		return m_ProjFile;
	}

	auto getStatBarMenu() const
	{
		return m_StatBarMenu.get();
	}

	void setStBarRectField(int field)
	{
		m_StBar_RectField = field;
	}
	 
	auto getStBarRectField() const 
	{
		return m_StBar_RectField;
	}


protected:		
	void OnFileMenuOpen(wxMenuEvent& event);
	void OnOpenProject(wxCommandEvent& event);
	void OnClose( wxCloseEvent& event );
	
	//status bar related
	void StBar_OnRightUp(StatBarMouseEvent& event);
	void OnCheckNewVersion(wxWebRequestEvent& event);

private:
	void RunLuaExtensions();
	bool CreateLockFile();
	bool CreateSnapshotDir();
	
	//Create sproj from snapshot directory
	void WriteProjFile();

	//Using terminal command, opens the project file in a separate instance
	void ExecuteProjFile(const std::filesystem::path& ProjPath);

	//
	void CheckAvailableNewVersion();

private:
		
	//Status bar currently divided into 3
	enum class SBREGION
	{ 
		SAVE=0, 
		INFO, //info on selection 
		VISIT //visit a website
	};
	
	std::unique_ptr<wxMenu> m_StatBarMenu;
	int m_StBar_RectField = -1;

	MODE m_Mode;

	wxNotebook* m_Notebook{ nullptr };
	InteractiveStatusBar* m_StBar{ nullptr };

	cmdedit::pnlCommandWindow* m_CmdWnd{ nullptr };
	ICELL::CWorkbook* m_Workbook{ nullptr };
	
	
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
	util::CDate m_ProjDate;



	wxMenuBar* m_menubar;
	wxMenu* m_FileMenu;
	wxMenu* m_WindowsMenu;
	wxMenu* m_RecentProjMenu;

	std::unique_ptr<util::CRecentFiles> m_RecentFiles;

	const int ID_PROJ_SAVE = wxNewId();
	const int ID_PROJ_OPEN = wxNewId();
	const int ID_RECENTPROJ{ wxNewId() };
	const int ID_EXTMNGR{ wxNewId() };
	const int ID_FULLSCREEN{ wxNewId() };

	std::promise<std::list<std::string>> m_Promise;
	wxWebRequest m_WebRequest;
};


