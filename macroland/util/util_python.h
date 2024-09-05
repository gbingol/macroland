#pragma once

#include <Python.h>

#include <string>
#include <filesystem>



namespace util
{
	bool RunExtensions();

	//runs all the Python files in a given directory
	bool RunDirectoryContents(const std::filesystem::path &DirPath);

	bool RunPythonFile(const std::filesystem::path &FilePath);
}