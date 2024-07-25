#include "pkg_scisuit.h"

#include <filesystem>

#include <script/scripting_funcs.h>

#include "../icell/workbook.h"
#include "../icell/worksheet.h"

#include "../cmdwnd/pnlcmdwnd.h"


extern std::filesystem::path glbExeDir;
extern ICELL::CWorkbook* glbWorkbook;



using namespace script;



namespace pkgscisuit::gui
{

	PyObject *messagebox(PyObject* self, PyObject* args, PyObject* kwargs)
	{
		PyObject* MsgObj = nullptr, *CaptionObj=nullptr;

		const char* kwlist[] = { "msg", "caption", NULL };
		if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OO", const_cast<char**>(kwlist), &MsgObj, &CaptionObj))
			return nullptr;

		std::wstring Msg = PyUnicode_AsWideCharString(MsgObj, nullptr);
		std::wstring Caption = PyUnicode_AsWideCharString(CaptionObj, nullptr);

		wxMessageBox(Msg, Caption);

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



	PyObject *findworksheet(PyObject *self, PyObject *args, PyObject *kwargs)
	{
		PyObject* TextObj = nullptr;
	
		const char* kwlist[] = { "name", NULL };
		if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O", const_cast<char**>(kwlist), &TextObj))
			return nullptr;

		std::wstring Text = PyUnicode_AsWideCharString(TextObj, nullptr);
		auto WS = glbWorkbook->GetWorksheet(Text);

		if(WS)
			return Python::Worksheet_FromCWorksheet((ICELL::CWorksheet*)WS);

		Py_RETURN_NONE;
	}
}