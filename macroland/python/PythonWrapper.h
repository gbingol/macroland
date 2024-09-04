#pragma once

#include <Python.h>

#include <vector>
#include <string>
#include <list>
#include <optional>
#include <functional>
#include <filesystem>



namespace ICELL{
    class CWorksheet;
}


extern PyTypeObject PythonWorksheet_Type;


namespace Python
{

    //if state=false the object is not usable anymore
    typedef struct {
        PyObject_HEAD
            ICELL::CWorksheet* ptrObj;
        bool state;
    } Worksheet;


	/*
		Example (from Worksheet) Usage:

		if (m_EvtCallBack[event].size() > 0) {
			const auto& List = m_EvtCallBack[event];
			for (const auto& CallBk : List)
				CallBk->call(CallBk->m_FuncObj, CallBk->m_FuncArgs, nullptr);
		}
	*/
	struct CEventCallbackFunc
	{
		PyObject* m_Func; //Function object
		PyObject* m_Args; //Function arguments

		CEventCallbackFunc() = default;

		~CEventCallbackFunc()
		{
			Py_XDECREF(m_Args);
		}

		static PyObject* call(
			PyObject* Func, 
			PyObject* Args)
		{
			auto gstate = PyGILState_Ensure();
			
			auto Result = PyObject_CallObject(Func, Args);

			PyGILState_Release(gstate);
			
			return Result;
		}
	};


    inline PyObject* Worksheet_FromCWorksheet(ICELL::CWorksheet* ws)
	{
		auto WorksheetObj = PyObject_New(Worksheet, &PythonWorksheet_Type);
		WorksheetObj->ptrObj = ws;
		WorksheetObj->state = true;

		return (PyObject*)WorksheetObj;
	}  


	bool RunExtensions(std::wstring_view fileName);

	//runs all the Python files in a given directory
	bool RunDirectoryContents(const std::filesystem::path &DirPath);
}



#ifndef IF_PYERR
#define IF_PYERR(EXPRESSION, ERROR, ERRMSG, RETURN)	\
	if((EXPRESSION)){							\
		PyErr_SetString(ERROR, ERRMSG);	\
		return RETURN;									\
	}
#endif



#ifndef IF_PYERRRUNTIME
#define IF_PYERRRUNTIME(EXPRESSION, ERRMSG, RETURN)	\
	IF_PYERR(EXPRESSION, PyExc_RuntimeError, ERRMSG, RETURN)
#endif 



#ifndef CATCHRUNTIMEEXCEPTION_RET
#define CATCHRUNTIMEEXCEPTION_RET()								\
	catch (std::exception& e){								\
		PyErr_SetString(PyExc_RuntimeError, e.what());		\
		return nullptr;										\
	}
#endif