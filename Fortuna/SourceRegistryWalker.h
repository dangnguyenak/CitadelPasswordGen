/*
	(c) Citadel Software Inc, 2003, www.citadelsoftware.ca
*/

// This is a test source to send data to the pools
#pragma once

#include "Source.h"

namespace CitadelSoftwareInc {

class Fortuna;
class SourceMgr;
class Pool;

// Walk the registry tree starting from a specified subkey
class SourceRegistryWalker : public Source
{
	public:
		SourceRegistryWalker(	const std::string& sName,
								Fortuna* pFortuna, 
								SourceMgr* pOwner, 
								std::vector<Pool*>& Pools,
								bool bLogFile,						// log data to a file
								HKEY hKeyRoot,
								unsigned char ucSourceNum,
								const std::string& sSubKey = std::string());

		virtual ~SourceRegistryWalker();

		// derived classes must implement this to return their data
		virtual bool GetChaoticData(vecuc& vData);

		void AddSubkeyToAvoid(const std::string& subKey)
		{
			m_AvoidSubKeys.push_back(subKey);
		}

	private:
		HKEY m_hKeyRoot;		// this is the root key to start from, ie HKCR, HKLM etc
		std::string m_sSubKey;  // this is the subkey beneath the root to start from
		std::vector<std::string> m_AvoidSubKeys;		// top level subkeys to avoid

		std::vector<std::pair<std::string,DWORD> > m_CurrentSubKeys;		// sub key stack
		size_t m_CurrentSubKeysSize;

		HKEY m_hCurrentSubKey;							// handle to the currently open subkey

		HKEY m_hWorkingRootKey;		// this is the handle to the working key to start at

		LONG m_resultKeyRoot;	// result of the RegOpenKeyEx for m_hKeyRoot\\m_sSubKey
		DWORD m_dwValueQueryIndex;   // dwIndex value passed to RegEnumValue
		DWORD m_dwSubKeyQueryIndex;	 // for top level subkeys

		unsigned char* m_pValueData;		// holds the value data returned by RegEnumValue
		DWORD          m_dwValueDataLen;	// size of allocated value data in m_pValueData
		std::string    m_currentSubkey;		// useful for debugging to know which subkey is being processed

		// opens the current subkey using the colln of keys in m_CurrentSubKeys
		LONG OpenCurrentSubkey();

		// Calls OpenCurrentSubkey, handling the case when access is not granted to the key being opened
		LONG OpenCurrentSubkey2();

		LONG LookForNextSubkey(vecuc& vData);

		FILE* m_pLogFile;		// log file

		void WriteToLogFile(const char* pString);
		void WriteToLogFile(const char* ps1, const char* ps2);

		// return true if the subkey is not in the m_AvoidSubkeys list
		bool ValidSubkey(const std::string& sKey);

	private:
		SourceRegistryWalker();												// don't allow default ctor
		SourceRegistryWalker(const SourceRegistryWalker& x);				// don't allow copy ctor
		SourceRegistryWalker& operator=(const SourceRegistryWalker& x);		// don't allow operator =
};

}	// end namespace CitadelSoftwareInc