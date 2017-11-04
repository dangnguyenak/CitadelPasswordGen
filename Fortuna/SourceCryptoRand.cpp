/*
	(c) Citadel Software Inc, 2003, www.citadelsoftware.ca
*/

/*!  \file SourceCryptoRand.cpp
*    \brief Microsoft Crypto API Random Generator Source events.
*
*
*/


#include "stdafx.h"
#include "SourceCryptoRand.h"
#include "Fortuna.h"
#include "FortunaUtils.h"


namespace CitadelSoftwareInc {

SourceCryptoRand::SourceCryptoRand(const std::string& sName, Fortuna* pFortuna, SourceMgr* pOwner, std::vector<Pool*>& Pools, unsigned char ucSourceNum)
	:
	Source(sName, pFortuna, pOwner, Pools, ucSourceNum),
	m_CryptoRand()
{
	bool bCtor = m_CryptoRand.GetCtorStatus();
	if (!bCtor)
	{
		assert(0);
	}

	m_dwExtraSleep = 100;		// protected member from Source to add extra sleep delay 
}

SourceCryptoRand::~SourceCryptoRand()
{
}


// derived classes must implement this to return their data
bool SourceCryptoRand::GetChaoticData(vecuc& vData)
{
	// do nothing; the base class adds in the last byte from the difference in the high resolution timer
	// around the WaitForSingleObject(hShutdownEvent) call
	size_t size = 24;
	m_vData.resize(size);

	bool bStatus = m_CryptoRand.GetRandomBytes(m_vData);

	if (!bStatus)
	{
		assert(0);
		return false;
	}

	unsigned char uc=0;
	unsigned int sum=0;
	int entropy=0;
	for(size_t i=0; i<size; ++i)
	{
		uc = m_vData[i];
		sum += uc;
		entropy = entropy ? 0 : 1;
		vData.push_back(uc);
	}

#ifdef FORTUNAMONITOR
	{
		size_t size = vData.size();
		std::string sText = BinaryToString(&vData[0], (int)size);
		SetPeekString(sText);
	}
#endif

	// make the extra sleep a function of the random bytes
	m_dwExtraSleep = 200 + sum % 100;		// protected member from Source base class

	return true;
}


}	// end namespace CitadelSoftwareInc