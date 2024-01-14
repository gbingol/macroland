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
		CDate(const time_t& time);

		CDate(const CDate& another) = default;
		CDate& operator=(const CDate& other);

		~CDate() = default;


		int GetYear() const;

		//returns a value between 1 and 12
		int GetMonth() const;

		//returns a value between 1-31
		int GetDay() const;

		//returns a value between 0-23
		int GetHour() const;

		//returns a value between 0-59
		int GetMinute() const;

		//returns a value between 0-59
		int GetSecond() const;

		std::string GetYearStr() const;

		//if Name=true then returns January, February..., otherwise appends 0 if month is smaller than 10
		std::string GetMonthStr(bool Name = false) const;

		//if day is smaller than 10, appends zero
		std::string GetDayStr() const;

		//if hour is smaller than 10, appends zero
		std::string GetHourStr() const;

		//if minute is smaller than 10, appends zero
		std::string GetMinuteStr() const;

		//if second is smaller than 10, appends zero
		std::string GetSecondStr() const;

		//Get the current time as hour:minute:second
		std::string GetTime(const char* sep = ":") const {
			return GetHourStr() + sep + GetMinuteStr() + sep + GetSecondStr();
		}

		//Gets the date as: day/month/year, reversed=true -> year/month/day 
		std::string GetDate(const char* sep = "/", bool reversed = false) const
		{
			if (reversed)
				return GetYearStr() + sep + GetMonthStr() + sep + GetDayStr();

			return  GetDayStr() + sep + GetMonthStr() + sep + GetYearStr();
		}

		bool operator<(const CDate& other);
		bool operator==(const CDate& other);
		bool operator>(const CDate& other);

	private:
		int m_Year, m_Month, m_Day;
		int m_Hour, m_Minute, m_Second;
	};

}