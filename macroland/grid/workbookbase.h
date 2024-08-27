#pragma once

#include <string>
#include <filesystem>
#include <stack>
#include <memory>
#include <wx/wx.h>
#include <wx/grid.h>
#include <wx/clipbrd.h>



namespace grid
{
	class CWorksheetNtbkBase;
	class CWorksheetBase;
	class WSUndoRedoEvent;

	class CWorkbookBase : public wxPanel
	{
		friend class CWorksheetNtbkBase;
	public:

		CWorkbookBase(wxWindow* parent);

		virtual ~CWorkbookBase();

		virtual CWorksheetNtbkBase* GetWorksheetNotebook() const
		{
			return m_WSNtbk;
		}

		virtual bool AddNewWorksheet(
							const std::wstring& tblname = L"", 
							int nrows = 1000, 
							int ncols = 50);


		void PushUndoEvent(std::unique_ptr<WSUndoRedoEvent> event);

		void EnableEditing(bool Enable = true);

		bool Write(const std::filesystem::path& SnapshotDir);
		
		//Assumes that the project file is unpacked to a snapshot directory
		bool Read(const std::filesystem::path& SnapshotDir);

		void ShowWorksheet(const std::wstring& worksheetname) const;
		void ShowWorksheet(const CWorksheetBase* worksheet) const;

		void MarkDirty();
		void MarkClean();

		CWorksheetBase* GetActiveWS() const;

		CWorksheetBase* GetWorksheet(const std::wstring& worksheetname) const;

		CWorksheetBase* GetWorksheet(const size_t PageNumber) const;

		//if paste is successful returns true
		bool PasteValues(const wxDataFormat& ClipbrdFormat);
		bool PasteFormat(const wxDataFormat& ClipbrdFormat, bool RefreshBlock = true);

		//return number of worksheets
		size_t size() const;

		auto& GetRedoStack() const{
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

		void ChangeCellAlignment(int wxAlignID, bool RefreshBlock = true); 
		void ChangeCellBGColor(const wxColor& BGColor, bool RefreshBlock = true);
		void ChangeTextColor(const wxColor& TxtColor, bool RefreshBlock = true);
		void ChangeFontFace(const wxString& fontFaceName, bool RefreshBlock = true);
		void ChangeFontSize(int FontPointSize, bool RefreshBlock = true);

		//if normal makes it bold and vice versa
		void ToggleFontWeight(bool RefreshBlock = true);

		//if normal makes it italic and vice versa
		void ToggleFontStyle(bool RefreshBlock = true);

		//if normal underlines it and vice versa
		void ToggleFontUnderlined(bool RefreshBlock = true);

		void ProcessUndoEvent();
		void ProcessRedoEvent();

	private:
		//Gets the TL and BR of selection coord (if no selection returns GridCursorRow and TL=BR)
		std::pair<wxGridCellCoords, wxGridCellCoords> GetSelectionCoords() const;

	protected:
		CWorksheetNtbkBase* m_WSNtbk{ nullptr };
		bool m_IsDirty = false;

	private:
		std::stack<std::unique_ptr<WSUndoRedoEvent>> m_UndoStack, m_RedoStack;
	};
}

