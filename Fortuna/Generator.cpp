/*
	(c) Citadel Software Inc, 2003, www.citadelsoftware.ca
*/

/*!  \file Generator.cpp
*    \brief Generates the random numbers using the pool data as a seed to the encryption method.
*
*
*/


#include "stdafx.h"
#include "Generator.h"
#include "Fortuna.h"
#include "PoolMgr.h"
#include "Pool.h"
#include "aescpp.h"

namespace CitadelSoftwareInc {

Generator::Generator(Fortuna* pOwner)
	:
	m_pOwner(pOwner),
	m_counter(),				// 128 bit counter
	m_pPoolMgr(0),
	m_reseedCount(0),
	m_dwLastReseedTime(0)
{
	// TODO - Init srand()
}

Generator::~Generator()
{
}

void Generator::SetPoolMgr(PoolMgr* pPoolMgr)
{
	m_pPoolMgr = pPoolMgr;
	m_pPoolMgr->GetPools(m_pools);
}

// generate numBytes random bytes
// Note that the caller must allocate pData
bool Generator::GetRandomBytes(unsigned char* pData, unsigned int numBytes)
{
	bool bStatus = true;
	bool bShuttingDown = false;
	int i=0;

	std::vector<unsigned char> seed;
	bStatus = m_pPoolMgr->Reseed(seed, bShuttingDown);

	if (bShuttingDown)
	{
		for (unsigned int i=0; i<numBytes; ++i)
			pData[i] = 0;

		return false;
	}

	if (!bStatus)
	{
		// unable to reseed, not enough entropy - if this is the first time through then we can not service this request
		if (m_counter.IsZero())
		{
			for (unsigned int i=0; i<numBytes; ++i)
				pData[i] = 0;

			return false;
		}
	}
	else
	{
		// a reseed was performed, update the encryption key
		m_key.Reset();
		for (int i=0; i<32; ++i)
			m_key.Add(seed[i]);

		for (int i=0; i<32; ++i)
			seed[i] = (unsigned char)(rand() % 256);

		++m_reseedCount;
		m_dwLastReseedTime = GetTickCount();
	}

	const int nBlocks = numBytes / 16;
	if (nBlocks)
	{
		bStatus = GenerateBlocks(pData, numBytes, nBlocks);
		if (!bStatus)
			return false;
	}

	const int nExtra = numBytes - nBlocks*16;
	if (nExtra)
	{
		unsigned char outblk[16];

		for (i=0; i<16; ++i)
			outblk[i] = 0;

		bStatus = GenerateBlocks(outblk, 16, 1);
		if (!bStatus)
			return false;

		const int iOffset = nBlocks*16;

		for (i=0; i<nExtra; ++i)
			pData[iOffset+i] = outblk[i];
	}

	// now generate a new seed / encryption key
	unsigned char ucseed[32];
	bStatus = GenerateBlocks(ucseed, 32, 2);
	if (!bStatus)
		return false;

	m_key.Reset();
	for (int i=0; i<32; ++i)
		m_key.Add(ucseed[i]);

	for (int i=0; i<32; ++i)
		ucseed[i] = (unsigned char)(rand() % 256);

	return bStatus;
}


bool Generator::GenerateBlocks(unsigned char* pData, const unsigned int iDataSize, const unsigned int iNumBlocks)
{	
	if (iDataSize < 16 || iNumBlocks < 0)
		return false;

	if (iDataSize < iNumBlocks * 16)
		return false;

	if (iNumBlocks == 0)
		return true;

		// use aes to encrypt the counter using the seed as a key
	AESencrypt aes;
	std::vector<unsigned char> vSeed;
	m_key.Extract(vSeed);
	aes.key256(&vSeed[0]);

	unsigned char inblk[16];
	unsigned char outblk[16];
	unsigned int i=0;

	for (i=0; i<16; ++i)
	{
		inblk[i] = 0;
		outblk[i] = 0;
	}

	unsigned int iBlock = 0;
	for (iBlock=0; iBlock<iNumBlocks; ++iBlock)
	{
		m_counter.Next();

		// now copy over the 128 bit 16 byte counter
		const unsigned char *pc = m_counter.GetState();
		for (i=0; i<16; ++i)
			inblk[i] = pc[i];

		aes.encrypt(inblk, outblk);

		for (i=0; i<16; ++i)
		{
			pData[ iBlock*16 + i] = outblk[i];
		}
	}

	return true;
}

// get the current encryption key (used originally to save to the seed file)
void Generator::GetCurrentKey(vecuc& vKey)
{
	m_key.Extract(vKey);
	assert(vKey.size() == 32);
}

// get the current counter value (used originally to save to the seed file)
void Generator::GetCounter(vecuc& vCounter)
{
	const unsigned char* pData = m_counter.GetState();
	for (int i=0; i<16; ++i)
		vCounter.push_back(pData[i]);
}

// sets the key state after being read back from the SeedFile
bool Generator::SetKey(vecuc& vKey)
{
	if (vKey.size() != 32)
	{
		assert(0);
		return false;
	}

	m_key.Reset();
	for (int i=0; i<32; ++i)
	{
		m_key.Add(vKey[i]);
	}

	return true;
}

bool Generator::SetCounter(vecuc& vCounter)
{
	if (vCounter.size() != 16)
	{
		assert(0);
		return false;
	}

	bool b = m_counter.SetState(vCounter);

	return b;
}

#ifdef FORTUNAMONITOR
std::string Generator::GetCounter128()
{
	std::string sRetVal;

	sRetVal = m_counter.ToString();

	return sRetVal;
}
#endif

#ifdef FORTUNAMONITOR
std::string Generator::GetKeyAsString()
{
	std::string sRetVal;

	vecuc vKey;
	m_key.Extract(vKey);

	if (vKey.empty())
		return sRetVal;

	assert(vKey.size() == 32);

	char buffer[65] = {'\0'};

	unsigned char *pData = (unsigned char*)&vKey[0];
	for (int i=0; i<32; ++i)
	{
		sprintf(buffer+ i*2, "%.2x", pData[i]);
	}

	sRetVal = std::string(buffer);

	return sRetVal;
}
#endif



}	// end namespace CitadelSoftwareInc

