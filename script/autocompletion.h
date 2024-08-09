#pragma once

#include <wx/wx.h>
#include <wx/stc/stc.h> 
#include <wx/listctrl.h>
#include <wx/popupwin.h>
#include <wx/minifram.h>
#include <wx/html/htmlwin.h>

#include "dllimpexp.h"



namespace script
{

	class AutoCompCtrl :public wxMiniFrame
	{
	public:
		DLLSCRIPT AutoCompCtrl(
			wxStyledTextCtrl* stc, 
			wxWindowID id = wxID_ANY,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxSize(200, 200));

		DLLSCRIPT ~AutoCompCtrl();

		//pair: first is key, second is the data type
		DLLSCRIPT void Show(const std::list<std::string>& List);

		auto GetListBox() const{
			return m_ListBox;
		}

		DLLSCRIPT void Hide();

		DLLSCRIPT wxString GetCurrentWord() const;


		//Is there an auto-completion list visible
		bool Active() const {
			return IsShown();
		}

		bool IsActive() const {
			return IsShown();
		}

		auto GetStringSelection() const{
			return m_ListBox->GetStringSelection();
		}

	protected:
		void OnKeyDown(wxKeyEvent& evt);
		void OnKeyUp(wxKeyEvent& evt);
		void OnParentWindow_KeyDown(wxKeyEvent& event);
		void OnParentWindow_KeyUp(wxKeyEvent& event);
	private:
		void InsertSelection();
		wxPoint ComputeShowPositon();

	private: 
		wxStyledTextCtrl* m_STC = nullptr;
		wxListBox* m_ListBox{ nullptr };
	};



	/***************************************** */

	class FuncParamsDocStr :public wxMiniFrame
	{
	public:
		DLLSCRIPT FuncParamsDocStr(
			wxStyledTextCtrl* stc, 
			wxWindowID id = wxID_ANY,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxSize(200, 200));

		DLLSCRIPT ~FuncParamsDocStr();

		DLLSCRIPT void Hide();

		//first is parameters and second is doc string
		DLLSCRIPT void Show(const std::pair<wxString, wxString> text);
	
	protected:
		void OnKeyDown(wxKeyEvent& evt);
		void OnParentWindow_KeyDown(wxKeyEvent& event);

	private:
		wxPoint ComputeShowPositon();

	private: 
		wxStyledTextCtrl* m_STC = nullptr;
		wxHtmlWindow* m_HTMLWnd{ nullptr };
	};
}