/*
	(c) Citadel Software Inc, 2003, www.citadelsoftware.ca
*/


#pragma once

#include "NCColln.h"
#include "Counter128.h"

namespace CitadelSoftwareInc {

class Fortuna;
class PoolMgr;
class Pool;

	/*! \brief Generator which generates the random numbers, requests reseed from the PoolMgr
	*
	*/

class Generator
{
	public:
		Generator(Fortuna* pOwner);
		~Generator();

		void SetPoolMgr(PoolMgr* pPoolMgr);

		// generate numBytes random bytes
		// Note that the caller must allocate pData
		bool GetRandomBytes(unsigned char* pData, unsigned int numBytes);

		// Generate a fixed number of 16 byte (128 bit) blocks
		bool GenerateBlocks(unsigned char* pData, const unsigned int iDataSize, const unsigned int iNumBlocks);

		// get the current encryption key (used originally to save to the seed file)
		void GetCurrentKey(vecuc& vKey);

		// get the current counter value (used originally to save to the seed file)
		void GetCounter(vecuc& vCounter);

		unsigned int GetReseedCount() const {return m_reseedCount; }

		DWORD GetLastReseedTime() const {return m_dwLastReseedTime; }

#ifdef FORTUNAMONITOR
		std::string GetCounter128();
		std::string Generator::GetKeyAsString();
#endif

	private:
		Fortuna *m_pOwner;

		Counter128 m_counter;							// 128 bit counter for the generator
//		NCColln<unsigned char> m_key;					// hashed pool data used as the encryption key
		FastList<unsigned char> m_key;					// hashed pool data used as the encryption key
		PoolMgr *m_pPoolMgr;							// Pool Manager
		std::vector<Pool*>   m_pools;					// these are all of the data / entropy pools
		unsigned int m_reseedCount;						// number of reseeds
		DWORD        m_dwLastReseedTime;				// time of last reseed in ticks

	private:
		// sets the key state and counter after being read back from the SeedFile
		bool SetKey(vecuc& vKey);
		bool SetCounter(vecuc& vCounter);
		friend class Fortuna;

	private:
		Generator();									// don't allow default ctor
		Generator(const Generator& x);					// don't allow copy ctor
		Generator& operator=(const Generator& x);		// don't allow operator =

};

}	// end namespace CitadelSoftwareInc