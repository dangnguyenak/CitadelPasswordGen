/*
	(c) Citadel Software Inc, 2003, www.citadelsoftware.ca
*/

/*!  \file ProcessInfo.cpp
*    \brief Extracts data for a process such IO, Memory Usage etc.
*
*
*/


#include "stdafx.h"
#include "ProcessInfo.h"
#include "Fortuna.h"
#include "ProcessData.h"
#include "FortunaUtils.h"
#include "psapi.h"

namespace CitadelSoftwareInc {


///////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////
bool ProcessInfoIoCounter::GetData(vecuc& vData, 
								   DWORD dwPid, 
								   ProcessData* pProcessData)
{
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, dwPid);

	if (hProcess == NULL)
		return false;

	IO_COUNTERS iocounters;

	BOOL bStatus = GetProcessIoCounters(hProcess, &iocounters);

	if (bStatus != FALSE)
	{
		unsigned char uc=0;
		int size = sizeof(IO_COUNTERS);
		const unsigned char* pData = (unsigned char*)(&iocounters);

		IO_COUNTERS*	pOld = pProcessData ? pProcessData->GetIoCounters() : NULL;
		const unsigned char* pOldData = (const unsigned char*)(pOld);

		// add one bit of entropy for each byte of the io_counters that is different
		for (int i=0; i<size; ++i)
		{
			uc = pData[i];
			if (uc)
			{
				vData.push_back(uc);
			}
		}

		if (pProcessData)
			pProcessData->ResetIoCounters(iocounters);
	}

	CloseHandle(hProcess);

	return true;
}


///////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////
bool ProcessInfoTimes::GetData(vecuc& vData, 
							   DWORD dwPid, 
							   ProcessData* pProcessData)
{
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, dwPid);

	if (hProcess == NULL)
		return false;

	FILETIME createTime;
	FILETIME exitTime;
	FILETIME kernelTime;
	FILETIME userTime;

	BOOL bStatus = GetProcessTimes(hProcess, &createTime, &exitTime, &kernelTime, &userTime);

	if (bStatus)
	{
		if (pProcessData)
		{
			AddFileTimeData2(vData, kernelTime, pProcessData->m_ftKernelTime);
			AddFileTimeData2(vData, userTime,	pProcessData->m_ftUserTime);
			pProcessData->m_ftKernelTime = kernelTime;
			pProcessData->m_ftUserTime   = userTime;
		}
		else
		{
			const unsigned char* pData = (unsigned char*)&kernelTime;
			DWORD dwSize = sizeof(FILETIME);
			AddBinaryData(vData, pData, dwSize);
			pData = (unsigned char*)&userTime;
			AddBinaryData(vData, pData, dwSize);
		}
	}

	CloseHandle(hProcess);

	return true;
}

///////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////
bool ProcessInfoMemory::GetData(vecuc& vData, 
							   DWORD dwPid, 
							   ProcessData* pProcessData)
{
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, dwPid);

	if (hProcess == NULL)
		return false;

	PROCESS_MEMORY_COUNTERS memorycounters;
	memset(&memorycounters, 0, sizeof(PROCESS_MEMORY_COUNTERS));
	memorycounters.cb = sizeof(PROCESS_MEMORY_COUNTERS);
	BOOL bStatus = GetProcessMemoryInfo(hProcess, &memorycounters, sizeof(PROCESS_MEMORY_COUNTERS));

	if (bStatus)
	{
		const unsigned char* pData = (unsigned char*)&memorycounters;
		const unsigned char* pOldData = pProcessData ? (unsigned char*)&(pProcessData->m_memorycounters) : NULL;

		if (pProcessData)
		{
			AddBinaryData2(vData, pData, sizeof(PROCESS_MEMORY_COUNTERS), pOldData);		
			pProcessData->m_memorycounters = memorycounters;
			pProcessData->m_workingsetsize = memorycounters.WorkingSetSize;
		}
		else
		{
			DWORD dwSize = sizeof(PROCESS_MEMORY_COUNTERS);
			AddBinaryData3(vData, pData, dwSize);
		}
	}

	CloseHandle(hProcess);

	return true;
}

///////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////
bool ProcessInfoPerformance::GetData(vecuc& vData, 
									   DWORD dwPid, 
									   ProcessData* pProcessData)
{
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, dwPid);

	if (hProcess == NULL)
		return false;

	PERFORMACE_INFORMATION perfinfo;
	memset(&perfinfo, 0, sizeof(PERFORMACE_INFORMATION));
	perfinfo.cb = sizeof(PERFORMACE_INFORMATION);

	//TODO - WINDOWSXPONLY
	BOOL bStatus = FALSE; // GetPerformanceInfo(&perfinfo, sizeof(PERFORMACE_INFORMATION));

	if (bStatus == TRUE)
	{
		const unsigned char* pData = (unsigned char*)&perfinfo;
		DWORD dwSize = sizeof(PROCESS_MEMORY_COUNTERS);

		if (pProcessData)
		{
			const unsigned char* pOldData = (unsigned char*)&(pProcessData->m_perfinfo);
			AddBinaryData2(vData, pData, dwSize, pOldData);		
			pProcessData->m_perfinfo = perfinfo;
		}
		else
		{
			AddBinaryData3(vData, pData, dwSize);
		}
	}

	CloseHandle(hProcess);

	return true;
}

///////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////
bool ProcessInfoQueryWorkingSet::GetData(vecuc& vData, 
									   DWORD dwPid, 
									   ProcessData* pProcessData)
{
	// determine the size of the working set
	size_t prevWorkingSetSize = pProcessData->m_workingsetsize;
	unsigned int BUFFSIZE=1024;

	if (prevWorkingSetSize)
		BUFFSIZE = (unsigned int)prevWorkingSetSize;

	// if the working set size is too large, then do not include it - this can use too many system resources
	if (BUFFSIZE > 1024*1024)
		return true;

	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ , FALSE, dwPid);

	if (hProcess == NULL)
		return false;

	DWORD** ppData = new DWORD*[BUFFSIZE+1];

	const int Dwordsize = sizeof(DWORD);
	const int Dwordsizep = sizeof(DWORD*);

	BOOL bStatus = QueryWorkingSet(hProcess, ppData, BUFFSIZE*sizeof(DWORD*));

	if (bStatus)
	{
#pragma warning (disable:4311)
		DWORD dwSize = (DWORD)ppData[0];
		const int numBytes = dwSize*sizeof(DWORD*);

		DWORD dwPrevSize = (DWORD)pProcessData->m_workingset.size();

		unsigned char* pData = (unsigned char*)(&ppData[1]);

		if (dwSize*sizeof(DWORD*) == dwPrevSize)
		{
			// sizes are the same, only different bytes add entropy
			const unsigned char* pPrevData = &pProcessData->m_workingset[0];
			AddBinaryData2(vData, pData, dwSize*sizeof(DWORD*), pPrevData);
		}
		else
		{
			// sizes are different, all non zero bytes add entropy
			AddBinaryData3(vData, pData, dwSize*sizeof(DWORD*));
		}

		pProcessData->m_workingset.resize(dwSize*sizeof(DWORD*));
		std::copy(pData, pData+numBytes, &pProcessData->m_workingset[0]);

	}

	delete [] ppData;

	CloseHandle(hProcess);

	return true;
}

}	// end namespace CitadelSoftwareInc