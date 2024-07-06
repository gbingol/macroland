#include "buttons.h"

#include <Python.h>
#include <script/scripting_funcs.h>


namespace {
	namespace fs = std::filesystem;
}


namespace lua
{
	

	void CToolBarDropButton::OnClick(wxCommandEvent& event)
	{
		wxMenu Menu;
		AttachToMenu(&Menu);
		GetWindow()->PopupMenu(&Menu);
	}




	/*****************************   Button   ******************************************/

	CButtonBase::CButtonBase(const std::wstring& Title) :CElement(Title) { }

	void CButtonBase::SetScriptPath(const fs::path& FilePath)
	{
		if (FilePath.empty())
		{
			m_ScriptPath = FilePath;
			m_IsOK = false;
			return;
		}

		m_ScriptPath = GetNormalizedPath(FilePath);
		m_IsOK = fs::exists(m_ScriptPath);
	}



	/******************** BUTTON COLLECTION ***********************/

	void CButtonCollection::OnMenuHandler(wxCommandEvent& event)
	{
		auto btnList = GetList();

		for (auto button : btnList)
			if (button->GetId() == event.GetId())
				button->OnClick(event);
	}


	void CButtonCollection::AttachToMenu(wxMenu* menu)
	{
		for (const auto& button : GetList())
		{
			std::filesystem::path bmppath = button->GetImagePath();
			wxBitmap bmp = GetBitmap(bmppath);

			auto MenuItem = menu->Append(button->GetId(), button->GetTitle());
			if (bmp.IsOk())
				MenuItem->SetBitmap(bmp);

			menu->Bind(wxEVT_MENU, &CButtonCollection::OnMenuHandler, this, button->GetId());
		}
	}


	/*****************************************************************/

	CToolBarHybridButton::CToolBarHybridButton(CButtonBase* mainBtn):CElement(mainBtn->GetTitle())
	{	
		m_MainBtn = mainBtn;	
	}


	void CToolBarHybridButton::OnHybridButton(wxAuiToolBarEvent& event)
	{
		if (!event.IsDropDownClicked())
		{
			m_MainBtn->OnClick(event);
			return;
		}

		wxMenu menu;
		for (const auto& button : GetButtonList())
		{
			std::filesystem::path bmppath = button->GetImagePath();
			wxBitmap bmp = GetBitmap(bmppath);

			auto MenuItem = menu.Append(button->GetId(), button->GetTitle());
			if (bmp.IsOk())
				MenuItem->SetBitmap(bmp);

			menu.Bind(wxEVT_MENU, &CToolBarHybridButton::OnMenuHandler, this, button->GetId());
		}

		GetWindow()->PopupMenu(&menu);
	}


	void CToolBarHybridButton::OnMenuHandler(wxCommandEvent& event)
	{
		auto btnList = GetButtonList();

		for (const auto& button : btnList)
		{
			if (button->GetId() != event.GetId())
				continue;

			//call the button's event
			button->OnClick(event);

			//add the MainButton to button's list
			m_Btns.push_back(m_MainBtn);

			//Change main button to the button (the selected menu button)
			m_MainBtn = button;

			//erase the button (menu button) from buttons list
			std::erase(m_Btns, button);

			//if dynamic_cast succeeds change the image and label to the selected menu button's
			if (dynamic_cast<wxAuiToolBar*>(GetWindow()))
			{
				auto TB = (wxAuiToolBar*)GetWindow();
				TB->SetToolBitmap(GetId(), button->GetBitmap(button->GetImagePath()));
				TB->SetToolLabel(GetId(), button->GetTitle());
			}
		}
	}



	/*********************************************************************/

	CButton::CButton(const std::wstring& Title) :CButtonBase(Title)
	{
	}

	void CButton::OnClick(wxCommandEvent& event)
	{
		if (!m_ScriptPath.empty())
			script::RunPyFile(m_ScriptPath);

		if (!m_ModulePath.empty())
			script::RunPyFunc(m_ModulePath, m_FuncName, m_Param);

		PyErr_Clear();
	}
}