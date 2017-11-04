/*
	(c) Citadel Software Inc, 2003, www.citadelsoftware.ca
*/


#pragma once

#include "CMclThread.h"
#include "CMclMutex.h"
#include "CMclEvent.h"
#include "CMclCritSec.h"


namespace CitadelSoftwareInc {

class Fortuna;
class SourceMgr;
class Pool;


class Source : public CMclThreadHandler
{
	public:
		Source( const std::string& sName, 
				Fortuna* pFortuna, 
				SourceMgr* pOwner, 
				std::vector<Pool*>& Pools,
				unsigned char ucSourceNum);

		virtual ~Source();

		HANDLE GetThreadHandle() const { return m_pThread->GetHandle(); }

		// Overloaded from CMclThreadHandler - this is the entry point
		virtual unsigned ThreadHandlerProc(void);

		// Starts the thread running
		virtual bool StartThread();

		// derived classes must implement this to return their data
		virtual bool GetChaoticData(std::vector<unsigned char>& vData)=0;

		unsigned int GetTotalBytesData() const		{ return m_TotalBytesData; }
		unsigned int GetNumberOfCycles() const		{ return m_NumberOfCycles; }

		const std::string& GetName() const { return m_sName; }

		unsigned int GetCycleTime() const {return m_CycleTime; }

		// return true if the derived class does not add any data in the GetChaoticData method
		virtual bool ZeroData() {return false; }

		//! Return true if shutting down
		bool ShuttingDown();	

	private:
		// sends the data accumulated in vData to the data pools
		bool SendDataToPools(std::vector<unsigned char>& vData);

	protected:
		Fortuna *m_pFortuna;
		SourceMgr *m_pOwner;
		std::string m_sName;

		CMclThread *m_pThread;

		std::vector<Pool*> m_pools;

		BOOL m_bSetThreadPriority;					// status of SetThreadPriority()

		unsigned int m_TotalBytesData;				// total number of data bytes for this source

		unsigned int m_NumberOfCycles;				// number of times the derived class has cycled through all information
		unsigned int m_CycleTime;					// cycle time in seconds
		DWORD        m_dwCycleStart;				// start time of cycle from GetTickCount() - ms since system started
		DWORD        m_dwExtraSleep;				// extra sleep time between each data access
		unsigned char m_ucSourceNum;				// source number
		HANDLE       m_HandleShutdown;				// shutdown event handle

	private:
		Source();									// don't allow default ctor
		Source(const Source& x);					// don't allow copy ctor
		Source& operator=(const Source& x);			// don't allow operator =

	private:
		size_t m_iPool;								// current pool to send data to
		size_t m_numPools;							// number of data pools

#ifdef FORTUNAMONITOR
	private:
		std::string m_sPeekString;					// peek into what the source is doing
		CMclCritSec m_PeekCritSec;
		std::deque<unsigned char> m_cache;			// raw data cache to used to capture last n byte of raw data
		const size_t m_cacheSize;					// size of cache
		void AddBytesToCache(const vecuc& vData);	// add the vector of data to the cache
		bool m_bCacheData;							// enable the cache
		CMclCritSec m_cacheCritSec;					// critical section for the cache
	protected:
		void		SetPeekString(std::string& sText);
	public:
		std::string GetPeekString();
		void SetCacheDataActive(const bool b) { m_bCacheData = b; }
		bool GetCacheDataActive() const {return m_bCacheData; }
		bool GetCacheData(vecuc& vData);
#endif
};

}	// end namespace CitadelSoftwareInc