/*
	(c) Citadel Software Inc, 2003, www.citadelsoftware.ca
*/

/*!  \file SourceTest.cpp
*    \brief Simple class used to obtain only timing info for source events (this class does nothing).
*
*
*/


#include "stdafx.h"
#include "SourceTest.h"
#include "Fortuna.h"


namespace CitadelSoftwareInc {

SourceTest::SourceTest(const std::string& sName, Fortuna* pFortuna, SourceMgr* pOwner, std::vector<Pool*>& Pools, unsigned char ucSourceNum)
	:
	Source(sName, pFortuna, pOwner, Pools, ucSourceNum)
{
}

SourceTest::~SourceTest()
{
}


// derived classes must implement this to return their data
bool SourceTest::GetChaoticData(vecuc& vData)
{
	// do nothing; the base class adds in the last byte from the difference in the high resolution timer
	// around the WaitForSingleObject(hShutdownEvent) call

	return true;
}


}	// end namespace CitadelSoftwareInc