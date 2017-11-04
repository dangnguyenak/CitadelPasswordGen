/*
	(c) Citadel Software Inc, 2003, www.citadelsoftware.ca
*/

// Use the Microsoft Crypto API CryptGenRandom
#pragma once

#include "Source.h"
#include "CryptoRand.h"

namespace CitadelSoftwareInc {

class Fortuna;
class SourceMgr;
class Pool;

class SourceCryptoRand : public Source
{
	public:
		SourceCryptoRand(const std::string& sName, Fortuna* pFortuna, SourceMgr* pOwner, std::vector<Pool*>& Pools, unsigned char ucSourceNum);
		virtual ~SourceCryptoRand();

		// derived classes must implement this to return their data
		virtual bool GetChaoticData(vecuc& vData);

	private:
		CryptoRand m_CryptoRand;
		vecuc      m_vData;

	private:
		SourceCryptoRand();										// don't allow default ctor
		SourceCryptoRand(const SourceCryptoRand& x);					// don't allow copy ctor
		SourceCryptoRand& operator=(const SourceCryptoRand& x);			// don't allow operator =
};

}	// end namespace CitadelSoftwareInc