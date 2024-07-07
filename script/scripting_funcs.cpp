#include "scripting_funcs.h"

#include <codecvt>
#include <locale>
#include <random>
#include "lexer.h"

namespace script
{
	bool IsNumpyInt(PyObject* obj)
	{
		std::string TypeName = obj->ob_type->tp_name;
		bool Int32 = std::strcmp(TypeName.c_str(), "numpy.int32") == 0 ? true : false;
		bool Int64 = std::strcmp(TypeName.c_str(), "numpy.int64") == 0 ? true : false;

		return Int32 || Int64;
	}


	bool IsNumpyFloat(PyObject* obj)
	{
		std::string TypeName = obj->ob_type->tp_name;
		bool Float32 = std::strcmp(TypeName.c_str(), "numpy.float32") == 0 ? true : false;
		bool Float64 = std::strcmp(TypeName.c_str(), "numpy.float64") == 0 ? true : false;

		return Float32 || Float64;
	}


	bool IsNumpyBool(PyObject* obj)
	{
		std::string TypeName = obj->ob_type->tp_name;
		return std::strcmp(TypeName.c_str(), "numpy.bool_") == 0 ? true : false;
	}

	bool IsNumpyString(PyObject* obj)
	{
		std::string TypeName = obj->ob_type->tp_name;
		return std::strcmp(TypeName.c_str(), "numpy.str_") == 0 ? true : false;
	}


	std::optional<double> ExtractRealNumber(PyObject* obj)
	{
		if (!obj)
			return std::nullopt;

		if (IsSubTypeFloat(obj) || IsNumpyFloat(obj))
			return PyFloat_AsDouble(obj);

		else if (IsSubTypeLong(obj) || IsNumpyInt(obj))
			return PyLong_AsDouble(obj);

		return std::nullopt;
	}


	std::optional<long> ExtractLong(PyObject* obj)
	{
		if (IsSubTypeLong(obj) || IsNumpyInt(obj))
			return PyLong_AsLong(obj);

		return std::nullopt;
	}



	std::list <std::wstring> ExtractSymbolTable(
		const std::wstring& ScriptText, 
		PyObject* Module)
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;

		std::list <std::wstring> SymbolTableKeys;
		if (Module == nullptr)
			return SymbolTableKeys;

		if (ScriptText.empty())
		{
			if (auto Dict = PyModule_GetDict(Module))
				SymbolTableKeys = Dict_GetKeysVals(Dict);

			return SymbolTableKeys;
		}			

		auto IdArray = GetIdArray(ScriptText, true);
		if (IdArray.size() == 0)
			return SymbolTableKeys;

		if (IdArray.size() == 1)
		{
			if (auto Dict = PyModule_GetDict(Module))
				SymbolTableKeys = Dict_GetKeysVals(Dict);
		}

		
		else if(IdArray.size() <= 3)
			SymbolTableKeys = ExtractKeysValueTypes_FromModule(Module, IdArray[0]);
			
		else
		{
			auto TopLevelDict = PyModule_GetDict(Module);
			if (!TopLevelDict)
				return SymbolTableKeys;

			auto ModuleFromVar = PyDict_GetItemString(TopLevelDict, converter.to_bytes(IdArray[0]).c_str());
			if (!ModuleFromVar)
				return SymbolTableKeys;

			std::string ModuleName = PyModule_GetName(ModuleFromVar);
			if (ModuleName.empty())
				return SymbolTableKeys;

			IdArray[0] = converter.from_bytes(ModuleName);
			auto LastWord = *IdArray.rbegin();
			auto TrigModule = MakedId(IdArray, 0, IdArray.size() - (LastWord == "." ? 1 : 2));

			auto TrigModObj = PyImport_ImportModule(converter.to_bytes(TrigModule).c_str()); //new reference
			if (!TrigModObj)
				return SymbolTableKeys;

			PyObject* Dict = PyModule_GetDict(TrigModObj);
			if (Dict)
				SymbolTableKeys = Dict_GetKeysVals(Dict);

			Py_XDECREF(TrigModObj);
		}
		
		return SymbolTableKeys;
	}



	std::wstring MakedId(
		const std::vector<std::wstring>& Words, 
		int Index)
	{
		size_t Size = Words.size();

		size_t AdjustedIndex = Index < 0 ? Size + Index : Index;

		if (AdjustedIndex >= Size)
			return wxEmptyString;

		std::wstringstream ss;

		for (size_t i = AdjustedIndex; i < Size; ++i)
			ss << Words[i] << ".";

		//remove the extra "."
		auto retStr = ss.str();
		retStr.pop_back();

		return retStr;
	}



	std::wstring MakedId(
		const std::vector<std::wstring>& Words, 
		size_t Start, 
		size_t End)
	{
		assert(Start <= End);

		size_t Size = Words.size();

		assert(Start < Size&& End <= Size);

		if (Start == End) return Words[Start];

		std::wstringstream ss;
		for (size_t i = Start; i < End; ++i)
			if (Words[i] != ".")
				ss << Words[i] << ".";


		//remove the extra "."
		auto retStr = ss.str();
		retStr.pop_back();

		return retStr;
	}



	std::vector<std::wstring> GetIdArray(
		const std::wstring& textRange,
		bool IncludeTriggerChar)
	{
		assert(textRange != L"");

		/*
			std::vector<std::string>		identifiers making up a Word: i.e.,
			If word=std.gui then v[0]=std, v[1]=gui
			if word=std.gui. then v[0]=std, v[1]=gui
											I
		*/

		std::vector<CToken*> tokVec;

		std::vector<std::wstring> Words;

		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		std::string EncodedStr = converter.to_bytes(textRange);
		try {
			CGenericLex lexer(EncodedStr);

			int NRawTokens = lexer.size();

			if (NRawTokens == 0)
				return Words;

			if (lexer.at(NRawTokens - 1)->type() == CToken::TYPE::NUMBER) //2.3
				return Words;

			for (int i = NRawTokens - 1; i >= 0; --i) 
			{
				auto tok = lexer.at(i);

				if (tok->type() == CToken::TYPE::IDENTIFIER)
					tokVec.insert(tokVec.begin(), tok);

				else if (tok->type() == CToken::TYPE::DELIMITER && (tok->value() == "."))
					tokVec.insert(tokVec.begin(), tok);

				else break;
			}

		}
		catch (std::exception&) {
			return std::vector<std::wstring>();
		}

		size_t NTokens = tokVec.size();

		if (NTokens == 0)
			return std::vector<std::wstring>();

		for (size_t i = 0; i < NTokens; ++i) 
		{
			if (tokVec[i]->value() == "." && IncludeTriggerChar == false)
				continue;

			Words.push_back(converter.from_bytes(tokVec[i]->value()));
		}

		return Words;
	}



	std::vector<std::wstring> RemoveTriggerChars(const std::vector<std::wstring>& IdArrayWithTriggerChars)
	{
		std::vector<std::wstring> retVec = IdArrayWithTriggerChars;

		size_t ElemsDel = std::erase_if(retVec, [](const std::wstring& str) 
		{
			return str == L"."; 
		});
		
		return retVec;
	}


	std::wstring GetDocString(
		const std::wstring& ScriptText, 
		const std::wstring& Identifier, 
		PyObject* PythonModule)
	{
		if (ScriptText.empty() ||
			Identifier.empty() ||
			PythonModule == nullptr)
			return L"";

		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;

		PyObject* Dictionary = PyModule_GetDict(PythonModule);
		if (!Dictionary)
			return L"";

		std::wstring BaseId, FullId, Command;

		auto IdArrTriggerChars = GetIdArray(ScriptText, true);

		if (IdArrTriggerChars.size() > 1)
		{
			auto IdArray = RemoveTriggerChars(IdArrTriggerChars);
			BaseId = MakedId(IdArray, 0, IdArray.size() - 1);

			auto BaseIdObj = PyDict_GetItemString(Dictionary, converter.to_bytes(BaseId).c_str());
			if (!BaseIdObj)
				return L"";

			bool IsModule = PyType_IsSubtype(BaseIdObj->ob_type, &PyModule_Type) == 0 ? false : true;
			if (BaseIdObj && IsModule == false)
				Command = BaseId + ".__class__" + "." + Identifier + ".__doc__";
			else
				Command = BaseId + "." + Identifier + ".__doc__";

			FullId = BaseId + "." + Identifier;
		}
		else
		{
			BaseId = Identifier;

			auto BaseIdObj = PyDict_GetItemString(Dictionary, converter.to_bytes(BaseId).c_str());
			if (!BaseIdObj)
				return L"";

			bool IsModule = PyType_IsSubtype(BaseIdObj->ob_type, &PyModule_Type) == 0 ? false : true;
			if (BaseIdObj && PyCallable_Check(BaseIdObj))
				Command = BaseId + L".__doc__";
			else if (BaseIdObj && IsModule == false)
				Command = BaseId + L".__class__.__doc__";
			else
				Command = BaseId + L".__doc__";

			FullId = BaseId;
		}


		PyObject* EvalObj = nullptr;

		//string might contain UTF entries, so we encode it
		if (auto CodeObject = Py_CompileString(converter.to_bytes(Command).c_str(), "", Py_eval_input))
		{
			EvalObj = PyEval_EvalCode(CodeObject, Dictionary, Dictionary);

			Py_DECREF(CodeObject);

			if (!EvalObj)
			{
				PyErr_Clear();
				return L"";
			}
		}
		else
		{
			PyErr_Clear();
			return L"";
		}

		auto StrObj = PyObject_Str(EvalObj);
		Py_DECREF(EvalObj);

		if (!StrObj)
			return L"";

		std::wstring DocString = PyUnicode_AsWideCharString(StrObj, nullptr);

		Py_DECREF(StrObj);

		std::wstringstream HTML;
		HTML << "<HTML><BODY>";
		HTML << "<h3>" << FullId << "</h3>";
		HTML << DocString;
		HTML << "</BODY></HTML>";

		return HTML.str();
	}


	std::list <std::wstring> Dict_GetKeysVals(PyObject* DictObj)
	{
		assert(DictObj != nullptr);

		std::list <std::wstring> retSet;

		PyObject* ObjKey, * ObjValue;
		Py_ssize_t pos = 0;

		while (PyDict_Next(DictObj, &pos, &ObjKey, &ObjValue))
		{
			if (!ObjKey || !ObjValue)
				continue;
			std::wstring key = PyUnicode_AsWideCharString(ObjKey, nullptr);

			//do not show keys starting with __, i.e. __doc__
			if (key.substr(0, 2) == L"__")
				continue;

			retSet.push_back(key);
		}

		return retSet;
	}




	std::list<std::wstring> ExtractKeysValueTypes_FromObject(PyObject* Object)
	{
		assert(Object != nullptr);

		std::list <std::wstring> retSet;

		PyObject* ListObj = PyObject_Dir(Object);
		if (!ListObj)
			return retSet;

		size_t szLst = PyList_GET_SIZE(ListObj);

		for (size_t i = 0; i < szLst; ++i)
		{
			PyObject* listItem = PyList_GetItem(ListObj, i);
			if (!listItem)
				continue;

			PyObject* StrObj = PyObject_Str(listItem);
			if (!StrObj)
				continue;

			std::wstring EntryName = PyUnicode_AsWideCharString(StrObj, nullptr);

			//do not show keys starting with __, i.e. __doc__
			if (EntryName.substr(0, 2) == L"__")
				continue;

			retSet.push_back(EntryName);
		}

		Py_DECREF(ListObj);

		return retSet;
	}



	std::list <std::wstring> ExtractKeysValueTypes_FromModule(
		PyObject* OwningModule, 
		const std::wstring& EntryName)
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		assert(OwningModule != nullptr);
		assert(EntryName != L"");

		if (auto OwningModule_DicObj = PyModule_GetDict(OwningModule))
		{
			if (auto DicItem = PyDict_GetItemString(OwningModule_DicObj, converter.to_bytes(EntryName).c_str()))
				return ExtractKeysValueTypes_FromObject(DicItem);
		}

		return std::list <std::wstring>();
	}


	std::set<std::wstring> GetInstalledPackages()
	{
		std::set<std::wstring> Packages;

		std::string PythonCmd = "import pkgutil \n"
			"x = pkgutil.iter_modules() \n"
			"ModuleList = [] \n"
			"for i in x: \n"
			"    if(i.ispkg==True): \n"
			"        ModuleList.append(i.name)";


		auto Run = RunString();
		auto ListObj = Run.run(PythonCmd, "ModuleList");

		if (ListObj)
		{
			size_t N = PyList_GET_SIZE(ListObj);
			for (size_t i = 0; i < N; ++i)
			{
				auto ListItem = PyUnicode_AsWideCharString(PyList_GetItem(ListObj, i), nullptr);
				Packages.insert(ListItem);
			}
		}

		Py_XDECREF(ListObj);

		return Packages;
	}


	std::vector<PyPackage> FindMissingPackages(const std::set<PyPackage>& RequiredPkg)
	{
		std::vector<PyPackage> MissingPkgs;

		auto InstPkgs = GetInstalledPackages();

		std::set<PyPackage> InstalledPkgs;
		for (const auto& Name : InstPkgs)
		{
			PyPackage Packg;
			Packg.m_Name = Name;
			Packg.m_Pip = Name;
			InstalledPkgs.insert(Packg);
		}

		std::set_difference(
			RequiredPkg.begin(), 
			RequiredPkg.end(), 
			InstalledPkgs.begin(), 
			InstalledPkgs.end(), 
			std::back_inserter(MissingPkgs));

		return MissingPkgs;
	}

	
	bool InstallMissingPackages(
		const std::vector<PyPackage>& MissingPkgs, 
		bool ShowMsg)
	{
		std::wstring ReqPkgNames; //those are missing and required
		std::wstring ReqPkgPips;
		size_t N = MissingPkgs.size();
		for (const auto& pkg : MissingPkgs)
		{
			ReqPkgNames += (pkg.m_Name + L" ");
			ReqPkgPips += (pkg.m_Pip + L" ");
		}

		std::wstringstream Msg;
		Msg << "[" << ReqPkgNames << "]" << (N > 1 ? " are" : " is") << " missing. Wanna install now ?";

		int YesNo = wxMessageBox(Msg.str(), "Install Missing Packages?", wxYES_NO);
		if (YesNo == wxNO)
			return false;

		std::filesystem::path PyHome = Py_GetPythonHome();
		auto PyExe = PyHome / "python.exe";

		//for paths containing spaces we need the whole path in double quotes
		std::wstring Cmd = L"\"" + PyExe.wstring() + L"\"";

		//add the pip name of the package to the command
		Cmd += " -m pip install " + ReqPkgPips;

		wxShell(Cmd);

		if (ShowMsg)
			wxMessageBox("Installation process completed.");

		return true;
	} 


	void RunPyFile(
		const std::filesystem::path& Path, 
		bool Close)
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;

		if (!std::filesystem::exists(Path))
			return;


		std::wstring Pth = Path.wstring();
		if (auto cp = _Py_wfopen(Pth.c_str(), L"rb"))
			PyRun_SimpleFileExFlags(cp, converter.to_bytes(Pth).c_str(), Close, 0);

	}


	void RunPyFunc(
		const std::wstring& modulePath, 
		const std::wstring& FuncName, 
		std::any param)
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;

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
			Py_DECREF(Dict);
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


	RunString::RunString()
	{
		//random string of length 8
		constexpr int N = 8;
		std::vector<int> Vec(N);

		std::random_device rd;
		std::mt19937 generator(rd());
		std::uniform_int_distribution<int> dist(1, 26);
		std::transform(Vec.begin(), Vec.end(), Vec.begin(), [&](int val) {return dist(generator); });

		std::stringstream ss;
		for (size_t i = 0; i < N; ++i)
			ss << char(('A' - 1) + Vec[i]);
		auto RandomStr = ss.str();

		m_Module = PyModule_New(RandomStr.c_str());
		m_RandModule = true;

		if (m_Module == nullptr)
			return;

		m_Dict = PyModule_GetDict(m_Module); //borrowed

		if (m_Dict == nullptr)
		{
			Py_DECREF(m_Module);
			return;
		}
	}


	RunString::RunString(PyObject* module)
	{
		m_Module = module;
		m_RandModule = false;

		if (m_Module == nullptr)
			return;

		m_Dict = PyModule_GetDict(m_Module); //borrowed
		if (m_Dict == nullptr)
		{
			Py_DECREF(m_Module);
			return;
		}
	}


	RunString::~RunString()
	{
		if (m_RandModule)
			Py_XDECREF(m_Module);
	}


	PyObject* RunString::run(
		const std::string& str, 
		const std::string& VariableName)
	{
		PyObject* retObj = nullptr;

		if (m_Dict == nullptr)
			return nullptr;

		//string might contain UTF entries, so we encode it
		//PyObject* CodeObject = Py_CompileString(str.c_str(), "", Py_file_input);

		auto ResultObj = PyRun_String(str.c_str(), Py_file_input, m_Dict, m_Dict);

		if (ResultObj)
		{
			if (VariableName.empty() == false)
				retObj = PyDict_GetItemString(m_Dict, VariableName.c_str());
			else
				retObj = ResultObj;
		}
		else
			PyErr_Clear();

		return retObj;
	}

	std::vector<PyObject*> RunString::run(const std::string& str, const std::initializer_list<std::string> VariableNames)
	{
		std::vector<PyObject*> retList;

		if (m_Dict == nullptr)
			return retList;

		//string might contain UTF entries, so we encode it
		//PyObject* CodeObject = Py_CompileString(str.c_str(), "", Py_file_input);

		auto ResultObj = PyRun_String(str.c_str(), Py_file_input, m_Dict, m_Dict);

		if (ResultObj)
		{
			for (const auto& VariableName : VariableNames)
				retList.push_back(PyDict_GetItemString(m_Dict, VariableName.c_str()));
		}
		else
			PyErr_Clear();

		return retList;
	}



	bool CStdOutErrCatcher::StartCatching() const
	{
		/*
		python code to redirect stdouts / stderr
		From: https://stackoverflow.com/questions/4307187/how-to-catch-python-stdout-in-c-code
		*/
		const std::string stdOutErr =
			"import sys\n\
class StdOutput:\n\
	def __init__(self):\n\
		self.value = ''\n\
		self.stdout=sys.stdout\n\
		self.stderr=sys.stderr\n\
	def write(self, txt):\n\
		self.value += txt\n\
	def restore(self):\n\
		sys.stdout=self.stdout\n\
		sys.stderr=self.stderr\n\
CATCHSTDOUTPUT = StdOutput()\n\
sys.stdout = CATCHSTDOUTPUT\n\
sys.stderr = CATCHSTDOUTPUT\n\
			";

		if (m_ModuleObj == nullptr)
			return false;

		if (auto py_dict = PyModule_GetDict(m_ModuleObj))
		{
			if (auto ResultObj = PyRun_String(stdOutErr.c_str(), Py_file_input, py_dict, py_dict))
				return true;
		}

		return false;
	}



	bool CStdOutErrCatcher::CaptureOutput(std::wstring& output) const
	{
		PyObject* py_dict = PyModule_GetDict(m_ModuleObj);
		if (!py_dict)
			return false;

		PyObject* catcher = PyDict_GetItemString(py_dict, "CATCHSTDOUTPUT");
		if (!catcher)
			return false;

		PyObject* OutputObj = PyObject_GetAttrString(catcher, "value");
		if (!OutputObj)
			return false;

		output = PyUnicode_AsWideCharString(OutputObj, nullptr);
		PyObject_SetAttrString(catcher, "value", Py_BuildValue("s", ""));

		return true;
	}



	bool CStdOutErrCatcher::RestorePreviousIO() const
	{
		PyObject* py_dict = PyModule_GetDict(m_ModuleObj);
		if (!py_dict)
			return false;

		PyObject* catcher = PyDict_GetItemString(py_dict, "CATCHSTDOUTPUT");
		if (!catcher)
			return false;

		auto CallResult = PyObject_CallMethodNoArgs(catcher, Py_BuildValue("s", "restore"));

		return true;
	}

}