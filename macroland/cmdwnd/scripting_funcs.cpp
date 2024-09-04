#include "scripting_funcs.h"

#include <codecvt>
#include <locale>
#include <random>

#include "../util/util_string.h"


namespace cmdedit
{

    std::list <std::string> GetObjectElements(
		std::string_view Text, 
		PyObject* ModuleObj)
	{
		std::list <std::string> Keys;
		if (ModuleObj == nullptr || Text.empty())
			return {};

		auto IdArray = util::split(Text, ".");

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
		if (!ItemObj && IsLastDot) 
		{
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
				if(!ItemObj) 
				{
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

		if(!PyObject_HasAttrString(EvalObj, "__call__"))
		{
			Py_DECREF(EvalObj);
			return {};
		}

		if(auto AttrObj = PyObject_GetAttrString(EvalObj, "__doc__"))
		{
			if(PyUnicode_Check(AttrObj))
				retVal.Doc = PyUnicode_AsUTF8(AttrObj);
			Py_DECREF(AttrObj);
		}


		auto InspectObj = PyImport_ImportModule("inspect");
		if(!InspectObj)
		{
			Py_DECREF(EvalObj);
			return {};
		}


		PyObject* InspectDictObj = PyModule_GetDict(InspectObj);
		Py_DECREF(InspectObj);
		if (!InspectDictObj)
			return {};

		if(auto SignatureObj = PyDict_GetItemString(InspectDictObj, "signature"))
		{
			if(auto FuncResultObj = PyObject_CallOneArg(SignatureObj, EvalObj))
			{
				Py_DECREF(EvalObj);
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

}