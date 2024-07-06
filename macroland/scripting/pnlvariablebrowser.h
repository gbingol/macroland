#pragma once

#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/popupwin.h>

#include <Python.h>


namespace scripting::cmdedit
{
	class CPopupContent : public wxPopupTransientWindow
	{
	public:
		CPopupContent(wxWindow* parent, const wxString& str);
	};



	class CVariableList :public wxListView
	{
	public:
		CVariableList(wxWindow* parent, PyObject* pymodule);
		~CVariableList() = default;

		void PopulateList();

	protected:
		void OnIdle(wxIdleEvent& event);
		void OnListEntrySelected(wxListEvent& event);
		void OnPopupMenu(wxCommandEvent& event);

	private:
		int m_SelIndex{ -1 };
		Py_ssize_t m_DictSize;
		PyObject* m_Module{ nullptr };

		const int ID_INSPECT{ wxNewId() };
	};




	/*********************************************/

	class pnlVariableBrowser : public wxPanel
	{
	public:
		pnlVariableBrowser(wxWindow* parent, PyObject* pymodule);
		~pnlVariableBrowser();

	protected:
		void OnBtnClick(wxCommandEvent& event);
		
	private:
		wxBitmapButton* m_btn;
		CVariableList* m_varList;
		PyObject* m_Module;
	};

}