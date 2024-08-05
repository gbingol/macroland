#pragma once

#include <vector>
#include <list>
#include <map>
#include <set>
#include <filesystem>

#include "../python/PythonWrapper.h" 

#include <wx/wx.h>
#include <wx/grid.h>
#include <wx/xml/xml.h>
#include <wx/textfile.h>
#include <wx/wfstream.h>
#include <wx/zipstrm.h>

#include <grid/worksheetbase.h>
#include <grid/rangebase.h>



wxDECLARE_EVENT(ssEVT_GRID_SELECTION_BEGUN, wxGridRangeSelectEvent);



namespace grid
{
	class CWorkbookBase;
}



namespace ICELL
{
	class CWorksheetNtbk;
	class CWorkbook;

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
	protected:
		CWorkbook* m_Workbook = nullptr;
		wxMenu* m_ContextMenu{nullptr};

	private:
		std::list <Python::Worksheet*> m_PyWS;
		std::map<std::string, std::list< Python::CEventCallbackFunc*>> m_EvtCallBack;

		//The file content of ws_selecting.py file
		wxString m_WS_Selecting_Py;
		bool m_SelectionBegun{ false };

		wxWindow* m_ParentWnd;
	};

}