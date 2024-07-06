#pragma once

#include <stdexcept>
#include <string>
#include <list>
#include <filesystem>
#include <any>

#include <wx/wx.h>
#include <wx/aui/aui.h>
#include <wx/aui/auibar.h>
#include <wx/menu.h>

#include "baseelement.h"


namespace lua
{
	class CButtonBase;

	class CButtonCollection : public CElement
	{
	public:
		CButtonCollection() = default;

		CButtonCollection(const wxString& Title):CElement(Title) { }

		Type GetType() const override {
			return CElement::Type::None;
		}

		auto GetList() const {
			return m_ButtonList;
		}

		void AddButton(CButtonBase* b) {
			m_ButtonList.push_back(b);
		}

		void AttachToMenu(wxMenu* menu);

		void OnMenuHandler(wxCommandEvent& event);

	private:
		std::list<CButtonBase*> m_ButtonList;
	};





	/****************************   CDropButton  ********************************/

	class CToolBarDropButton : public CButtonCollection
	{
	public:
		CToolBarDropButton(const wxString& Title) :CButtonCollection(Title) {}

		Type GetType() const override {
			return CElement::Type::DropButton;
		}

		void OnClick(wxCommandEvent& event);
	};



	/****************************   CHybridButton  ********************************/

	class CToolBarHybridButton : public CElement
	{
	public:
		CToolBarHybridButton(CButtonBase* mainBtn);

		Type GetType() const override {
			return CElement::Type::HybridBtn;
		}

		auto GetMainButton() const {
			return m_MainBtn;
		}

		void SetMainButton(CButtonBase* btn)
		{
			m_MainBtn = btn;
		}

		const auto GetButtonList() const {
			return m_Btns;
		}

		void AddButton(CButtonBase* btn) {
			m_Btns.push_back(btn);
		}

		void OnHybridButton(wxAuiToolBarEvent& event);
		void OnMenuHandler(wxCommandEvent& event);
	
	private:
		CButtonBase* m_MainBtn;
		std::list<CButtonBase*> m_Btns;

	};



	/****************************   CButton  ********************************/

	class CButtonBase : public CElement
	{

	public:
		CButtonBase(const std::wstring& Title);

		Type GetType() const override {
			return CElement::Type::Button;
		}

		//path is normalized
		void SetScriptPath(const std::filesystem::path& Path);

		auto GetScriptPath() const
		{
			return m_ScriptPath;
		}

		void SetModulePath(const wxString& Path)
		{
			m_ModulePath = Path;
		}

		auto GetModulePath() const
		{
			return m_ModulePath;
		}

		void SetFuncName(const wxString& fName)
		{
			m_FuncName = fName;
		}

		auto GetFuncName() const
		{
			return m_FuncName;
		}

		void SetParam(const std::any param)
		{
			m_Param = param;
		}

		auto GetParam() const {
			return m_Param;
		}		

		virtual void OnClick(wxCommandEvent& event) = 0;

		virtual ~CButtonBase() = default;


	protected:
		std::filesystem::path m_ScriptPath;

		std::wstring m_ModulePath; //scisuit.plot
		std::wstring m_FuncName; // def MyFunc: ( it is MyFunc)
		std::any m_Param; // def MyFunc(x): (it is x)
	};



	/**********************************************************************/

	class CButton : public CButtonBase
	{
	public:
		CButton(const std::wstring& Title);

		void OnClick(wxCommandEvent& event) override;
	};
}