#pragma once

#include <vector>
#include <list>
#include <map>
#include <set>
#include <filesystem>


#include <wx/wx.h>
#include <wx/aui/auibook.h>
#include <wx/aui/aui.h>
#include <wx/popupwin.h>
#include <wx/tglbtn.h>
#include <wx/colordlg.h>

#include "../grid/worksheetbase.h"
#include "../grid/ntbkbase.h"
#include "../grid/workbookbase.h"

#include "../python/PythonWrapper.h"







namespace grid
{
	class CWorkbookBase;
}

namespace extension
{
	class CToolBarNtbk;
	class CDropDownTool;
}





namespace ICELL
{
	class CWorksheetNtbk;
	class CWorkbook;

	wxDECLARE_EVENT(ssEVT_WB_PAGECHANGED, wxAuiNotebookEvent);
	wxDECLARE_EVENT(ssEVT_GRID_SELECTION_BEGUN, wxGridRangeSelectEvent);


	class CWorksheet :public grid::CWorksheetBase
	{
	public:
		CWorksheet(wxWindow* panel,
			CWorkbook* workbook,
			wxWindowID id,
			const wxPoint& pos,
			const wxSize& size,
			long style,
			wxString WindowName,
			int nrows = 1000,
			int ncols = 100);

		virtual ~CWorksheet();


		void BindPythonFunction(
			std::string EventName,
			Python::CEventCallbackFunc* Callbackfunc);

		void UnbindPythonFunction(
			std::string EventName,
			PyObject* FunctionObj);


		void RegisterPyWS(Python::Worksheet* ws) {
			m_PyWS.push_back(ws);
		}

		CWorkbook* GetWorkbook() const {
			return m_Workbook;
		}

		wxWindow* GetParent() const {
			return m_ParentWnd;
		}

		wxMenu* GetContextMenu() const
		{
			return m_ContextMenu;
		}


	protected:
		void OnRightClick(wxGridEvent& event);

		void OnRangeSelectionBegun(wxGridRangeSelectEvent& event);
		void OnRangeSelecting(wxGridRangeSelectEvent& event);
		void OnRangeSelected(wxGridRangeSelectEvent& event);

		void OnSelectCell(wxGridEvent& event);

	private:
		void CallRegisteredPyFuncs(const std::string& event);

	protected:
		CWorkbook* m_Workbook = nullptr;
		wxMenu* m_ContextMenu{nullptr};

	private:
		std::list <Python::Worksheet*> m_PyWS;
		std::map<std::string, std::list< Python::CEventCallbackFunc*>> m_EvtCallBack;

		//The file content of ws_selecting.py file
		wxString m_WS_Selecting_Py;
		bool m_SelectionBegun{ false };

		bool m_FirePySelectingEvt{true};
		bool m_FirePySelectedEvt{true};

		wxWindow* m_ParentWnd;
	};


	/********************************************************* */

	class CWorksheetNtbk : public grid::CWorksheetNtbkBase
	{

	public:
		CWorksheetNtbk(CWorkbook* parent);
		virtual ~CWorksheetNtbk();

		grid::CWorksheetBase* CreateWorksheet(
			wxWindow* wnd, 
			const std::wstring& Label, 
			int nrows, 
			int ncols) const override;

		auto GetContextMenu() const
		{
			return m_ContextMenu;
		}


	protected:
		void OnPageChanged(wxAuiNotebookEvent& evt);
		void OnTabRightDown(wxAuiNotebookEvent& evt) override;

	private:
		CWorkbook* m_Workbook{nullptr};
	};



	/************************************************************** */


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
		void OnFontChanged(wxCommandEvent& event); //Font size or face
		void OnFillFontColor(wxAuiToolBarEvent& event);
		void OnUndoRedoStackChanged(wxCommandEvent& event);

	private:
		void Init_TB_Home();
		

	private:
		wxComboBox* m_ComboFontFace, *m_ComboFontSize;
		wxColour m_FillColor, m_FontColor;
		extension::CToolBarNtbk* m_PagedTB;
		wxAuiToolBar* m_TB_Home;

		const int ID_FILLCOLOR = wxNewId();
		const int ID_FONTCOLOR = wxNewId();
		const int ID_FONTBOLD = wxNewId();
		const int ID_FONTITALIC = wxNewId();
		const int ID_FONTSIZE = wxNewId();
		const int ID_FONTFACE = wxNewId();

		const int ID_PASTE = wxNewId();
		const int ID_UNDO = wxNewId();
		const int ID_REDO = wxNewId();

		const int ID_COMMIT = wxNewId();
		const int ID_ALIGN{ wxNewId() };

		std::map<std::string, std::list< Python::CEventCallbackFunc*>> m_EvtCallBack;
	};
}