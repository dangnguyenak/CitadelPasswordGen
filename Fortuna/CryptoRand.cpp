/*
	(c) Citadel Software Inc, 2003, www.citadelsoftware.ca
*/

/*!  \file CryptoRand.cpp
*    \brief Wrapper around the Microsoft Crypto API CryptGenRandom call for generating random numbers.
*
*
*/


#include "stdafx.h"
#include "CryptoRand.h"


namespace CitadelSoftwareInc {

CryptoRand::CryptoRand()
	:
	m_hProv(NULL),
	m_bCtor(false)
{
	BOOL bStatus = CryptAcquireContext(&m_hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT);
	if (bStatus)
		m_bCtor = true;
}


CryptoRand::~CryptoRand()
{
	if (m_hProv)
	{
		CryptReleaseContext(m_hProv,0);
	}
}

// Fills in vData with random bytes.
// vData must be sized by the caller
bool CryptoRand::GetRandomBytes(std::vector<unsigned char>& vData)
{
	if (vData.empty())
		return false;

	DWORD dwLen = (DWORD)vData.size();
	BOOL bStatus = CryptGenRandom(m_hProv, dwLen, &vData[0]);

	return bStatus ? true : false;
}



}	// end namespace CitadelSoftwareInc