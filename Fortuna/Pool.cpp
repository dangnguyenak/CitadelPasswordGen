/*
	(c) Citadel Software Inc, 2003, www.citadelsoftware.ca
*/

/*!  \file Pool.cpp
*    \brief Threadsafe pool object accumulates events from sources and compresses data using SHA-256 hash when buffer gets large enough
*
*
*/


#include "stdafx.h"
#include "Pool.h"
#include "Fortuna.h"
#include "FortunaUtils.h"
#include "Sha2.h"
#include "Timer.h"

namespace CitadelSoftwareInc {

Pool::Pool(Fortuna* pFortuna, PoolMgr* pOwner, int iPoolNum)
	:
	m_pFortuna(pFortuna),
	m_pOwner(pOwner),
	m_pThread(NULL),
	m_PoolMutex(),
	m_CompactPoolDataSize(1024),
	m_hPoolMutex(0),
	m_CompactPoolEvent(TRUE),  // manual reset event
	m_hCompactPoolEvent(0),
	m_TotalBytes(0),
	m_hShutdownEvent(0),
	m_iPoolNum(iPoolNum),
	m_CompactPoolCount(0)
{
	for (int i=0; i<32; ++i)
		m_redherring[i] = 0xcd;
}

Pool::~Pool()
{
	if (m_pThread)
	{
		delete m_pThread;
		m_pThread = NULL;
	}

}

// Starts the thread running
bool Pool::StartThread()
{
	m_hCompactPoolEvent = m_CompactPoolEvent.GetHandle();
	m_hPoolMutex = m_PoolMutex.GetHandle();

	m_pThread = new CMclThread(this);

	return true;
}

// Overloaded from CMclThreadHandler - this is the thread entry point
unsigned Pool::ThreadHandlerProc(void)
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

	m_hShutdownEvent = m_pFortuna->GetShutdownEventHandle();
	
	HANDLE harray[2] = {m_hShutdownEvent, m_hCompactPoolEvent};

	while(1)
	{
		DWORD dwResult = WaitForMultipleObjects(2, harray, FALSE, INFINITE);
		if (dwResult == WAIT_OBJECT_0)
		{
			// shutdown
//			printf("Pool %d shutting down\n", m_iPoolNum);
			break;
		}
		else if (dwResult == WAIT_OBJECT_0 + 1)
		{
			CompactPool();
		}
	}

	return 0;
}

bool Pool::CompactPool()
{
	// first aquire the mutex that protects the pool
	// copy the data from the pool
	vecuc poolData;
	DWORD dwResult = WaitForSingleObject(m_hPoolMutex, INFINITE);

	const unsigned int modnum = 255;
	m_pool.ExtractAndErase(poolData, modnum);

	// now compact the pool
	sha256_ctx ctx[1];
    sha256_begin(ctx);

	vecuc hashData;
	m_hashPool.ExtractAndErase(hashData, modnum);

	// add in the existing hash data
	if (!hashData.empty())
	{
		unsigned long size = (unsigned long)hashData.size();
		sha256_hash(&hashData[0], size, ctx);
	}

	// add in the new raw data
	if (!poolData.empty())
	{
		unsigned long size = (unsigned long)poolData.size();
		sha256_hash(&poolData[0], size, ctx);
	}

	unsigned char hval[32];
	sha256_end(hval, ctx);

	int i=0;
	for (i=0; i<32; ++i)
	{
		m_hashPool.Add(hval[i]);  // this is the real hashed pool data
		hval[i] = rand() % 256;
	}

	EraseHash(ctx);

	// update the red herring data each cycle so an attacker will see something changing at this memory location
	for (i=0; i<32; ++i)
		m_redherring[i] = rand() % 256;

	++m_CompactPoolCount;

	// release the pool mutex
	// having two mutexes to protect the raw data pool and the hashed data pool didn't seem to offer many benefits
	// because the reseed operation requires access to both pools simultaneously
	m_PoolMutex.Release();

	m_CompactPoolEvent.Reset();

	return true;
}

// add a byte to the pool
bool Pool::AddByte(const unsigned char byte)
{
	DWORD dwResult = WaitForSingleObject(m_hPoolMutex, INFINITE);

	unsigned int size = m_pool.Add(byte);
	++m_TotalBytes;

	m_PoolMutex.Release();

	// when the pool grows beyond a certain size, compact the pool 
	// using the Pool's thread to do that actual work
	if (size > m_CompactPoolDataSize)
	{
		m_CompactPoolEvent.Set();
	}

	return true;
}

// add an array of bytes to the pool
bool Pool::AddBytes(const unsigned char* pData, unsigned int size)
{
	DWORD dwResult = WaitForSingleObject(m_hPoolMutex, INFINITE);

	for (unsigned int i=0; i<size; ++i)
	{
		m_pool.Add(pData[i]);
	}

	unsigned int poolSize = m_pool.Size();

	m_TotalBytes += size;

	m_PoolMutex.Release();

	// when the pool grows beyond a certain size, compact the pool 
	// using the Pool's thread to do that actual work
	if (poolSize > m_CompactPoolDataSize)
	{
		m_CompactPoolEvent.Set();
	}

	return true;
}


// Extract the pools and erase the pool data.
// This is used when reseeding the generator.
bool Pool::ExtractAndErase(std::vector<unsigned char>& vBytes, bool& bShuttingDown)
{
	HANDLE hVec[2] = {m_hShutdownEvent, m_hPoolMutex };

	DWORD dwResult = WaitForMultipleObjects(2, hVec, FALSE, INFINITE);

	if (dwResult == WAIT_OBJECT_0)
	{
		bShuttingDown = true;
		return false;		// shutting down
	}
	else if(dwResult == WAIT_OBJECT_0 + 1)
	{
		bShuttingDown = false;		// pool mutex aquired
	}

	// reserve space for the pools 
	vBytes.resize(m_pool.Size() + m_hashPool.Size());

	std::vector<unsigned char> bytes;
	m_pool.ExtractAndErase(bytes, (unsigned int)256);

	std::copy(bytes.begin(), bytes.end(), vBytes.begin());

	size_t i=0;
	size_t size = bytes.size();
	for (i=0; i<size; ++i)
		bytes[i] = (unsigned char)(rand() % 256);

	m_hashPool.ExtractAndErase(bytes, (unsigned int)256);

	std::copy(bytes.begin(), bytes.end(), vBytes.begin() + size);

	size = bytes.size();
	for (i=0; i<size; ++i)
		bytes[i] = (unsigned char)(rand() % 256);

	m_TotalBytes = 0;
	m_CompactPoolCount = 0;

	m_PoolMutex.Release();

	return true;
}

// Extract the HashedPoolData
// This is used by the seed file to get the current state to write out to the seed file
bool Pool::GetHashedPoolData(std::vector<unsigned char>& vData)
{
	HANDLE hVec[2] = {m_hShutdownEvent, m_hPoolMutex };

	DWORD dwResult = WaitForMultipleObjects(2, hVec, FALSE, INFINITE);

	if (dwResult == WAIT_OBJECT_0)
	{
		return false;		// shutting down
	}
	else if(dwResult == WAIT_OBJECT_0 + 1)
	{
		;	// pool mutex aquired, do nothing
	}
	else
	{
		assert(0);
	}

	vData.clear();
	m_hashPool.Extract(vData);	// note that the data is not being erased

	m_PoolMutex.Release();

	return true;
}

bool Pool::SetHashPoolState(vecuc& vState)
{
	size_t size = vState.size();
	if (size != 32)
	{
		assert(0);
		return false;
	}

	m_hashPool.Reset();
	for (size_t i=0; i<size; ++i)
		m_hashPool.Add(vState[i]);

	return true;
}


// this method is not thread safe - it is used after reading in the seed file 
// to protect against the same seed file being used twice.
// The machine signature contains a signature that is common to the machine 
// and when adding to the pool some other pool specific data is also added
void Pool::AddMachineSignatureToPool(vecuc& vMachineSig)
{
	vecuc vPool;		// pool specific data
	unsigned int iThis = (unsigned int)this;
	const unsigned char* pData = (unsigned char*)&iThis;
	AddBinaryData(vPool, pData, sizeof(unsigned int));

	// adds in bytes for each of the linked list items
	m_hashPool.AddRandomData(vPool);

	vecuc vHashPool;
	vHashPool.reserve(1024);
	m_hashPool.ExtractAndErase(vHashPool);
	m_hashPool.Reset();

	AddToVector(vHashPool, vPool);
	EraseVector(vPool);

	AddToVector(vHashPool, vMachineSig);
	EraseVector(vMachineSig);

	vecuc vHash;
	HashVector(vHashPool, vHash);
	EraseVector(vHashPool);

	size_t size = vHash.size();
	assert( size == 32);

	for (size_t i=0; i<size; ++i)
		m_hashPool.Add(vHash[i]);

	EraseVector(vHash);
}

#ifdef FORTUNAMONITOR
std::string Pool::GetName() const
{
	char buffer[64] = {'\0'};
	sprintf(buffer,"Pool %d", m_iPoolNum);

	return std::string(buffer);
}
#endif

#ifdef FORTUNAMONITOR
std::string Pool::GetRawPool()
{
	std::string str;

	HANDLE hVec[2] = {m_hShutdownEvent, m_hPoolMutex };
	DWORD dwResult = WaitForMultipleObjects(2, hVec, FALSE, INFINITE);

	if (dwResult == WAIT_OBJECT_0)
		return str;		// shutting down
	else if(dwResult == WAIT_OBJECT_0 + 1)
	{
		;	// pool mutex aquired, do nothing
	}

	vecuc vData;
	m_pool.Extract(vData);

	str = BinaryToString(&vData[0], (int)vData.size());

	m_PoolMutex.Release();

	return str;
}
#endif

#ifdef FORTUNAMONITOR
std::string Pool::GetHashedPool()
{
	std::string str;

	HANDLE hVec[2] = {m_hShutdownEvent, m_hPoolMutex };
	DWORD dwResult = WaitForMultipleObjects(2, hVec, FALSE, INFINITE);

	if (dwResult == WAIT_OBJECT_0)
		return str;		// shutting down
	else if(dwResult == WAIT_OBJECT_0 + 1)
	{
		;	// pool mutex aquired, do nothing
	}

	vecuc vData;
	m_hashPool.Extract(vData);

	str = BinaryToString(&vData[0], (int)vData.size());

	m_PoolMutex.Release();

	return str;
}
#endif

}	// end namespace CitadelSoftwareInc