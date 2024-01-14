#pragma once

#include <Python.h>

#include <vector>
#include <string>
#include <list>
#include <optional>
#include <functional>
#include <complex>

namespace core
{
    class CVector;
}


namespace ICELL
{
    class CWorksheet;
    class CRange;
    class CWorkbook;
}


extern PyTypeObject PythonWorksheet_Type;
extern PyTypeObject PythonWorkbook_Type;
extern PyTypeObject PythonRange_Type;


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


    
    typedef struct {
        PyObject_HEAD
            ICELL::CRange* ptrObj;

        bool state; 
        int iter; //to make it iteratable

    } Range;


    typedef struct {
        PyObject_HEAD
            ICELL::CWorkbook* ptrObj;

    } Workbook;

    //Is obj a Python list
    static bool IsSubTypeList(PyObject* obj)
    {
        return PyType_IsSubtype(obj->ob_type, &PyList_Type) == 0 ? false : true;
    }

    PyObject* Object_FromString(const std::wstring& str);

    PyObject* Dict_FromCArray(
        const std::vector<std::wstring>& Arr,
        const std::wstring& header);

    PyObject* Dict_FromCArray(
        const std::vector<std::vector<std::wstring>>& Table,
        const std::vector<std::wstring>& Headers);

    PyObject* List_FromVectorString(const std::vector<std::wstring>& Arr);
    PyObject* List_FromVectorString(const std::vector<std::vector<std::wstring>>& Table);

    std::vector<std::wstring> List_AsArray(PyObject* Obj);

    PyObject* Range_FromCRange(ICELL::CRange* rng);

    PyObject* Worksheet_FromCWorksheet(ICELL::CWorksheet* ws);

    static bool IsTypeWorksheet(PyObject* obj)
    {
        return PyType_IsSubtype(obj->ob_type, &PythonWorksheet_Type) == 0 ? false : true;
    }    
}



#ifndef IF_PYERR_RET
#define IF_PYERR_RET(EXPRESSION, ERROR, ERRMSG)	\
	if((EXPRESSION)){							\
		PyErr_SetString(ERROR, ERRMSG);	\
		return nullptr;									\
	}
#endif


#ifndef IF_PYERR_RETDEF
#define IF_PYERR_RETDEF(EXPRESSION, ERROR, ERRMSG, RETVAL)	\
	if((EXPRESSION)){							\
		PyErr_SetString(ERROR, ERRMSG);	\
		return RETVAL;									\
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


#ifndef CHECKRANGE_RET
#define CHECKRANGE_RET(OBJ, MIN, MAX, ERRMSG)										\
	if ((OBJ) < 0 || (OBJ) > MAX){													\
		PyErr_SetString(PyExc_ValueError, ERRMSG);								\
		return nullptr;															\
	}
#endif



#ifndef ASSERT_EXACTREALNUMBER_RET
#define ASSERT_EXACTREALNUMBER_RET(OBJ, ERRMSG) \
    if (IsExactTypeRealNumber((OBJ)) == false){ \
        PyErr_SetString(PyExc_TypeError, (ERRMSG)); \
        return nullptr;                                                     \
    }
#endif


#ifndef ASSERT_ASSIGN_REALNUMBER_RET
#define ASSERT_ASSIGN_REALNUMBER_RET(OBJ, VARIABLE, ERRMSG) \
  if (auto Elem = ExtractRealNumber((OBJ)))             \
    VARIABLE = Elem.value();                            \
  else {                                                 \
      PyErr_SetString(PyExc_TypeError, ERRMSG);         \
      return nullptr;                                   \
  }
#endif


#ifndef ASSERT_ASSIGN_INT_RET
#define ASSERT_ASSIGN_INT_RET(OBJ, VARIABLE, ERRMSG) \
  if (auto Elem = Python::ExtractLong((OBJ)))             \
    VARIABLE = Elem.value();                            \
  else {                                               \
      PyErr_SetString(PyExc_TypeError, ERRMSG);         \
      return nullptr;                                   \
  }
#endif


#ifndef ASSERT_ASSIGN_EXACTINT_RET
#define ASSERT_ASSIGN_EXACTINT_RET(OBJ, VARIABLE, ERRMSG) \
  if (IsExactTypeLong((OBJ)) == false) {                  \
      PyErr_SetString(PyExc_TypeError, ERRMSG);         \
      return nullptr;                                   \
  }                                                     \
  VARIABLE = PyLong_AsLong((OBJ));
#endif


#ifndef ASSERT_EXACTINT_RET
#define ASSERT_EXACTINT_RET(OBJ, ERRMSG) \
  if (IsExactTypeLong((OBJ)) == false) {                  \
      PyErr_SetString(PyExc_TypeError, ERRMSG);         \
      return nullptr;                                   \
  }                                                   
#endif


#ifndef ASSERT_CALLABLE_RET
#define ASSERT_CALLABLE_RET(OBJ, ERRMSG) \
    if (PyCallable_Check((OBJ)) == false){ \
        PyErr_SetString(PyExc_TypeError, (ERRMSG));     \
        return nullptr;                                  \
    }
#endif


#ifndef ASSERT_BOOL_RET
#define ASSERT_BOOL_RET(OBJ, ERRMSG) \
    if (IsExactTypeBool((OBJ)) == false){ \
        PyErr_SetString(PyExc_TypeError, (ERRMSG));     \
        return nullptr;                                   \
    }
#endif


#ifndef ASSERT_ASSIGN_BOOL_RET
#define ASSERT_ASSIGN_BOOL_RET(OBJ, VARIABLE, ERRMSG) \
    if (IsExactTypeBool((OBJ)) == false){ \
        PyErr_SetString(PyExc_TypeError, (ERRMSG));     \
        return nullptr;                                  \
    }                                                   \
    VARIABLE = PyObject_IsTrue(OBJ)
#endif


#ifndef ASSERT_LIST_RET
#define ASSERT_LIST_RET(OBJ, ERRMSG) \
    if (IsSubTypeList((OBJ)) == false){ \
        PyErr_SetString(PyExc_TypeError, (ERRMSG));     \
        return nullptr;                                   \
    }
#endif


#ifndef ASSERT_LIST_NUMPYARRAY_RET
#define ASSERT_LIST_NDARRAY_RET(OBJ, ERRMSG, RET) \
    import_array1(RET); \
    if (!(IsSubTypeList((OBJ)) || PyArray_Check(OBJ))){ \
        PyErr_SetString(PyExc_TypeError, (ERRMSG));     \
        return RET;                                   \
    }
#endif



#ifndef ASSERT_ASSIGN_LIST_NUMPYARRAY_RET
#define ASSERT_ASSIGN_LIST_NUMPYARRAY_RET(OBJ, VARIABLE, ERRMSG, RET) \
    ASSERT_LIST_NDARRAY_RET(OBJ, ERRMSG, RET); \
    if (PyArray_Check(OBJ)) \
       VARIABLE = NDArray_As1DVector<decltype(VARIABLE)::value_type>((PyArrayObject*)OBJ); \
     else \
        VARIABLE = List_As1DVector(OBJ);
#endif



#ifndef CATCHRUNTIMEEXCEPTION_RET
#define CATCHRUNTIMEEXCEPTION_RET()								\
	catch (std::exception& e){								\
		PyErr_SetString(PyExc_RuntimeError, e.what());		\
		return nullptr;										\
	}
#endif