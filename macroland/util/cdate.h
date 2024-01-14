#pragma once

#include <string>
#include <time.h>


//TODO: Use C++20 date and time utilities, no need for CDate
namespace util
{
	class CDate
	{
	public:
		CDate();

		CDate(const CDate& another) = default;
		CDate& operator=(const CDate& other) = default;

		bool operator<(const CDate& other) const;
		bool operator==(const CDate& other) const;
		bool operator>(const CDate& other) const;

		~CDate() = default;

		std::string year() const;

		//if Name=true then returns January, February..., otherwise appends 0 if month is smaller than 10
		std::string month() const;

		//if day is smaller than 10, appends zero
		std::string day() const;

		//if hour is smaller than 10, appends zero
		std::string hour() const;

		//if minute is smaller than 10, appends zero
		std::string minute() const;

		//if second is smaller than 10, appends zero
		std::string second() const;

		//Get the current time as hour:minute:second
		std::string GetTime(const char* sep = ":") const 
		{
			return hour() + sep + minute() + sep + second();
		}

		//Gets the date as: day/month/year, reversed=true -> year/month/day 
		std::string GetDate(const char* sep = "/", bool reversed = false) const
		{
			if (reversed)
				return year() + sep + month() + sep + day();

			return  day() + sep + month() + sep + year();
		}

	private:
		int m_Year, m_Month, m_Day;
		int m_Hour, m_Minute, m_Second;
	};

}