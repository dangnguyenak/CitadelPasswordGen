/*
	(c) Citadel Software Inc, 2003, www.citadelsoftware.ca
*/

// This is a test source to send data to the pools
#pragma once

namespace CitadelSoftwareInc {

typedef std::vector<unsigned char>  vecuc;						// a vector of unsigned chars
typedef std::vector<std::vector<unsigned char> > vecvecuc;		// a vector of vectors of unsigned chars

typedef std::deque<unsigned char> dequc;						// a deque of unsigned chars

typedef std::vector<std::pair<unsigned char, int> > vecucint;	// vector of uc,int pairs

}	// end namespace CitadelSoftwareInc