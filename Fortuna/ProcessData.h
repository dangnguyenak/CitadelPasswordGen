/*
	(c) Citadel Software Inc, 2003, www.citadelsoftware.ca
*/

#pragma once

#include "psapi.h"

namespace CitadelSoftwareInc {

	/*! \brief Contains a snapshot of the Process data for a single process. 
	*          Used to determine the change in process data when the next snapshot is taken.
	*
	*/

class ProcessData
{
	public:
		ProcessData(DWORD dwPid);
		virtual ~ProcessData();

		IO_COUNTERS* GetIoCounters() { return &m_iocounters; }
		void ResetIoCounters(const IO_COUNTERS& ioc) { m_iocounters = ioc; }

		const char* GetName() const {return m_pFileName; }

		FILETIME m_ftKernelTime;
		FILETIME m_ftUserTime;
		PROCESS_MEMORY_COUNTERS m_memorycounters;
		PERFORMACE_INFORMATION  m_perfinfo;

		std::vector<unsigned char> m_workingset;		// data from QueryWorkingSet

		size_t m_workingsetsize;						// from GetProcessMemoryInfo, PROCESS_MEMORY_COUNTERS

		size_t m_usageCounter;							// number of times this object has been referenced

		void UseObject() { ++m_usageCounter; }

	private:
		DWORD m_dwPid;

		IO_COUNTERS m_iocounters;

		char* m_pFileName;

};


}	// end namespace CitadelSoftwareInc