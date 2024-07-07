#include "PythonWrapper.h"

#include <wx/wx.h>

#include <codecvt>



namespace Python
{
   
    PyObject* Object_FromString(const std::wstring& str)
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        std::string s = converter.to_bytes(str);

        if (s.empty())
            Py_RETURN_NONE;

        char* stringpart;
        double value = strtod(s.c_str(), &stringpart);

        if (strcmp(stringpart, ""))
            return PyUnicode_FromWideChar(str.c_str(), -1);
        else
        {
            if (long(value) == value)
                return PyLong_FromLong(long(value));
            else
                return PyFloat_FromDouble(value);
        }

        Py_RETURN_NONE;
    }



    PyObject* Dict_FromCArray(
        const std::vector<std::wstring>& Arr,
        const std::wstring& header)
    {
        if (Arr.size() == 0)
            return nullptr;

        auto Dict = PyDict_New();
        auto Header = Object_FromString(header);

        auto List = List_FromVectorString(Arr);
        PyDict_SetItem(Dict, Header, List);

        return Dict;
    }


    PyObject* Dict_FromCArray(
        const std::vector<std::vector<std::wstring>>& Table,
        const std::vector<std::wstring>& Headers)
    {
        if (Table.size() == 0 || Headers.size() == 0)
            return nullptr;

        if (Table.size() != Headers.size())
            return nullptr;

        auto Dict = PyDict_New();

        for (size_t i = 0; i < Table.size(); ++i)
        {
            auto Header = Object_FromString(Headers[i]);
            auto List = List_FromVectorString(Table[i]);

            PyDict_SetItem(Dict, Header, List);
        }

        return Dict;
    }


    PyObject* List_FromVectorString(const std::vector<std::wstring>& Arr)
    {
        if (Arr.size() == 0)
            return nullptr;

        auto RetList = PyList_New(Arr.size());

        for (size_t i = 0; i < Arr.size(); ++i)
            PyList_SetItem(RetList, i, Object_FromString(Arr[i]));

        return RetList;
    }




    PyObject* List_FromVectorString(const std::vector<std::vector<std::wstring>>& Table)
    {
        PyObject* retList = PyList_New(0);

        for (const auto& Vec : Table)
            PyList_Append(retList, List_FromVectorString(Vec));

        return retList;
    }


    std::vector<std::wstring> List_AsArray(PyObject* Obj)
    {
        std::vector<std::wstring> retArr;

        if (IsSubTypeList(Obj) == false)
            throw std::exception("object must be of type list");

        int N = PyList_GET_SIZE(Obj);
        for (size_t i = 0; i < N; ++i)
        {
            PyObject* ListItem = PyList_GetItem(Obj, i);

            if (!ListItem)
                throw std::exception("invalid list entry");
     
            retArr.push_back(PyUnicode_AsWideCharString(ListItem, nullptr));
        }

        return retArr;
    }




    PyObject* Range_FromCRange(ICELL::CRange* rng)
    {
        auto RangeObj = PyObject_New(Range, &PythonRange_Type);
        RangeObj->ptrObj = rng;
        RangeObj->iter = 0;
        RangeObj->state = true;

        return (PyObject*)RangeObj;
    }


    PyObject* Worksheet_FromCWorksheet(ICELL::CWorksheet* ws)
    {
        auto WorksheetObj = PyObject_New(Worksheet, &PythonWorksheet_Type);
        WorksheetObj->ptrObj = ws;
        WorksheetObj->state = true;

        return (PyObject*)WorksheetObj;
    }


}