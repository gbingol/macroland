#pragma once

#include <vector>
#include <list>
#include <map>
#include <set>
#include <filesystem>

#include "../python/PythonWrapper.h" 
#include "../python/callbackfunction.h"

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
	class CRange;
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

		void RegisterPyRng(Python::Range* rangeObj) {
			m_PyRngObj.push_back(rangeObj);
		}

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


		std::unique_ptr<CRange> GetSelection();

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
		wxMenu* m_ContextMenu;

	private:
		std::list <Python::Worksheet*> m_PyWS;
		std::list<Python::Range*> m_PyRngObj;
		std::map<std::string, std::list< Python::CEventCallbackFunc*>> m_EvtCallBack;

		//The file content of ws_selecting.py file
		wxString m_WS_Selecting_Py;
		bool m_SelectionBegun{ false };



		const int ID_CLEARCELLCONTENT{ wxNewId() };
		const int ID_COPY{ wxNewId() };
		const int ID_CUT{ wxNewId() };

		wxWindow* m_ParentWnd;
	};




	/******************************** RANGE   ************************************/



	class CRange : public grid::CRangeBase
	{
	public:
		CRange() = default;
		CRange(grid::CWorksheetBase* ws, const wxGridCellCoords& TL, const wxGridCellCoords& BR);

		CRange(const wxString& str, grid::CWorkbookBase* wb); //there is a selection text

		CRange(const CRange& rhs) = default;
		CRange& operator=(const CRange& rhs) = default;

		CRange(CRange&& rhs) noexcept = default;
		CRange& operator=(CRange&& rhs) noexcept = default;

		virtual ~CRange();

		CWorksheet* GetWorksheet() const override 
		{
			return (CWorksheet*)m_WSheet;
		}

		//whole range (axis:1, row by row, axis:0, col by col)
		std::vector<std::vector<std::wstring>>
			toArrays(size_t axis = 1) const;

		std::vector<std::wstring> toArray() const;
	};
}