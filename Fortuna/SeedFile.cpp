/*
	(c) Citadel Software Inc, 2003, www.citadelsoftware.ca
*/

/*!  \file SeedFile.cpp
*    \brief Stores encrypted seed file data to disk.  Each pool state is persisted, as well as generator info.
*
*
*/


#include "stdafx.h"
#include "SeedFile.h"
#include "Fortuna.h"
#include "Pool.h"
#include "FortunaUtils.h"
#include "Sha2.h"
#include "Generator.h"

namespace CitadelSoftwareInc {

SeedFile::SeedFile(Fortuna* pFortuna, PoolMgr* pPoolMgr, std::vector<Pool*>& Pools)
	:
	m_pFortuna(pFortuna),
	m_pPoolMgr(pPoolMgr),
	m_hShutdownEvent(0),
	CMclThreadHandler(),
	m_bShuttingDown(false)
{
	const size_t size = Pools.size();
	m_Pools.resize(size);
	std::copy(Pools.begin(), Pools.end(), m_Pools.begin());
}

SeedFile::~SeedFile()
{
	if (m_pThread)
	{
		delete m_pThread;
		m_pThread = NULL;
	}
}

// Starts the thread running
bool SeedFile::StartThread()
{
	m_pThread = new CMclThread(this);

	return true;
}

// Overloaded from CMclThreadHandler - this is the thread entry point
unsigned SeedFile::ThreadHandlerProc(void)
{
	// give each thread a different seed for rand()
	InitRand(this);

	m_hShutdownEvent = m_pFortuna->GetShutdownEventHandle();
	
	HANDLE harray[2] = {m_hShutdownEvent};

	DWORD dwSleepCount = 1000 * 60 * 10;

	while(1)
	{
		DWORD dwResult = WaitForMultipleObjects(1, harray, FALSE, dwSleepCount);
		if (dwResult == WAIT_OBJECT_0)
		{
			// shutdown
			m_bShuttingDown = true;
			break;
		}
		else if(dwResult == WAIT_TIMEOUT)
		{
			if (m_bShuttingDown)
				break;
			WriteSeedFile();
		}
	}

	return 0;
}


// Write out a new seed file
bool SeedFile::WriteSeedFile()
{
	bool bRetVal = true;

	std::vector<unsigned char> keyFileState;
	CreateFileState(keyFileState);

	AddHMac(keyFileState);

	std::string sFileName;
	m_pFortuna->GetSeedFilename(sFileName);

	FILE *pFile = fopen(sFileName.c_str(), "wb");
	if (!pFile)
	{
		assert(0);
		return false;
	}

	size_t numBytes = fwrite(&keyFileState[0], 1, keyFileState.size(), pFile);

	if (numBytes != keyFileState.size())
	{
		assert(0);
		bRetVal = false;
	}

	fclose(pFile);
	pFile = NULL;

	return bRetVal;
}

// create the data that goes into the seed file
// this does encryption and creates the binary stream for the seed file
bool SeedFile::CreateFileState(std::vector<unsigned char>& keyFileState)
{
	keyFileState.clear();
	keyFileState.reserve(10240);		// avoid reallocations
	assert(keyFileState.size() == 0);

	// add the version number to the state
	int iVersion = 0x01;
	AddToVector<int>(keyFileState, iVersion);

	// add the CRT encryption mode nonce and counter
	CTRNonceCounter nonceCounter;
	nonceCounter.Randomize();
	nonceCounter.AddToVector(keyFileState);

	vecuc vPlainText;		// holds all the plain text to be encrypted

	Generator *pGenerator = m_pFortuna->GetGenerator();
	pGenerator->GetCurrentKey(vPlainText);

	vecuc vTemp;
	pGenerator->GetCounter(vTemp);
	AddToVector(vPlainText, vTemp);
	EraseVector(vTemp);

	int numPools=0;
	int sizePools=0;
	std::vector<unsigned char> hashData;
	GetPoolData(hashData, numPools, sizePools);

	AddToVector(vPlainText, numPools);
	AddToVector(vPlainText, sizePools);
	AddToVector(vPlainText, hashData);

	EraseVector(hashData);

	std::vector<unsigned char> vKey;
	m_pFortuna->GetSeedFileKey(vKey);

	bool bStatus = EncryptCTRMode(vPlainText, vKey, nonceCounter);  // plaintext is encrypted on output

	EraseVector(vKey);

	// now add the unencrypted header to the encrypted data
	const size_t sizePlain = vPlainText.size();
	for (size_t i=0; i<sizePlain; ++i)
		keyFileState.push_back(vPlainText[i]);

	EraseVector(vPlainText);

	return bStatus;
}

bool SeedFile::AddHMac(vecuc& keyFileState)
{
	sha256_ctx ctx[1];
    sha256_begin(ctx);

	// hash the file state
	sha256_hash(&keyFileState[0], (unsigned long)keyFileState.size(), ctx);

	// hash in the encryption key
	std::vector<unsigned char> vKey;
	m_pFortuna->GetSeedFileKey(vKey);
	assert(vKey.size() == 32);
	sha256_hash(&vKey[0], (unsigned long)vKey.size(), ctx);
	EraseVector(vKey);

	unsigned char hval[32];
	sha256_end(hval, ctx);

	for (int i=0; i<32; ++i)
	{
		keyFileState.push_back(hval[i]);
		hval[i] = rand() % 256;
	}

	return true;
}

// extract the pool data
bool SeedFile::GetPoolData(std::vector<unsigned char>& hashData,
						   int& numPoolsArg,
						   int& sizePoolsArg)
{
	hashData.clear();
	size_t numPools = m_Pools.size();
	numPoolsArg = (int)numPools;
	sizePoolsArg = 32;
	// 32 bytes of hash data per pool
	// 4 byte marker 0xbaadf00d to signal end of pool hash data
	// 4 bytes per pool for the total number of bytes in the pool
	hashData.resize(numPools*32 + 4 + 4*numPools);			// allocate the space for all of the pool data
	for (size_t i=0; i<hashData.size(); ++i)
		hashData[i] = 0;

	std::vector<Pool*>::iterator iter     = m_Pools.begin();
	std::vector<Pool*>::iterator end_iter = m_Pools.end();

	unsigned int count=0;
	unsigned int countMax = 1000;

	std::vector<unsigned int> vTotalBytesInPool;
	vTotalBytesInPool.resize(numPools);

	size_t iPool=0;
	std::vector<unsigned char> tempData;
	BOOL bStatus = TRUE;
	HANDLE hCompactPoolEvent = 0;
	unsigned int uiCompactPoolCount=0;
	unsigned int uiTest = 0;
	Pool *pPool = NULL;
	int offset=0;
	for (; iter != end_iter; ++iter, offset += 32, ++iPool)
	{
		pPool = *iter;
		uiCompactPoolCount = pPool->GetCompactPoolCount();
		hCompactPoolEvent  = pPool->GetCompactPoolEventHandle();

		bStatus = SetEvent(hCompactPoolEvent);
		if (bStatus == FALSE)
		{
			assert(0);
		}

		// now wait for the pool to be compacted
		while(count < countMax)
		{
			Sleep(50);
			uiTest = pPool->GetCompactPoolCount();
			if (uiTest != uiCompactPoolCount)
				break;
		}

		if (uiTest == uiCompactPoolCount)
		{
			assert(0);
		}

		vTotalBytesInPool[iPool] = pPool->GetTotalBytes();
		pPool->GetHashedPoolData(tempData);

		if (!tempData.empty())
		   std::copy(tempData.begin(), tempData.end(), hashData.begin()+offset);
	}

	// add the end of pool marker
	unsigned char baadFood[4] = {0x0d, 0xf0, 0xad, 0xba};	// LITTLE ENDIAN
	std::copy(&baadFood[0], &baadFood[4], hashData.begin() + offset);

	// now add in the number of bits of entropy in each pool
	offset += 4;

	iPool = 0;
	unsigned int TotalBytes = 0;
	for (iPool=0; iPool < numPools; offset += 4, ++iPool)
	{
		TotalBytes = vTotalBytesInPool[iPool];
		const unsigned char* pData = (unsigned char*)&TotalBytes;
		if ( hashData.begin()+offset + sizeof(unsigned int) > hashData.end())
		{
			assert(0);
		}
		std::copy(pData, pData+4, hashData.begin()+offset);
	}

	return true;
}


}	// end namespace CitadelSoftwareInc