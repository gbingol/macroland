#pragma once

#include <string>
#include <filesystem>
#include <stack>
#include <memory>
#include <wx/wx.h>
#include <wx/grid.h>
#include <wx/clipbrd.h>

#include "dllimpexp.h"

namespace grid
{
	class CWorksheetNtbkBase;
	class CWorksheetBase;
	class WorksheetUndoRedoEvent;

	class CWorkbookBase : public wxPanel
	{
		friend class CWorksheetNtbkBase;
	public:

		DLLGRID CWorkbookBase(wxWindow* parent);

		virtual DLLGRID ~CWorkbookBase();

		virtual DLLGRID CWorksheetNtbkBase* GetWorksheetNotebook() const
		{
			return m_WSNtbk;
		}

		virtual DLLGRID bool AddNewWorksheet(const std::wstring& tblname = L"", int nrows = 1000, int ncols = 50);


		DLLGRID void PushUndoEvent(std::unique_ptr<WorksheetUndoRedoEvent> event);

		DLLGRID void EnableEditing(bool Enable = true);

		DLLGRID void TurnOnGridSelectionMode(bool IsOn = true);

		DLLGRID bool Write(const std::filesystem::path& SnapshotDir);
		
		//Assumes that the project file is unpacked to a snapshot directory
		DLLGRID bool Read(const std::filesystem::path& SnapshotDir);

		DLLGRID void ShowWorksheet(const std::wstring& worksheetname) const;
		DLLGRID void ShowWorksheet(const CWorksheetBase* worksheet) const;

		DLLGRID void MarkDirty();
		DLLGRID void MarkClean();

		DLLGRID CWorksheetBase* GetActiveWS() const;

		DLLGRID CWorksheetBase* GetWorksheet(const std::wstring& worksheetname) const;

		DLLGRID CWorksheetBase* GetWorksheet(const size_t PageNumber) const;

		//if paste is successful returns true
		DLLGRID bool PasteValues(const wxDataFormat& ClipbrdFormat);
		DLLGRID bool PasteFormat(const wxDataFormat& ClipbrdFormat, bool RefreshBlock = true);

		//return number of worksheets
		DLLGRID size_t size() const;

		auto& GetRedoStack()	const{
			return m_RedoStack;
		}

		auto& GetUndoStack() const{
			return m_UndoStack;
		}

		bool IsDirty() const {
			return m_IsDirty;
		}

	protected:
		/*
			All below will:
			1) Mark the WorksheetBase as dirty
			2) PushUndoEvent
		*/

		DLLGRID void ChangeCellAlignment(int wxAlignID, bool RefreshBlock = true); 
		DLLGRID void ChangeCellBGColor(const wxColor& BGColor, bool RefreshBlock = true);
		DLLGRID void ChangeTextColor(const wxColor& TxtColor, bool RefreshBlock = true);
		DLLGRID void ChangeFontFace(const wxString& fontFaceName, bool RefreshBlock = true);
		DLLGRID void ChangeFontSize(int FontPointSize, bool RefreshBlock = true);

		//if normal makes it bold and vice versa
		DLLGRID void ToggleFontWeight(bool RefreshBlock = true);

		//if normal makes it italic and vice versa
		DLLGRID void ToggleFontStyle(bool RefreshBlock = true);

		//if normal underlines it and vice versa
		DLLGRID void ToggleFontUnderlined(bool RefreshBlock = true);

		DLLGRID void ProcessUndoEvent();
		DLLGRID void ProcessRedoEvent();

	private:
		//Gets the TL and BR of selection coord (if no selection returns GridCursorRow and TL=BR)
		std::pair<wxGridCellCoords, wxGridCellCoords> GetSelectionCoords() const;

	protected:
		CWorksheetNtbkBase* m_WSNtbk{ nullptr };
		bool m_IsDirty = false;

	private:
		std::stack<std::unique_ptr<WorksheetUndoRedoEvent>> m_UndoStack;
		std::stack<std::unique_ptr<WorksheetUndoRedoEvent>> m_RedoStack;
	};
}

