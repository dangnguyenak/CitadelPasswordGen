/*
	(c) Citadel Software Inc, 2003, www.citadelsoftware.ca
*/

/*!  \file SourceClsid.cpp
*    \brief Registry HKCR/CLSID source event data.
*
*
*/


#include "stdafx.h"
#include "SourceClsid.h"
#include "Fortuna.h"


namespace CitadelSoftwareInc {

void FillinGuid(const char* pData, unsigned char guidvec[16])
{
	char sbuf[3];
	sbuf[2] = '\0';

	// strip out the { and } and - chars
	char buffer[40];
	memset(buffer,0,sizeof(buffer));
	int target=0;

	memset(guidvec,0,sizeof(guidvec));

	char c;
	while (*pData)
	{
		// all guids in the clsid section of the registry have the same format
		c = *pData;
		if (c == '{' || c == '}' || c == '-')
			;
		else
			buffer[target++] = c;

		++pData;
	}

	unsigned int uc=0;
	for (int i=0; i<16; ++i)
	{
		sbuf[0] = buffer[2*i];
		sbuf[1] = buffer[2*i+1];
		sbuf[2] = '\0';
		sscanf(sbuf, "%x", &uc);
		guidvec[i] = (unsigned char)uc;
	}

}


SourceClsid::SourceClsid(const std::string& sName, Fortuna* pFortuna, SourceMgr* pOwner, std::vector<Pool*>& Pools, unsigned char ucSourceNum)
	:
	Source(sName, pFortuna, pOwner, Pools, ucSourceNum),
	m_hKeyClsid(0),
	m_resultClsid(0),
	m_dwQueryIndex(0),
	m_numberClsids(0),
	m_numberClsidsProcessed(0)
{
	m_resultClsid = RegOpenKeyEx(HKEY_CLASSES_ROOT, "CLSID", 0, KEY_ENUMERATE_SUB_KEYS, &m_hKeyClsid);
	memset(m_guidvec,0,sizeof(m_guidvec));
}



SourceClsid::~SourceClsid()
{
	if (m_hKeyClsid)
	{
		RegCloseKey(m_hKeyClsid);
	}
}


// return the chaotic deterministic data to be entered into the pools
// This class will enter all of the bytes from the guids
bool SourceClsid::GetChaoticData(std::vector<unsigned char>& vData)
{
	int i=0;
	const DWORD dwLen = 1024;
	DWORD dwSizeValueName = dwLen;
	char sValueName[dwLen+1];
	DWORD dwType=0;

	DWORD dwSizeClass=dwLen;
	char sClass[dwLen];

	FILETIME filetime;

	LONG lResult = RegEnumKeyEx(m_hKeyClsid, m_dwQueryIndex++, sValueName, &dwSizeValueName, NULL,
		                        sClass, &dwSizeClass, &filetime);

	if (lResult == ERROR_SUCCESS)
	{
#ifdef FORTUNAMONITOR
		if (dwSizeValueName > 1)
			SetPeekString(std::string(sValueName));
#endif
		if (dwSizeValueName == 38)
		{
			vData.resize(16);
			m_numberClsidsProcessed++;

			unsigned char guidvec[16];
			FillinGuid(sValueName, guidvec);

			// Return all the data to the caller, but only claim 1 bit of entropy for each byte of the guid
			// that is different.  Note that guids are stored in sorted order.
			for (i=0; i<16; ++i)
			{
				vData[i] = guidvec[i];

				m_guidvec[i] = guidvec[i];	// update the byte for the next comparison
			}
			// use the last byte of the filetime
			unsigned char uc = (unsigned char)(filetime.dwLowDateTime & 0xff);
			vData.push_back(uc);
		}
	}
	else if (lResult == ERROR_NO_MORE_ITEMS)
	{
		m_dwQueryIndex = 0;
		m_numberClsids = m_numberClsidsProcessed;
		m_numberClsidsProcessed = 0;		// start a new cycle
		++m_NumberOfCycles;					// protected data member from Source

		// update the cycle time
		DWORD dwCycleEnd = GetTickCount();
		m_CycleTime = (dwCycleEnd - m_dwCycleStart)/1000;		// cycle time in seconds
		m_dwCycleStart = dwCycleEnd;
	}
	
	if (m_NumberOfCycles)
	{
		// do some extra sleeping each time through, based on the number of cycles
		const DWORD nSleepMax = 60 * 1000;
		DWORD nSleep = m_NumberOfCycles * 10;
		if (nSleep > nSleepMax)
			nSleep = nSleepMax;

		// sleep as a function of the data collected
		int sum = 0;
		size_t size = vData.size();
		for (size_t i=0; i<size; ++i)
			sum += vData[i];

		nSleep += sum % 7;

		Sleep(nSleep);
	}

	return true;
}


}	// end namespace CitadelSoftwareInc