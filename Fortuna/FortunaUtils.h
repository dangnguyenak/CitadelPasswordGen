/*
	(c) Citadel Software Inc, 2003, www.citadelsoftware.ca
*/

    /*! \file FortunaUtils.h
	*   \brief Utility functions used throughout Fortuna
	*
	*/


#pragma once

#include "sha2.h"
#include "CTRNonceCounter.h"

namespace CitadelSoftwareInc {

	void AddFileTimeData(vecuc& vData, const FILETIME& filetime);

	// different bytes in the two timestamps add a byte of entropy
	void AddFileTimeData2(vecuc& vData, const FILETIME& filetime, const FILETIME& oldfiletime);
	
	void AddStringData(vecuc& vData, const unsigned char* pData, const DWORD dwSize);

	void AddBinaryData(vecuc& vData, const unsigned char* pData, const DWORD dwSize);

//	void AddBinaryData(vecuc& vData, const unsigned char* pData, int size);

	void AddString(vecuc& vData, const unsigned char* pString);

	// add a bit of entropy for each byte that is different between old data and new data
	void AddBinaryData2(vecuc& vData, 
						const unsigned char* pData, 
						const DWORD dwSize,
						const unsigned char* pOldData);

	// all non zero bytes add one bit of entropy
	void AddBinaryData3(vecuc& vData, const unsigned char* pData, const DWORD dwSize);

	// add in different time sources, ie current wall clock time, tick count (time since system startup), high resolution timer etc
	void AddTimeData(vecuc& vData);

	template<typename T>
	void AddChaoticBytes(vecuc& vData, const T& t)
	{
		const int numBytes = sizeof(T);
		const unsigned char* p = (const unsigned char*)(&t);
		for (int i=0; i<numBytes; ++i)
		{
			vData.push_back(*p++);
		}
	}

	template<typename T>
	void GetRandomValue(T& t)
	{
		unsigned char* pData = (unsigned char*)&t;
		int numBytes = sizeof(T);
		for (int i=0; i<numBytes; ++i)
		{
			pData[i] = rand() % 256;
		}
	}

	template<typename T>
	void AddToVector(vecuc& vData, const T& t)
	{
		unsigned char uc=0;
		T temp(t);
		unsigned char* pData = (unsigned char*)&t;
		int numBytes = sizeof(T);
		for (int i=0; i<numBytes; ++i)
		{
			// LITTLE ENDIAN (lowest byte first)
			uc = (unsigned char)(temp & 0xff);
			vData.push_back(uc);
			temp = temp >> 8;
		}
	}

	template<typename T>
	void AddToVector2(vecuc& vData, const T& t)
	{
		unsigned char* pData = (unsigned char*)&t;
		int numBytes = sizeof(T);
		for (int i=0; i<numBytes; ++i)
		{
			if (pData[i])
				vData.push_back(pData[i]);
		}
	}


	// append the source vector to the target vector
	template<typename T>
	void AddToVector(std::vector<T>& target, std::vector<T>& source)
	{
		std::vector<T>::iterator iter     = source.begin();
		std::vector<T>::iterator end_iter = source.end();
		for (; iter != end_iter; ++iter)
		{
			target.push_back(*iter);
		}
	}

	template<typename T>
	T GetFromVector(vecuc& vData)
	{
		T t=0;
	
		unsigned char uc=0;
		unsigned char* pData = (unsigned char*)&t;
		int numBytes = sizeof(T);
		for (int i=0; i<numBytes; ++i)
		{
			uc = vData[i];
			t = t + (uc << (i*8));
		}

		return t;
	}

	// computes the SHA256 hash of vData and returns the result in vHash
	void HashVector(vecuc& vData, vecuc& vHash);

	// computes the SHA256 hash of vData and returns the result in vData
	void HashVector(vecuc& vData);

	// overwrite the vector with data from rand() and then clear the vector
	void EraseVector(vecuc& vData);

	// fill in the string with random characters and then erase
	void EraseString(std::string& sString);

	// Erase the state in the hash context structure
	void EraseHash(sha256_ctx ctx[1]);

	// encrypt plain text to cipher text using the encryption key
	// on input, text contains the bytes to encrypt
	// on output, the bytes are encrypted
	bool EncryptCTRMode(vecuc& vInOut,
						vecuc& vEncKey,
						CTRNonceCounter& nonceCounter);

	// initialize srand with different quantities to make it different for each thread
	void InitRand(void* pVoid);

	template<typename T>
	T ExtractValue(dequc& deqData)
	{
		T t=0;
		unsigned char uc=0;
		unsigned char* pData = (unsigned char*)&t;
		const size_t numBytes = sizeof(T);

		if (numBytes > deqData.size())
		{
			assert(0);
			return 0;
		}

		for (int i=0; i<numBytes; ++i)
		{
			uc = deqData[0];
			pData[i] = uc;
			deqData.pop_front();
//  this doesn't work under Visual C++ 7.0 for __int64, it appears that you can't left shift more than 24 bits
//			uc = deqData[0];
//			T temp = (T)(uc << (i*8));
//			t += temp;
//			deqData.pop_front();
		}

		return t;
	}

	// vData must be presized before calling this routine
	int ExtractVector(dequc& deqData, vecuc& vData);

	// initialize winsock
	bool InitWinsock(unsigned char major, unsigned char minor);

#ifdef FORTUNAMONITOR
	std::string BinaryToString(const unsigned char* pData, const int size, const int maxBytes=1024);
#endif

}	// end namespace CitadelSoftwareInc