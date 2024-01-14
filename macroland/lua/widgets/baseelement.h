#pragma once

#include <stdexcept>
#include <string>
#include <list>
#include <filesystem>

#include <wx/wx.h>

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
}