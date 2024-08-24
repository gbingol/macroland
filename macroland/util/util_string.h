#pragma once

#include <vector>
#include <string>



namespace util
{
	std::string to_string(std::wstring wstr);
	std::wstring to_wstring(std::string str);

	void ltrim(std::string& s);
	void rtrim(std::string& s);

	inline std::string trim(std::string& s)
	{
		ltrim(s);
		rtrim(s);
		return s;
	}

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
}