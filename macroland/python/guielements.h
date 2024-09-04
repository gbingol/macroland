#pragma once

#include <stdexcept>
#include <string>
#include <list>
#include <filesystem>
#include <any>

#include <Python.h>

#include <wx/wx.h>
#include <wx/aui/aui.h>
#include <wx/aui/auibar.h>
#include <wx/menu.h>
#include <wx/notebook.h>



namespace extension
{
	class CElement :public wxEvtHandler
	{
	public:
		enum class Type { None = 0, Button, DropButton, HybridBtn, Panel, Page };

		static wxBitmap GetBitmap(const std::filesystem::path& BitmapPath);

	public:
		CElement(const wxString& title = "")
		{
			m_ID = wxNewId();
			m_Label = title;
		}


		virtual Type GetType() const = 0;

		virtual bool IsOK() const
		{
			return m_IsOK;
		}


		void SetTitle(const std::wstring& label)
		{
			m_Label = label;
		}

		std::wstring GetTitle() const {
			return m_Label;
		}


		//sets as normalized path
		void SetImgPath(const std::filesystem::path& Path);

		auto GetImagePath() const {
			return m_ImgPath;
		}


		auto GetWindow() const
		{
			return m_Window;
		}

		auto SetWindow(wxWindow* wnd)
		{
			m_Window = wnd;
		}

		auto GetId() const
		{
			return m_ID;
		}


	protected:
		std::wstring m_Name{};
		std::wstring m_Label{};

		std::filesystem::path m_ImgPath;
		wxWindow* m_Window{ nullptr };

		bool m_IsOK{ true };
		int m_ID{ -1 };
	};


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

	class CHybridButton : public CElement
	{
	public:
		CHybridButton(CButtonBase* mainBtn);

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


		void SetFunc(PyObject* func)
		{
			m_Func = func;
		}

		auto GetFuncName() const
		{
			return m_Func;
		}

		void SetArgs(PyObject* param)
		{
			m_Args = param;
		}

		auto GetArgs() const {
			return m_Args;
		}		

		virtual void OnClick(wxCommandEvent& event) = 0;

		virtual ~CButtonBase() = default;


	protected:
		PyObject *m_Func;
		PyObject* m_Args; 
	};



	/**********************************************************************/

	class CButton : public CButtonBase
	{
	public:
		CButton(const std::wstring& Title);

		void OnClick(wxCommandEvent& event) override;
	};


	/***************************************************************** */

	class CMenu : public CButtonCollection
	{
	public:
		CMenu(const wxString& Title) :CButtonCollection(Title) {}

		Type GetType() const override {
			return CElement::Type::DropButton;
		}

		wxMenu* GetAsMenu() const;
	};



	/********************************************************************** */

	class CToolBarPage : public wxPanel
	{
	public:
		CToolBarPage(wxWindow* parent, const std::wstring& title);

		auto GetElementList() const {
			return m_ElemList;
		}

		void AddElement(CElement* elem) {
			m_ElemList.push_back(elem);
		}


		auto GetToolBar() const {
			return m_ToolBar;
		}

		auto GetTitle() const {
			return m_Title;
		}


	private:
		std::list<CElement*> m_ElemList;
		std::filesystem::path m_ExtensionPath;
		wxAuiToolBar* m_ToolBar;
		std::wstring m_Title;
	};



	/*************************************************************/

	class CToolBarNtbk : public wxNotebook
	{
	public:
		CToolBarNtbk(wxWindow* parent, int ID = wxID_ANY);
		~CToolBarNtbk();

		CToolBarPage* AddPage(const std::wstring& label);
		void AddPage(CToolBarPage* page);

		CToolBarPage* FindPage(const std::wstring& label);
	};
}