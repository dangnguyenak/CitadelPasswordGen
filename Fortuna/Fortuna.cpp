/*
	(c) Citadel Software Inc, 2003, www.citadelsoftware.ca
*/

/*!  \file Fortuna.cpp
*    \brief This is the object that the user interacts with for accessing the Fortuna PRNG.
*
*
*/

/*!  \mainpage Fortuna
*	 <p>This is an implementation of the Fortuna PRNG by Niels Ferguson and Bruce Schneier.
*	 The design is from their book 'Practical Cryptography'.
*    I have tried to follow their design as closely as possible, but there are a few differences.</p>
*    <p>
*    The main differences are:
*    <ul>
*    <li>The seed file is encrypted with a user supplied password.
*    <li>The state of each of the 32 entropy pools is written to the seed file.
*    <li>I developed a fast linked list structure to hold the pool data in memory.
*    </ul>
*    </p>
*
*    \section intro Introduction
*    <p>Generating good random numbers is an essential part of any cryptographic system.
*    Random numbers are used to generate keys, initialization vectors, nonces etc.
*    Particularly in the case of encryption keys generated from a software PRNG,
*    an attacker must not be able to predict past or future keys by examining the output from the
*    PRNG.</p>
*
*    \section fortuna Fortuna
*    <p>Fortuna has a number of novel features as designed by Ferguson and Schneier.
*    Fortuna does not involve any entropy estimators.  This has the benefit that even if entropy
*    is not being fed to the PRNG for a period of time, Fortuna will continue to generate good PRN's
*    as long as the state of the entropy pools has not been compromised.</p>
*    <p>Fortuna uses 32 entropy pools, to allow recovery from a compromise of the pool state.  
*    Pool i is only used once every 2^i reseeds.  Because some pools will not be used very often 
*    an attacker will not be able to deduce the pool contents very easily.</p>
*	
*
*    \section overview Overview
*    <p>
*    Fortuna is composed of 4 parts:
*    <ul>
*    <li> Generator - generates the actual pseudo random numbers
*    <li> Pool Manager and Pools - accumluates entropy and hashes state as required
*    <li> Source Manager and Sources - collects entropy and feeds entropy to the pools
*    <li> Seed File - Encrypts the prng state and reads from and writes to disk
*
*    </ul>
*
*    <p>
*
*    \section generator Generator
*    <p>The generator uses AES encryption in CTR mode to generate random numbers.
*    The entropy pools are used to create a 256 bit key by hashing their state using SHA-256.
*    A 64 bit nonce and 64 bit counter are encrypted in a 128 bit block using the key from the hashed entropy 
*    pools.  The encrypted nonce and key become the random data returned by Fortuna.
*    When a specific request for random numbers is complete, the key is replaced by generating another 256 bytes
*    of random data.  This prevents a user from compromising previously generated PRN's.</p>
*
*
*    \section poolmgr   PoolMgr
*    <p>Each pool executes on it's own thread.  Pools accumulate the entropy sent by the Source objects.  When a pool
*    exceeds a certain size, a manual reset event is signalled.  The pool thread then wakes up and compacts the pool state
*    using SHA-256.</p>
*
*    \section sourcemgr  SourceMgr
*    <p>
*    Each souce executes on it's own thread.  Sources are as follows:
*    <ul>
*    <li> QueryPerformanceCounter - there are 32 threads that collect entropy by using QueryPerformanceCounter on Sleep(1)
*    <li> There are 4 threads that walk the registry.  This forces an attacker to capture the registry of the machine running Fortuna.
*    <li> There is 1 thread monitoring the process data for each process (virtual memory, page faults, I/O Read and Write bytes, etc)
*    <li> There is 1 thread which uses the Microsoft Crypto API as a random data source.  This way an attacker has to break the
*         Microsoft Crypto API as part of attacking this implementation of Fortuna.
*    </ul>
*    
*    Adding new sources is very easy in this design.  Each source is derived from the base class Source, see Source.cpp / .h for details.
*    </p>
*
*    \section seedfile   SeedFile
*    The seed file persists the state of the generator and the 32 entropy pools using password based encryption.
*
*/


#include "stdafx.h"
#include "Fortuna.h"
#include "Generator.h"
#include "PoolMgr.h"
#include "SourceMgr.h"
#include "Pool.h"
#include "Source.h"
#include "FastList.h"
#include "SeedFile.h"
#include "Sha2.h"
#include "FortunaUtils.h"
#include "MachineSig.h"

namespace CitadelSoftwareInc {

Fortuna::Fortuna()
	:	
	m_ShutdownEvent(TRUE),				// true -> manual reset event
	m_sSalt("a*u&0-pLk~B!G6.$,X"),
	m_bWinsock(false)
{
	m_pGenerator = new Generator(this);
	// NOTE: the PoolMgr MUST be constructed before the SourceMgr because the SourceMgr asks the PoolMgr
	// for a pointer to all of the Pools (each source keeps a copy to each of the pools)
	m_pPoolMgr   = new PoolMgr(this);
	m_pSourceMgr = new SourceMgr(this);
	m_pGenerator->SetPoolMgr(m_pPoolMgr);

	m_pPoolMgr->GetPools(m_vPools);

	m_pSeedFile = new SeedFile(this, m_pPoolMgr, m_vPools);

	m_bWinsock = InitWinsock(2,0);

//	bool b1 = TestFastList();
//	assert(b1);
}

Fortuna::~Fortuna()
{
	delete m_pGenerator;
	delete m_pPoolMgr;
	delete m_pSourceMgr;

	if (m_bWinsock)
		WSACleanup();
}

bool Fortuna::StartThreads()
{
	bool b1 = m_pPoolMgr->StartThread();
	bool b2 = m_pSourceMgr->StartThread();
	bool b3 = m_pSeedFile->StartThread();

	return b1 && b2 && b3;
}

bool Fortuna::Stop()
{
	m_pSeedFile->SetShuttingDown();		// ensure that this is the last time the seed file is written
	m_pSeedFile->WriteSeedFile();		// write the seed file one last time before everything shuts down


	HANDLE hPoolMgrThread   = m_pPoolMgr->GetThreadHandle();
	HANDLE hSourceMgrThread = m_pSourceMgr->GetThreadHandle();
	HANDLE hSeedFileThread  = m_pSeedFile->GetThreadHandle();

	m_ShutdownEvent.Set();

	DWORD dwResult = WaitForSingleObject(hSourceMgrThread, INFINITE);
	dwResult = WaitForSingleObject(hPoolMgrThread, INFINITE);
	dwResult = WaitForSingleObject(hSeedFileThread, INFINITE);

	return true;
}

Pool *Fortuna::GetPool(const int num)
{
	return m_pPoolMgr->GetPool(num);
}

Source *Fortuna::GetSource(const int num)
{
	return m_pSourceMgr->GetSource(num);
}

bool Fortuna::GetRandomChar( unsigned char&  uchar)
{
	bool bRetVal = true;

	bRetVal = GetRandomBytes(&uchar, sizeof(unsigned char));	

	return bRetVal;
}

bool Fortuna::GetRandomShort(unsigned short& ushort)
{
	bool bRetVal = true;

	bRetVal = GetRandomBytes((unsigned char*)&ushort, sizeof(unsigned short));

	return bRetVal;
}

bool Fortuna::GetRandomInt(unsigned int&   uint)
{
	bool bRetVal = true;

	bRetVal = GetRandomBytes((unsigned char*)&uint, sizeof(unsigned int));

	return bRetVal;
}


bool Fortuna::GetRandomBytes(unsigned char* pData, unsigned int numBytes)
{
	if (!pData)
		return false;

	if (numBytes <= 0)
		return false;

	bool bStatus = m_pGenerator->GetRandomBytes(pData, numBytes);

	return true;
}

// set the seedfile name and the password used to encrypt the seed file
bool Fortuna::SetSeedFile(const std::string& sFileName, std::string& sPassword)
{
	if (sFileName.empty())
		return false;

	if (sPassword.empty())
		return false;

	m_sSeedFileName = sFileName;
		
	int i=0;
	std::string sPwdSalt;
	sPwdSalt = sPassword + m_sSalt;
	EraseString(sPassword);

	m_RandomKey.Reset();
	unsigned char vKey2[32];
	for (i=0; i<32; ++i)
	{
		vKey2[i] = rand() % 256;
		m_RandomKey.Add(vKey2[i]);
	}

	// now create the key
	sha256_ctx ctx[1];
    sha256_begin(ctx);

	sha256_hash((const unsigned char*)sPwdSalt.c_str(), (unsigned long)sPwdSalt.size(), ctx);
	EraseString(sPwdSalt);

	unsigned char hval[32];
	sha256_end(hval, ctx);

	EraseHash(ctx);

	// encrypt the actual key
	for (i=0; i<32; ++i)
	{
		hval[i] ^= vKey2[i];
		m_SeedFileKey.Add(hval[i]);
		hval[i]  = rand() % 256;
		vKey2[i] = rand() % 256;
	}

	return true;
}

bool Fortuna::ReadSeedFile(FortunaErrors& fError)
{
	fError = eNoErrors;

	if (m_sSeedFileName.empty())
	{
		fError = eSeedFileNotSpecified;
		assert(0);
		return false;
	}

	FILE *pFile = fopen(m_sSeedFileName.c_str(), "rb");

	if (!pFile)
	{
		fError = eCannotOpenSeedFile;
//		assert(0);
		return false;
	}

	vecuc vState2;
	vState2.resize(10240);

	size_t numItems = fread(&vState2[0], 1, vState2.size()-1, pFile);

	if (numItems == 0 || feof(pFile) == 0)
	{
		assert(0);
		fError = eErrorReadingSeedFile;
		fclose(pFile);
		return false;
	}

	fclose(pFile);

	// copy the data to a deque so we can peel values off the front and the hmac off the end
	dequc vState;
	vState.resize(numItems);
	for(size_t i=0; i<numItems; ++i)
		vState[i] = vState2[i];

	bool bHMac = CheckHMac(vState);
	if (!bHMac)
	{
		fError = eHMacFailed;
		assert(0);
		return false;
	}

	// get the version number
	int Version = ExtractValue<int>(vState);
	if (Version != 0x01)
	{
		fError = eBadVersionNumberInSeedFile;
		assert(0);
		return false;
	}

	__int64 nonce   = ExtractValue<__int64>(vState);
	__int64 counter = ExtractValue<__int64>(vState);

	CTRNonceCounter nonceCounter(nonce, counter);

	vecuc vPlainText;
	vPlainText.resize(vState.size());
	for (i=0; i<vState.size(); ++i)
		vPlainText[i] = vState[i];

	std::vector<unsigned char> vKey;
	GetSeedFileKey(vKey);

	bool bStatus = EncryptCTRMode(vPlainText, vKey, nonceCounter);  // plaintext is encrypted on output

	EraseVector(vKey);

	vState.clear();
	vState.resize(vPlainText.size());
	for (i=0; i<vState.size(); ++i)
		vState[i] = vPlainText[i];

	// extract the generator key - 32 bytes
	vecuc vTemp;
	vTemp.resize(32);
	ExtractVector(vState, vTemp);
	m_pGenerator->SetKey(vTemp);

	// exract the generator counter - 16 bytes
	vTemp.resize(16);
	ExtractVector(vState, vTemp);
	m_pGenerator->SetCounter(vTemp);

	int numPools = ExtractValue<int>(vState);
	int poolSize = ExtractValue<int>(vState);

	if (numPools != 32 || poolSize != 32)
	{
		assert(0);
		return false;
	}

	size_t totalSize = numPools*poolSize;
	size_t temp = vState.size();

	if (totalSize > temp)
	{
		assert(0);
		return false;
	}

	vecuc ucTemp;
	ucTemp.resize(32);
	int iPool=0;
	for (iPool=0; iPool < numPools; ++iPool)
	{
		for (int i=0; i<poolSize; ++i)
		{
			ucTemp[i] = vState[0];
			vState.pop_front();
		}

		Pool *pPool = m_vPools[iPool];
		pPool->SetHashPoolState(ucTemp);
	}

	unsigned int baadfood = ExtractValue<unsigned int>(vState);

	if (baadfood != 0xbaadf00d)
	{
		assert(0);
		return false;
	}

	temp = vState.size();
	if (temp != sizeof(unsigned int)*32)
	{
		assert(0);
		return false;
	}

	unsigned int TotalBytes = 0;
	for (iPool=0; iPool<numPools; ++iPool)
	{
		Pool *pPool = m_vPools[iPool];
		
		TotalBytes = ExtractValue<unsigned int>(vState);
		pPool->SetTotalBytesInPool(TotalBytes);
	}

	// now get the machine signature to hash into all the entropy pools
	// this is done to avoid problems if the same seed file is reused
	vecuc vMachineSig;
	GetMachineSignature(vMachineSig);

	// add the machine signature data to each pool
	for (iPool=0; iPool<numPools; ++iPool)
	{
		Pool *pPool = m_vPools[iPool];

		// also adds in some pool specific data
		pPool->AddMachineSignatureToPool(vMachineSig);
	}

	return true;
}

bool Fortuna::CheckHMac(dequc& vState)
{
	size_t numBytes = vState.size();
	if (numBytes <= 32)
		return false;

	bool bRetVal = true;

	size_t i=0;
	// the hmac is the last 32 bytes of the data stream
	vecuc vHMac;
	vHMac.resize(32);
	for (i=0; i<32; ++i)
	{
		vHMac[31-i] = vState[numBytes-1-i];
		vState.pop_back();
	}

	vecuc vTemp;
	vTemp.resize(vState.size());
	for (i=0; i<numBytes-32; ++i)
		vTemp[i] = vState[i];

	sha256_ctx ctx[1];
    sha256_begin(ctx);

	sha256_hash(&vTemp[0], (unsigned long)(numBytes-32), ctx);
	
	vecuc vKey;
	GetSeedFileKey(vKey);
	sha256_hash(&vKey[0], 32, ctx);
	EraseVector(vKey);

	unsigned char hval[32];
	sha256_end(hval, ctx);

	// now compare the hash values
	int badcount = 0;
	unsigned char uc = 0;
	for (i=0; i<32; ++i)
	{
		uc = vHMac[i];
		if (uc != hval[i])
		{
			++badcount;
		}

	}

	if (badcount)
		bRetVal = false;

	return bRetVal;
}



// get the encryption key for the seed file
bool Fortuna::GetSeedFileKey(vecuc& vKey)
{
	if (m_SeedFileKey.Size() != 32)
		return false;

	if (m_RandomKey.Size() != 32)
		return false;

	vecuc vRandomKey;
	m_RandomKey.Extract(vRandomKey);
	m_SeedFileKey.Extract(vKey);

	for (int i=0; i<32; ++i)
		vKey[i] ^= vRandomKey[i];

	EraseVector(vRandomKey);

	return true;
}


}	// end namespace CitadelSoftwareInc