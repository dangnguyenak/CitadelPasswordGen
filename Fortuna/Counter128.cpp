/*
	(c) Citadel Software Inc, 2003, www.citadelsoftware.ca
*/

/*!  \file Counter128.cpp
*    \brief A 128 bit counter.
*
*
*/


#include "stdafx.h"
#include "Counter128.h"


namespace CitadelSoftwareInc {

Counter128::Counter128()
{
	m_state[0] = 0;
	m_state[1] = 0;
}


Counter128::~Counter128()
{
}

void Counter128::Next()
{
	m_critsec.Enter();

	++m_state[0];
	if (m_state[0] == 0xffffffffffffffff)
	{
		++m_state[1];
		m_state[0] = 0;
	}

	m_critsec.Leave();
}

bool Counter128::SetState(vecuc& vState)
{
	if (vState.size() != 16)
	{
		assert(0);
		return false;
	}

	m_critsec.Enter();

	unsigned char* pData = (unsigned char*)&m_state[0];
	for (int i=0; i<16; ++i)
		pData[i] = vState[i];

	m_critsec.Leave();

//	std::string sTest = ToString();

	return true;
}

#ifdef FORTUNAMONITOR
		//! Return the state of the Counter as a string
std::string Counter128::ToString()
{
	std::string sRetVal;

	char buffer[129] = {'\0'};

	m_critsec.Enter();

	unsigned char *pData = (unsigned char*)&m_state[0];
	for (int i=0; i<16; ++i)
	{
		sprintf(buffer+ i*2, "%.2x", pData[15-i]);
	}

	m_critsec.Leave();

	sRetVal = std::string(buffer);

	return sRetVal;
}

#endif



}	// end namespace CitadelSoftwareInc