/*
	(c) Citadel Software Inc, 2003, www.citadelsoftware.ca
*/

#pragma once

#include "Source.h"

namespace CitadelSoftwareInc {

class Fortuna;
class SourceMgr;
class Pool;

class ProcessInfoBase;
class ProcessData;

class SourceProcess : public Source
{
	public:
		SourceProcess(const std::string& sName, Fortuna* pFortuna, SourceMgr* pOwner, std::vector<Pool*>& Pools, unsigned char ucSourceNum);
		virtual ~SourceProcess();

		// derived classes must implement this to return their data
		virtual bool GetChaoticData(vecuc& vData);

	private:

		DWORD* m_pdwPids;				// array of process ids
		DWORD  m_iPidSize;				// size of m_pdwPids
		DWORD  m_numPids;				// number of pids in m_pdwPids
		DWORD  m_nextPid;				// index of next pid in array m_pdwPids
		DWORD  m_nextInfo;				// next info object to query
		DWORD  m_dwCurrPid;				// current pid

		DWORD m_dwError;				// error from GetLastError()

		std::map<DWORD, ProcessData*> m_mapProcessData;

		// return an existing process data object or create a new one as required
		ProcessData* GetProcessData(DWORD dwPid);

		bool AddProcessInfo(DWORD pid, vecuc& vData);
		bool AddModuleData(DWORD pid, vecuc& vData);

		// adds in the data for a specific module
		bool AddModuleData2(HANDLE hProcess, HMODULE hModule, vecuc& vData);

		std::vector<ProcessInfoBase*> m_vProcessInfo;		// process info objects

		SourceProcess();										// don't allow default ctor
		SourceProcess(const SourceProcess& x);					// don't allow copy ctor
		SourceProcess& operator=(const SourceProcess& x);		// don't allow operator =
};



}	// end namespace CitadelSoftwareInc