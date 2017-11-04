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

// iterate through clsid's in the registry
class SourceClsid : public Source
{
	public:
		SourceClsid(const std::string& sName, Fortuna* pFortuna, SourceMgr* pOwner, std::vector<Pool*>& Pools, unsigned char ucSourceNum);
		virtual ~SourceClsid();

		// derived classes must implement this to return their data
		virtual bool GetChaoticData(std::vector<unsigned char>& vData);

	private:
		HKEY m_hKeyClsid;		// handle to HKCR\\Clsid
		LONG m_resultClsid;		// result of the RegOpenKeyEx for HKCR\\Clsid
		DWORD m_dwQueryIndex;   // dwIndex value passed to RegEnumKeyEx
		unsigned char m_guidvec[16];		// holds the data for a guid
		int   m_numberClsids;	// total number of clsid's present in the registry
		unsigned int m_numberClsidsProcessed;		// reset each time through the clsid's (number for this cycle)

	private:
		SourceClsid();										// don't allow default ctor
		SourceClsid(const SourceClsid& x);					// don't allow copy ctor
		SourceClsid& operator=(const SourceClsid& x);		// don't allow operator =
};

}	// end namespace CitadelSoftwareInc