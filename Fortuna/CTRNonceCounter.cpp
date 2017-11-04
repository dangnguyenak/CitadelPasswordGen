/*
	(c) Citadel Software Inc, 2003, www.citadelsoftware.ca
*/


/*!  \file CTRNonceCounter.cpp
*    \brief 64 bit nonce and 64 bit counter to use for encryption in CTR mode
*
*/

#include "stdafx.h"
#include "CTRNonceCounter.h"
#include "FortunaUtils.h"

namespace CitadelSoftwareInc {

CTRNonceCounter::CTRNonceCounter()
{
	m_state[0] = 0;
	m_state[1] = 0;
}


CTRNonceCounter::CTRNonceCounter(const __int64& nonce, const __int64& counter)
{
	m_state[0] = nonce;
	m_state[1] = counter;
}

CTRNonceCounter::~CTRNonceCounter()
{
}

void CTRNonceCounter::Next()
{
	++m_state[0];
}

void CTRNonceCounter::Randomize()
{
	GetRandomValue<__int64>(m_state[0]);
	GetRandomValue<__int64>(m_state[1]);
}

// add the state of the nonce and counter to the vector (used to serialize for the seed file
void CTRNonceCounter::AddToVector(vecuc& vData)
{
	CitadelSoftwareInc::AddToVector<__int64>(vData, m_state[0]);
	CitadelSoftwareInc::AddToVector<__int64>(vData, m_state[1]);
}


}	// end namespace CitadelSoftwareInc