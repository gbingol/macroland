#include "util_string.h"

#include <windows.h>
#include <sstream>


namespace util
{
	std::string to_string(std::wstring wstr)
	{
		if (wstr.empty()) return std::string();

		int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), nullptr, 0, nullptr, nullptr);
		std::string strTo(size_needed, 0);
		WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, nullptr, nullptr);
		return strTo;
	}



	std::wstring to_wstring(std::string str)
	{
		size_t len = mbstowcs(nullptr, str.c_str(), 0); // Get required buffer size (excluding null-terminator)
		if (len == static_cast<size_t>(-1)) {
			throw std::runtime_error("Invalid multibyte character sequence");
		}
    
    std::vector<wchar_t> buf(len + 1);
    mbstowcs(buf.data(), str.c_str(), len + 1);

    return std::wstring(buf.data());
	}



	void rtrim(std::string& s)
	{
		s.erase(std::find_if(s.rbegin(), s.rend(), [](const char ch) 
		{
			return !std::isspace(ch);
		}).base(), s.end());
	}


	void ltrim(std::string &s)
	{
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](const char ch)
		{
			return !std::isspace(ch);
		}));
	}

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

}