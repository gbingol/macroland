#pragma once
#include <string>
#include <optional>

#include <wx/wx.h>
#include <wx/aui/auibook.h>
#include <wx/mdi.h>



namespace grid
{
	class CWorksheetBase;
	class CWorkbookBase;

	class CWorksheetNtbkBase : public wxAuiNotebook
	{

	public:
		CWorksheetNtbkBase(CWorkbookBase* parent);
		virtual ~CWorksheetNtbkBase();

		virtual bool AddNewWorksheet(
			const std::wstring& Label = L"", 
			int nrows = 1000, 
			int ncols = 50);
		
		bool ImportAsNewWorksheet(
			const std::wstring& tblname, 
			int nrows, 
			int ncols);

		bool RemoveWorksheet(CWorksheetBase* worksheet);
		bool RemoveWorksheet(const std::wstring& worksheetname);

		CWorksheetBase* GetWorksheet(const std::wstring& name) const;

		//finds the position of the worksheet
		std::optional<size_t> FindWorksheet(const std::wstring& name) const;

		//GetPage returns wxPanel and this is a helper to find Worksheet in the panel
		CWorksheetBase* FindWorksheet(const size_t PageNumber) const;

		size_t size() const
		{
			return GetPageCount();
		}

		CWorksheetBase* GetActiveWorksheet() const
		{
			return m_ActiveWS;
		}


	protected:
		void OnPageClose(wxAuiNotebookEvent& evt);
		void OnClsButton(wxAuiNotebookEvent& event);
		void OnEndDrag(wxAuiNotebookEvent& event);

	protected:
		virtual CWorksheetBase* CreateWorksheet(
			wxWindow* wnd, 
			const std::wstring& Label, 
			int nrows, 
			int ncols) const;

		virtual void OnTabRightDown(wxAuiNotebookEvent& evt);

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

