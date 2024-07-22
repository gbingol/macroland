#pragma once
#include <string>
#include <optional>

#include <wx/wx.h>
#include <wx/aui/auibook.h>
#include <wx/mdi.h>

#include "dllimpexp.h"

namespace grid
{
	class CWorksheetBase;
	class CWorkbookBase;

	class CWorksheetNtbkBase : public wxAuiNotebook
	{

	public:
		DLLGRID CWorksheetNtbkBase(CWorkbookBase* parent);
		virtual DLLGRID ~CWorksheetNtbkBase();

		virtual DLLGRID bool AddNewWorksheet(
			const std::wstring& Label = L"", 
			int nrows = 1000, 
			int ncols = 50);
		
		DLLGRID bool ImportAsNewWorksheet(
			const std::wstring& tblname, 
			int nrows, 
			int ncols);

		DLLGRID bool RemoveWorksheet(CWorksheetBase* worksheet);
		DLLGRID bool RemoveWorksheet(const std::wstring& worksheetname);

		DLLGRID CWorksheetBase* GetWorksheet(const std::wstring& name) const;

		//finds the position of the worksheet
		DLLGRID std::optional<size_t> FindWorksheet(const std::wstring& name) const;

		//GetPage returns wxPanel and this is a helper to find Worksheet in the panel
		DLLGRID CWorksheetBase* FindWorksheet(const size_t PageNumber) const;

		size_t size() const
		{
			return GetPageCount();
		}

		CWorksheetBase* GetActiveWorksheet() const
		{
			return m_ActiveWS;
		}


	protected:
		DLLGRID void OnPageClose(wxAuiNotebookEvent& evt);
		DLLGRID void OnClsButton(wxAuiNotebookEvent& event);
		DLLGRID void OnEndDrag(wxAuiNotebookEvent& event);

	protected:
		virtual DLLGRID CWorksheetBase* CreateWorksheet(
			wxWindow* wnd, 
			const std::wstring& Label, 
			int nrows, 
			int ncols) const;

		virtual DLLGRID void OnTabRightDown(wxAuiNotebookEvent& evt);

		bool WorksheetExists(const std::wstring& WorksheetName);
		bool Rename(const std::wstring& NewName);

	private:
		//only this class binds this event
		void __OnTabRightDown(wxAuiNotebookEvent& evt);

		void OnWSRename(wxCommandEvent& evt);
		void OnWSDel(wxCommandEvent& evt);
		void OnWSAdd(wxCommandEvent& evt);

	protected:
		//pointer to the current grid in the notebook
		CWorksheetBase* m_ActiveWS = nullptr;
		CWorkbookBase* m_WorkbookBase{ nullptr };

		//appends a number to the default worksheet naming, such as Worksheet 2
		int m_WorksheetNumber;

		wxMenu* m_ContextMenu{nullptr};

		const int ID_WSDEL = wxNewId();
		const int ID_WSRENAME = wxNewId();
		const int ID_WSADD = wxNewId();
	};
}

