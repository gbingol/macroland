#include "pnlpathbrowse.h"

#include <Python.h>

#include <wx/artprov.h>

#include "scriptviewer.h"
#include "scripting_funcs.h"

#include "icons/py_logo16.xpm"
#include "icons/pyc_logo16.xpm"





static 	wxBitmap CreateBitmap(
	const wxString& Text,
	const wxSize& szBMP,
	const wxFont& Font,
	const wxColour& BG,
	const wxColour& TxtFG,
	wxWindow* wnd)
{
	int BMPWidth = szBMP.GetWidth();
	int BMPHeight = szBMP.GetHeight();

	wxBitmap bmp(BMPWidth, BMPHeight);

	wxMemoryDC memDC;
	memDC.SelectObject(bmp);
	if (BG.IsOk())
		memDC.SetBackground(BG);
	else
		memDC.SetBackground(*wxWHITE);
	memDC.Clear();

	if (TxtFG.IsOk())
		memDC.SetTextForeground(TxtFG);

	if (Font.IsOk())
		memDC.SetFont(Font);

	wxSize szTxt;
	if (wnd)
		szTxt = wnd->FromDIP(memDC.GetTextExtent(Text));
	else
		szTxt = memDC.GetTextExtent(Text);

	int y = BMPHeight / 2 - szTxt.GetHeight() / 2;
	int x = BMPWidth / 2 - szTxt.GetWidth() / 2;

	if (!Text.empty())
		memDC.DrawText(Text, wxPoint(x, y));

	return bmp;
}




namespace script
{
	pnlPathBrowser::pnlPathBrowser(wxWindow* parent, const std::list<std::filesystem::path>& PathList) :
		wxPanel(parent, wxID_ANY)
	{
		SetSizeHints(wxDefaultSize, wxDefaultSize);
		auto bSizer = new wxBoxSizer(wxVERTICAL);

		m_TreeCtrl = new wxTreeCtrl(this);
		auto Root = m_TreeCtrl->AddRoot("sys.path");

		for (const auto& path : PathList)
		{
			auto DirNode = m_TreeCtrl->AppendItem(Root, path.wstring());
			Append(path, m_TreeCtrl, DirNode);
		}

		bSizer->Add(m_TreeCtrl, 1, wxEXPAND | wxALL, 5);
		SetSizerAndFit(bSizer);
		Layout();

		wxFont ImgFont = wxFontInfo(12).FaceName("Consolas");
		wxColor ImgBG = wxColour(144, 238, 144); //light green
		wxColor ImgTxtFG = *wxBLACK;

		m_ImgList = new wxImageList(16, 16, true);
		m_ImgList->Add(wxArtProvider::GetBitmap(wxART_FOLDER));
		m_ImgList->Add(wxBitmap(pyc_logo16_xpm));
		m_ImgList->Add(wxBitmap(py_logo16_xpm));
		m_ImgList->Add(CreateBitmap("C", wxSize(16, 16), ImgFont, ImgBG, ImgTxtFG, nullptr)); //py class
		m_ImgList->Add(CreateBitmap("F", wxSize(16, 16), ImgFont, ImgBG, ImgTxtFG, nullptr)); //py function

		m_TreeCtrl->SetImageList(m_ImgList);

		m_TreeCtrl->Bind(wxEVT_COMMAND_TREE_ITEM_ACTIVATED, &pnlPathBrowser::OnTreeItemActivated, this);
		m_TreeCtrl->Bind(wxEVT_COMMAND_TREE_ITEM_RIGHT_CLICK, &pnlPathBrowser::OnTreeItemRightClick, this);

		m_TreeCtrl->Bind(wxEVT_COMMAND_TREE_ITEM_EXPANDING, &pnlPathBrowser::OnTreeItemExpanding, this);
		m_TreeCtrl->Bind(wxEVT_COMMAND_TREE_ITEM_COLLAPSED, &pnlPathBrowser::OnTreeItemCollapsed, this);
	}



	void pnlPathBrowser::RemakeTree(const std::list<std::filesystem::path>& PathList)
	{
		m_TreeCtrl->DeleteAllItems();

		auto Root = m_TreeCtrl->AddRoot("sys.path");

		for (const auto& path : PathList)
		{
			auto DirNode = m_TreeCtrl->AppendItem(Root, path.wstring());
			Append(path, m_TreeCtrl, DirNode);
		}
	}


	void pnlPathBrowser::OnTreeItemActivated(wxTreeEvent& event)
	{
		auto Node = event.GetItem();
		auto Data = (TreeItemData*)m_TreeCtrl->GetItemData(Node);
		if (Data)
		{
			auto Path = Data->GetPath();
			if (Path.extension() == ".py")
			{
				frmSriptViewer* viewer{ nullptr };

				int VisibleLine = Data->GetLineStart();
				if (VisibleLine > 0)
					viewer = new frmSriptViewer(this, Path, VisibleLine - 1);
				else
					viewer = new frmSriptViewer(this, Path);

				viewer->Show();
			}
			else
				wxMessageBox("Viewer can only be used for *.py files");
		}
		event.Skip();
	}


	void pnlPathBrowser::OnTreeItemRightClick(wxTreeEvent& event)
	{
		event.Skip();
	}


	void pnlPathBrowser::OnTreeItemExpanding(wxTreeEvent& event)
	{
		auto Node = event.GetItem();

		if (Node == m_TreeCtrl->GetRootItem())
		{
			event.Skip();
			return;
		}

		auto Data = (TreeItemData*)m_TreeCtrl->GetItemData(Node);
		if (Data)
		{
			auto Type = Data->GetType();
			if (Type == TreeItemData::TYPE::FOLDER)
				Append(Data->GetPath(), m_TreeCtrl, Node);
			else if (Type == TreeItemData::TYPE::FILE)
				AppendPyInfo(Data->GetPath(), m_TreeCtrl, Node);

		}
		event.Skip();
	}


	void pnlPathBrowser::OnTreeItemCollapsed(wxTreeEvent& event)
	{
		auto Node = event.GetItem();
		auto Root = m_TreeCtrl->GetRootItem();
		if (Node == Root || m_TreeCtrl->GetItemParent(Node) == Root)
		{
			event.Skip();
			return;
		}

		m_TreeCtrl->DeleteChildren(Node);
		m_TreeCtrl->SetItemHasChildren(Node);
		event.Skip();
	}


	void pnlPathBrowser::Append(const std::filesystem::path& Path, wxTreeCtrl* TreeCtrl, wxTreeItemId parentNode)
	{
		if (!std::filesystem::exists(Path))
			return;
		if (!std::filesystem::is_directory(Path))
			return;

		for (const auto& DirEntry : std::filesystem::directory_iterator(Path))
		{
			auto pth = std::filesystem::path(DirEntry);

			if (std::filesystem::is_directory(pth))
			{
				TreeItemData* ItemData = new TreeItemData(pth, TreeItemData::TYPE::FOLDER);
				auto Node = TreeCtrl->AppendItem(parentNode, wxString::FromUTF8(pth.filename().string()), 0, -1, ItemData);
				TreeCtrl->SetItemHasChildren(Node);
			}
			else
			{
				auto ItemData = new TreeItemData(pth, TreeItemData::TYPE::FILE);
				auto Ext = pth.extension();
				if (Ext == ".py" || Ext == ".pyc")
				{
					int ImgPos = Ext == ".pyc" ? 1 : 2;
					auto Node = TreeCtrl->AppendItem(parentNode, wxString::FromUTF8(pth.filename().string()), ImgPos, -1, ItemData);
					if (Ext == ".py")
						TreeCtrl->SetItemHasChildren(Node);
				}
			}
		}
	}


	void pnlPathBrowser::AppendPyInfo(const std::filesystem::path& Path, wxTreeCtrl* TreeCtrl, wxTreeItemId parentNode)
	{
		wxString cmd = "import ast \n";
		cmd << "path = r\"" << Path.wstring() << "\" \n";
		cmd << "r = open(path,'r') \n";
		cmd << "tree = ast.parse(r.read()) \n";

		std::string visitor =
			"class ASTNodeVisitor(ast.NodeVisitor): \n"
			"    def __init__(self) : \n"
			"        super() \n"
			"        self.m_FunctionDef = [] \n"
			"        self.m_ClassDef = [] \n"

			"    def visit_ClassDef(self, node) : \n"
			"        self.m_ClassDef.append([node.name, node.lineno, node.end_lineno]) \n"
			"        self.generic_visit(node) \n"

			"    def visit_FunctionDef(self, node) : \n"
			"        self.m_FunctionDef.append([node.name, node.lineno, node.end_lineno]) \n"
			"        self.generic_visit(node) \n"

			"    def ClassDef(self) : \n"
			"        return self.m_ClassDef \n"

			"    def FunctionDef(self) : \n"
			"        return self.m_FunctionDef \n"
			"visitor=ASTNodeVisitor() \n"
			"visitor.visit(tree) \n"

			"ClassDef=visitor.ClassDef() \n"
			"FuncDef=visitor.FunctionDef()";

		cmd << visitor;


		auto RunCmd = RunString();
		auto PyRes = RunCmd.run(cmd.ToStdString(wxConvUTF8), { "ClassDef", "FuncDef" });


		auto ParseList = [=](PyObject* MainList, TreeItemData::TYPE type)
		{
			if (MainList == nullptr)
				return;

			auto N = PyList_Size(MainList);
			for (Py_ssize_t i = 0; i < N; ++i)
			{
				PyObject* SubList = PyList_GetItem(MainList, i);
				if (SubList == nullptr)
					continue;
		
				auto Data = new TreeItemData(Path, type);

				wxString IdName = PyUnicode_AsWideCharString(PyList_GetItem(SubList, 0), nullptr);
				Data->SetLineStart(PyLong_AsLong(PyList_GetItem(SubList, 1)));
				Data->SetLineEnd(PyLong_AsLong(PyList_GetItem(SubList, 2)));

				int ImgPos = type == TreeItemData::TYPE::CLASS ? 3 : 4;

				if (type == TreeItemData::TYPE::CLASS)
					IdName.Prepend("class ");
				else if (type == TreeItemData::TYPE::FUNC)
					IdName.Prepend("def ");

				TreeCtrl->AppendItem(parentNode, IdName, ImgPos, -1, Data);
			}
		};

		ParseList(PyRes[0], TreeItemData::TYPE::CLASS);
		ParseList(PyRes[1], TreeItemData::TYPE::FUNC);
	}
}