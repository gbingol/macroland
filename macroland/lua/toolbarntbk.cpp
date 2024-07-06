#include "toolbarntbk.h"

#include "buttons.h"



namespace lua
{
	CToolBarPage::CToolBarPage(wxWindow* parent, const std::wstring& title) :wxPanel(parent, wxID_ANY)
	{
		m_Title = title;

		m_ToolBar = new wxAuiToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_TB_HORIZONTAL | wxAUI_TB_TEXT);

		auto szrMain = new wxBoxSizer(wxVERTICAL);
		szrMain->Add(m_ToolBar, 0, wxEXPAND | wxALL, 1);
		SetSizer(szrMain);
		Layout();
	}



	/***************************************************************/

	CToolBarNtbk::CToolBarNtbk(wxWindow* parent, int ID) :
		wxNotebook(parent, ID, wxDefaultPosition, wxDefaultSize, 0) { }

	CToolBarNtbk::~CToolBarNtbk() = default;


	CToolBarPage* CToolBarNtbk::AddPage(const std::wstring& label)
	{
		auto panel = new CToolBarPage(this, label);
		wxNotebook::AddPage(panel, label);

		return panel;
	}

	void CToolBarNtbk::AddPage(CToolBarPage* page)
	{
		auto label = page->GetTitle();
		wxNotebook::AddPage(page, label);

		auto TB = page->GetToolBar();

		auto Elems = page->GetElementList();
		for (auto& elem : Elems)
		{
			int btnID = elem->GetId();
			wxString Title = elem->GetTitle();
			wxBitmap bmp = elem->GetBitmap(elem->GetImagePath());
			auto Type = elem->GetType();

			if (Type == CElement::Type::Button)
			{
				TB->AddTool(btnID, Title, bmp);
				TB->Bind(wxEVT_TOOL, &CButtonBase::OnClick, (CButtonBase*)elem, btnID);
			}
			else if (Type == CElement::Type::DropButton)
			{
				elem->SetWindow(TB);

				TB->AddTool(btnID, Title, bmp);
				TB->Bind(wxEVT_TOOL, &CToolBarDropButton::OnClick, (CToolBarDropButton*)elem, btnID);
			}

			else if (Type == CElement::Type::HybridBtn)
			{
				elem->SetWindow(TB);

				auto MainBtn = ((CToolBarHybridButton*)elem)->GetMainButton();
				TB->AddTool(btnID, MainBtn->GetTitle(), MainBtn->GetBitmap(MainBtn->GetImagePath()));
				TB->SetToolDropDown(btnID, true);
				
				TB->Bind(wxEVT_AUITOOLBAR_TOOL_DROPDOWN, &CToolBarHybridButton::OnHybridButton, (CToolBarHybridButton*)elem, btnID);
			}
		}

		TB->Realize();
	}


	CToolBarPage* CToolBarNtbk::FindPage(const std::wstring& label)
	{
		size_t N = GetPageCount();
		for (size_t i = 0; i < N; ++i)
		{
			auto Page = GetPage(i);
			if (Page->GetLabel() == label)
				return (CToolBarPage*)Page;
		}

		return nullptr;
	}

}