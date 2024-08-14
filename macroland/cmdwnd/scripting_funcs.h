#pragma once

#include <stack>
#include <optional>
#include <list>
#include <string>
#include <set>
#include <filesystem>
#include <any>

#include <Python.h>

#include <wx/wx.h>



namespace script
{
	/*
		txt = "abc.def", delim = "."   -> {"abc", "def"}
		txt = "abc.def.", delim = "."  -> {"abc", "def", ""}
	*/
	std::vector<std::string> split(
			std::string_view txt, 
			std::string_view delim);


	/*
		Arr={"abc", "def"}, delim="." ->"abc.def"
		Arr={"abc"}, delim="." ->"abc"
	*/
	std::string join(
		const std::vector<std::string>& Arr, 
		std::string_view delim);

	
	//Finds the last identifier and then its dictionary keys
	std::list<std::string> GetObjectElements(
		std::string_view ScriptText, 
		PyObject* Module);


	
	struct ParamDocStr
	{
		std::string Params;
		std::string Doc;
	};

	//Given a word (numpy, numpy.random.rand) return function's signature
	ParamDocStr GetfrmParamsDocStr(
		std::string_view Word,
		PyObject* PythonModule);


	//Get any container's items as strings
	std::list <std::string> Object_ToStrings(PyObject* Object);


	//Runs the pythonfile using PyRun_SimpleFileEx
	void RunPyFile(const std::filesystem::path& Path);

	void RunPyFunc(
		const std::wstring& modulePath, 
		const std::wstring& FuncName, 
		std::any param);

	

	class GILStateEnsure
	{
	public:
		GILStateEnsure() {
			_state = PyGILState_Ensure();
		}
		~GILStateEnsure() {
			PyGILState_Release(_state);
		}
	private:
		PyGILState_STATE _state;
	};


	class DECREFOBJ
	{
	public:
		DECREFOBJ(PyObject* obj):_obj{obj} { }
		~DECREFOBJ() {
			Py_XDECREF(_obj);
		}

	private:
		PyObject* _obj;
	};
}