/*
	(c) Citadel Software Inc, 2003, www.citadelsoftware.ca
*/
/*!  \file SourceProcess.cpp
*    \brief Source events from process monitoring.
*
*
*/


#include "stdafx.h"
#include "SourceProcess.h"
#include "Fortuna.h"

#include "ProcessData.h"
#include "ProcessInfo.h"
#include "FortunaUtils.h"

#include "Psapi.h"

namespace CitadelSoftwareInc {

SourceProcess::SourceProcess(const std::string& sName, 
							 Fortuna* pFortuna, 
							 SourceMgr* pOwner, 
							 std::vector<Pool*>& Pools,
							 unsigned char ucSourceNum)
	:
	Source(sName, pFortuna, pOwner, Pools, ucSourceNum),
	m_dwError(0),
	m_pdwPids(0),
	m_iPidSize(1024),
	m_numPids(0),
	m_nextInfo(0),
	m_dwCurrPid(0)
{
	m_pdwPids = new DWORD[m_iPidSize];
	memset(m_pdwPids,0,sizeof(DWORD)*m_iPidSize);

	m_vProcessInfo.push_back(new ProcessInfoIoCounter());
	m_vProcessInfo.push_back(new ProcessInfoTimes());
	m_vProcessInfo.push_back(new ProcessInfoMemory());
	m_vProcessInfo.push_back(new ProcessInfoPerformance());
	m_vProcessInfo.push_back(new ProcessInfoQueryWorkingSet());

}

SourceProcess::~SourceProcess()
{
	size_t size = m_vProcessInfo.size();
	for (size_t i=0; i<size; ++i)
	{
		ProcessInfoBase* p = m_vProcessInfo[i];
		delete p;
		m_vProcessInfo[i] = NULL;
	}
}


// derived classes must implement this to return their data
bool SourceProcess::GetChaoticData(vecuc& vData)
{
	// do nothing; the base class adds in the last byte from the difference in the high resolution timer
	// around the WaitForSingleObject(hShutdownEvent) call

	if (m_numPids == 0)
	{
		// get the pids again
		DWORD dwNeeded = 0;
		BOOL bStatus = EnumProcesses(m_pdwPids, m_iPidSize*sizeof(DWORD), &dwNeeded);
		m_numPids = dwNeeded / sizeof(DWORD);
		m_nextPid = 1;
		m_dwCurrPid = m_pdwPids[0];

		m_nextInfo = 0;
	}
	else if (m_nextPid >= m_numPids)
	{
		// all done, time to get new pids
		m_numPids = 0;

		++m_NumberOfCycles;		// protected member from class Source

		// update the cycle time
		DWORD dwCycleEnd = GetTickCount();
		m_CycleTime = (dwCycleEnd - m_dwCycleStart)/1000;		// cycle time in seconds
		m_dwCycleStart = dwCycleEnd;
	}
	else if (m_nextInfo == 0)
		m_dwCurrPid = m_pdwPids[m_nextPid++];

	if (m_numPids)
	{
		AddProcessInfo(m_dwCurrPid, vData);
	}

#pragma warning (disable: 4311)
	unsigned int isleep = (unsigned int)this;
	isleep = (isleep % 256) + 10;

	// sleep as a function of the data collected
	size_t dataSize = vData.size();
	if (dataSize > 100)
		dataSize = 100;

	unsigned int sum=0;
	for (size_t i=0; i<dataSize; ++i)
	{
		sum += vData[i];
	}

	isleep += sum % 256;

	Sleep(isleep);

	return true;
}

bool SourceProcess::AddProcessInfo(DWORD pid, vecuc& vData)
{
	size_t size = m_vProcessInfo.size();

	if (m_nextInfo >= size)
	{
		m_nextInfo = 0;
		return true;
	}

	ProcessInfoBase *p = m_vProcessInfo[m_nextInfo++];

	ProcessData *pProcessData = GetProcessData(pid);

	if (pProcessData->m_usageCounter == 0)
	{
		const char* pFileName = pProcessData->GetName();
		if (pFileName)
			AddStringData(vData, (const unsigned char*)pFileName,0);

		// a new process was just added, add in time data
		AddTimeData(vData);
		AddModuleData(pid, vData);
	}

	bool bStatus = p->GetData(vData, m_dwCurrPid, pProcessData);

#ifdef FORTUNAMONITOR
	{
		size_t size = vData.size();
		char buffer[256] = {'\0'};
		sprintf(buffer,"pid=%d size=%u bytes=", m_dwCurrPid, size);
		std::string sBytes = BinaryToString(&vData[0], (int)size);
		std::string sPeek = std::string(buffer) + sBytes;
		SetPeekString(sPeek);
	}
#endif


	if (bStatus == false)
	{
		// unable to open the process handle, go to the next process
		m_nextInfo = 0;
	}
	
	return true;
}

bool SourceProcess::AddModuleData(DWORD dwPid, vecuc& vData)
{
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ , FALSE, dwPid);

	if (hProcess == NULL)	// not all processes grant this access
		return false;

	const int BUFFSIZE=1024;
	HMODULE hModules[BUFFSIZE+1];
	DWORD cb = BUFFSIZE*sizeof(HMODULE);
	DWORD cbNeeded = 0;

	BOOL bStatus = EnumProcessModules(hProcess, hModules, cb, &cbNeeded);
	
	if (bStatus && cbNeeded < cb)
	{
		DWORD dwNumModules = cbNeeded / sizeof(HMODULE);
		for (DWORD i=0; i<dwNumModules; ++i)
		{
			HMODULE hModule = hModules[i];

			AddModuleData2(hProcess, hModule, vData);
		}
	}

	CloseHandle(hProcess);

	return true;
}

// adds in the data for a specific module
bool SourceProcess::AddModuleData2(HANDLE hProcess, HMODULE hModule, vecuc& vData)
{
	AddBinaryData(vData, (const unsigned char*)&hModule, sizeof(HMODULE));

	const int BUFFSIZE=1024;
	char buffer[BUFFSIZE+1];
	buffer[BUFFSIZE] = '\0';
	DWORD nSize = BUFFSIZE*sizeof(char);

	DWORD dwSize = GetModuleBaseName(hProcess, hModule, buffer, nSize);

	if(dwSize)
		AddStringData(vData, (const unsigned char*)buffer, dwSize);		

	dwSize = GetModuleFileNameEx(hProcess, hModule, buffer, nSize);
	if (dwSize)
		AddStringData(vData, (const unsigned char*)buffer, dwSize);

	MODULEINFO modinfo;
	DWORD cb = sizeof(MODULEINFO);
	BOOL bStatus = GetModuleInformation(hProcess, hModule, &modinfo, cb);

	if (bStatus)
	{
		AddBinaryData(vData, (const unsigned char*)&modinfo, sizeof(MODULEINFO));
	}

	return true;
}

// return an existing process data object or create a new one as required
ProcessData* SourceProcess::GetProcessData(DWORD dwPid)
{
	ProcessData* pProcessData = NULL;

	std::map<DWORD,ProcessData*>::iterator iter = m_mapProcessData.find(dwPid);

	if (iter == m_mapProcessData.end())
	{
		pProcessData = new ProcessData(dwPid);
		m_mapProcessData[dwPid] = pProcessData;
	}
	else
	{
		pProcessData = (*iter).second;
		pProcessData->UseObject();			// increments usage counter
	}

	return pProcessData;
}


}	// end namespace CitadelSoftwareInc