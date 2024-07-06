#include "pkg_scisuit.h"

#include <filesystem>

#include <script/scripting_funcs.h>

#include "../icell/workbook.h"
#include "../icell/worksheet.h"

#include "../scripting/pnlcmdwnd.h"


extern std::filesystem::path glbExeDir;
extern ICELL::CWorkbook* glbWorkbook;



using namespace script;



namespace pkgscisuit::gui
{

	PyObject* activeworksheet(PyObject* self)
	{
		//at the start of MacroLand App, glbWorkbook is nullptr
		if (glbWorkbook)
		{
			auto ws = (ICELL::CWorksheet*)glbWorkbook->GetActiveWS();
			auto WSObj = Python::Worksheet_FromCWorksheet(ws);

			ws->RegisterPyWS((Python::Worksheet*)WSObj); 

			return WSObj;
		}

		Py_RETURN_NONE;
	}


	PyObject* statbar_write(PyObject* self, PyObject* args, PyObject* kwargs)
	{
		PyObject* TextObj = nullptr;
		int n = 0;

		const char* kwlist[] = { "text", "n", NULL };
		if (!PyArg_ParseTupleAndKeywords(args, kwargs, "Oi", const_cast<char**>(kwlist), &TextObj, &n))
			return nullptr;

		std::wstring Text = PyUnicode_AsWideCharString(TextObj, nullptr);

		IF_PYERRRUNTIME_RET(!glbWorkbook, "No workbook found.");
		glbWorkbook->SetStatusText(Text, n); 

		Py_RETURN_NONE;
	}


}