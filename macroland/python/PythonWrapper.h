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
			if (PyCallable_Check(FuncObj))
			{
				if (FuncArgs == nullptr)
					FuncArgs = PyTuple_New(0);

				return PyObject_Call(FuncObj, FuncArgs, FuncKWArgs);
			}
			return nullptr;
		}
	};


    inline PyObject* Worksheet_FromCWorksheet(ICELL::CWorksheet* ws)
	{
		auto WorksheetObj = PyObject_New(Worksheet, &PythonWorksheet_Type);
		WorksheetObj->ptrObj = ws;
		WorksheetObj->state = true;

		return (PyObject*)WorksheetObj;
	}  
}



#ifndef IF_PYERR_RET
#define IF_PYERR_RET(EXPRESSION, ERROR, ERRMSG)	\
	if((EXPRESSION)){							\
		PyErr_SetString(ERROR, ERRMSG);	\
		return nullptr;									\
	}
#endif



#ifndef IF_PYERRRUNTIME_RET
#define IF_PYERRRUNTIME_RET(EXPRESSION, ERRMSG)	\
	IF_PYERR_RET(EXPRESSION, PyExc_RuntimeError, ERRMSG)
#endif 



#ifndef CHECKPOSITIVE_RET
#define CHECKPOSITIVE_RET(OBJ, ERRMSG)		\
	IF_PYERR_RET(OBJ <= 0, PyExc_ValueError, ERRMSG)	
#endif


#ifndef CHECKNONNEGATIVE_RET
#define CHECKNONNEGATIVE_RET(OBJ, ERRMSG)								\
	IF_PYERR_RET(OBJ < 0.0, PyExc_ValueError, ERRMSG)
#endif


#ifndef CATCHRUNTIMEEXCEPTION_RET
#define CATCHRUNTIMEEXCEPTION_RET()								\
	catch (std::exception& e){								\
		PyErr_SetString(PyExc_RuntimeError, e.what());		\
		return nullptr;										\
	}
#endif