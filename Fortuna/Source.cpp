/*
	(c) Citadel Software Inc, 2003, www.citadelsoftware.ca
*/

/*!  \file Source.cpp
*    \brief Base Class for all source events.
*
*
*/


#include "stdafx.h"
#include "Source.h"
#include "Fortuna.h"
#include "Pool.h"
#include "Timer.h"
#include "FortunaUtils.h"

namespace CitadelSoftwareInc {

Source::Source(const std::string& sName, Fortuna* pFortuna, SourceMgr* pOwner, std::vector<Pool*>& Pools, unsigned char ucSourceNum)
	:
	m_pFortuna(pFortuna),
	m_pOwner(pOwner),
	m_bSetThreadPriority(FALSE),
	m_TotalBytesData(0),
	m_NumberOfCycles(0),
	m_sName(sName),
	m_CycleTime(0),
	m_dwCycleStart(0),
	m_iPool(0),
	m_numPools(0),
	m_dwExtraSleep(0),
	m_ucSourceNum(ucSourceNum),
#ifdef FORTUNAMONITOR
	m_cacheSize(32*1024),
	m_bCacheData(false),
#endif
	m_HandleShutdown(0)
{
	const int size = (int)Pools.size();
	m_pools.resize(size);
	std::copy(Pools.begin(), Pools.end(), m_pools.begin());
	m_HandleShutdown = m_pFortuna->GetShutdownEventHandle();
#ifdef FORTUNAMONITOR
#endif
}

Source::~Source()
{
}

// Starts the thread running
bool Source::StartThread()
{
	m_pThread = new CMclThread(this);

	return true;
}

// Overloaded from CMclThreadHandler - this is the thread entry point
unsigned Source::ThreadHandlerProc(void)
{
	// give each thread a different seed for rand()
	unsigned int seed = GetCurrentThreadId();
	seed += (unsigned int)this;
	seed += (unsigned int)time(NULL);
	{
		Timer hptimer;
		hptimer.Curr();
		seed += hptimer.m_currTime.LowPart;
	}
	srand(seed);

	LARGE_INTEGER start;
	LARGE_INTEGER end;
	unsigned int diff=0;
	unsigned char ucdiff=0;
	start.HighPart = 0;
	start.LowPart  = 0;
	end.HighPart = 0;
	end.LowPart = 0;
	bool bStatus = true;
	int i=0;

	m_dwCycleStart = GetTickCount();	// ms since system started up

	HANDLE hShutdownEvent = m_pFortuna->GetShutdownEventHandle();

	m_numPools = m_pools.size();

	unsigned long ul = (unsigned long)this;
	ul += (unsigned long)m_pFortuna;

	ul += (unsigned long)GetCurrentThreadId();
	DWORD dwPid = GetCurrentProcessId();
	ul += (unsigned long)dwPid;
	DWORD dwSleep = 0;

	Sleep(10);

	// Set the thread priority
	// SetThreadPriority returns 0 if it fails, use GetLastError to get the specific error
	HANDLE hThreadId = GetThreadHandle();
	m_bSetThreadPriority = SetThreadPriority(hThreadId, THREAD_PRIORITY_BELOW_NORMAL);

	ul += (unsigned long)hThreadId;

	std::vector<unsigned char> vChaoticData;

	// determine which pool to start distributing to - different for each source object because of the
	// this and GetThreadId used to determine ul

	m_iPool = ul % m_numPools;

	Timer hpTimer;

	size_t chaoticDataSize = 0;

	DWORD dwResult = 0;
	unsigned char byte = 0;
	while(1)
	{
		hpTimer.Start();
		dwResult = WaitForSingleObject(hShutdownEvent, 1);
		hpTimer.Stop();
		hpTimer.AddElapsedTime(vChaoticData);

#ifdef FORTUNAMONITOR
		{
			if (ZeroData())
			{
				size_t size = vChaoticData.size();
				std::string sText = BinaryToString(&vChaoticData[0], (int)size);
				SetPeekString(sText);
			}
		}
#endif

		SendDataToPools(vChaoticData);

		if (dwResult == WAIT_OBJECT_0) 
			break;

		// The timer class doesn't add any data, so this step can be skipped
		if (!ZeroData())
		{
			hpTimer.Start();
			bStatus = GetChaoticData(vChaoticData);
			hpTimer.Stop();
			hpTimer.AddElapsedTime(vChaoticData);

			SendDataToPools(vChaoticData);
		}
		Sleep(m_dwExtraSleep + 100);
	}	// while 1


	return 0;
}



bool Source::SendDataToPools(std::vector<unsigned char>& vData)
{
#ifdef FORTUNAMONITOR
	if (m_bCacheData)
		AddBytesToCache(vData);
#endif

	size_t chaoticDataSize = vData.size();

	if (chaoticDataSize > 32)
	{
		HashVector(vData);
		chaoticDataSize = vData.size();
	}

	m_TotalBytesData += (unsigned int)chaoticDataSize;

	// add the source number and the size of the data
	size_t vsize = vData.size();
	vData.push_back(m_ucSourceNum);
	const unsigned char* pData = (const unsigned char*)&vsize;
	AddBinaryData3(vData,pData,sizeof(size_t));

	if (m_iPool >= m_numPools)
		m_iPool = 0;

	Pool* pPool = m_pools[m_iPool++];
	pPool->AddBytes(&vData[0], (unsigned int)vData.size());

	vData.clear();

	return true;
}

//! Return true if shutting down
bool Source::ShuttingDown()
{
	DWORD dwResult = WaitForSingleObject(m_HandleShutdown, 0);
	if (dwResult == WAIT_OBJECT_0)
		return true;

	return false;
}

#ifdef FORTUNAMONITOR
void Source::SetPeekString(std::string& sText)
{
	m_PeekCritSec.Enter();

	m_sPeekString = sText;

	m_PeekCritSec.Leave();
}

std::string Source::GetPeekString()
{
	std::string str;
	m_PeekCritSec.Enter();

	str = m_sPeekString;

	m_PeekCritSec.Leave();

	return str;
}

void Source::AddBytesToCache(const vecuc& vData)			// add the vector of data to the cache
{
	m_cacheCritSec.Enter();

	if (vData.size() < m_cacheSize)
	{
		const size_t vsize = vData.size();
		for(size_t i=0; i<vsize; ++i)
		{
			m_cache.push_back(vData[i]);
		}

		const size_t cacheSize = m_cache.size();
		if (cacheSize > m_cacheSize)
		{
			const size_t count = cacheSize - m_cacheSize;
			for(size_t i=0; i<count; ++i)
			{
				m_cache.pop_front();
			}
		}
	}
	else
	{
		m_cache.clear();
		const size_t start = vData.size() - m_cacheSize;
		const size_t end   = vData.size();
		for (size_t i=start; i<end; ++i)
		{
			m_cache.push_back(vData[i]);
		}
	}

	m_cacheCritSec.Leave();
}

bool Source::GetCacheData(vecuc& vData)
{
	m_cacheCritSec.Enter();

	size_t size = m_cache.size();
	vData.clear();

	if (size)
	{
		vData.resize(size);
		for (size_t i=0; i<size; ++i)
			vData[i] = m_cache[i];
	}

	m_cacheCritSec.Leave();

	return true;
}

#endif

}	// end namespace CitadelSoftwareInc