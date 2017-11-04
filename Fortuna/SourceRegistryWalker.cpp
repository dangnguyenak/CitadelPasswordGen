/*
	(c) Citadel Software Inc, 2003, www.citadelsoftware.ca
*/

/*!  \file SourceRegistryWalker.cpp
*    \brief Generic class for walking the Windows Registry to create source events.
*
*
*/


#include "stdafx.h"
#include "SourceRegistryWalker.h"
#include "Fortuna.h"
#include "FortunaUtils.h"


namespace CitadelSoftwareInc {



SourceRegistryWalker::SourceRegistryWalker(const std::string& sName,
										   Fortuna* pFortuna, 
										   SourceMgr* pOwner, 
										   std::vector<Pool*>& Pools,
										   bool bLogFile,
										   HKEY hKeyRoot,
										   unsigned char ucSourceNum,
										   const std::string& sSubKey)
	:
	Source(sName, pFortuna, pOwner, Pools, ucSourceNum),
	m_hKeyRoot(hKeyRoot),
	m_sSubKey(sSubKey),
	m_resultKeyRoot(0),
	m_dwValueQueryIndex(0),
	m_dwSubKeyQueryIndex(0),
	m_hWorkingRootKey(0),
	m_hCurrentSubKey(0),
	m_pValueData(0),
	m_dwValueDataLen(64*1024),
	m_pLogFile(0),
	m_CurrentSubKeysSize(0)
{
	if (m_sSubKey.empty())
		m_resultKeyRoot = RegOpenKeyEx(m_hKeyRoot, NULL, 0, KEY_ENUMERATE_SUB_KEYS, &m_hWorkingRootKey);
	else
		m_resultKeyRoot = RegOpenKeyEx(m_hKeyRoot, m_sSubKey.c_str(), 0, KEY_ENUMERATE_SUB_KEYS, &m_hWorkingRootKey);

	m_pValueData = new unsigned char[m_dwValueDataLen];
	memset(m_pValueData,0,m_dwValueDataLen);

	if (bLogFile)
	{
#pragma warning(disable:4311)
		unsigned int num = (unsigned int)this;
		char buffer[256];
		memset(buffer,0,sizeof(buffer));
		sprintf(buffer,"RegistryWalker-%x.txt", num);
		m_pLogFile = fopen(buffer,"wt");
	}
}

SourceRegistryWalker::~SourceRegistryWalker()
{
	if (m_hWorkingRootKey)
	{
		LONG lResult = RegCloseKey(m_hWorkingRootKey);
		if (lResult != ERROR_SUCCESS)
			{
				assert(0);
			}
	}

	delete [] m_pValueData;
	m_pValueData = NULL;

	if (m_pLogFile)
		fclose(m_pLogFile);
}


// return the chaotic deterministic data to be entered into the pools
// This class will enter all of the bytes from the guids
bool SourceRegistryWalker::GetChaoticData(vecuc& vData)
{
	int i=0;
	const DWORD dwLen = 1024;
	DWORD dwSizeValueName = dwLen;
	char sValueName[dwLen+1];
	memset(sValueName,0,sizeof(sValueName));

	DWORD dwSizeClass=dwLen;
	char sClass[dwLen];
	memset(sClass,0,sizeof(sClass));

	FILETIME filetime;
	filetime.dwHighDateTime = 0;
	filetime.dwLowDateTime = 0;
	std::string subKey;

	// establish a new top level subkey
	if (m_hCurrentSubKey == 0)
	{
		// sanity check - reset the subkeys
		m_CurrentSubKeysSize = m_CurrentSubKeys.size();		// debugging
		m_CurrentSubKeys.clear();
		m_CurrentSubKeysSize = m_CurrentSubKeys.size();

		// open up a subkey to start at
		LONG lResult = ERROR_SUCCESS;
		while(lResult == ERROR_SUCCESS)
		{
			dwSizeValueName = dwLen;
			dwSizeClass = dwLen;

			lResult = RegEnumKeyEx(m_hWorkingRootKey, m_dwSubKeyQueryIndex++, sValueName, &dwSizeValueName, NULL,
									sClass, &dwSizeClass, &filetime);

			if (lResult == ERROR_SUCCESS)	// check for subkeys which are being excluded from the walk
			{
				if(ValidSubkey(sValueName))
					break;
			}
		}

#ifdef FORTUNAMONITOR
		if (dwSizeValueName > 1)
			SetPeekString(std::string(sValueName));
#endif

		if (lResult == ERROR_SUCCESS)
		{
			if (m_pLogFile)
				WriteToLogFile(sValueName);

			std::pair<std::string,DWORD> pairdata(sValueName, 0);
			m_CurrentSubKeys.push_back(pairdata);
			m_CurrentSubKeysSize = m_CurrentSubKeys.size();
			LONG lResult = RegOpenKeyEx( m_hWorkingRootKey, sValueName, 0, KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE, 
										&m_hCurrentSubKey);

			if(lResult == ERROR_ACCESS_DENIED)
			{
				m_CurrentSubKeys.clear();
				m_hCurrentSubKey = 0;
			}
			else if(lResult == ERROR_FILE_NOT_FOUND)
			{
				m_CurrentSubKeys.clear();		// can happen if there is a bad substring (non ascii)
				m_hCurrentSubKey = 0;
			}
			else if (lResult != ERROR_SUCCESS)
			{
				assert(0);
			}

			m_dwValueQueryIndex = 0;

			const int numChars = (int)strlen(sValueName);
			AddStringData(vData, (const unsigned char*)sValueName, numChars);
			AddFileTimeData(vData, filetime);

			return true;
		}
		else if (lResult == ERROR_NO_MORE_ITEMS)
		{
			// start over from the top of this registry tree entry
			m_dwSubKeyQueryIndex = 0;
			if (m_hCurrentSubKey)
			{
				LONG lResult = RegCloseKey(m_hCurrentSubKey);
				if (lResult != ERROR_SUCCESS)
					assert(0);
			}

			m_hCurrentSubKey = 0;
			++m_NumberOfCycles;		// protected member from class Source

			// update the cycle time
			DWORD dwCycleEnd = GetTickCount();
			m_CycleTime = (dwCycleEnd - m_dwCycleStart)/1000;		// cycle time in seconds
			m_dwCycleStart = dwCycleEnd;

			if (m_pLogFile)
				WriteToLogFile("\n***** End of Registry Tree***** (Starting over)\n\n");

			return true;
		}
		else
		{
			assert(0);	// should never get here
		}
	}
	
	// subkey must be set to proceed from here
	if ( m_hCurrentSubKey == 0)
		return true;

	// enumerate the next value from the current subkey
	dwSizeValueName = dwLen;
	DWORD dwType=0;
	DWORD dwDataLen= m_dwValueDataLen;
	LONG lResult = RegEnumValue(m_hCurrentSubKey, m_dwValueQueryIndex, sValueName, &dwSizeValueName, 
								0, &dwType, m_pValueData, &dwDataLen);

	if (lResult == ERROR_MORE_DATA)
	{
		delete [] m_pValueData;
		m_dwValueDataLen = dwDataLen + 64*1024;
		m_pValueData = new unsigned char[m_dwValueDataLen];
		memset(m_pValueData,0,m_dwValueDataLen);

		lResult = RegEnumValue(m_hCurrentSubKey, m_dwValueQueryIndex, sValueName, &dwSizeValueName, 
								0, &dwType, m_pValueData, &dwDataLen);

		if (lResult != ERROR_SUCCESS)
		{
			assert(0);
			// this should never happen, dwDataLen is the new data length
			RegCloseKey(m_hCurrentSubKey);
			m_hCurrentSubKey = 0;
			Sleep(100);
			return true;
		}
	}

	if (lResult == ERROR_SUCCESS)
	{
		int sValueNameLen = (int)strlen(sValueName);
		if (sValueNameLen)
		{
			AddStringData(vData, (const unsigned char*)sValueName, sValueNameLen);
#ifdef FORTUNAMONITOR
		if (sValueNameLen > 1)
			SetPeekString(std::string(sValueName));
#endif
		}

		if (dwType == REG_SZ)
		{
			if (dwDataLen>1)
			{
				AddStringData(vData, (const unsigned char*)m_pValueData, dwDataLen);
#ifdef FORTUNAMONITOR
		{
			const char* pString = (const char*)m_pValueData;
			if (sValueNameLen > 1 && dwDataLen > 1)
			{
				std::string s1 = std::string(sValueName) + std::string(pString);
				SetPeekString(s1);
			}
			else if (dwDataLen > 1)
			{
				SetPeekString(std::string(pString));
			}
		}
#endif
			}
		}
		else if (dwDataLen)
		{
			AddBinaryData(vData, (const unsigned char*)m_pValueData, dwDataLen);
		}

		if (dwType == REG_SZ && m_pLogFile && dwDataLen > 1)
				WriteToLogFile((const char*)sValueName, (const char*)m_pValueData);

		++m_dwValueQueryIndex;
	}
	else if(lResult == ERROR_NO_MORE_ITEMS)
	{
		// any more subkeys to enumerate for this key?
		lResult = LookForNextSubkey(vData);

		bool bPanic=false;
		int iCount=0;
		while (lResult == ERROR_NO_MORE_ITEMS && !m_CurrentSubKeys.empty())
		{
			lResult = LookForNextSubkey(vData);

			if (++iCount > 1000)		// avoid infinite loops - 
			{
				bPanic = true;
				break;
			}
		}

		if (bPanic)
		{
			assert(0);	// should reset to top of registry
			m_CurrentSubKeys.clear();
			m_CurrentSubKeysSize = 0;
			LONG lResult = RegCloseKey(m_hCurrentSubKey);
			if (lResult != ERROR_SUCCESS)
			{
				assert(0);
			}

			m_hCurrentSubKey = 0;
			m_dwSubKeyQueryIndex = 0;
		}
	}
	else
	{
		assert(0);		// should never get here
	}

	// sanity check
	if (m_hCurrentSubKey == 0 && !m_CurrentSubKeys.empty())
	{
		assert(0);
		m_CurrentSubKeys.clear();
	}

	// sleep a little extra to reduce the load on the system
	int nSleep = (m_dwSubKeyQueryIndex % 3) + 3 + ( (unsigned int)this % 3);

	// add in a sleep value from the current key
	size_t sksize = m_currentSubkey.size();
	int sum = 0;
	for (i=0; i<(int)sksize; ++i)
	{
		sum = sum + (int)m_currentSubkey[i];
	}
	nSleep += sum % 5;

	// add in something from the data
	sum = 0;
	if (dwDataLen < m_dwValueDataLen)
	{
		nSleep += dwDataLen % 3;

		int size = (int)dwDataLen;
		if (size > 100)
			size = 100;
		for (i=0; i<size; ++i)
			sum = sum + m_pValueData[i];
	}
	nSleep += sum % 3;

	Sleep( nSleep );

	return true;
}

LONG SourceRegistryWalker::LookForNextSubkey(vecuc& vData)
{
	const DWORD dwLen = 1024;
	DWORD dwSizeValueName = dwLen;
	char sValueName[dwLen+1];
	memset(sValueName,0,sizeof(sValueName));

	DWORD dwSizeClass=dwLen;
	char sClass[dwLen];
	memset(sClass,0,sizeof(sClass));

	FILETIME filetime;
	filetime.dwHighDateTime = 0;
	filetime.dwLowDateTime = 0;
	std::string subKey;

	DWORD dwIndex = 0;
	m_dwValueQueryIndex = 0;

	if (m_CurrentSubKeys.empty())
	{
		return ERROR_NO_MORE_ITEMS;
	}

	std::vector<std::pair<std::string,DWORD> >::iterator enditer = m_CurrentSubKeys.end();
	--enditer;
	dwIndex = (*enditer).second;

	LONG lResult = ERROR_SUCCESS;
	dwSizeValueName = dwLen;
	dwSizeClass = dwLen;
	lResult = RegEnumKeyEx(m_hCurrentSubKey, dwIndex++, sValueName, &dwSizeValueName, NULL,
								sClass, &dwSizeClass, &filetime);

	// update for the next subkey
	(*enditer).second = dwIndex;

	if (lResult == ERROR_SUCCESS)
	{
		// add the timestamp on the registry entry
		AddFileTimeData(vData, filetime);
		AddStringData(vData,(const unsigned char*)sValueName, dwSizeValueName);

		if (m_pLogFile)
			WriteToLogFile(sValueName);

		// yes, there are more subkeys, push this new subkey onto the subkey stack
		std::pair<std::string,DWORD> pairdata(std::string(sValueName), 0);
		m_CurrentSubKeys.push_back(pairdata);
		m_CurrentSubKeysSize = m_CurrentSubKeys.size();

		lResult = OpenCurrentSubkey2();
	}
	else if (lResult == ERROR_NO_MORE_ITEMS)
	{
		// no, there are no more subkeys
		// all done with this subkey
		m_CurrentSubKeys.pop_back();
		m_CurrentSubKeysSize = m_CurrentSubKeys.size();

		if (m_CurrentSubKeys.empty())
		{
			LONG lResult = RegCloseKey(m_hCurrentSubKey);
			if (lResult != ERROR_SUCCESS)
			{
				assert(0);
			}
			m_hCurrentSubKey = 0;
			return lResult;
		}

		LONG lResult2 = OpenCurrentSubkey2();
		if (lResult2 == ERROR_FILE_NOT_FOUND)
		{
			assert(0);
		}
	}
	else if (lResult == ERROR_FILE_NOT_FOUND)
	{
		// registry key string is incorrect
		assert(0);
	}
	else
	{
		assert(0);		// unhandled error, should never happen
	}

	return lResult;
}


LONG SourceRegistryWalker::OpenCurrentSubkey2()
{
	LONG lResult = OpenCurrentSubkey();

	while(lResult == ERROR_ACCESS_DENIED && !m_CurrentSubKeys.empty())
	{
		// no access to current key, rewind the stack
		m_CurrentSubKeys.pop_back();
		lResult = OpenCurrentSubkey();
	}

	while (lResult == ERROR_FILE_NOT_FOUND && !m_CurrentSubKeys.empty())
	{
//		assert(0);		// this should never happen - but it does when there is a non ascii registry string
		// oops, bad key, rewind the stack
		m_CurrentSubKeys.pop_back();
		lResult = OpenCurrentSubkey();
	}

	return lResult;
}

LONG SourceRegistryWalker::OpenCurrentSubkey()
{
	LONG lResult = 0;
	// Build up the name of the subkey
	int i=0;
	std::string subKey;
	std::vector<std::pair<std::string,DWORD> >::iterator iter = m_CurrentSubKeys.begin();
	std::vector<std::pair<std::string,DWORD> >::iterator enditer = m_CurrentSubKeys.end();
	for (i=0; iter != enditer; ++iter, ++i)
	{
		std::pair<std::string,DWORD> pairdata(*iter);
		std::string sTemp = pairdata.first;
		if (i == 0)
			subKey = sTemp;
		else
		{
			subKey = subKey + std::string("\\");
			subKey = subKey + sTemp;
		}
	}

	m_currentSubkey = std::string(subKey.c_str());		// for debugging...

	// close the previous key
	if (m_hCurrentSubKey)
	{
		lResult = RegCloseKey(m_hCurrentSubKey);
		if (lResult != ERROR_SUCCESS)
			assert(0);
	}

	m_hCurrentSubKey = 0;
	// now open the key
	lResult = RegOpenKeyEx( m_hWorkingRootKey, subKey.c_str(), 0, KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE, 
								&m_hCurrentSubKey);

//	if (lResult != ERROR_SUCCESS)
//	{
//		assert(0);
//	}

	return lResult;
}

void SourceRegistryWalker::WriteToLogFile(const char* pString)
{
	if (m_pLogFile)
	{
		size_t indent = m_CurrentSubKeys.size();
		for (size_t i=0; i<indent; ++i)
			fprintf(m_pLogFile,"  ");

		fprintf(m_pLogFile,"%s\n",pString);
		fflush(m_pLogFile);
	}
}

void SourceRegistryWalker::WriteToLogFile(const char* ps1, const char* ps2)
{
	if (m_pLogFile)
	{
		size_t indent = m_CurrentSubKeys.size();
		for (size_t i=0; i<indent; ++i)
			fprintf(m_pLogFile,"  ");

		fprintf(m_pLogFile,"%s : %s\n",ps1, ps2);
		fflush(m_pLogFile);
	}
}


// return true if the subkey is not in the m_AvoidSubkeys list
bool SourceRegistryWalker::ValidSubkey(const std::string& sKey)
{
	std::vector<std::string>::iterator iter    = m_AvoidSubKeys.begin();
	std::vector<std::string>::iterator enditer = m_AvoidSubKeys.end();

	for (; iter != enditer; ++iter)
	{
		const std::string& s = *iter;
		if (stricmp(s.c_str(), sKey.c_str()) == 0)
			return false;
	}

	return true;
}


}	// end namespace CitadelSoftwareInc