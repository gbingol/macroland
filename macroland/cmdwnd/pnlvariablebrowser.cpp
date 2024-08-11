#include "pnlvariablebrowser.h"

#include <numeric>
#include <wx/artprov.h>

#include "../python/PythonWrapper.h"
#include "pnlcmdwnd.h"

#include "../macrolandapp.h"
#include "../mainfrm/frmmacroland.h"






namespace cmdedit
{

	CPopupContent::CPopupContent(wxWindow* parent, const wxString& str) :wxPopupTransientWindow(parent)
	{
		auto szrMain = new wxBoxSizer(wxVERTICAL);
		auto txtContent = new wxTextCtrl(this, wxID_ANY, str, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_WORDWRAP);
		szrMain->Add(txtContent, 1, wxALL | wxEXPAND, 5);

		SetSizer(szrMain);
		Layout();
	}




	/***********************************************************************/

	CVariableList::CVariableList(wxWindow* parent, PyObject* pymodule) : wxListView(parent)
	{
		m_Module = pymodule;

		SetFont(wxFont(11, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, "Arial"));
		wxItemAttr ColumnHeader;
		ColumnHeader.SetFont(wxFontInfo(14).FaceName("Consolas").Italic());
		ColumnHeader.SetTextColour(wxColor(255, 0, 0));
		SetHeaderAttr(ColumnHeader);

		InsertColumn(0, "Name", wxLIST_FORMAT_CENTER);
		InsertColumn(1, "Type", wxLIST_FORMAT_CENTER);

		PopulateList();

		Bind(wxEVT_LIST_ITEM_SELECTED, &CVariableList::OnListEntrySelected, this);


		auto Notebook = ((frmMacroLand*)wxTheApp->GetTopWindow())->getNotebook();
		Notebook->Bind(wxEVT_NOTEBOOK_PAGE_CHANGED, [this, Notebook](wxNotebookEvent& event)
		{
			int Sel = event.GetSelection();
			if (Sel == wxNOT_FOUND || Notebook->GetPageCount() == 0)
				return;

			auto Wnd = Notebook->GetPage(Sel);

			if (dynamic_cast<pnlCommandWindow*>(Wnd))
				Bind(wxEVT_IDLE, &CVariableList::OnIdle, this);
			else
				Unbind(wxEVT_IDLE, &CVariableList::OnIdle, this);
		});
	}



	void CVariableList::PopulateList()
	{
		auto Dict = PyModule_GetDict(m_Module);
		m_DictSize = PyDict_Size(Dict);

		PyObject* ObjKey, * ObjValue;
		Py_ssize_t pos = 0;

		long index = 0;
		while (PyDict_Next(Dict, &pos, &ObjKey, &ObjValue))
		{
			std::wstring VarName = PyUnicode_AsWideCharString(ObjKey, nullptr);	

			if (VarName[0] == L'_')
				continue;

			if (VarName == L"StdOutput" || VarName == L"CATCHSTDOUTPUT")
				continue;

			auto ObjType = Py_TYPE(ObjValue);
			if (!ObjType)
				continue;
			
			InsertItem(index, VarName);
			SetItem(index, 1, ObjType->tp_name);

			index++;
		}
	}


	void CVariableList::OnIdle(wxIdleEvent& event)
	{
		auto Dict = PyModule_GetDict(m_Module);
		auto Size = PyDict_Size(Dict);
		
		if (Size != m_DictSize)
		{
			m_DictSize = Size;

			DeleteAllItems();
			PopulateList();
		}

		event.Skip();
	}


	void CVariableList::OnListEntrySelected(wxListEvent& event)
	{
		long item = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

		if (item < 0)
		{
			event.Skip();
			return;
		}

		m_SelIndex = item;

		wxString varName = GetItemText(item, 0);

		wxMenu menu;
		auto Item = menu.Append(ID_INSPECT, "Inspect " + varName);
		Item->SetBitmap(wxArtProvider::GetBitmap(wxART_FIND));

		menu.Bind(wxEVT_MENU, &CVariableList::OnPopupMenu, this, ID_INSPECT);

		PopupMenu(&menu);

		event.Skip();
	}



	void CVariableList::OnPopupMenu(wxCommandEvent& event)
	{
		int ID = event.GetId();
		if (ID == ID_INSPECT)
		{
			PyObject* Dict = PyModule_GetDict(m_Module);

			wxString Key = GetItemText(m_SelIndex, 0);
			PyObject* Value = PyDict_GetItemString(Dict, Key.mb_str(wxConvUTF8));
			std::wstring Str = PyUnicode_AsWideCharString(PyObject_Str(Value), nullptr);
			
			auto frmPop = new CPopupContent(this, Str);
			frmPop->SetSize(wxSize(300, 300));

			auto ClientPos = GetPosition();
			auto ScreenPos = ClientToScreen(ClientPos);
			frmPop->SetPosition(ScreenPos);
			frmPop->Show();
		}
	}




	/***************************************************************/

	pnlVariableBrowser::pnlVariableBrowser(wxWindow* parent, PyObject* pymodule) : wxPanel(parent)
	{
		m_Module = pymodule;

		//Refresh Button
		m_btn = new wxBitmapButton(this, wxID_ANY, wxArtProvider::GetBitmap(wxART_REFRESH));
		m_btn->SetToolTip("Update the list since one of the variable's type has been changed since the last update.");
	
		m_varList = new CVariableList(this, m_Module);

		auto szrbtn = new wxBoxSizer(wxHORIZONTAL);
		szrbtn->Add(m_btn, 0, wxALL, 5);

		auto mainSizer = new wxBoxSizer(wxVERTICAL);
		mainSizer->Add(szrbtn, 0, wxEXPAND, 5);
		mainSizer->Add(m_varList, 1, wxEXPAND | wxALL, 5);

		SetSizerAndFit(mainSizer);
		Layout();

		int N = m_varList->GetColumnCount();

		//there is an unused column left by the system so taking up its space
		for (size_t i = 0; i < N; ++i)
			m_varList->SetColumnWidth(i, GetClientRect().width * 0.6); 

		m_btn->Bind(wxEVT_BUTTON, &pnlVariableBrowser::OnBtnClick, this);
	}


	pnlVariableBrowser::~pnlVariableBrowser() = default;


	void pnlVariableBrowser::OnBtnClick(wxCommandEvent& event)
	{
		m_varList->DeleteAllItems();
		m_varList->PopulateList();
	}

}
