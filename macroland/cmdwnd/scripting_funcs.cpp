#include "scripting_funcs.h"

#include <codecvt>
#include <locale>
#include <random>



namespace cmdedit
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
		std::string_view Text, 
		PyObject* ModuleObj)
	{
		std::list <std::string> Keys;
		if (ModuleObj == nullptr || Text.empty())
			return {};

		auto IdArray = split(Text, ".");

		if(IdArray.rbegin()->empty())
			IdArray.pop_back();

		size_t ArrSize = IdArray.size();
		if(ArrSize == 0) return {};

		auto gs = GILStateEnsure();

		//The top-level dictionary (borrowed-reference)
		auto TopDict = PyModule_GetDict(ModuleObj);
		if (!TopDict)
			return {};
		
		bool IsLastDot = Text.ends_with(".");

		auto ItemObj = PyDict_GetItemString(TopDict, IdArray[0].c_str());
		if (!ItemObj && IsLastDot) {
			PyErr_Clear();
			return {};
		}
		
		if (ArrSize == 1)
			return Object_ToStrings(IsLastDot ? ItemObj: ModuleObj);	
		
		else if(ArrSize > 1)
		{
			size_t n = IsLastDot ? ArrSize: ArrSize-1;
			for(size_t i=1; i<n; i++) 
			{
				ItemObj = PyObject_GetAttrString(ItemObj, IdArray[i].c_str());
				if(!ItemObj) {
					PyErr_Clear();
					return {};
				}
			}
			return Object_ToStrings(ItemObj);
		}
		
		return {};
	}



	ParamDocStr GetfrmParamsDocStr(std::string_view Word, PyObject *ModuleObj)
	{
		if (Word.empty() || !ModuleObj)
			return {};


		ParamDocStr retVal;

		//Get the GIL
		auto gstate = GILStateEnsure();

		auto DictObj = PyModule_GetDict(ModuleObj);

		PyObject *EvalObj{nullptr}, *CodeObj{nullptr};
		//string might contain UTF entries, so we encode it
		CodeObj = Py_CompileString(std::string(Word).c_str(), "", Py_eval_input);
		if (!CodeObj)
		{
			PyErr_Clear();
			return {};
		}

		EvalObj = PyEval_EvalCode(CodeObj, DictObj, DictObj);
		Py_DECREF(CodeObj);

		if(!EvalObj)
		{
			PyErr_Clear();
			return {};
		}

		auto decEval = DECREFOBJ(EvalObj);

		if(!PyObject_HasAttrString(EvalObj, "__call__"))
			return {};

		if(auto AttrObj = PyObject_GetAttrString(EvalObj, "__doc__"))
		{
			if(PyUnicode_Check(AttrObj))
				retVal.Doc = PyUnicode_AsUTF8(AttrObj);
			Py_DECREF(AttrObj);
		}


		auto InspectObj = PyImport_ImportModule("inspect");
		if(!InspectObj)
			return {};

		auto decInspect = DECREFOBJ(InspectObj);

		PyObject* InspectDictObj = PyModule_GetDict(InspectObj);
		if (!InspectDictObj)
			return {};

		if(auto SignatureObj = PyDict_GetItemString(InspectDictObj, "signature"))
		{
			if(auto FuncResultObj = PyObject_CallOneArg(SignatureObj, EvalObj))
			{
				if(auto StrObj = PyObject_Str(FuncResultObj))
				{
					retVal.Params = PyUnicode_AsUTF8(StrObj);
					Py_DECREF(StrObj);
				}

				Py_DECREF(FuncResultObj);
			}
		}

		PyErr_Clear();

		return retVal;
	}




	std::list<std::string> Object_ToStrings(PyObject* Object)
	{
		if(!Object)
			return {};

		//Get the GIL
		auto gstate = GILStateEnsure();

		auto ListObj = PyObject_Dir(Object);
		if (!ListObj)
			return {};

		size_t NPrvt=0, NPublic=0;
		std::list <std::string> retSet;

		for (size_t i = 0; i < PyList_GET_SIZE(ListObj); ++i)
		{
			auto listItem = PyList_GetItem(ListObj, i);
			if (!listItem)
				continue;

			auto StrObj = PyObject_Str(listItem);
			if (!StrObj)
				continue;

			std::string str = PyUnicode_AsUTF8(StrObj);
			if(str.substr(0, 2) == "__") NPrvt++; else NPublic++;

			Py_DECREF(StrObj);

			retSet.push_back(str);
		}
		Py_DECREF(ListObj);


		if(NPublic>0 && NPrvt>0)
		{
			std::erase_if(retSet, [=](std::string e)
			{
				return e.substr(0, 2) == "__";
			});
		}

		return retSet;
	}



    void RunPyFile(const std::filesystem::path& Path)
	{
		if (!std::filesystem::exists(Path))
			return;

		auto Pth = Path.wstring();
		if (auto cp = _Py_wfopen(Pth.c_str(), L"rb"))  
		{
			std::wstring_convert<std::codecvt_utf8<wchar_t>> con;
			auto gstate = GILStateEnsure();
			PyRun_SimpleFileExFlags(cp, con.to_bytes(Path).c_str(), true, 0);
		}
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

		auto decModuleObj = DECREFOBJ(Module);

		auto Dict = PyModule_GetDict(Module);
		if (!Dict)
			throw std::exception("cannot access module's dictionary");

		PyObject *FuncObj{nullptr};
		if(auto FuncNameObj = PyUnicode_FromWideChar(FuncName.c_str(), -1))
		{
			auto decFunObj = DECREFOBJ(FuncNameObj);
			FuncObj = PyDict_GetItem(Dict, FuncNameObj);
		}
		else
			throw std::exception("Function name is not valid.");

		if (!FuncObj)
			throw std::exception("function name does not exist");

		if(!PyObject_HasAttrString(FuncObj, "__call__"))
			throw std::exception("Invalid function. It is not callable.");

		PyObject* RetObj = nullptr;
		if (!param.has_value())
			RetObj = PyObject_CallNoArgs(FuncObj);
		else
		{
			if (auto v = std::any_cast<double>(&param))
				RetObj = PyObject_CallOneArg(FuncObj, Py_BuildValue("d", *v));

			else if (auto v = std::any_cast<int>(&param))
				RetObj = PyObject_CallOneArg(FuncObj, Py_BuildValue("i", *v));

			else if (auto v = std::any_cast<std::wstring>(&param))
			{
				if(auto Obj = PyUnicode_FromWideChar(v->c_str(), -1))
				{
					RetObj = PyObject_CallOneArg(FuncObj, Obj);
					Py_DECREF(Obj);
				}
			}
		}

		Py_XDECREF(RetObj);
	}

}