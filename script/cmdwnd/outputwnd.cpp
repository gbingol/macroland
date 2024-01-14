#include "outputwnd.h"

#include <wx/artprov.h>
#include <wx/clipbrd.h>

namespace script
{
	COutputWnd::COutputWnd(wxWindow* parent,
		wxWindowID id,
		script::CStyledTextCtrl* InputWnd) : CStyledTextCtrl(parent, id, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE)
	{
		m_PrntWnd = parent;
		m_InputWnd = InputWnd;

		//Since the goal is NOT to edit (this is output wnd), no need to show extra characters and bloat output
		SetViewEOL(false);
		SetViewWhiteSpace(false);

		//when number of outputs increase showing code folding makes the appearance rather confusing
		SetMarginWidth(FOLDMARGIN, 0);

		SetUseHorizontalScrollBar(false);

		Bind(wxEVT_STC_MODIFIED, &COutputWnd::OnModified, this);
		Bind(wxEVT_RIGHT_UP, &COutputWnd::OnRightUp, this);

		Bind(wxEVT_SET_FOCUS, &COutputWnd::OnSetFocus, this);
		Bind(wxEVT_KILL_FOCUS, &COutputWnd::OnKillFocus, this);

		m_PrntWnd->Bind(wxEVT_SIZE, [&](wxSizeEvent& event)
			{
				wxStyledTextEvent evt;
				evt.SetEventType(wxEVT_STC_MODIFIED);
				wxPostEvent(this, evt);

				event.Skip();
			});
	}


	wxSize COutputWnd::DoGetBestSize() const
	{
		wxSize sz = GetBestSize();
		return sz;
	}


	void COutputWnd::AppendOutput(const wxString& txt, bool PrependLine)
	{
		if (!GetSelectedText().empty())
			SelectNone();

		if (!GetText().empty() && PrependLine)
			AppendLine();

		AppendText(txt);

		if (!IsShown())
			Show();
	}


	void COutputWnd::OnModified(wxStyledTextEvent& event)
	{
		wxSize szPrnt = m_PrntWnd->GetClientSize();

		int Line_H = TextHeight(0);
		int NLines = GetLineCount();

		if (NLines == 0)
		{
			Hide();
			return;
		}

		int Height = Line_H * NLines;
		int InputWnd_H = 0;
		if (m_InputWnd)
			InputWnd_H = m_InputWnd->FromDIP(m_InputWnd->TextHeight(0));

		int line = NLines;
		while ((Height + InputWnd_H) >= szPrnt.y && line <= NLines)
			Height = Line_H * (line--);

		int W = szPrnt.x;

		SetSize(wxSize(W, Height));
		ShowPosition(GetLastPosition());
	}


	void COutputWnd::OnSetFocus(wxFocusEvent& event)
	{
		wxClientDC dc(this);
		wxSize szTxt = dc.GetMultiLineTextExtent(GetText());

		wxFont fnt = GetFont();
		auto px = fnt.GetPixelSize();
		int pt = fnt.GetPointSize();

		SetScrollWidth(szTxt.x * px.y / pt);
		SetUseHorizontalScrollBar(true);

		event.Skip();
	}

	void COutputWnd::OnKillFocus(wxFocusEvent& event)
	{
		SetUseHorizontalScrollBar(false);
		event.Skip();
	}


	void COutputWnd::OnRightUp(wxMouseEvent& event)
	{
		wxMenu menu;

		wxString Lbl = m_ShowLineNo ? wxString("Hide Line Numbers") : wxString("Show Line Numbers");

		auto Item = menu.Append(ID_SHOWLINENO, Lbl);
		Item->SetBitmap(wxArtProvider::GetBitmap(wxART_REFRESH));
		menu.Bind(wxEVT_COMMAND_MENU_SELECTED, &COutputWnd::OnPopMenu, this, ID_SHOWLINENO);

		Item = menu.Append(ID_SAVE, "Save");
		Item->SetBitmap(wxArtProvider::GetBitmap(wxART_FILE_SAVE));
		menu.Bind(wxEVT_COMMAND_MENU_SELECTED, &COutputWnd::OnPopMenu, this, ID_SAVE);

		if (!GetSelectedText().empty())
		{
			Item = menu.Append(ID_COPY, "Copy");
			Item->SetBitmap(wxArtProvider::GetBitmap(wxART_COPY));
			menu.Bind(wxEVT_COMMAND_MENU_SELECTED, &COutputWnd::OnPopMenu, this, ID_COPY);
		}

		menu.AppendSeparator();

		Item = menu.Append(ID_DELALL, "Delete All");
		Item->SetBitmap(wxArtProvider::GetBitmap(wxART_DELETE));
		menu.Bind(wxEVT_COMMAND_MENU_SELECTED, &COutputWnd::OnPopMenu, this, ID_DELALL);

		PopupMenu(&menu);
	}


	void COutputWnd::OnPopMenu(wxCommandEvent& event)
	{
		int evtID = event.GetId();

		if (evtID == ID_COPY)
		{
			wxString Txt = GetSelectedText();
			if (wxTheClipboard->Open()) 
			{
				wxTheClipboard->SetData(new wxTextDataObject(Txt));
				wxTheClipboard->Close();
			}
		}
		else if (evtID == ID_DELALL)
		{
			ClearAll();
		}
		else if (evtID == ID_SHOWLINENO)
		{
			SetMarginWidth(LINENUMBERMARGIN, m_ShowLineNo ? 0 : TextWidth(wxSTC_STYLE_LINENUMBER, "9999"));
			m_ShowLineNo = !m_ShowLineNo;
		}
		else if (evtID == ID_SAVE)
		{
			wxFileDialog dlgSave(this, "Save contents", "", "", "*.py|*.py", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
			int resp = dlgSave.ShowModal();

			if (resp == wxID_OK)
			{
				try
				{
					std::filesystem::path Path = dlgSave.GetPath().ToStdWstring();
					SaveStyledText(Path);
				}
				catch (const std::exception& e)
				{
					wxMessageBox(e.what());
				}
			}
		}

	}
}
