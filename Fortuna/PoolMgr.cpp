/*
	(c) Citadel Software Inc, 2003, www.citadelsoftware.ca
*/

/*!  \file PoolMgr.cpp
*    \brief Creates and gives access to the entropy pools.
*
*
*/


#include "stdafx.h"
#include "PoolMgr.h"
#include "Fortuna.h"
#include "Pool.h"
#include "Sha2.h"

namespace CitadelSoftwareInc {

PoolMgr::PoolMgr(Fortuna* pOwner)
	:
	m_pOwner(pOwner),
	m_pThread(NULL),
	m_dwLastReseedTime(0),
	m_reseedcount(0)
{
	m_Pools.resize(eNumberOfPools);
	for (int i=0; i<eNumberOfPools; ++i)
	{
		Pool *p = new Pool(m_pOwner, this, i);
		m_Pools[i] = p;
	}
}

PoolMgr::~PoolMgr()
{
	for (int i=0; i<eNumberOfPools; ++i)
	{
		Pool *p = m_Pools[i];
		delete p;
		m_Pools[i] = NULL;
	}

	if (m_pThread)
	{
		delete m_pThread;
		m_pThread = NULL;
	}
}


// Starts the thread running
bool PoolMgr::StartThread()
{
	m_pThread = new CMclThread(this);

	return true;
}


// Overloaded from CMclThreadHandler - this is the thread entry point
unsigned PoolMgr::ThreadHandlerProc(void)
{
	// give each thread a different seed for rand()
	unsigned int seed = GetCurrentThreadId();
	seed += (unsigned int)this;
	seed += (unsigned int)time(NULL);
	srand(seed);

	for (int i=0; i<eNumberOfPools; ++i)
	{
		Pool *p = m_Pools[i];
		p->StartThread();
	}

	HANDLE hShutdownEvent = m_pOwner->GetShutdownEventHandle();
	DWORD dwResult = WaitForSingleObject(hShutdownEvent, INFINITE);

	if (dwResult == WAIT_FAILED)
	{
		m_sStatus += "Wait for ShutdownEventHandle Failed";
	}

	std::vector<HANDLE> vPoolHandles(eNumberOfPools);
	for (i=0; i<eNumberOfPools; ++i)
	{
		vPoolHandles[i] = m_Pools[i]->GetThreadHandle();
	}

	DWORD nCount = eNumberOfPools;
	dwResult = WaitForMultipleObjects(nCount, &vPoolHandles[0], TRUE, INFINITE);

	if (dwResult == WAIT_OBJECT_0)
	{
		;	// do nothing
	}
	else if (dwResult == WAIT_FAILED)
	{
		m_sStatus += "WaitForMultipleObjects on all pool threads failed";
		assert(0);
	}


	return 0;
}

// give out a copy of pointers to all of the pools
bool PoolMgr::GetPools(std::vector<Pool*>& Pools)
{
	Pools.resize(eNumberOfPools);
	std::copy(m_Pools.begin(), m_Pools.end(), Pools.begin());

	return true;
}


// return NULL when num exceeds actual number of Pools
Pool *PoolMgr::GetPool(const int num)
{
	Pool *pPool = NULL;

	if (num < 0)
		return pPool;

	if (num >= (int)m_Pools.size())
		return pPool;

	pPool = m_Pools[num];

	return pPool;
}


//! Returns true if the reseed was performed
//! Reseeds are only performed when there is enough entropy available in Pool p0 and
//! the last reseed was over 100 ms ago
bool PoolMgr::Reseed(std::vector<unsigned char>& seed, bool& bShuttingDown)
{
	int i=0; 
	DWORD dwTime = GetTickCount();

	if (dwTime - m_dwLastReseedTime < 100)		// don't allow a reseed within 100ms of the previous reseed
		return false;

	Pool* pool = m_Pools[0];
	unsigned int totalBytes = pool->GetTotalBytes();

	if (totalBytes < 128)
		return false;

	m_dwLastReseedTime = dwTime;

	int seedSizeAlloc = 1024;
	int seedSize      = 0;
	unsigned char *pSeeds = new unsigned char[seedSizeAlloc];
	for (i=0; i<seedSizeAlloc; ++i)
		pSeeds[i] = (unsigned char)(rand() % 256);		// fill in with random looking data

	bool bStatus = false;
	bShuttingDown=false;

	++m_reseedcount;

	unsigned int two_i = 1;		// 2^i
	std::vector<unsigned char> poolState;		// includes the pool hash and the current pool state
	for (i=0; i<eNumberOfPools; ++i)
	{
		if (i == 0 || m_reseedcount % two_i == 0)
		{
			std::vector<unsigned char> tempState;
			pool = m_Pools[i];
			bStatus = pool->ExtractAndErase(tempState, bShuttingDown);		// extract both the hash and the current pool state
			if (!bStatus || bShuttingDown)
			{
				delete [] pSeeds;
				return false;
			}

			int tempSize = (int)tempState.size();
			if (seedSize + tempSize < seedSizeAlloc)
			{
				std::copy(tempState.begin(), tempState.end(), pSeeds+seedSize);
				seedSize += tempSize;
			}
			else
			{
				int seedSizeAllocNew = seedSizeAlloc + tempSize + 64*1024;
				unsigned char *pSeedsNew = new unsigned char[seedSizeAllocNew];
				std::copy(pSeeds, pSeeds+seedSize, pSeedsNew);
				std::copy(tempState.begin(), tempState.end(), pSeedsNew+seedSize);

				// put random data in the end of the new vector (past the existing data)
				for (int i=seedSize + tempSize; i<seedSizeAllocNew; ++i)
					pSeedsNew[i] = (unsigned char)(rand() % 256);

				// overwrite the previous seed with random looking data
				for (int i=0; i<seedSize; ++i)
					pSeeds[i] = (unsigned char)(rand() % 256);

				seedSizeAlloc = seedSizeAllocNew;
				seedSize      += tempSize;
				delete [] pSeeds;
				pSeeds = pSeedsNew;
			}
		}
		else
			break;				// astute readers will note...

		two_i *= 2;				// next value of 2^i
	}

	// all of the pool state has been accumulated into pSeeds and is of data size seedSize and allocated seedSizeAlloc

	// now hash the seeds
	sha256_ctx ctx[1];
    sha256_begin(ctx);

	sha256_hash(pSeeds, seedSize, ctx);

	for (i=0; i<seedSize; ++i)
		pSeeds[i] = (unsigned char)(rand() % 256);

	delete [] pSeeds;

	unsigned char hval[32];
	sha256_end(hval, ctx);

	seed.resize(32);
	std::copy(&hval[0], &hval[0]+32, seed.begin());

	for (i=0; i<32; ++i)
		hval[i] = (unsigned char)(rand() % 256);

	return true;
}


}	// end namespace CitadelSoftwareInc