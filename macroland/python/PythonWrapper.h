#pragma once

#include <Python.h>

#include <vector>
#include <string>
#include <list>
#include <optional>
#include <functional>



namespace ICELL{
    class CWorksheet;
    class CRange;
}


extern PyTypeObject PythonWorksheet_Type;


namespace Python
{

    /*
    if false the object is not usable anymore
    bool: state
    */

    typedef struct {
        PyObject_HEAD
            ICELL::CWorksheet* ptrObj;

        bool state;

    } Worksheet;



	struct CEventCallbackFunc
	{
		/*
		Example (from Worksheet) Usage:

		if (m_EvtCallBack[event].size() > 0) {
			const auto& List = m_EvtCallBack[event];
			for (const auto& CallBk : List)
				CallBk->call(CallBk->m_FuncObj, CallBk->m_FuncArgs, nullptr);
		}
		*/

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


    PyObject* Worksheet_FromCWorksheet(ICELL::CWorksheet* ws);   
}



#ifndef IF_PYERR_RET
#define IF_PYERR_RET(EXPRESSION, ERROR, ERRMSG)	\
	if((EXPRESSION)){							\
		PyErr_SetString(ERROR, ERRMSG);	\
		return nullptr;									\
	}
#endif



#ifndef ELSE_PYERR_RET
#define ELSE_PYERR_RET(ERROR, ERRMSG)	\
	else{							\
		PyErr_SetString(ERROR, ERRMSG);	\
		return nullptr;									\
	}
#endif 


#ifndef ELSE_PYERR_RETDEF
#define ELSE_PYERR_RETDEF(ERROR, ERRMSG, RETVAL)	\
	else{							\
		PyErr_SetString(ERROR, ERRMSG);	\
		return RETVAL;									\
	}
#endif 


#ifndef IF_PYERRRUNTIME_RET
#define IF_PYERRRUNTIME_RET(EXPRESSION, ERRMSG)	\
	IF_PYERR_RET(EXPRESSION, PyExc_RuntimeError, ERRMSG)
#endif 


#ifndef IF_PYERRVALUE_RET
#define IF_PYERRVALUE_RET(EXPRESSION, ERRMSG)	\
	IF_PYERR_RET(EXPRESSION, PyExc_ValueError, ERRMSG)
#endif // !ASSERTEXPRESSION


#ifndef CHECKPOSITIVE_RET
#define CHECKPOSITIVE_RET(OBJ, ERRMSG)		\
	IF_PYERR_RET(OBJ <= 0, PyExc_ValueError, ERRMSG)	
#endif


#ifndef CHECKNONNEGATIVE_RET
#define CHECKNONNEGATIVE_RET(OBJ, ERRMSG)								\
	IF_PYERR_RET(OBJ < 0.0, PyExc_ValueError, ERRMSG)
#endif


#ifndef CHECKNONNEGATIVERETTYPE
#define CHECKNONNEGATIVE_RETTYPE(OBJ, ERRMSG, RET)								\
	if ((OBJ) < 0.0){												\
		PyErr_SetString(PyExc_ValueError, ERRMSG);				\
		return RET;											\
	}
#endif



#ifndef ASSERT_CALLABLE_RET
#define ASSERT_CALLABLE_RET(OBJ, ERRMSG) \
    if (PyCallable_Check((OBJ)) == false){ \
        PyErr_SetString(PyExc_TypeError, (ERRMSG));     \
        return nullptr;                                  \
    }
#endif



#ifndef CATCHRUNTIMEEXCEPTION_RET
#define CATCHRUNTIMEEXCEPTION_RET()								\
	catch (std::exception& e){								\
		PyErr_SetString(PyExc_RuntimeError, e.what());		\
		return nullptr;										\
	}
#endif