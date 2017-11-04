/*
	(c) Citadel Software Inc, 2003, www.citadelsoftware.ca
*/


#pragma once

#include "CMclThread.h"

class CMclThread;

namespace CitadelSoftwareInc {

class Fortuna;
class Pool;
class SourceMgr;
class Generator;

	/*! \brief Creates, Deletes and gives access to the Pools
	*
	*/

class PoolMgr : public CMclThreadHandler
{
	public:
		PoolMgr(Fortuna* pOwner);
		~PoolMgr();

		// Overloaded from CMclThreadHandler - this is the entry point
		virtual unsigned ThreadHandlerProc(void);

		// Starts the thread running
		bool StartThread();

		HANDLE GetThreadHandle() const { return m_pThread->GetHandle(); }

		// return NULL when num exceeds actual number of Pools
		Pool *GetPool(const int num);

		// Returns true if the reseed was performed
		// Reseeds are only performed when there is enough entropy available in Pool p0
		bool Reseed(std::vector<unsigned char>& seed, bool& bShuttingDown);

#ifdef FORTUNAMONITOR
	public:
#else
	private:
#endif
		// give out a copy of pointers to all of the pools
		bool GetPools(std::vector<Pool*>& Pools);

	private:
		Fortuna *m_pOwner;
		CMclThread *m_pThread;
		
		enum {eNumberOfPools=32};

		std::string m_sStatus;		// status string


		DWORD m_dwLastReseedTime;			// time of the last reseed

		unsigned int m_reseedcount;			// reseed counter


	private:
		PoolMgr();									// don't allow default ctor
		PoolMgr(const PoolMgr& x);					// don't allow copy ctor
		PoolMgr& operator=(const PoolMgr& x);		// don't allow operator =

		std::vector<Pool*> m_Pools;

		friend class SourceMgr;
		friend class Generator;
		friend class Fortuna;
};

}	// end namespace CitadelSoftwareInc