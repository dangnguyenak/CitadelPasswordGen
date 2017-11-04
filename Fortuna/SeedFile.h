/*
	(c) Citadel Software Inc, 2003, www.citadelsoftware.ca
*/


#pragma once

#include "CMclThread.h"
#include "CMclMutex.h"
#include "CMclEvent.h"

namespace CitadelSoftwareInc {

class Fortuna;
class PoolMgr;
class Pool;

class SeedFile : public CMclThreadHandler
{
	public:
		SeedFile(Fortuna* pFortuna, PoolMgr* pOwner, std::vector<Pool*>& Pools);
		~SeedFile();

		// Overloaded from CMclThreadHandler - this is the entry point
		virtual unsigned ThreadHandlerProc(void);

		// Starts the thread running
		bool StartThread();

		HANDLE GetThreadHandle() const { return m_pThread->GetHandle(); }

	private:
		Fortuna *m_pFortuna;
		PoolMgr *m_pPoolMgr;
		CMclThread *m_pThread;
		HANDLE    m_hShutdownEvent;			// fortuna shutdown event
		std::vector<Pool*> m_Pools;
		bool      m_bShuttingDown;			// true when shutting down
		void      SetShuttingDown()  { m_bShuttingDown = true; }

		// Write out a new seed file
		bool WriteSeedFile();

		// create the data that goes into the seed file
		bool CreateFileState(std::vector<unsigned char>& vData);

		// extract the pool data
		bool GetPoolData(std::vector<unsigned char>& hashData, int& numPools, int& sizePools);

		// add the hmac to the key file state
		bool AddHMac(vecuc& keyFileState);

	private:
		friend class Fortuna;
		SeedFile();										// don't allow default ctor
		SeedFile(const SeedFile& x);					// don't allow copy ctor
		SeedFile& operator=(const SeedFile& x);			// don't allow operator =

		std::string m_SeedFileName;						// full path and name of the seed file
};

}	// end namespace CitadelSoftwareInc