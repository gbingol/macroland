#include "pnlpathbrowse.h"

#include <wx/artprov.h>

#include "../../python/PythonWrapper.h"
#include <script/scripting_funcs.h>


pnlPythonPathBrowser::pnlPythonPathBrowser(wxWindow* parent, PyObject* pymodule) : wxPanel(parent)
{
	m_Module = pymodule;

	//Refresh Button
	m_btn = new wxBitmapButton(this, wxID_ANY, wxArtProvider::GetBitmap(wxART_REFRESH));
	m_btn->SetToolTip("Refresh the list since sys.path has changed.");

	std::string PyCmd = "import sys \n"
		"path = sys.path";

	auto runStr = script::RunString(m_Module);
	auto PyList = runStr.run(PyCmd, "path");
	auto Array = Python::List_AsArray(PyList);

	auto PathList = std::list<std::filesystem::path>();

	PyCmd = "del path";
	runStr = script::RunString(m_Module);
	PyList = runStr.run(PyCmd);

	for (const auto& Elem : Array)
	{
		std::filesystem::path p = Elem;
		if (!p.empty())
			PathList.push_back(p);
	}

	m_pnlBrowse = new script::pnlPathBrowser(this, PathList);


	auto szrbtn = new wxBoxSizer(wxHORIZONTAL);
	szrbtn->Add(m_btn, 0, wxALL, 5);

	auto mainSizer = new wxBoxSizer(wxVERTICAL);
	mainSizer->Add(szrbtn, 0, wxEXPAND, 5);
	mainSizer->Add(m_pnlBrowse, 1, wxEXPAND | wxALL, 5);


	SetSizerAndFit(mainSizer);
	Layout();

	m_btn->Bind(wxEVT_BUTTON, &pnlPythonPathBrowser::OnButtonClick, this);
}


pnlPythonPathBrowser::~pnlPythonPathBrowser() = default;


void pnlPythonPathBrowser::OnButtonClick(wxCommandEvent& event)
{
	std::string PyCmd = "import sys \n"
		"path = sys.path";

	auto runStr = script::RunString(m_Module);
	auto PyList = runStr.run(PyCmd, "path");
	auto Array = Python::List_AsArray(PyList);

	auto PathList = std::list<std::filesystem::path>();

	PyCmd = "del path";
	runStr = script::RunString(m_Module);
	PyList = runStr.run(PyCmd);

	for (const auto& Elem : Array)
	{
		std::filesystem::path p = Elem;
		if (!p.empty())
			PathList.push_back(p);
	}

	m_pnlBrowse->RemakeTree(PathList);
}


