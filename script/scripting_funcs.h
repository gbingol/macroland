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
	DLLSCRIPT std::vector<std::string> split(
			std::string_view txt, 
			std::string_view delim);

	
	//Finds the last identifier and then its dictionary keys
	DLLSCRIPT std::list<std::wstring> ExtractSymbolTable(
		const std::wstring& ScriptText, 
		PyObject* Module);


	/*
		Given an Identifier Array, will make an identifiers
		Array is v[0]]="a", v[1]="b", v[2]="c"

		Index=-1 -> return "c"
		Index=0 -> return a.b.c
		Index=1 or Index=-2-> return b.c
	*/
	DLLSCRIPT std::wstring MakedId(
		const std::vector<std::wstring>& Words, 
		int Index = -1);


	/*
	Given an Identifier Array, will make an identifiers
	Array is v[0]]="a", v[1]="b", v[2]="c" (size=3)

	start=0, end=3 ->a.b.c
	start=1, end=2 ->b
	*/
	DLLSCRIPT std::wstring MakedId(
		const std::vector<std::wstring>& Words, 
		size_t Start, 
		size_t End);


	/*
		last word(i.e., 2 + std or 2.3 + std.gui) will return v[0] = std, v[1] = gui
		Include : Should include trigger char?
	*/
	DLLSCRIPT std::vector<std::wstring> GetIdArray(
		const std::wstring& textRange, 
		bool Include = false);


	//IdArray includes trigger char (.)
	DLLSCRIPT std::vector<std::wstring> RemoveTriggerChars(const std::vector<std::wstring>& IdArray);

	DLLSCRIPT std::wstring GetDocString(
		const std::wstring& ScriptText,
		const std::wstring& Identifier,
		PyObject* PythonModule);



	//dictionary already owns the types
	DLLSCRIPT std::list <std::wstring> Dict_GetKeysVals(PyObject* DictObj);


	//list already owns the types
	DLLSCRIPT std::list <std::wstring> ExtractKeysValueTypes_FromObject(PyObject* Object);


	//The module already knows the type, we use dir to extract
	DLLSCRIPT std::list <std::wstring> ExtractKeysValueTypes_FromModule(
		PyObject* OwningModule,
		const std::wstring& EntryName);


	class GILStateEnsure
	{
	public:
		GILStateEnsure()
		{
			_state = PyGILState_Ensure();
		}
		~GILStateEnsure()
		{
			PyGILState_Release(_state);
		}
	private:
		PyGILState_STATE _state;
	};


	struct PyPackage
	{
		std::wstring m_Pip; //install name for pip (scikit-learn)
		std::wstring m_Name; //package name when imported (sklearn)

		bool operator==(const PyPackage& other) const {
			return m_Name == other.m_Name;
		}

		bool operator<(const PyPackage& other) const {
			return m_Name < other.m_Name;
		}
	};



	//Runs the pythonfile using PyRun_SimpleFileEx
	DLLSCRIPT void RunPyFile(
		const std::filesystem::path& Path, 
		bool Close = true);

	DLLSCRIPT void RunPyFunc(
		const std::wstring& modulePath, 
		const std::wstring& FuncName, 
		std::any param);

	class RunString
	{
	public:
		//create a random module and initialize dictionary
		DLLSCRIPT RunString();

		DLLSCRIPT RunString(PyObject* module);

		//decref module and dictionary
		DLLSCRIPT ~RunString();

		/*
			str: string to be run
			VariableName: The variable to be extracted from the module's dictionary
			If VariableName is empty, then returns the run result, otherwise returns the variable
		*/
		DLLSCRIPT PyObject* run(
			const std::string& str, 
			const std::string& VariableName = std::string());

		DLLSCRIPT std::vector<PyObject*> run(
			const std::string& str, 
			const std::initializer_list<std::string> VarNames);

	private:
		PyObject* m_Module = nullptr, * m_Dict = nullptr;
		bool m_RandModule{ false };
	};



	class CStdOutErrCatcher
	{
	public:
		DLLSCRIPT CStdOutErrCatcher(PyObject* moduleObj = nullptr)
		{
			m_ModuleObj = moduleObj;
		}


		DLLSCRIPT void SetModule(PyObject* moduleObj)
		{
			m_ModuleObj = moduleObj;
		}

		DLLSCRIPT bool StartCatching() const;

		//returns false if something goes wrong, resets the internal value property
		DLLSCRIPT bool CaptureOutput(std::wstring& output) const;


		//restores the previous IO state
		DLLSCRIPT bool RestorePreviousIO() const;

	private:
		PyObject* m_ModuleObj = nullptr;

	};
}