#include "scripting_funcs.h"

#include <codecvt>
#include <locale>
#include <random>



namespace script
{
    std::vector<std::string> split(std::string_view txt, std::string_view delim)
    {
        std::vector<std::string> retVec;
    	auto npos = txt.find(delim);
    	size_t start = 0;
		
		while(npos != std::string::npos)
		{
			retVec.push_back(std::string(txt.substr(start, npos-start)));
			start = npos + delim.length();
			npos = start;
			npos = txt.find(delim, npos);  
		}
		retVec.push_back(std::string(txt.substr(start, npos-start)));

    	return retVec;
    }


    std::string join(
		const std::vector<std::string> &Arr, 
		std::string_view delim)
    {
		size_t len = Arr.size();
		if(len == 1)
			return Arr[0];
		
		std::stringstream ss;
        for(size_t i=0; i<len - 1; ++i)
			ss<<Arr[i]<<delim;
		
		ss<<*Arr.rbegin();
		return ss.str();
    }



    std::list <std::string> GetObjectElements(
		std::string_view ScriptText, 
		PyObject* Module)
	{
		std::list <std::string> Keys;
		if (Module == nullptr || ScriptText.empty())
			return {};

		auto IdArray = split(ScriptText, ".");

		if(IdArray.rbegin()->empty())
			IdArray.pop_back();

		auto gs = GILStateEnsure();

		if (IdArray.size() == 1)
		{
			auto DictObj = PyModule_GetDict(Module); //borrowed
			if(!DictObj)
				return {};

			if(ScriptText.ends_with("."))
			{
				if (auto DictItem = PyDict_GetItemString(DictObj, IdArray[0].c_str())) //borrowed
					Keys = Object_ToStrings(DictItem);
			}
			else
				Keys = Object_ToStrings(Module);
		}
		
		else if(IdArray.size() > 1)
		{
			//np.random.
			auto TopLevelDict = PyModule_GetDict(Module);
			if (!TopLevelDict)
				return {};

			//find np
			auto PythonObj = PyDict_GetItemString(TopLevelDict, IdArray[0].c_str());
			if (!PythonObj)
				return {};

			if(!PyModule_Check(PythonObj))
				return Object_ToStrings(PythonObj);
			
			//get np's module name (numpy)
			std::string ModuleName = PyModule_GetName(PythonObj);
			if (ModuleName.empty())
				return {};

			IdArray[0] = ModuleName;

			std::string Identifier;
			if(!ScriptText.ends_with("."))
			{
				Identifier = *IdArray.rbegin();
				IdArray.pop_back(); //remove identifier
			}

			auto ModuleToImport = join(IdArray, ".");
			auto ModuleObj = PyImport_ImportModule(ModuleToImport.c_str()); //new reference
			if (!ModuleObj)
				return {};
			
			Keys = Object_ToStrings(ModuleObj);
			
			Py_DECREF(ModuleObj);
		}
		
		return Keys;
	}



	std::string GetDocString(
		std::string_view ScriptText, 
		std::string_view Identifier, 
		PyObject* PyModule)
	{
		if (ScriptText.empty() ||
			Identifier.empty() ||
			PyModule == nullptr)
			return "";

		//Get the GIL
		auto gstate = GILStateEnsure();

		PyObject* DictObj = PyModule_GetDict(PyModule);
		if (!DictObj)
			return "";

		std::string Cmd;

		bool LastTrig = false;
		auto IdArr = split(ScriptText, ".");
		if(IdArr.rbegin()->empty())
		{
			LastTrig = true;
			IdArr.pop_back();
		}

		if (IdArr.size() == 1)
			Cmd = std::string(Identifier) + ".__doc__";
		else
		{
			if(!LastTrig)
				IdArr.pop_back();
			IdArr.push_back(std::string(Identifier));
			auto FullStr = join(IdArr, ".");
			Cmd = FullStr + L".__doc__";
		}

		PyObject* EvalObj = nullptr;
		//string might contain UTF entries, so we encode it
		if (auto CodeObject = Py_CompileString(Cmd.c_str(), "", Py_eval_input))
		{
			EvalObj = PyEval_EvalCode(CodeObject, DictObj, DictObj);
			Py_DECREF(CodeObject);

			if (!EvalObj){
				PyErr_Clear();
				return "";
			}
		}
		else
		{
			PyErr_Clear();
			return "";
		}

		auto StrObj = PyObject_Str(EvalObj);
		Py_DECREF(EvalObj);

		if (!StrObj)
			return "";

		auto DocString = PyUnicode_AsUTF8(StrObj);
		Py_DECREF(StrObj);

		std::stringstream HTML;
		HTML << "<HTML><BODY>";
		HTML << "<h3>" << Identifier << "</h3>";
		HTML << DocString;
		HTML << "</BODY></HTML>";

		return HTML.str();
	}


	std::list <std::string> Dict_GetKeys(PyObject* DictObj)
	{
		assert(DictObj != nullptr);

		std::list <std::string> retSet;

		PyObject* ObjKey, * ObjValue;
		Py_ssize_t pos = 0;

		//Get the GIL
		auto gstate = GILStateEnsure();

		while (PyDict_Next(DictObj, &pos, &ObjKey, &ObjValue))
		{
			std::string key = PyUnicode_AsUTF8(ObjKey);

			//do not show keys starting with __, i.e. __doc__
			if (key.substr(0, 2) == L"__")
				continue;

			retSet.push_back(key);
		}

		return retSet;
	}




	std::list<std::string> Object_ToStrings(PyObject* Object)
	{
		assert(Object != nullptr);

		//Get the GIL
		auto gstate = GILStateEnsure();

		PyObject* ListObj = PyObject_Dir(Object);
		if (!ListObj)
			return {};

		size_t szLst = PyList_GET_SIZE(ListObj);
		
		std::list <std::string> retSet;
		for (size_t i = 0; i < szLst; ++i)
		{
			PyObject* listItem = PyList_GetItem(ListObj, i);
			if (!listItem)
				continue;

			PyObject* StrObj = PyObject_Str(listItem);
			if (!StrObj)
				continue;

			std::string EntryName = PyUnicode_AsUTF8(StrObj);

			//do not show keys starting with __, i.e. __doc__
			if (EntryName.substr(0, 2) == L"__")
				continue;

			retSet.push_back(EntryName);
		}
		Py_DECREF(ListObj);

		return retSet;
	}




	void RunPyFile(
		const std::filesystem::path& Path, 
		bool Close)
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;

		if (!std::filesystem::exists(Path))
			return;

		auto Pth = Path.wstring();
		auto gstate = GILStateEnsure();

		if (auto cp = _Py_wfopen(Pth.c_str(), L"rb"))
			PyRun_SimpleFileExFlags(cp, converter.to_bytes(Pth).c_str(), Close, 0);
	}


	void RunPyFunc(
		const std::wstring& modulePath, 
		const std::wstring& FuncName, 
		std::any param)
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;

		auto gstate = GILStateEnsure();

		auto Module = PyImport_ImportModule(converter.to_bytes(modulePath).c_str());
		if (!Module)
			throw std::exception("module not found");

		auto Dict = PyModule_GetDict(Module);
		if (!Dict)
		{
			Py_DECREF(Module);
			throw std::exception("cannot access module's dictionary");
		}

		auto Func = PyDict_GetItemString(Dict, converter.to_bytes(FuncName).c_str());
		if (!Func)
		{
			Py_DECREF(Module);
			throw std::exception("function does not exist");
		}

		PyObject* Ret = nullptr;
		if (!param.has_value())
			Ret = PyObject_CallNoArgs(Func);
		else
		{
			if (auto v = std::any_cast<double>(&param))
				Ret = PyObject_CallOneArg(Func, Py_BuildValue("d", *v));

			else if (auto v = std::any_cast<int>(&param))
				Ret = PyObject_CallOneArg(Func, Py_BuildValue("i", *v));

			else if (auto v = std::any_cast<std::wstring>(&param))
			{
				auto Obj = PyUnicode_FromWideChar(v->c_str(), -1);
				Ret = PyObject_CallOneArg(Func, Obj);
			}
		}

		Py_DECREF(Module);
	}

}