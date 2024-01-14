#include "pnlHistory.h"

#include "cmdline.h"

#include <wx/artprov.h>
#include <wx/clipbrd.h>


namespace scripting::cmdedit
{
	pnlBrowseHistory::pnlBrowseHistory(
		wxWindow* parent, 
		CCmdLine* cmdline) : wxPanel(parent)
	{
		assert(cmdline != nullptr);

		m_CmdLine = cmdline;

		SetSizeHints(wxDefaultSize, wxDefaultSize);
		
		m_btn = new wxBitmapButton(this, wxID_ANY, wxArtProvider::GetBitmap(wxART_COPY));
		m_btn->Enable(false);
		m_lstBox = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_SINGLE | wxLB_EXTENDED);

		auto History = m_CmdLine->GetCommandHist();

		for (auto it{ std::crbegin(History) }; it != std::crend(History); ++it)
		{
			const auto& Elem = *it;
			
			if (std::holds_alternative<wxString>(Elem))
				m_lstBox->Append(std::get<wxString>(Elem));
			else
			{
				const auto& Lst = std::get<std::list<wxString>>(Elem);
				for (const auto& cmd : Lst)
				{
					if (cmd.empty())
						continue;

					m_lstBox->Append(cmd);
				}
			}

		}
		auto szrbtns = new wxBoxSizer(wxHORIZONTAL);
		szrbtns->Add(m_btn, 0, wxALL | wxEXPAND, 5);

		auto mainSizer = new wxBoxSizer(wxVERTICAL);
		mainSizer->Add(szrbtns, 0, wxEXPAND, 0);
		mainSizer->Add(m_lstBox, 1, wxALL | wxEXPAND, 5);
		SetSizerAndFit(mainSizer);
		Layout();


		cmdline->GetInputWnd()->Bind(ssEVT_SCRIPTCTRL_RETURN, &pnlBrowseHistory::OnCmdExecuted, this);
		m_btn->Bind(wxEVT_BUTTON, &pnlBrowseHistory::OnCopyBtn, this);
		m_lstBox->Bind(wxEVT_LISTBOX, &pnlBrowseHistory::OnListBox, this);
		m_lstBox->Bind(wxEVT_LISTBOX_DCLICK, &pnlBrowseHistory::OnListBoxDClick, this);
	}



	void pnlBrowseHistory::OnCmdExecuted(wxCommandEvent& event)
	{
		const auto& History = m_CmdLine->GetCommandHist();

		const auto& Elem = *std::crbegin(History);

		if (std::holds_alternative<wxString>(Elem))
			m_lstBox->Insert(std::get<wxString>(Elem), 0);
		else
		{
			const auto& Lst = std::get<std::list<wxString>>(Elem);
			for (size_t i=0; const auto& cmd : Lst)
			{
				if (cmd.empty())
					continue;

				m_lstBox->Insert(cmd, i++);
			}
		}
	}



	void pnlBrowseHistory::OnCopyBtn(wxCommandEvent& event)
	{
		wxArrayInt Indexes;
		m_lstBox->GetSelections(Indexes);

		if (Indexes.size() == 0)
			return;


		wxString SelTxt;
		for (const auto i : Indexes)
		{
			wxString Str = m_lstBox->GetString(i);
			if (Str.empty())
				continue;

			Str = Str + "\n";

			SelTxt += Str;
		}

		if (wxTheClipboard->Open())
		{
			wxTheClipboard->SetData(new wxTextDataObject(SelTxt));
			wxTheClipboard->Flush();
			wxTheClipboard->Close();
		}
	}


	void pnlBrowseHistory::OnListBox(wxCommandEvent& event)
	{
		wxArrayInt Selections;
		m_lstBox->GetSelections(Selections);

		m_btn->Enable(Selections.size() > 1);
	}


	void pnlBrowseHistory::OnListBoxDClick(wxCommandEvent& event)
	{
		wxArrayInt Indexes;
		m_lstBox->GetSelections(Indexes);

		if (Indexes.size() == 0)
			return;


		wxString SelTxt;
		for (const auto i : Indexes)
		{
			wxString Str = m_lstBox->GetString(i);
			if (Str.empty())
				continue;

			SelTxt += Str;
		}

		m_CmdLine->GetInputWnd()->AppendText(SelTxt);
	}
}