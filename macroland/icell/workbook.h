#pragma once

#include <optional>
#include <fstream>
#include <filesystem>

#include <stack>
#include <map>

#include <Python.h>

#include <wx/wx.h>
#include <wx/aui/aui.h>

#include <wx/clipbrd.h>
#include <wx/colordlg.h>
#include <wx/ribbon/bar.h>
#include <wx/ribbon/buttonbar.h>
#include <wx/ribbon/toolbar.h>
#include <wx/aui/auibook.h>
#include <wx/artprov.h>
#include <wx/popupwin.h>
#include <wx/tglbtn.h>

#include "../python/callbackfunction.h"
#include <grid/workbookbase.h>

#include "wsntbk.h"



namespace grid
{
	class CWorksheetBase;
}


namespace lua
{
	class CToolBarNtbk;
}



namespace ICELL
{
	
	class CDropDownTool;

	class CWorkbook;

	class CPopupAlignment : public wxPopupTransientWindow
	{
	public:
		CPopupAlignment(CWorkbook* parent, const wxPoint& Position);

	protected:
		void OnToggleHoriz(wxCommandEvent& event);
		void OnToggleVert(wxCommandEvent& event);

		void OnDismiss() override;

	private:
		CWorkbook* m_Parent;
		wxBitmapToggleButton* m_btnHorLeft;
		wxBitmapToggleButton* m_btnHorCenter;
		wxBitmapToggleButton* m_btnHorRight;
		wxBitmapToggleButton* m_btnVertBtm;
		wxBitmapToggleButton* m_btnVertCenter;
		wxBitmapToggleButton* m_btnVertTop;
	};



	class CWorkbook : public grid::CWorkbookBase
	{
	public:
		CWorkbook(wxWindow* parent);
		virtual ~CWorkbook();

		CWorksheetNtbk* GetWorksheetNotebook() const override
		{
			return (CWorksheetNtbk*)m_WSNtbk;
		}

		bool AddNewWorksheet(const std::wstring& tblname = L"", int nrows = 1000, int ncols = 50) override
		{
			return GetWorksheetNotebook()->AddNewWorksheet(tblname, nrows, ncols);
		}

		void ChangeCellAlignment(int ID)
		{
			grid::CWorkbookBase::ChangeCellAlignment(ID);
		}

		auto GetToolBarNtbk() const
		{
			return m_PagedTB;
		}

		void SetStatusText(const wxString& text, int number = 0);

		void BindPythonFunction(std::string EventName, Python::CEventCallbackFunc* Callbackfunc);
		void UnbindPythonFunction(std::string EventName, PyObject* FunctionObj);

	protected:

		void OnUpdateUI(wxUpdateUIEvent& evt);
		void OnWorkbookPageChanged(wxAuiNotebookEvent& event);
		void OnPaste(wxAuiToolBarEvent& event);
		void OnFontChanged(wxCommandEvent& event); //Font size or face
		void OnFillFontColor(wxAuiToolBarEvent& event);
		void OnUndoRedoStackChanged(wxCommandEvent& event);

	private:
		void Init_TB_Home();
		

	private:
		wxComboBox* m_ComboFontFace, *m_ComboFontSize;
		wxColour m_FillColor, m_FontColor;
		lua::CToolBarNtbk* m_PagedTB;
		wxAuiToolBar* m_TB_Home;

		const int ID_FILLCOLOR = wxNewId();
		const int ID_FONTCOLOR = wxNewId();
		const int ID_FONTBOLD = wxNewId();
		const int ID_FONTITALIC = wxNewId();
		const int ID_FONTSIZE = wxNewId();
		const int ID_FONTFACE = wxNewId();

		const int ID_COPY = wxNewId();
		const int ID_CUT = wxNewId();
		const int ID_PASTE = wxNewId();
		const int ID_PASTE_VALUES = wxNewId();
		const int ID_PASTE_FORMAT = wxNewId();
		const int ID_UNDO = wxNewId();
		const int ID_REDO = wxNewId();

		const int ID_COMMIT = wxNewId();
		const int ID_ALIGN{ wxNewId() };

		std::map<std::string, std::list< Python::CEventCallbackFunc*>> m_EvtCallBack;
	};

}
