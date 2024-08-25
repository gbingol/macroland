#include "pnlcmdwnd.h"

#include <wx/artprov.h>
#include <wx/clipbrd.h>

#include "cmdline.h"



namespace cmdedit
{
	pnlCommandWindow::pnlCommandWindow(wxWindow* parent, wxWindowID id) :
		wxPanel(parent, id)
	{
		auto sci = PyImport_ImportModule("__SCISUIT");
		auto sci_dict = PyModule_GetDict(sci);
		auto Module = PyDict_GetItemString(sci_dict, "COMMANDWINDOW");
		Py_XDECREF(sci);

		m_cmdLine = new CCmdLine(this, Module);
		auto History = new pnlHistory(this, m_cmdLine);

		m_mgr = new wxAuiManager(this, wxAUI_MGR_LIVE_RESIZE | wxAUI_MGR_DEFAULT);
		
		m_mgr->AddPane(m_cmdLine, wxAuiPaneInfo().
			Name("CmdWnd").Caption("Command Window").
			Center().CloseButton(false).MaximizeButton(true).Layer(0).Position(0));

		m_mgr->AddPane(History, wxAuiPaneInfo().
			Name("InfoPane").Caption("Info Pane").
			Right().MinSize(FromDIP(200), -1).CloseButton(false).
			Resizable(true).MaximizeButton(true).Layer(0).Position(0));

		m_mgr->Update();

		//Allow to be resized below minimum size
		m_mgr->GetPane("InfoPane").min_size = wxSize(-1, -1);
	}



	/**************************************************************************** */

	pnlHistory::pnlHistory(
		wxWindow* parent, 
		CCmdLine* cmdline) : wxPanel(parent)
	{
		assert(cmdline != nullptr);

		m_CmdLine = cmdline;

		SetSizeHints(wxDefaultSize, wxDefaultSize);
		
		m_btn = new wxBitmapButton(this, wxID_ANY, wxArtProvider::GetBitmap(wxART_COPY));
		m_btn->Enable(false);
		m_chkExecuteCmd = new wxCheckBox(this, wxID_ANY, "Execute Command");
		m_chkExecuteCmd->SetValue(true);
		
		m_lstBox = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_SINGLE | wxLB_EXTENDED);

		auto History = m_CmdLine->GetCommandHist();

		for (auto it{ std::crbegin(History) }; it != std::crend(History); ++it)
			m_lstBox->Append(wxString::FromUTF8(*it));

		auto szrbtns = new wxBoxSizer(wxHORIZONTAL);
		szrbtns->Add(m_btn, 0, wxALL | wxEXPAND, 5);
		szrbtns->Add(m_chkExecuteCmd, 0, wxALL | wxEXPAND, 5);

		auto mainSizer = new wxBoxSizer(wxVERTICAL);
		mainSizer->Add(szrbtns, 0, wxEXPAND, 0);
		mainSizer->Add(m_lstBox, 1, wxALL | wxEXPAND, 5);
		SetSizerAndFit(mainSizer);
		Layout();


		cmdline->GetInputWnd()->Bind(ssEVT_SCRIPTCTRL_RETURN, &pnlHistory::OnCmdExecuted, this);
		m_btn->Bind(wxEVT_BUTTON, &pnlHistory::OnCopyBtn, this);
		m_lstBox->Bind(wxEVT_LISTBOX, &pnlHistory::OnListBox, this);
		m_lstBox->Bind(wxEVT_LISTBOX_DCLICK, &pnlHistory::OnListBoxDClick, this);
	}



	void pnlHistory::OnCmdExecuted(wxCommandEvent& event)
	{
		const auto& History = m_CmdLine->GetCommandHist();
		size_t N = History.size() - m_lstBox->GetCount();

		for (size_t i = 0; i < N; ++i)
		{
			const auto &Elem = *(std::rbegin(History) + i);
			m_lstBox->Insert(wxString::FromUTF8(Elem), 0);
		}
	}



	void pnlHistory::OnCopyBtn(wxCommandEvent& event)
	{
		auto Copy = [](wxString txt)
		{
			if (wxTheClipboard->Open())
			{
				wxTheClipboard->SetData(new wxTextDataObject(txt));
				wxTheClipboard->Flush();
				wxTheClipboard->Close();
			}
		};

		wxArrayInt Indexes;
		m_lstBox->GetSelections(Indexes);

		wxMenu menu;
		auto Item = menu.Append(wxID_ANY, "Copy");
		Bind(wxEVT_MENU, [&](wxCommandEvent)
		{
			wxString SelTxt;
			for (auto i : Indexes)
			{
				wxString Str = m_lstBox->GetString(i);
				if (Str.empty())
					continue;
				Str = Str + "\n";
				SelTxt += Str;
			}
			Copy(SelTxt);
		}, Item->GetId());


		Item = menu.Append(wxID_ANY, "Copy (order reversed)");
		Bind(wxEVT_MENU, [&](wxCommandEvent)
		{
			wxString SelTxt;
			for (auto i = Indexes.rbegin(); i != Indexes.rend(); ++i)
			{
				auto Str = m_lstBox->GetString(*i);
				if (Str.empty())
					continue;
				Str = Str + "\n";
				SelTxt += Str;
			}
			Copy(SelTxt);
		}, Item->GetId());

		m_btn->PopupMenu(&menu);
	}


	void pnlHistory::OnListBox(wxCommandEvent& event)
	{
		wxArrayInt Selections;
		m_lstBox->GetSelections(Selections);

		auto N = Selections.size();

		m_btn->Enable(N > 1);
		m_chkExecuteCmd->Enable(N == 1);
	}


	void pnlHistory::OnListBoxDClick(wxCommandEvent& event)
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

		auto InputWnd = m_CmdLine->GetInputWnd();

		InputWnd->AppendText(SelTxt);
		
		if(m_chkExecuteCmd->GetValue())
		{
			wxCommandEvent cmdEvt;
			cmdEvt.SetEventType(ssEVT_SCRIPTCTRL_RETURN);
			wxPostEvent(InputWnd->GetScriptCtrl(), cmdEvt);
		}

		InputWnd->GetScriptCtrl()->SetFocus();
	}

}