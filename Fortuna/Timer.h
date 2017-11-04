/*
	(c) Citadel Software Inc, 2003, www.citadelsoftware.ca
*/

// This is a test source to send data to the pools
#pragma once

namespace CitadelSoftwareInc {

	class Timer
	{
	public:
		Timer();

		~Timer();

		void Start();
		void Stop();
		void Curr();		// establishes the current time

		int AddElapsedTime(vecuc& vData);

		// only add those bytes that are different
		int AddElapsedTimeDifference(vecuc& vData);

		unsigned int GetLowByteTimeDiff() { return (unsigned int)((m_stopTime.LowPart-m_startTime.LowPart) & 0xff); }

	public:
		bool m_bTimer;					// true if a high performance timer is present
		LARGE_INTEGER m_frequency;
		LARGE_INTEGER m_startTime;
		LARGE_INTEGER m_stopTime;
		LARGE_INTEGER m_currTime;

	};


}	// end namespace CitadelSoftwareInc