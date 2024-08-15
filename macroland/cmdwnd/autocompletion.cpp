#include "autocompletion.h"

#include <sstream>

#include "styledtxtctrl.h"


#define IF_SKIP_RET(cond) \
	if(cond) {event.Skip(); return;}



namespace
{
	wxPoint ComputeShowPositon(int curPos, wxStyledTextCtrl* STC, const wxMiniFrame* frm)
	{
		long col, line;
		STC->PositionToXY(curPos, &col, &line);

		wxPoint pos = STC->PointFromPosition(curPos);
		int TxtHeight = STC->TextHeight(line);

		auto TL = STC->ClientToScreen(pos);
		TL.y += TxtHeight;

		int Btm_Y = STC->GetScreenPosition().y + frm->GetSize().GetHeight();
		int ScreenY = wxGetDisplaySize().GetHeight();

		if (Btm_Y > ScreenY)
			TL.y = TL.y - frm->GetSize().y;

		return TL;
	}
}


namespace cmdedit
{

	wxDEFINE_EVENT(ssEVT_FLOATFRAME_SHOWN, wxCommandEvent);

	AutoCompCtrl::AutoCompCtrl(	wxStyledTextCtrl* stc, wxWindowID id, const wxPoint& pos, const wxSize& size) :
		wxMiniFrame(stc, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0 | wxTAB_TRAVERSAL)
	{
		m_STC = stc;

		m_ListBox = new wxListBox(this, id, pos, size);

		auto Szr = new wxBoxSizer(wxVERTICAL);
		Szr->Add(m_ListBox, 1, wxEXPAND, 5);
		SetSizerAndFit(Szr);
		Layout();
		
		Bind(wxEVT_SHOW, &AutoCompCtrl::OnShow, this);

		m_ListBox->Bind(wxEVT_KEY_DOWN, &AutoCompCtrl::OnKeyDown, this);
		m_ListBox->Bind(wxEVT_KEY_UP, &AutoCompCtrl::OnKeyUp, this);
		m_ListBox->Bind(wxEVT_LISTBOX_DCLICK, [this](wxCommandEvent &evt)
		{
			InsertSelection();
			return; 
		});
	}

	AutoCompCtrl::~AutoCompCtrl() = default;

	void AutoCompCtrl::OnShow(wxShowEvent &evt)
	{
		if(evt.IsShown())
		{
			m_STC->Bind(wxEVT_KEY_DOWN, &AutoCompCtrl::OnParent_KeyDown, this);
			m_STC->Bind(wxEVT_KEY_UP, &AutoCompCtrl::OnParent_KeyUp, this);
		}
		else
		{
			m_STC->Unbind(wxEVT_KEY_DOWN, &AutoCompCtrl::OnParent_KeyDown, this);
			m_STC->Unbind(wxEVT_KEY_UP, &AutoCompCtrl::OnParent_KeyUp, this);
		}
		evt.Skip();
	}

	void AutoCompCtrl::OnKeyDown(wxKeyEvent &event)
	{
		int evtCode = event.GetKeyCode();
		if (evtCode == WXK_ESCAPE)
			Hide();

		event.Skip();
	}


	void AutoCompCtrl::OnKeyUp(wxKeyEvent& event)
	{
		int evtCode = event.GetKeyCode();
		if (evtCode == WXK_RETURN || evtCode == WXK_TAB)
			InsertSelection();

		event.Skip();
	}


	void AutoCompCtrl::OnParent_KeyDown(wxKeyEvent& event)
	{
		int evtCode = event.GetKeyCode();

		if (evtCode == WXK_ESCAPE)
			Hide();

		else if (evtCode == WXK_UP || evtCode == WXK_DOWN || 
				evtCode == WXK_RETURN || evtCode == WXK_TAB)
		{
			wxMiniFrame::SetFocus();
			if(m_ListBox->GetSelection() == -1)
				m_ListBox->SetSelection(0);
			
			m_ListBox->SetFocus();

			//dont let parents OnKeyDown to be called, as AutoComp has the focus
			return;
		}

		else if(evtCode == WXK_BACK)
		{
			int pos = m_STC->GetCurrentPos();
			auto str = m_STC->GetTextRange(pos-1, pos);
			if(str == ".")
				Hide();
		}

		event.Skip();
	}


	void AutoCompCtrl::OnParent_KeyUp(wxKeyEvent& event)
	{
		int evtCode = event.GetKeyCode();
		auto UniCode = event.GetUnicodeKey();

		wxString word = GetCurrentWord();
		if (word.empty())
		{
			Hide();
			event.Skip();
			return;
		}

		for (size_t i = 0; i < m_ListBox->GetCount(); ++i)
		{
			auto s = m_ListBox->GetString(i);
			if(s.substr(0, word.length()) == word)
			{
				m_ListBox->SetSelection(i);
				break;
			}
		}
			
		event.Skip();
	}



	void AutoCompCtrl::Hide()
	{
		wxMiniFrame::Hide();
		m_STC->SetFocus();
	}


	void AutoCompCtrl::Show(const std::list<std::string>& List)
	{
		if (List.size() == 0) {
			Hide();
			return;
		}

		m_ListBox->Clear();

		for (const auto& elem : List)
			m_ListBox->Append(elem);

		wxString word = GetCurrentWord();
		if (!word.empty())
		{
			for (size_t i = 0; i < m_ListBox->GetCount(); ++i)
			{
				auto s = m_ListBox->GetString(i);
				if(s.substr(0, word.length()) == word)
				{
					m_ListBox->SetSelection(i);
					break;
				}
			}
		}

		SetPosition(ComputeShowPositon());
		wxMiniFrame::Show(true);

		wxCommandEvent showEvt;
		showEvt.SetEventType(ssEVT_FLOATFRAME_SHOWN);
		showEvt.SetEventObject(this);
		wxPostEvent(this, showEvt);
	}



	wxString AutoCompCtrl::GetCurrentWord() const
	{
		int StartPos = m_STC->WordStartPosition(m_STC->GetCurrentPos(), true);
		int EndPos = m_STC->WordEndPosition(StartPos, true);

		if (EndPos > StartPos)
			return m_STC->GetTextRange(StartPos, EndPos);

		return wxEmptyString;
	}


	void AutoCompCtrl::InsertSelection()
	{
		int posSt = m_STC->WordStartPosition(m_STC->GetCurrentPos(), true);
		int posEnd = m_STC->WordEndPosition(posSt, true);
		m_STC->DeleteRange(posSt, posEnd - posSt);

		wxString SelTxt = m_ListBox->GetStringSelection();
		m_STC->WriteText(SelTxt);
		m_STC->SetCurrentPos(posSt + SelTxt.length());

		Hide();
		m_STC->SetFocus();
	}

	wxPoint AutoCompCtrl::ComputeShowPositon()
	{
		int curPos = m_STC->WordStartPosition(m_STC->GetCurrentPos(), true);
		return ::ComputeShowPositon(curPos, m_STC, this);
	}



	/************************************************************************ */

	frmParamsDocStr::frmParamsDocStr(	wxStyledTextCtrl* stc, wxWindowID id, const wxPoint& pos, const wxSize& size) :
		wxMiniFrame(stc, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxRESIZE_BORDER)
	{
		auto ScSize = FromDIP(wxGetDisplaySize());
		SetSize(ScSize.GetWidth()/4, ScSize.GetHeight()/5);

		m_STC = stc;

		m_InfoWnd = new CStyledTextCtrl(this);
		m_InfoWnd->SetMarginWidth(0, 0);
		m_InfoWnd->SetViewWhiteSpace(false);
		m_InfoWnd->SetIndentationGuides(false);

		auto Szr = new wxBoxSizer(wxVERTICAL);
		Szr->Add(m_InfoWnd, 1, wxEXPAND, 5);
		SetSizer(Szr);
		Layout();

		Bind(wxEVT_SHOW, &frmParamsDocStr::OnShow, this);
		m_InfoWnd->Bind(wxEVT_KEY_DOWN, &frmParamsDocStr::OnKeyDown, this);
	}

	frmParamsDocStr::~frmParamsDocStr() = default;


	void frmParamsDocStr::OnShow(wxShowEvent &evt)
	{
		if(evt.IsShown())
			m_STC->Bind(wxEVT_KEY_DOWN, &frmParamsDocStr::OnParent_KeyDown, this);
		else
			m_STC->Unbind(wxEVT_KEY_DOWN, &frmParamsDocStr::OnParent_KeyDown, this);
		evt.Skip();
	}


	void frmParamsDocStr::OnKeyDown(wxKeyEvent &evt)
	{
		int evtCode = evt.GetKeyCode();	 
		if (evtCode == WXK_ESCAPE) 
			Hide();

		evt.Skip();
	}


	void frmParamsDocStr::OnParent_KeyDown(wxKeyEvent &event)
	{
		int evtCode = event.GetKeyCode();

		if(evtCode == WXK_ESCAPE)
			Hide();
		
		else if(evtCode == WXK_BACK)
		{
			int pos = m_STC->GetCurrentPos();
			if(pos>=0)
			{
				auto str = m_STC->GetTextRange(pos-1, pos);
				if(str == "(")
					Hide();
			}
		}

		event.Skip();
	}


	void frmParamsDocStr::Show(const std::pair<wxString, wxString> text)
	{
		const auto [Params, Doc] = text;

		m_InfoWnd->ClearAll();

		if(!Params.empty())
		{
			m_InfoWnd->AppendText(Params);
			
			if(!Doc.empty())
			{
				m_InfoWnd->AppendLine();
				m_InfoWnd->AppendText("___________________________________________");
				m_InfoWnd->AppendLine();
			}
		}
		
		if(!Doc.empty())
			m_InfoWnd->AppendText(Doc);


		SetPosition(ComputeShowPositon());
		wxMiniFrame::Show(true);

		wxCommandEvent showEvt;
		showEvt.SetEventType(ssEVT_FLOATFRAME_SHOWN);
		showEvt.SetEventObject(this);
		wxPostEvent(this, showEvt);
	}

	

	void frmParamsDocStr::Hide()
	{
		wxMiniFrame::Hide();
		m_STC->SetFocus();
	}



	wxPoint frmParamsDocStr::ComputeShowPositon()
	{
		int curPos = m_STC->GetCurrentPos();
		return ::ComputeShowPositon(curPos, m_STC, this);
	}


}