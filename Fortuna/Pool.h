/*
	(c) Citadel Software Inc, 2003, www.citadelsoftware.ca
*/


#pragma once

#include "CMclThread.h"
#include "CMclMutex.h"
#include "CMclEvent.h"

#include "NCColln.h"
#include "FastList.h"

namespace CitadelSoftwareInc {

class Fortuna;
class PoolMgr;

	/*! \brief Threadshafe Pool object which runs on it's own thread, accumulates entropy from sources, and compacts
	*          the pool using SHA-256 when the pool requires compacting.  The hash is done on the pool's thread.
	*/

class Pool : public CMclThreadHandler
{
	public:
		Pool(Fortuna* pFortuna, PoolMgr* pOwner, int iPoolNum);
		~Pool();

		// Overloaded from CMclThreadHandler - this is the entry point
		virtual unsigned ThreadHandlerProc(void);

		// Starts the thread running
		bool StartThread();

		HANDLE GetThreadHandle() const { return m_pThread->GetHandle(); }

		// add a byte to the pool
		bool AddByte(const unsigned char byte);

		// add an array of bytes to the pool
		bool AddBytes(const unsigned char* pData, unsigned int size);

		unsigned int GetTotalBytes() const { return m_TotalBytes; }
		void         SetTotalBytesInPool(const unsigned int i) { m_TotalBytes = i; }

		bool ExtractAndErase(std::vector<unsigned char>& vBytes, bool& bShuttingDown);

		unsigned int GetCompactPoolCount() const {return m_CompactPoolCount; }

		HANDLE GetCompactPoolEventHandle() const {return m_CompactPoolEvent.GetHandle(); }

		// Extract the HashedPoolData
		// This is used by the seed file to get the current state to write out to the seed file
		bool GetHashedPoolData(std::vector<unsigned char>& vData);

#ifdef FORTUNAMONITOR
	public:
		std::string GetName() const;
		std::string GetRawPool();
		std::string GetHashedPool();
#endif

	private:
		Fortuna *m_pFortuna;
		PoolMgr *m_pOwner;
		CMclThread *m_pThread;
		FastList<unsigned char> m_pool;

		FastList<unsigned char> m_hashPool;  // this is a hash of the pool
		CMclMutex m_PoolMutex;				// mutex to protect access to the pool, m_pool
		HANDLE    m_hPoolMutex;				// handle to the pool mutex m_PoolMutex
		CMclEvent m_CompactPoolEvent;		// time to compact the pool
		HANDLE    m_hCompactPoolEvent;	    // handle to the compact pool event
		HANDLE    m_hShutdownEvent;			// fortuna shutdown event

		unsigned int m_CompactPoolCount;	// number of times the pool has been compacted

		unsigned int m_CompactPoolDataSize;			// compact the pool when there are this many bytes of raw data

		bool CompactPool();

		unsigned int m_TotalBytes;			// total number of bytes added to the pool

		unsigned char m_redherring[32];		// red herring so that an attacker will see some random looking data in contiguous memory

		int m_iPoolNum;


	private:
		bool SetHashPoolState(vecuc& vState);

		void AddMachineSignatureToPool(vecuc& vMachineSig);
		friend class Fortuna;

	private:
		Pool();									// don't allow default ctor
		Pool(const Pool& x);					// don't allow copy ctor
		Pool& operator=(const Pool& x);			// don't allow operator =

};

}	// end namespace CitadelSoftwareInc