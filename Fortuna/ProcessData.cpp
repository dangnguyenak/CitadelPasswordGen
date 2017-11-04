/*
	(c) Citadel Software Inc, 2003, www.citadelsoftware.ca
*/

/*!  \file ProcessData.cpp
*    \brief Caches data for a process so that the bytes of change can be determined.
*
*
*/



#include "stdafx.h"
#include "ProcessData.h"
#include "Fortuna.h"

namespace CitadelSoftwareInc {

	ProcessData::ProcessData(DWORD dwPid)
		:
		m_dwPid(dwPid),
		m_pFileName(NULL),
		m_workingsetsize(0),
		m_usageCounter(0)
	{
		memset((void*)(&m_iocounters),		0, sizeof(IO_COUNTERS));
		memset((void*)(&m_ftKernelTime),	0, sizeof(FILETIME));
		memset((void*)(&m_ftUserTime),		0, sizeof(FILETIME));
		memset((void*)(&m_memorycounters),  0, sizeof(PROCESS_MEMORY_COUNTERS));
		memset((void*)(&m_perfinfo),        0, sizeof(PERFORMACE_INFORMATION));

		HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, dwPid);

		if (hProcess)
		{
			const DWORD BUFFSIZE=1024;
			char buffer[BUFFSIZE+1];
			// TODO WINDOWSXPONLY
			DWORD dwLen = 0; // GetProcessImageFileName(hProcess, buffer, BUFFSIZE);

			if (dwLen)
			{
				m_pFileName = new char[dwLen+1];
				strcpy(m_pFileName, buffer);
			}

			CloseHandle(hProcess);
		}
	}

ProcessData::~ProcessData()
{
	delete [] m_pFileName;
}

}	// end namespace CitadelSoftwareInc