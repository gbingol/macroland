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
#include <wx/notebook.h>



namespace lua
{
	class CElement :public wxEvtHandler
	{
	public:
		enum class Type { None = 0, Button, DropButton, HybridBtn, Panel, Page };

		static std::filesystem::path GetNormalizedPath(const std::filesystem::path& Path);
		static wxBitmap GetBitmap(const std::filesystem::path& BitmapPath);
		static void SetRootPath(const std::filesystem::path& rootpath)
		{
			s_RootPath = rootpath;
		}

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

		static std::filesystem::path s_RootPath;

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




	/*************************************************************** */

	class CPanel : public CElement
	{
	public:
		CPanel(const std::wstring& title);

		Type GetType() const
		{
			return Type::Panel;
		}

		auto GetElementList() const 
		{
			return m_ElemList;
		}

		void AddElement(CElement* elem)
		{
			m_ElemList.push_back(elem);
		}


		auto GetExtPath() const {
			return m_ExtensionPath;
		}

		void SetExtensionPath(const std::filesystem::path& path) 
		{
			m_ExtensionPath = path;
		}


		auto GetTitle() const
		{
			return m_Title;
		}


	private:
		std::list<CElement*> m_ElemList;
		std::filesystem::path m_ExtensionPath;
		std::wstring m_Title;
	};



	/********************************************************************** */

	class CToolBarPage : public wxPanel
	{
	public:
		enum class PAGETYPE
		{
			NONE = 0, //no specific type
			EXTENSION //is an extension
		};
	public:
		CToolBarPage(wxWindow* parent, const std::wstring& title);

		auto GetElementList() const 
		{
			return m_ElemList;
		}

		void AddElement(CElement* elem)
		{
			m_ElemList.push_back(elem);
		}

		auto GetPageType() const {
			return m_PageType;
		}

		void SetPageType(PAGETYPE type) 
		{
			m_PageType = type;
		}

		auto GetExtensionPath() const {
			return m_ExtensionPath;
		}

		void SetExtensionPath(const std::filesystem::path& path) 
		{
			m_ExtensionPath = path;
		}

		auto GetToolBar() const
		{
			return m_ToolBar;
		}

		auto GetTitle() const
		{
			return m_Title;
		}


	private:
		std::list<CElement*> m_ElemList;
		PAGETYPE m_PageType;
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