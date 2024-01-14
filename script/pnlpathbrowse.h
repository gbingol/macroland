#pragma once

#include <filesystem>

#include <wx/wx.h>
#include <wx/minifram.h>
#include <wx/treectrl.h>

#include "dllimpexp.h"

namespace script
{
	class TreeItemData :public wxTreeItemData
	{
	public:
		//file (*.py), or folder, python function or python class
		enum class TYPE { FILE = 0, FOLDER = 1, FUNC, CLASS };

		DLLSCRIPT TreeItemData(const std::filesystem::path& path, TreeItemData::TYPE type) : m_Path{ path }, m_Type{ type }
		{
			m_LineStart = 0;
			m_LineEnd = 0;
		}
		DLLSCRIPT auto GetPath() const { return m_Path; }
		DLLSCRIPT auto GetType() const { return m_Type; }

		DLLSCRIPT void SetLineStart(int linest) { m_LineStart = linest; }
		DLLSCRIPT auto GetLineStart() const { return m_LineStart; }

		DLLSCRIPT void SetLineEnd(int lineend) { m_LineEnd = lineend; }
		DLLSCRIPT auto GetLineEnd() const { return m_LineEnd; }

	private:
		std::filesystem::path m_Path;
		TYPE m_Type;
		int m_LineStart, m_LineEnd;
	};


	class pnlPathBrowser : public wxPanel
	{
	public:
		DLLSCRIPT pnlPathBrowser(wxWindow* parent, const std::list<std::filesystem::path>& PathList);
		DLLSCRIPT ~pnlPathBrowser() = default;

		DLLSCRIPT void RemakeTree(const std::list<std::filesystem::path>& PathList);

		wxTreeCtrl* GetTreeCtrl() const
		{
			return m_TreeCtrl;
		}

	protected:
		wxTreeCtrl* m_TreeCtrl;

		DLLSCRIPT void OnTreeItemActivated(wxTreeEvent& event);
		DLLSCRIPT void OnTreeItemRightClick(wxTreeEvent& event);

		DLLSCRIPT void OnTreeItemExpanding(wxTreeEvent& event);
		DLLSCRIPT void OnTreeItemCollapsed(wxTreeEvent& event);

	private:
		void Append(const std::filesystem::path& Path, wxTreeCtrl* TreeCtrl, wxTreeItemId parentNode);
		void AppendPyInfo(const std::filesystem::path& Path, wxTreeCtrl* TreeCtrl, wxTreeItemId parentNode);

	private:
		wxImageList* m_ImgList;
	};
}

