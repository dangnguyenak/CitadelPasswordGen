/*
	(c) Citadel Software Inc, 2003, www.citadelsoftware.ca
*/

// This is the 128 nonce counter object for CTR encryption mode
// There is an 8 byte nonce and an 8 byte counter
#pragma once

namespace CitadelSoftwareInc {

	/*! \brief 64 Bit Nonce and 64 Bit Counter used for CTR Encryption Mode
	*
	*/

	class CTRNonceCounter
	{
	public:
		CTRNonceCounter();

		CTRNonceCounter(const __int64& nonce, const __int64& counter);

		~CTRNonceCounter();

		void Next();

		void SetNonce(__int64& nonce) { m_state[0] = nonce; }
		void SetCounter(__int64& ctr) { m_state[1] = ctr; }

		const unsigned char* GetState() const {return (const unsigned char*)(&m_state[0]);}

		// generate random values for the nonce and the counter
		void Randomize();

		// add the state of the nonce and counter to the vector (used to serialize for the seed file
		void AddToVector(vecuc& vData);

	private:
		// m_state[0] is the nonce
		// m_state[1] is the counter
		__int64 m_state[2];

	};


}	// end namespace CitadelSoftwareInc