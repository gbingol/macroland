#pragma once

#include <fstream>
#include <filesystem>

#include <wx/wx.h>

#include <grid/ntbkbase.h>



namespace grid
{
	class CWorksheetBase;
}


namespace ICELL
{
	class CWorksheet;
	class CWorkbook;

	wxDECLARE_EVENT(ssEVT_WB_PAGECHANGED, wxAuiNotebookEvent);

	class CWorksheetNtbk : public grid::CWorksheetNtbkBase
	{

	public:
		CWorksheetNtbk(CWorkbook* parent);
		virtual ~CWorksheetNtbk();

		grid::CWorksheetBase* CreateWorksheet(wxWindow* wnd, const std::wstring& Label, int nrows, int ncols) const;

		auto GetContextMenu() const
		{
			return m_ContextMenu;
		}


	protected:
		void OnPageChanged(wxAuiNotebookEvent& evt);
		void OnTabRightDown(wxAuiNotebookEvent& evt) override;

	private:
		CWorkbook* m_Workbook;
	};
}
