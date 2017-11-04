/*
	(c) Citadel Software Inc, 2003, www.citadelsoftware.ca
*/

// This is a test source to send data to the pools
#pragma once

#include "CMclCritSec.h"

namespace CitadelSoftwareInc {

	/*! \brief 128 Bit Counter, uses two int64's
	*
	*/
	class Counter128
	{
	public:
		Counter128();

		~Counter128();

		void Next();

		const unsigned char* GetState() const {return (const unsigned char*)(&m_state[0]);}

		bool SetState(vecuc& vState);

		bool IsZero() const { return m_state[0] == 0 && m_state[1] == 0 ? true : false; }

#ifdef FORTUNAMONITOR
		//! Return the state of the Counter as a string
		std::string ToString();
#endif

	private:
		__int64 m_state[2];
		CMclCritSec m_critsec;

	};


}	// end namespace CitadelSoftwareInc