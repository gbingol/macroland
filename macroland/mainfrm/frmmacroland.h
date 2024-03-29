#pragma once

#include <filesystem>
#include <fstream>

#include <wx/wx.h>
#include <wx/notebook.h>

#include "../util/cdate.h"

#include "topbar.h"
#include "interactstatbar.h"



namespace scripting::cmdedit 
{ 
	class pnlCommandWindow; 
}

namespace scripting::editor 
{ 
	class frmScriptEditor;
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

	auto getTopBar() const
	{
		return m_TopBar;
	}


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
	void OnClose( wxCloseEvent& event );

	void OnExtensionMngr(wxCommandEvent& event);
	
	//status bar related
	void StBar_OnRightUp(StatBarMouseEvent& event);

private:
		
	//Run ExePath/init.py file
	void RunInitPyFile();
	void RunLuaExtensions();
	bool CreateLockFile();
	bool CreateSnapshotDir();
	
	//Create sproj from snapshot directory
	void WriteProjFile();

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
	CTopBar* m_TopBar;

	scripting::cmdedit::pnlCommandWindow* m_CmdWnd{ nullptr };
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
};


