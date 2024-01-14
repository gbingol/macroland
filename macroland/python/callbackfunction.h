#pragma once

#include <Python.h>


namespace Python
{

	struct CEventCallbackFunc
	{
		//member variables are populated 

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

	/*
	Example (from Worksheet) Usage:

	if (m_EvtCallBack[event].size() > 0) 
	{
		const auto& List = m_EvtCallBack[event];

		for (const auto& CallBk : List)
			CallBk->call(CallBk->m_FuncObj, CallBk->m_FuncArgs, nullptr);
	}
	
	*/
}