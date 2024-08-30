#pragma once

#include <Python.h>

#include <vector>
#include <string>
#include <list>
#include <optional>
#include <functional>



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
		PyObject* m_FuncObj; //Function object
		PyObject* m_FuncArgs; //Function arguments
		PyObject* m_FuncKWArgs; //Function named arguments

		static PyObject* call(
			PyObject* FuncObj, 
			PyObject* FuncArgs = PyTuple_New(0), 
			PyObject* FuncKWArgs = nullptr)
		{
			PyObject* Result{nullptr};

			auto gstate = PyGILState_Ensure();
			
			if (PyCallable_Check(FuncObj))
			{				
				if (FuncArgs == nullptr)
					FuncArgs = PyTuple_New(0);

				Result = PyObject_Call(FuncObj, FuncArgs, FuncKWArgs);
			}

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