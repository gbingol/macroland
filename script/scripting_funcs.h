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

#include "dllimpexp.h"


namespace script
{
	/*
		txt = "abc.def", delim = "."   -> {"abc", "def"}
		txt = "abc.def.", delim = "."  -> {"abc", "def", ""}
	*/
	DLLSCRIPT std::vector<std::string> split(
			std::string_view txt, 
			std::string_view delim);


	/*
		Arr={"abc", "def"}, delim="." ->"abc.def"
		Arr={"abc"}, delim="." ->"abc"
	*/
	DLLSCRIPT std::string join(
		const std::vector<std::string>& Arr, 
		std::string_view delim);

	
	//Finds the last identifier and then its dictionary keys
	DLLSCRIPT std::list<std::string> GetObjectElements(
		std::string_view ScriptText, 
		PyObject* Module);


	
	DLLSCRIPT std::string GetDocString(
		std::string_view Text,
		std::string_view ID,
		PyObject* PythonModule);



	struct ParamDocStr
	{
		std::string Params;
		std::string Doc;
	};

	//Given a word (numpy, numpy.random.rand) return function's signature
	DLLSCRIPT ParamDocStr GetFuncParamsDocStr(
		std::string_view Word,
		PyObject* PythonModule);


	//Get any container's items as strings
	DLLSCRIPT std::list <std::string> Object_ToStrings(PyObject* Object);

	//search for the index of txt in a list
	DLLSCRIPT std::optional<size_t> List_FindItem(PyObject* List, std::string txt);


	//Runs the pythonfile using PyRun_SimpleFileEx
	DLLSCRIPT void RunPyFile(
		const std::filesystem::path& Path, 
		bool Close = true);

	DLLSCRIPT void RunPyFunc(
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

}