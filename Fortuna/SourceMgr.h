/*
	(c) Citadel Software Inc, 2003, www.citadelsoftware.ca
*/

#pragma once

#include "CMclThread.h"

namespace CitadelSoftwareInc {

class Fortuna;
class Source;
class Pool;

class SourceMgr : public CMclThreadHandler
{
	public:
		SourceMgr(Fortuna* pOwner);
		~SourceMgr();

		bool GetCtorStatus() const {return m_bCtorStatus; }

		// Overloaded from CMclThreadHandler - this is the entry point
		virtual unsigned ThreadHandlerProc(void);

		// Starts the thread running
		bool StartThread();

		HANDLE GetThreadHandle() const { return m_pThread->GetHandle(); }

		// return NULL when num exceeds actual number of Sources
		Source* GetSource(const int num);

#ifdef FORTUNAMONITOR
		void GetSources(std::vector<Source*>& vSources);
#endif

	private:
		bool     m_bCtorStatus;
		Fortuna *m_pOwner;
		CMclThread *m_pThread;

		// adds individual source types
		bool AddSources();

		std::vector<Source*> m_Sources;		// these are child objects of hte SourceMgr
		std::vector<Pool*>   m_pools;		// these are all of the pools

		std::string m_sStatus;		// status string
	private:
		SourceMgr();									// don't allow default ctor
		SourceMgr(const SourceMgr& x);					// don't allow copy ctor
		SourceMgr& operator=(const SourceMgr& x);		// don't allow operator =
};

}	// end namespace CitadelSoftwareInc