#include "cdate.h"

#include <ctime>
#include <stdlib.h>


namespace util
{
	CDate::CDate()
	{
		time_t Time = std::time(0);
		std::tm* TimeInfo = std::localtime(&Time);

		//Since subsequent calls change the time, we need to store them so that time will not change when same CDate object is called

		m_Year = 1900 + TimeInfo->tm_year;
		m_Month = TimeInfo->tm_mon + 1;
		m_Day = TimeInfo->tm_mday;
		m_Hour = TimeInfo->tm_hour;
		m_Minute = TimeInfo->tm_min;
		m_Second = TimeInfo->tm_sec;
	}

	CDate::CDate(const time_t& time)
	{
		time_t Time = time;
		std::tm* TimeInfo = std::localtime(&Time);

		//Since subsequent calls change the time, we need to store them so that time will not change when same CDate object is called
		m_Year = 1900 + TimeInfo->tm_year;
		m_Month = TimeInfo->tm_mon + 1;
		m_Day = TimeInfo->tm_mday;
		m_Hour = TimeInfo->tm_hour;
		m_Minute = TimeInfo->tm_min;
		m_Second = TimeInfo->tm_sec;
	}

	CDate& CDate::operator=(const CDate& other)
	{
		if (this == &other)
			return *this;

		m_Year = other.m_Year;
		m_Month = other.m_Month;
		m_Day = other.m_Day;
		m_Hour = other.m_Hour;
		m_Minute = other.m_Minute;
		m_Second = other.m_Second;

		return *this;
	}


	int CDate::GetYear() const {
		return m_Year;
	}

	int CDate::GetMonth() const {
		return m_Month;
	}

	int CDate::GetDay() const {
		return m_Day;
	}

	int CDate::GetHour() const {
		return m_Hour;
	}

	int CDate::GetMinute() const {
		return m_Minute;
	}

	int CDate::GetSecond() const {
		return m_Second;
	}

	std::string CDate::GetYearStr() const {
		return std::to_string(GetYear());
	}

	std::string CDate::GetMonthStr(bool Name) const
	{
		std::string Months[] = {
			"January", "February", "March", "April", "May", "June",
			"July", "August", "September", "October", "November", "December"
		};

		if (Name) return Months[GetMonth() - 1];

		return (GetMonth() < 10) ?
			std::string("0") + std::to_string(GetMonth()) :
			std::to_string(GetMonth());
	}

	std::string CDate::GetDayStr() const
	{
		return (GetDay() < 10) ?
			std::string("0") + std::to_string(GetDay()) :
			std::to_string(GetDay());
	}

	std::string CDate::GetHourStr() const
	{
		return (GetHour() < 10) ?
			std::string("0") + std::to_string(GetHour()) :
			std::to_string(GetHour());
	}

	std::string CDate::GetMinuteStr() const
	{
		return (GetMinute() < 10) ?
			std::string("0") + std::to_string(GetMinute()) :
			std::to_string(GetMinute());
	}

	std::string CDate::GetSecondStr() const
	{
		return (GetSecond() < 10) ?
			std::string("0") + std::to_string(GetSecond()) :
			std::to_string(GetSecond());
	}


	bool CDate::operator<(const CDate& other)
	{
		return  !(GetYear() > other.GetYear() ||
			GetMonth() > other.GetMonth() ||
			GetDay() > other.GetDay() ||
			GetHour() > other.GetHour() ||
			GetMinute() > other.GetMinute() ||
			GetSecond() >= other.GetSecond());
	}

	bool CDate::operator==(const CDate& other)
	{
		return (GetYear() == other.GetYear() &&
			GetMonth() == other.GetMonth() &&
			GetDay() == other.GetDay() &&
			GetHour() == other.GetHour() &&
			GetMinute() == other.GetMinute() &&
			GetSecond() == other.GetSecond());
	}

	bool CDate::operator>(const CDate& other)
	{
		return (!(*this < other) && !(*this == other));
	}
}