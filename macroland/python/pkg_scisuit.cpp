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
}


namespace pkgscisuit::workbook
{

	PyObject *numberofworksheets(PyObject *self)
	{
		if(!glbWorkbook)
			return nullptr;

		return Py_BuildValue("i", glbWorkbook->GetWorksheetNotebook()->GetPageCount());
	}



	PyObject *findworksheet(PyObject *self, PyObject *args)
	{
		if(!glbWorkbook)
			return nullptr;

		PyObject* Obj = nullptr;
	
		if (!PyArg_ParseTuple(args, "O", &Obj))
			return nullptr;

		ICELL::CWorksheet *WS;

		if(PyUnicode_Check(Obj))
		{
			std::wstring Text = PyUnicode_AsWideCharString(Obj, nullptr);
			WS = (ICELL::CWorksheet*)glbWorkbook->GetWorksheet(Text);
		}

		else if(PyLong_Check(Obj))
		{
			size_t PageNum = PyLong_AsLong(Obj);
			WS = (ICELL::CWorksheet*)glbWorkbook->GetWorksheet(PageNum);
		}	

		if(WS)
			return Python::Worksheet_FromCWorksheet(WS);

		Py_RETURN_NONE;
	}



	PyObject *BindFunction(PyObject *self, PyObject *args)
	{
		/*
			Parameter checks are done from Python side
		*/
		if(!glbWorkbook)
			return nullptr;

		PyObject* EventNameObj = PyTuple_GetItem(args, 0);
		std::string EventName = PyUnicode_AsUTF8(EventNameObj);

		PyObject* FuncObj = PyTuple_GetItem(args, 1);
		
		size_t NArgs = PyTuple_GET_SIZE(args);
		size_t NFuncArgs = NArgs - 2;

		Python::CEventCallbackFunc* CallbackFunc = new Python::CEventCallbackFunc();
		CallbackFunc->m_FuncObj = FuncObj;

		PyObject* FuncArgs = nullptr;

		try
		{
			if (NFuncArgs > 0)
			{
				FuncArgs = PyTuple_New(NFuncArgs);

				for (size_t i = 2, j = 0; i < NArgs; ++i, ++j)
					PyTuple_SetItem(FuncArgs, j, PyTuple_GetItem(args, i));
			}

			CallbackFunc->m_FuncArgs = FuncArgs;
			glbWorkbook->BindPythonFunction(EventName, CallbackFunc);
		}
		catch (std::exception& e)
		{
			PyErr_SetString(PyExc_RuntimeError, e.what());

			Py_XDECREF(FuncArgs);

			delete CallbackFunc;

			return nullptr;
		}

		Py_RETURN_NONE;
	}


	PyObject *UnbindFunction(PyObject *self, PyObject *args)
	{
		/*
			Parameter checks are done from Python side
		*/
		if(!glbWorkbook)
			return nullptr;

		PyObject* EventNameObj = PyTuple_GetItem(args, 0);

		std::string EventName = PyUnicode_AsUTF8(EventNameObj);
		PyObject* FuncObj = PyTuple_GetItem(args, 1);

		try
		{
			glbWorkbook->UnbindPythonFunction(EventName, FuncObj);
		}
		CATCHRUNTIMEEXCEPTION_RET();

    	Py_RETURN_NONE;
	}


} //namespace