/*
	(c) Citadel Software Inc, 2003, www.citadelsoftware.ca
*/

/*!  \file MacineSig.cpp
*    \brief Determines a machine signature based on the current process information, MAC addresses, etc etc
*
*
*/


#include "stdafx.h"
#include "MachineSig.h"
#include "FortunaUtils.h"
#include "Timer.h"
#include "Psapi.h"
#include "ProcessInfo.h"
#include "Sha2.h"
#include "iphlpapi.h"

namespace CitadelSoftwareInc {

		// add in the current process data
void AddProcessData(vecuc& vData)
{
	Timer hpTimer;

	DWORD dwNeeded = 0;
	const int MAXSIZE=1024;
	std::vector<DWORD>dwPids;
	dwPids.resize(MAXSIZE);

	hpTimer.Start();
	BOOL bStatus = EnumProcesses(&dwPids[0], MAXSIZE*sizeof(DWORD), &dwNeeded);
	hpTimer.Stop();
	hpTimer.AddElapsedTime(vData);

	if (!bStatus)
		return;

	DWORD dwNumPids = dwNeeded / sizeof(DWORD);

	DWORD dwPid=0;
	DWORD i=0;
	for (i=0; i<dwNumPids; ++i)
	{
		dwPid = dwPids[i];
		ProcessInfoIoCounter piioc;
		hpTimer.Start();
		piioc.GetData(vData, dwPid, NULL);
		hpTimer.Stop();
		hpTimer.AddElapsedTime(vData);

		// next...
		ProcessInfoTimes pitimes;
		hpTimer.Start();
		pitimes.GetData(vData, dwPid, NULL);
		hpTimer.Stop();
		hpTimer.AddElapsedTime(vData);

		ProcessInfoMemory pim;
		hpTimer.Start();
		pim.GetData(vData, dwPid, NULL);
		hpTimer.Stop();
		hpTimer.AddElapsedTime(vData);

		ProcessInfoPerformance pip;
		hpTimer.Start();
		pip.GetData(vData, dwPid, NULL);
		hpTimer.Stop();
		hpTimer.AddElapsedTime(vData);
	}

	// hash the process data
	vecuc vHash;
	HashVector(vData, vHash);
	EraseVector(vData);

	// copy the hash to vData and then erase vHash
	vData.resize(vHash.size());
	std::copy(vHash.begin(), vHash.end(), vData.begin());
	EraseVector(vHash);
}

void AddComputerName(vecuc& vData)
{
	COMPUTER_NAME_FORMAT names[8] = {ComputerNameNetBIOS, ComputerNameDnsHostname, ComputerNameDnsDomain, ComputerNameDnsFullyQualified,
									 ComputerNamePhysicalNetBIOS, ComputerNamePhysicalDnsHostname, ComputerNamePhysicalDnsDomain,
									 ComputerNamePhysicalDnsFullyQualified};

	const int MAXSIZE=1024;
	char buffer[MAXSIZE+1];
	memset(buffer,0,sizeof(buffer));
	DWORD dwSize = MAXSIZE;

	BOOL bStatus = TRUE;
	for(int i=0; i<8; ++i)
	{
		dwSize = MAXSIZE;
		bStatus = GetComputerNameEx(names[i], buffer, &dwSize);
		if (bStatus)
		{
			AddString(vData, (const unsigned char*)buffer);
		}
	}

}

void AddWindowsVersion(vecuc& vData)
{
	OSVERSIONINFO version;
	memset(&version,0,sizeof(OSVERSIONINFO));
	version.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	BOOL bStatus = GetVersionEx(&version);

	if (bStatus)
	{
		int size = sizeof(OSVERSIONINFO);
		const unsigned char* pData = (unsigned char*)&version;
		AddBinaryData(vData, pData, size);
	}

}

void AddSystemInfo(vecuc& vData)
{
	SYSTEM_INFO info;
	memset(&info,0,sizeof(SYSTEM_INFO));

	GetSystemInfo(&info);

	const unsigned char* pData = (unsigned char*)&info;
	int size = sizeof(SYSTEM_INFO);

	AddBinaryData(vData, pData, size);
}

void AddCurrentHwProfile(vecuc& vData)
{
	HW_PROFILE_INFO info;
	memset(&info, 0, sizeof(HW_PROFILE_INFO));
	BOOL bStatus = GetCurrentHwProfile(&info);

	if(bStatus)
	{
		const unsigned char* pData = (unsigned char*)&info;
		int size = sizeof(HW_PROFILE_INFO);
		AddBinaryData(vData, pData, size);
	}

}

void AddEnvStringData(vecuc& vData)
{
	char* pVoid = GetEnvironmentStrings();

	if (!pVoid)
		return;

	unsigned char uc=0;
	const char* pString = (const char*)pVoid;

	for (; *pString; ++pString)
	{
		while(*pString)
		{
			uc = *pString++;
			vData.push_back(uc);
		}
	}

	BOOL bStatus = FreeEnvironmentStrings(pVoid);
	if (!bStatus)
	{
		assert(0);
	}
}


void AddMacAddresses(vecuc& vData)
{
	IP_ADAPTER_INFO AdapterInfo[16];       // Allocate information 
                                           // for up to 16 NICs
	DWORD dwBufLen = sizeof(AdapterInfo);  // Save memory size of buffer

	DWORD dwStatus = GetAdaptersInfo(      // Call GetAdapterInfo
			AdapterInfo,                   // [out] buffer to receive data
			&dwBufLen);                    // [in] size of receive data buffer
  
	assert(dwStatus == ERROR_SUCCESS);  // Verify return value is 
                                        // valid, no buffer overflow

  PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo; // Contains pointer to

  int size = sizeof(IP_ADAPTER_INFO);
  
  do {
	const unsigned char* pData = (unsigned char*)pAdapterInfo;

	AddBinaryData(vData, pData, size);

    pAdapterInfo = pAdapterInfo->Next;    // Progress through 
                                          // linked list
  }
  while(pAdapterInfo);                    


}

void AddStartupInfo(vecuc& vData)
{
	STARTUPINFO info;
	info.cb = sizeof(STARTUPINFO);

	GetStartupInfo(&info);

	int size = sizeof(info);
	const unsigned char* pData =(unsigned char*)&info;
	AddBinaryData(vData, pData, size);
}

void AddGlobalMemoryStatus(vecuc& vData)
{

	MEMORYSTATUSEX status;
	status.dwLength = sizeof(MEMORYSTATUSEX);
	BOOL bStatus = GlobalMemoryStatusEx(&status);
	if (bStatus)
	{
		int size = sizeof(MEMORYSTATUSEX);
		const unsigned char* pData = (unsigned char*)&status;
		AddBinaryData(vData, pData, size);
	}
}


void AddWindowingInfo(vecuc& vData)
{
	AddToVector2(vData, GetActiveWindow());
	AddToVector2(vData, GetCapture());
	AddToVector2(vData, GetClipboardOwner());
	AddToVector2(vData, GetClipboardViewer());
	AddToVector2(vData, GetCurrentProcess());
	AddToVector2(vData, GetCurrentProcessId());
	AddToVector2(vData, GetCurrentThread());
	AddToVector2(vData, GetCurrentProcessId());
	AddToVector2(vData, GetDesktopWindow());
	AddToVector2(vData, GetFocus());
	AddToVector2(vData, GetInputState());
	AddToVector2(vData, GetMessagePos());
	AddToVector2(vData, GetMessageTime());
	AddToVector2(vData, GetOpenClipboardWindow());
	AddToVector2(vData, GetProcessHeap());
	AddToVector2(vData, GetProcessWindowStation());
	AddToVector2(vData, GetQueueStatus(QS_ALLEVENTS));
	AddToVector2(vData, GetTickCount());
}


	// Get a hash of some quantities that will provide a machine signature.
	// This is used after the seed file is processed to add something diffent to the pool hashes stored in the 
	// machine file to avoid provlems with reusing the same seed file.
	void GetMachineSignature(vecuc& vData)
	{
		{
			unsigned int count = 0;
			Timer hpTimer;
			for (int i=0; i<100; ++i)
			{
				hpTimer.Start();
				Sleep(0);
				hpTimer.Stop();
				count += hpTimer.AddElapsedTimeDifference(vData);
			}
			assert(count);
		}


		Timer hpTimer;
		hpTimer.Start();

		vData.clear();
		vData.reserve(10240);

		// add in the current time data
		AddTimeData(vData);

		// add in the frequency of the high performance timer
		LARGE_INTEGER liTemp;
		BOOL bStatus = QueryPerformanceFrequency(&liTemp);
		if (bStatus)
		{
			AddToVector2(vData, liTemp);
		}

		// add in the user name
		char buffer[1025];
		memset(buffer,0,sizeof(buffer));
		DWORD dwSize = 1024;
		bStatus = GetUserName(buffer, &dwSize);
		if (bStatus)
		{
			AddString(vData, (const unsigned char*)buffer);
		}

		hpTimer.Start();
		AddProcessData(vData);
		hpTimer.Stop();
		hpTimer.AddElapsedTime(vData);

		hpTimer.Start();
		AddComputerName(vData);
		hpTimer.Stop();
		hpTimer.AddElapsedTime(vData);

		hpTimer.Start();
		AddWindowsVersion(vData);
		hpTimer.Stop();
		hpTimer.AddElapsedTime(vData);

		hpTimer.Start();
		AddSystemInfo(vData);
		hpTimer.Stop();
		hpTimer.AddElapsedTime(vData);

		hpTimer.Start();
		AddCurrentHwProfile(vData);
		hpTimer.Stop();
		hpTimer.AddElapsedTime(vData);

		hpTimer.Start();
		AddEnvStringData(vData);
		hpTimer.Stop();
		hpTimer.AddElapsedTime(vData);

		hpTimer.Start();
		AddMacAddresses(vData);
		hpTimer.Stop();
		hpTimer.AddElapsedTime(vData);

		hpTimer.Start();
		AddStartupInfo(vData);
		hpTimer.Stop();
		hpTimer.AddElapsedTime(vData);

		hpTimer.Start();
		AddGlobalMemoryStatus(vData);
		hpTimer.Stop();
		hpTimer.AddElapsedTime(vData);

		hpTimer.Start();
		AddWindowingInfo(vData);
		hpTimer.Stop();
		hpTimer.AddElapsedTime(vData);

		vecuc vHash;
		HashVector(vData, vHash);
		EraseVector(vData);
		AddToVector(vData,vHash);
		EraseVector(vHash);
	}

}	// end namespace CitadelSoftwareInc