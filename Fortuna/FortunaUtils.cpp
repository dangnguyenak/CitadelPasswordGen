/*
	(c) Citadel Software Inc, 2003, www.citadelsoftware.ca
*/

/*!  \file FortunaUitls.cpp
*    \brief Utilities shared by all objects.
*
*
*/


#include "stdafx.h"
#include "FortunaUtils.h"
#include "aescpp.h"
#include "Counter128.h"
#include "Timer.h"
#include "Psapi.h"
#include "ProcessInfo.h"
#include "Sha2.h"


namespace CitadelSoftwareInc {

void AddFileTimeData(vecuc& vData, const FILETIME& filetime)
{
	AddChaoticBytes<DWORD>(vData, filetime.dwHighDateTime);
	AddChaoticBytes<DWORD>(vData, filetime.dwLowDateTime);

}

void AddFileTimeData2(vecuc& vData, const FILETIME& filetime, const FILETIME& oldfiletime)
{
	const unsigned char *pNewData = (unsigned char*)(&filetime);
	const unsigned char *pOldData = (unsigned char*)(&oldfiletime);

	const int size = sizeof(FILETIME);
	for (int i=0; i<size; ++i)
	{
		if (pNewData[i] || pOldData[i])
		{
			vData.push_back(pNewData[i]);
		}
	}
}

//void AddBinaryData(vecuc& vData, const unsigned char* pData, int size)
//{
//	for (int i=0; i<size; ++i)
//	{
//		if (pData[i])
//			vData.push_back(pData[i]);
//	}
//}

void AddString(vecuc& vData, const unsigned char* pString)
{
	if (!pString)
		return;

	while(*pString)
	{
		vData.push_back(*pString++);
	}
}

void AddBinaryData2(vecuc& vData, 
					const unsigned char* pData, 
					const DWORD dwSize,
					const unsigned char* pOldData)
{
	for (DWORD i=0; i<dwSize; ++i)
	{
		if (pData[i] || pOldData[i])
		{
			vData.push_back(pData[i]);
		}
	}
}

void AddBinaryData3(vecuc& vData, 
						const unsigned char* pData, 
						const DWORD dwSize)
{
	for (DWORD i=0; i<dwSize; ++i)
	{
		if (pData[i])
		{
			vData.push_back(pData[i]);
		}
	}
}



void AddStringData(vecuc& vData, 
				   const unsigned char* pData, 
				   const DWORD dwSize2)
{
	if (!pData)
		return;

	size_t dwSize = (size_t)dwSize2;
	if (dwSize2 == 0)
		dwSize = strlen((const char*)pData);

	for (int i=0; i<(int)dwSize; ++i)
	{
		vData.push_back(pData[i]);
	}
}

void AddBinaryData(vecuc& vData, 
				   const unsigned char* pData, 
				   const DWORD dwSize)
{
	unsigned char uc=0;
	for (int i=0; i<(int)dwSize; ++i)
	{
		uc = pData[i];
		
		if (uc != 0x00 && uc != 0xff)
			vData.push_back(uc);
	}
}

// add in different time sources, ie current wall clock time, tick count (time since system startup), high resolution timer etc
void AddTimeData(vecuc& vData)
{
	// local time, wall clock local time, year, month, day, hour, minute, sec, ms
	SYSTEMTIME systemtime;
	GetLocalTime(&systemtime);
	AddBinaryData(vData, (const unsigned char*)&systemtime, sizeof(SYSTEMTIME));

	// ms since the sytem was started
	DWORD dwTime = GetTickCount();
	AddBinaryData(vData, (const unsigned char*)&dwTime, sizeof(DWORD));

	// high performance timer if available
	LARGE_INTEGER performanceCount;
	BOOL bTimer = QueryPerformanceCounter(&performanceCount);

	if (bTimer)
		AddBinaryData(vData, (const unsigned char*)&performanceCount, sizeof(LARGE_INTEGER));

}

// overwrite the vector with data from rand() and then clear the vector
void EraseVector(vecuc& vData)
{
	int i=0;
	std::vector<unsigned char>::iterator iter = vData.begin();
	std::vector<unsigned char>::iterator end_iter = vData.end();
	for (; iter != end_iter; ++iter, ++i)
	{
		vData[i] = rand() % 256;
	}

	vData.clear();
}

// fill in the string with random characters and then erase
void EraseString(std::string& sString)
{
	size_t size = sString.size();
	for(size_t i=0; i<size; ++i)
	{
		sString[i] = rand() % 128;
	}
	sString.clear();
}

// Erase the state in the hash context structure
void EraseHash(sha256_ctx ctx[1])
{
	// overwrite the data in ctx
	for (int i=0; i<8; ++i)
		ctx[0].hash[i] = rand();

	for (int i=0; i<16; ++i)
		ctx[0].wbuf[i] = rand();
}


// encrypt plain text to cipher text using the encryption key
// on input, text contains the bytes to encrypt
// on output, the bytes are encrypted
bool EncryptCTRMode(vecuc& vInOut,
					vecuc& vKey,
					CTRNonceCounter& nonceCounter)
{
	if (vInOut.size() == 0)
		return true;			// nothing to do

	if(vKey.size() != 32)
	{
		assert(0);
		return false;
	}

		// use aes to encrypt the counter using the seed as a key
	AESencrypt aes;
	std::vector<unsigned char> vSeed;

	std::vector<unsigned char> outBlk;
	outBlk.resize(16);

	aes.key256(&vKey[0]);

	size_t numBytes = vInOut.size();
	size_t numBlocks = numBytes / 16;
	size_t numExtra =  numBytes - numBlocks*16;

	const unsigned char* pCounter = nonceCounter.GetState();

	size_t i=0;
	size_t iBlock=0;
	for (iBlock=0; iBlock<numBlocks; ++iBlock)
	{
		aes.encrypt(pCounter,&outBlk[0]);

		// use the encrypted cipher text as a stream cipher
		for (i=0; i<16; ++i)
			vInOut[iBlock*16+i] ^= outBlk[i];
		nonceCounter.Next();
	}
	
	if (numExtra)
	{
		aes.encrypt(pCounter, &outBlk[0]);
		for (i=0; i<numExtra; ++i)
			vInOut[numBlocks*16+i] ^= outBlk[i];
	}


	return true;
}

// initialize srand with different quantities to make it different for each thread
void InitRand(void* pVoid)
{
	unsigned int seed = GetCurrentThreadId();
#pragma warning(disable:4311)
	seed += (unsigned int)pVoid;
	seed += (unsigned int)time(NULL);

	Timer hpTimer;
	hpTimer.Curr();

	seed += hpTimer.m_currTime.LowPart;
	seed += hpTimer.m_currTime.HighPart;

	seed += GetCurrentProcessId();

	srand(seed);
}


	// vData must be presized before calling this routine
	int ExtractVector(dequc& deqData, vecuc& vData)
	{
		size_t deqSize = deqData.size();
		size_t vecSize = vData.size();

		if (vData.empty())
			return 0;

		if (vecSize > deqSize)
		{
			assert(0);
			return 0;
		}

		for (size_t i=0; i<vecSize; ++i)
		{
			vData[i] = deqData[0];
			deqData.pop_front();
		}

		return (int)vecSize;
	}


// computes the SHA256 hash of vData and returns the result in vHash
void HashVector(vecuc& vData, vecuc& vHash)
{
	EraseVector(vHash);

	size_t size = vData.size();
	vHash.resize(32);

	sha256_ctx ctx[1];
    sha256_begin(ctx);

	sha256_hash(&vData[0], (unsigned long)size, ctx);

	sha256_end(&vHash[0], ctx);

	EraseHash(ctx);
}

//! Hash the contents of vData and return with the hash in the vector
void HashVector(vecuc& vData)
{
	vecuc vHash;
	HashVector(vData, vHash);

	EraseVector(vData);
	vData.resize(vHash.size());

	std::copy(vHash.begin(), vHash.end(), vData.begin());

	EraseVector(vHash);
}

// initialize winsock
bool InitWinsock(unsigned char major, unsigned char minor)
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	 
	wVersionRequested = MAKEWORD( major, minor );
	 
	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 ) {
		/* Tell the user that we could not find a usable */
		/* WinSock DLL.                                  */
		return false;
	}
	 
	/* Confirm that the WinSock DLL supports 2.2.*/
	/* Note that if the DLL supports versions greater    */
	/* than 2.2 in addition to 2.2, it will still return */
	/* 2.2 in wVersion since that is the version we      */
	/* requested.                                        */
	 
	if ( LOBYTE( wsaData.wVersion ) != major ||
			HIBYTE( wsaData.wVersion ) != minor ) {
		/* Tell the user that we could not find a usable */
		/* WinSock DLL.                                  */
		WSACleanup( );
		return false; 
	}


	return true;
}

#ifdef FORTUNAMONITOR
std::string BinaryToString(const unsigned char* pData, const int sizeArg, const int maxBytes)
{
	std::string sRetVal;

	if (sizeArg == 0)
		return sRetVal;

	const int size = (maxBytes > 0) && (sizeArg > maxBytes) ? maxBytes : sizeArg;

	const int buffSize = 2*size + 1;
	char* pBuffer = new char[buffSize];
	pBuffer[buffSize-1] = '\0';
	
	for (int i=0; i<size; ++i)
	{
		sprintf(pBuffer+i*2, "%.2x", pData[i]);
	}

	sRetVal = std::string(pBuffer);

	delete [] pBuffer;

	return sRetVal;
}
#endif

}	// end namespace CitadelSoftwareInc