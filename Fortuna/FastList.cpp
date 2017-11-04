/*
	(c) Citadel Software Inc, 2003, www.citadelsoftware.ca
*/

/*!  \file FastList.cpp
*    \brief Fast linked list implementation with special erase method and test harness.
*
*
*/


#include "stdafx.h"
#include "FastList.h"

namespace CitadelSoftwareInc {

bool TestFastList001()
{
	FastList<unsigned char> fl;

	unsigned char i=0;
	for(i=0; i<10; ++i)
		fl.Add(i);

//	std::vector<unsigned char> ucvec;
	vecuc ucvec;

	fl.Extract(ucvec);

	unsigned char uc = 0;
	for (i=0; i<10; ++i)
	{
		uc = ucvec[i];
		assert(uc == i);
	}

	fl.ExtractAndErase(ucvec);

	uc = 0;
	for (i=0; i<10; ++i)
	{
		uc = ucvec[i];
		assert(uc == i);
	}

	assert(fl.Size() == 0);

	// reuse the list - there should be no new allocations
	for(i=0; i<10; ++i)
		fl.Add(i+10);
	
	fl.Extract(ucvec);

	uc = 0;
	for (i=0; i<10; ++i)
	{
		uc = ucvec[i];
		assert(uc-10 == i);
	}

	// add in 10 new items
	for(i=0; i<10; ++i)
		fl.Add(i+20);
	
	fl.Extract(ucvec);

	uc = 0;
	for (i=0; i<10; ++i)
	{
		uc = ucvec[i];
		assert(uc-10 == i);
	}

	for (i=10; i<20; ++i)
	{
		uc = ucvec[i];
		assert(uc-20 == i-10);
	}


	return true;
}

bool TestFastList()
{
	bool b1 = TestFastList001();

	return b1;
}

}	// end namespace CitadelSoftwareInc