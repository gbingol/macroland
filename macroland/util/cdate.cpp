#include "cdate.h"

#include <ctime>
#include <stdlib.h>


namespace util
{
	CDate::CDate()
	{
		time_t Time = std::time(0);
		std::tm* TimeInfo = std::localtime(&Time);

		m_Year = 1900 + TimeInfo->tm_year;
		m_Month = TimeInfo->tm_mon + 1;
		m_Day = TimeInfo->tm_mday;
		m_Hour = TimeInfo->tm_hour;
		m_Minute = TimeInfo->tm_min;
		m_Second = TimeInfo->tm_sec;
	}

	std::string CDate::year() const {
		return std::to_string(m_Year);
	}

	std::string CDate::month() const
	{
		return (m_Month < 10) ?
			std::string("0") + std::to_string(m_Month) :
			std::to_string(m_Month);
	}

	std::string CDate::day() const
	{
		return (m_Day < 10) ?
			std::string("0") + std::to_string(m_Day) :
			std::to_string(m_Day);
	}

	std::string CDate::hour() const
	{
		return (m_Hour < 10) ?
			std::string("0") + std::to_string(m_Hour) :
			std::to_string(m_Hour);
	}

	std::string CDate::minute() const
	{
		return (m_Minute < 10) ?
			std::string("0") + std::to_string(m_Minute) :
			std::to_string(m_Minute);
	}

	std::string CDate::second() const
	{
		return (m_Second < 10) ?
			std::string("0") + std::to_string(m_Second) :
			std::to_string(m_Second);
	}


	bool CDate::operator<(const CDate& other) const
	{
		return  !(
			m_Year > other.m_Year ||
			m_Month > other.m_Month ||
			m_Day > other.m_Day ||
			m_Hour > other.m_Hour ||
			m_Minute > other.m_Minute ||
			m_Second >= other.m_Second);
	}

	bool CDate::operator==(const CDate& other) const
	{
		return (
			m_Year == other.m_Year &&
			m_Month == other.m_Month &&
			m_Day == other.m_Day &&
			m_Hour == other.m_Hour &&
			m_Minute == other.m_Minute &&
			m_Second == other.m_Second);
	}

	bool CDate::operator>(const CDate& other) const
	{
		return (
			!(*this < other) && 
			!(*this == other));
	}
}