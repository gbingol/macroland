#pragma once

#include <wx/wx.h>
#include <wx/stc/stc.h> 
#include <wx/listctrl.h>
#include <wx/popupwin.h>
#include <wx/minifram.h>




namespace cmdedit
{

	wxDECLARE_EVENT(ssEVT_FLOATFRAME_SHOWN, wxCommandEvent);

	class CStyledTextCtrl;

	class AutoCompCtrl :protected wxMiniFrame
	{
	public:
		AutoCompCtrl(
			wxStyledTextCtrl* stc, 
			wxWindowID id = wxID_ANY,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxSize(200, 200));

		~AutoCompCtrl();

		bool IsShown() const override
		{
			return wxMiniFrame::IsShown();
		}

		//pair: first is key, second is the data type
		void Show(const std::list<std::string>& List);

		auto GetListBox() const{
			return m_ListBox;
		}

		void Hide();

		wxString GetCurrentWord() const;

		auto GetStringSelection() const{
			return m_ListBox->GetStringSelection();
		}

	protected:
		void OnShow(wxShowEvent &evt);
		void OnKeyDown(wxKeyEvent& evt);
		void OnKeyUp(wxKeyEvent& evt);
		void OnParent_KeyDown(wxKeyEvent& event);
		void OnParent_KeyUp(wxKeyEvent& event);
	private:
		void InsertSelection();
		wxPoint ComputeShowPositon();

	private: 
		wxStyledTextCtrl* m_STC = nullptr;
		wxListBox* m_ListBox{ nullptr };
	};



	/***************************************** */

	class frmParamsDocStr :protected wxMiniFrame
	{
	public:
		frmParamsDocStr(
			wxStyledTextCtrl* stc, 
			wxWindowID id = wxID_ANY,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxSize(200, 200));

		~frmParamsDocStr();

		bool IsShown() const override
		{
			return wxMiniFrame::IsShown();
		}

		void Hide();

		//first is parameters and second is doc string
		void Show(const std::pair<wxString, wxString> text);
	
	protected:
		void OnShow(wxShowEvent &evt);
		void OnKeyDown(wxKeyEvent& evt);
		void OnParent_KeyDown(wxKeyEvent& event);

	private:
		wxPoint ComputeShowPositon();

	private: 
		wxStyledTextCtrl* m_STC = nullptr;
		CStyledTextCtrl* m_InfoWnd{ nullptr };
	};
}