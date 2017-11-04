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

class SourceTest : public Source
{
	public:
		SourceTest(const std::string& sName, Fortuna* pFortuna, SourceMgr* pOwner, std::vector<Pool*>& Pools, unsigned char ucSourceNum);
		virtual ~SourceTest();

		// derived classes must implement this to return their data
		virtual bool GetChaoticData(vecuc& vData);

		// return true if the derived class does not add any data in the GetChaoticData method
		virtual bool ZeroData() {return true; }

	private:
		SourceTest();										// don't allow default ctor
		SourceTest(const SourceTest& x);					// don't allow copy ctor
		SourceTest& operator=(const SourceTest& x);			// don't allow operator =
};

}	// end namespace CitadelSoftwareInc