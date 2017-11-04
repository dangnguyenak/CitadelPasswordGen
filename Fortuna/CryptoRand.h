/*
	(c) Citadel Software Inc, 2003, www.citadelsoftware.ca
*/

// Random numbers using the Windows Crypto API
#pragma once

#include <wincrypt.h>

namespace CitadelSoftwareInc {

	/*! \brief Wrapper for Win32 Crypto API, ensures handle to context is released
	*
	*/

	class CryptoRand
	{
	public:
		CryptoRand();

		~CryptoRand();

		bool GetCtorStatus() {return m_bCtor; }

		// Fills in vData with random bytes.
		// vData must be sized by the caller
		bool GetRandomBytes(std::vector<unsigned char>& vData);

	private:
		HCRYPTPROV m_hProv;
		bool       m_bCtor;			// set to the return of CryptAcquireContext
	};


}	// end namespace CitadelSoftwareInc