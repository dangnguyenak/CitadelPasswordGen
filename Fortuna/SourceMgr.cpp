/*
	(c) Citadel Software Inc, 2003, www.citadelsoftware.ca
*/

/*!  \file SourceMgr.cpp
*    \brief Creates and gives access to the Source event objects.
*
*
*/


#include "stdafx.h"
#include "SourceMgr.h"
#include "Fortuna.h"
#include "Source.h"
#include "SourceTest.h"
#include "PoolMgr.h"
#include "Pool.h"
#include "SourceClsid.h"
#include "SourceRegistryWalker.h"
#include "SourceProcess.h"
#include "SourceCryptoRand.h"
#include "SourcePing.h"

namespace CitadelSoftwareInc {

SourceMgr::SourceMgr(Fortuna* pOwner)
	:
	m_pOwner(pOwner),
	m_bCtorStatus(false)
{
	// set up the pointers to the pools
	PoolMgr* pPoolMgr = pOwner->GetPoolMgr();
	pPoolMgr->GetPools(m_pools);

	m_bCtorStatus = AddSources();
}

SourceMgr::~SourceMgr()
{
	const size_t count = m_Sources.size();
	for (size_t i=0; i<count; ++i)
	{
		Source *p = m_Sources[i];
		delete p;
		m_Sources[i] = NULL;
	}
}

// add individual chaotic sources
bool SourceMgr::AddSources()
{
	// add the test objects which just enter in one byte from the high resolution timer
	const int numTestObjs = 32;
	m_Sources.reserve(200);

	Source* pSource = NULL;

	unsigned char ucSourceNum = 0;

	std::string sName("Timing Source");
	char buffer[256];
	for (int i=0; i<numTestObjs; ++i)
	{
		sprintf(buffer,"Timing Source %d", i);
		sName = std::string(buffer);
		pSource = new SourceTest(sName, m_pOwner, this, m_pools, ++ucSourceNum);
		m_Sources.push_back(pSource);
	}

	bool bAddRegistryData = true;

	if (bAddRegistryData)
	{
		// HKEY_CLASSES_ROOT CLSID walker
		sName = "CLSID Walker";
		pSource = new SourceClsid(sName, m_pOwner, this, m_pools, ++ucSourceNum);
		m_Sources.push_back(pSource);

		// Walk HKEY_CLASSES_ROOT excluding \CLSID
		bool bLogFile = false;
		std::string subKey;
		sName = "HKCR except CLSID";
		SourceRegistryWalker *pRegWalker = new SourceRegistryWalker(sName, m_pOwner, this, m_pools, bLogFile, HKEY_CLASSES_ROOT, ++ucSourceNum, subKey);
		// CLSID's are added by their own source object, SourceClsid
		pRegWalker->AddSubkeyToAvoid("CLSID");
		m_Sources.push_back(pRegWalker);

		// HKEY_CURRENT_USER
		bLogFile = false;
		subKey.clear();
		sName = "HKCU";
		pRegWalker = new SourceRegistryWalker(sName, m_pOwner, this, m_pools, bLogFile, HKEY_CURRENT_USER, ++ucSourceNum, subKey);
		m_Sources.push_back(pRegWalker);

		// HKEY_LOCAL_MACHINE
		bLogFile = false;
		subKey.clear();
//		subKey = std::string("SOFTWARE\\Microsoft\\Windows\\CurrentVersion");
		sName = "HKLM";
		pRegWalker = new SourceRegistryWalker(sName, m_pOwner, this, m_pools, bLogFile, HKEY_LOCAL_MACHINE, ++ucSourceNum, subKey);
		m_Sources.push_back(pRegWalker);
	}

	SourceProcess* pSourceProcess = new SourceProcess("Process Info", m_pOwner, this, m_pools, ++ucSourceNum);
	m_Sources.push_back(pSourceProcess);

	SourceCryptoRand *pSourceCryptoRand = new SourceCryptoRand("Microsoft Crypto API CryptGenRandom", m_pOwner, this, m_pools, ++ucSourceNum);
	m_Sources.push_back(pSourceCryptoRand);

	SourcePing *pSourcePing = new SourcePing("Ping", m_pOwner, this, m_pools, ++ucSourceNum);
	m_Sources.push_back(pSourcePing);

	return true;
}


// Starts the thread running
bool SourceMgr::StartThread()
{
	m_pThread = new CMclThread(this);

	return true;
}

// Overloaded from CMclThreadHandler - this is the thread entry point
unsigned SourceMgr::ThreadHandlerProc(void)
{
	// start up each of the sources
	const size_t numSources = m_Sources.size();
	for (size_t i=0; i<numSources; ++i)
	{
		Source *pSource = m_Sources[i];
		pSource->StartThread();
	}

	Sleep(10);
	std::vector<HANDLE> hVector;
	hVector.resize(numSources);
	for (i=0; i<numSources; ++i)
	{
		Source *pSource = m_Sources[i];
		hVector[i] = pSource->GetThreadHandle();
	}

	HANDLE hShutdownEvent = m_pOwner->GetShutdownEventHandle();
	DWORD dwResult = WaitForSingleObject(hShutdownEvent, INFINITE);

	if (dwResult == WAIT_FAILED)
	{
		m_sStatus += "Wait for ShutdownEventHandle Failed";
	}

	// now wait for all the source threads to become signalled

	DWORD nCount = (DWORD)numSources;
	dwResult = WaitForMultipleObjects(nCount, &hVector[0], TRUE, 1000*10);

	if (dwResult == WAIT_OBJECT_0)
	{
		;		// DO NOTHING
	}
	else if (dwResult == WAIT_TIMEOUT)
	{
		assert(0);
	}
	else if(dwResult == WAIT_ABANDONED_0)
	{
		assert(0);
	}

	return 0;
}

// return NULL when out of sources
Source* SourceMgr::GetSource(const int num)
{
	Source *pSource = NULL;

	if (num < 0)
		return pSource;

	if (num >= (int)m_Sources.size())
		return pSource;

	pSource = m_Sources[num];

	return pSource;
}

#ifdef FORTUNAMONITOR
void SourceMgr::GetSources(std::vector<Source*>& vSources)
{
	vSources.resize(m_Sources.size());

	std::copy(m_Sources.begin(), m_Sources.end(), vSources.begin());
}

#endif


}	// end namespace CitadelSoftwareInc