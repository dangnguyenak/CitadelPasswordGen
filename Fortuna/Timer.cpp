/*
	(c) Citadel Software Inc, 2003, www.citadelsoftware.ca
*/

/*!  \file Timer.cpp
*    \brief Simple wrapper around QueryPerformanceFrequency and QueryPerformanceCounter
*
*
*/


#include "stdafx.h"
#include "Timer.h"


namespace CitadelSoftwareInc {

Timer::Timer()
{
	m_bTimer = QueryPerformanceFrequency(&m_frequency) ? true : false;

	if (!m_bTimer)
	{
		m_frequency.HighPart = 0;
		m_frequency.LowPart = 1000;		// just use GetTickCount()
	}

	m_startTime.QuadPart = 0;
	m_stopTime.QuadPart = 0;
	m_currTime.QuadPart = 0;
}


Timer::~Timer()
{
}

void Timer::Start()
{
	m_stopTime.QuadPart = 0;
	m_currTime.QuadPart = 0;
	if (m_bTimer)
		QueryPerformanceCounter(&m_startTime);
	else
		m_startTime.LowPart = GetTickCount();
}

void Timer::Stop()
{
	if (m_bTimer)
		QueryPerformanceCounter(&m_stopTime);
	else
		m_stopTime.LowPart = GetTickCount();
}

void Timer::Curr()
{
	if (m_bTimer)
		QueryPerformanceCounter(&m_currTime);
	else
		m_currTime.LowPart = GetTickCount();
}


int Timer::AddElapsedTime(vecuc& vData)
{
	int count=0;
	const unsigned char* pStart = (const unsigned char*)&m_startTime;
	const unsigned char* pStop = (const unsigned char*)&m_stopTime;

	unsigned char uc1=0, uc2=0;
	const int iSize = sizeof(LARGE_INTEGER);
	for (int i=0; i<iSize; ++i)
	{
		uc1 = pStart[i];
		uc2 = pStop[i];

		if (uc1 && uc2)
		{
			if( uc1 == uc2 )
			{
				vData.push_back(uc1);
				count += 1;
			}
			else
			{
				vData.push_back(uc1);
				vData.push_back(uc2);
				count += 2;
			}
		}

	}
	return count;
}


int Timer::AddElapsedTimeDifference(vecuc& vData)
{
	int count=0;
	const unsigned char* pStart = (const unsigned char*)&m_startTime;
	const unsigned char* pStop = (const unsigned char*)&m_stopTime;

	unsigned char uc1=0, uc2=0;
	const int iSize = sizeof(LARGE_INTEGER);
	for (int i=0; i<iSize; ++i)
	{
		uc1 = pStart[i];
		uc2 = pStop[i];

		if (uc1 && uc2)
		{
			if( uc1 == uc2 )
			{
				break;
			}
			else
			{
				vData.push_back(uc1);
				vData.push_back(uc2);
				count += 1;
			}
		}

	}
	return count;
}


}	// end namespace CitadelSoftwareInc