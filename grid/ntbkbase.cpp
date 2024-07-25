#include "ntbkbase.h"

#include <chrono>
#include <wx/artprov.h>

#include "worksheetbase.h"
#include "workbookbase.h"
#include "undoredo.h"
#include "events.h"



namespace grid
{
	CWorksheetNtbkBase::CWorksheetNtbkBase(CWorkbookBase* parent) :
		wxAuiNotebook(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_NB_DEFAULT_STYLE | wxAUI_NB_BOTTOM)
	{
		m_WorksheetNumber = 0;
		m_WorkbookBase = parent;

		Bind(wxEVT_AUINOTEBOOK_PAGE_CLOSE, &CWorksheetNtbkBase::OnPageClose, this);
		Bind(wxEVT_AUINOTEBOOK_BUTTON, &CWorksheetNtbkBase::OnClsButton, this);
		Bind(wxEVT_AUINOTEBOOK_END_DRAG, &CWorksheetNtbkBase::OnEndDrag, this);

		Bind(wxEVT_AUINOTEBOOK_TAB_RIGHT_DOWN, &CWorksheetNtbkBase::__OnTabRightDown, this);
	}


	CWorksheetNtbkBase::~CWorksheetNtbkBase()
	{
		Unbind(wxEVT_AUINOTEBOOK_PAGE_CLOSE, &CWorksheetNtbkBase::OnPageClose, this);
		Unbind(wxEVT_AUINOTEBOOK_BUTTON, &CWorksheetNtbkBase::OnClsButton, this);
		Unbind(wxEVT_AUINOTEBOOK_END_DRAG, &CWorksheetNtbkBase::OnEndDrag, this);

		Unbind(wxEVT_AUINOTEBOOK_TAB_RIGHT_DOWN, &CWorksheetNtbkBase::__OnTabRightDown, this);
	}


	void CWorksheetNtbkBase::OnPageClose(wxAuiNotebookEvent& evt)
	{
		if (GetPageCount() > 1)
			RemoveWorksheet(m_ActiveWS);
		else
			wxMessageBox("Workbook must have at least 1 worksheet.");
	}


	void CWorksheetNtbkBase::OnClsButton(wxAuiNotebookEvent& event)
	{
		OnPageClose(event);
	}

	void CWorksheetNtbkBase::OnEndDrag(wxAuiNotebookEvent& event)
	{
		m_WorkbookBase->MarkDirty();
		event.Skip();
	}

	void CWorksheetNtbkBase::__OnTabRightDown(wxAuiNotebookEvent& evt)
	{
		//Case: User is clicking on Sheet1 when the active worksheet is Sheet 2 
		auto WS_RDown = FindWorksheet(evt.GetSelection());
		if (WS_RDown != m_ActiveWS)
		{
			ChangeSelection(evt.GetSelection());

			/*
				ChangeSelection function does not trigger OnNotebookPageChanged event.
				Therefore, manually setting the m_ActiveWS to the changed page.
			*/
			m_ActiveWS = WS_RDown;
			m_ActiveWS->ClearSelection();

			wxAuiNotebookEvent PageChangedEvt;
			PageChangedEvt.SetEventType(wxEVT_AUINOTEBOOK_PAGE_CHANGED);
			wxPostEvent(m_WorkbookBase, evt);
		}

		if (m_ContextMenu)
		{
			delete m_ContextMenu;
			m_ContextMenu = nullptr;
		}
		m_ContextMenu = new wxMenu();

		auto AddNew = m_ContextMenu->Append(ID_WSADD, "Add Worksheet");
		AddNew->SetBitmap(wxArtProvider::GetBitmap(wxART_NEW));

		if (GetPageCount() > 1)
		{
			auto DeleteMenuItem = m_ContextMenu->Append(ID_WSDEL, "Delete");
			DeleteMenuItem->SetBitmap(wxArtProvider::GetBitmap(wxART_DELETE));
		}

		auto RenameMenuItem = m_ContextMenu->Append(ID_WSRENAME, "Rename");
		RenameMenuItem->SetBitmap(wxArtProvider::GetBitmap(wxART_EDIT));

		m_ContextMenu->Bind(wxEVT_MENU, &CWorksheetNtbkBase::OnWSRename, this, ID_WSRENAME);
		m_ContextMenu->Bind(wxEVT_MENU, &CWorksheetNtbkBase::OnWSDel, this, ID_WSDEL);
		m_ContextMenu->Bind(wxEVT_MENU, &CWorksheetNtbkBase::OnWSAdd, this, ID_WSADD);

		OnTabRightDown(evt);
	}


	void CWorksheetNtbkBase::OnWSRename(wxCommandEvent& evt)
	{
		wxString WSName = wxGetTextFromUser("Name of the worksheet:", "Rename Worksheet", m_ActiveWS->GetWSName(), this);
		if (Rename(WSName.ToStdWstring()))
			m_WorkbookBase->MarkDirty();
	}


	void CWorksheetNtbkBase::OnWSDel(wxCommandEvent& evt)
	{
		//Note that upon deletion the page change event occurs and that changes the m_ActiveWS
		//therefore we need to store it in another variable
		auto curWorksheet = m_ActiveWS;

		if (!RemoveWorksheet(m_ActiveWS))
			return;
	}


	void CWorksheetNtbkBase::OnWSAdd(wxCommandEvent& evt)
	{
		if (!AddNewWorksheet())
			wxMessageBox("Cannot add the worksheet!");

		m_WorkbookBase->MarkDirty();
	}


	CWorksheetBase* CWorksheetNtbkBase::CreateWorksheet(wxWindow* wnd, const std::wstring& Label, int nrows, int ncols) const
	{
		return  new CWorksheetBase(wnd, m_WorkbookBase, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, Label, nrows, ncols);;
	}


	bool CWorksheetNtbkBase::AddNewWorksheet(const std::wstring& Label, int nrows, int ncols)
	{
		std::wstring WSLabel = Label;

		if (!WSLabel.empty())
		{
			if (WSLabel.find(L"!") != std::wstring::npos || WorksheetExists(WSLabel))
				return false;

			m_WorksheetNumber++;
		}

		if (WSLabel.empty())
		{
			do
			{
				WSLabel = L"Sheet " + std::to_wstring(++m_WorksheetNumber);

			} while (WorksheetExists(WSLabel));
		}


		auto panel = new wxPanel(this);
		auto sizer = new wxBoxSizer(wxVERTICAL);

		auto varGrid = CreateWorksheet(panel, WSLabel, nrows, ncols);
		varGrid->Bind(ssEVT_GRID_DIRTY, [&](wxCommandEvent& event)
			{
				m_WorkbookBase->MarkDirty();
			});

		sizer->Add(varGrid, 1, wxALL | wxEXPAND, 5);
		panel->SetSizer(sizer);

		m_ActiveWS = varGrid; //current grid

		AddPage(panel, WSLabel, true);

		m_WorkbookBase->MarkDirty();

		return true;
	}


	bool CWorksheetNtbkBase::ImportAsNewWorksheet(const std::wstring& tblname, int nrows, int ncols)
	{
		auto WSName = tblname;

		if (WorksheetExists(WSName))
			WSName = L" " + std::format("{:%F %T}", std::chrono::system_clock::now());

		return AddNewWorksheet(WSName, nrows, ncols);
	}


	bool CWorksheetNtbkBase::RemoveWorksheet(CWorksheetBase* worksheet)
	{
		CWorksheetBase* temp = 0;
		if (!worksheet)
			return false;

		int answer = wxNO;

		bool IsDirty = worksheet->IsDirty();

		if (IsDirty)
			answer = wxMessageBox("Are you sure to delete?", "Confirm", wxYES_NO);

		if (IsDirty && answer == wxNO)
			return false;

		for (size_t i = 0; i < GetPageCount(); i++)
		{
			temp = FindWorksheet(i);
			if (temp == worksheet)
			{
				DeletePage(i);
				m_WorkbookBase->MarkDirty();

				break;
			}
		}

	
		auto ProcessStack = [&](std::stack<std::unique_ptr<WorksheetUndoRedoEvent>>& stk)
		{
			std::stack<std::unique_ptr<WorksheetUndoRedoEvent>> temp;

			while (!stk.empty())
			{
				auto UndoRedoEvt = std::move(stk.top());

				if (UndoRedoEvt->GetEventSource() != worksheet)
					temp.push(std::move(UndoRedoEvt));

				stk.pop();
			}

			while (!temp.empty())
			{
				stk.push(std::move(temp.top()));
				temp.pop();
			}
		};

		ProcessStack(m_WorkbookBase->m_UndoStack);
		ProcessStack(m_WorkbookBase->m_RedoStack);

		return true;
	}


	bool CWorksheetNtbkBase::RemoveWorksheet(const std::wstring& name)
	{
		auto worksheet = GetWorksheet(name);

		if (worksheet)
			return RemoveWorksheet(worksheet);

		return false;
	}


	bool CWorksheetNtbkBase::WorksheetExists(const std::wstring& name)
	{
		if (GetWorksheet(name))
			return true;

		return false;
	}


	bool CWorksheetNtbkBase::Rename(const std::wstring& NewName)
	{
		if (NewName.empty())
		{
			wxMessageBox("Worksheet name cannot be empty");
			return false;
		}

		if (NewName.find(L"!") != std::wstring::npos)
		{
			wxMessageBox("Worksheet name cannot contain \"!\" character");
			return false;
		}

		if (WorksheetExists(NewName)) 
		{
			wxMessageBox(NewName + " already exists!");
			return false;
		}

		m_ActiveWS->SetWSName(NewName);
		SetPageText(GetSelection(), NewName);

		return true;
	}


	void CWorksheetNtbkBase::OnTabRightDown(wxAuiNotebookEvent& evt)
	{
		PopupMenu(m_ContextMenu);
	}



	grid::CWorksheetBase* CWorksheetNtbkBase::GetWorksheet(const std::wstring& name) const
	{
		grid::CWorksheetBase* ws{ nullptr };
		for (size_t i = 0; i < GetPageCount(); i++)
		{
			ws = FindWorksheet(i);

			if (ws->GetWSName() == name)
				return ws;
		}

		return nullptr;
	}


	std::optional<size_t>
		CWorksheetNtbkBase::FindWorksheet(const std::wstring& worksheetname) const
	{
		for (size_t i = 0; i < GetPageCount(); ++i)
		{
			std::wstring Name{ FindWorksheet(i)->GetWSName() };
			if (Name == worksheetname)
				return i;
		}

		return std::nullopt;
	}


	grid::CWorksheetBase* CWorksheetNtbkBase::FindWorksheet(const size_t PageNumber) const
	{
		/*
			Note that CWorksheet is a child of wxPanel therefore
			GetPage returns type wxPanel.
		*/
		auto window = GetPage(PageNumber);

		if(!window)
			return nullptr;

		for (auto elem : window->GetChildren())
		{
			if (elem->IsKindOf(wxCLASSINFO(wxGrid)))
				return (grid::CWorksheetBase*)elem;
		}

		return nullptr;
	}
}