#pragma once

#include <wx/wx.h>
#include <wx/caret.h>
#include <wx/stc/stc.h> 
#include <wx/html/htmlwin.h>
#include <wx/listctrl.h>
#include <wx/popupwin.h>
#include <wx/minifram.h>

#include "dllimpexp.h"




DLLSCRIPT wxDECLARE_EVENT(ssEVT_AUTOCOMP_CANCELLED, wxCommandEvent);


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

		//Attach a help window to set its position automatically when showing the AutoCompletion
		DLLSCRIPT void AttachHelpWindow(wxWindow* const HelpWindow);


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
		wxPoint ComputeShowPositon();

	private: 
		wxStyledTextCtrl* m_STC = nullptr;
		wxListBox* m_ListBox{ nullptr };

		wxTopLevelWindow* m_PrntWnd{ nullptr };

		//The help window that will be shown when  selections are made at autocompletion
		wxWindow* m_HelpWnd = nullptr;
	};
}