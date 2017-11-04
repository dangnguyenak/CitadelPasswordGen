/*
	(c) Citadel Software Inc, 2003, www.citadelsoftware.ca
*/

#pragma once



namespace CitadelSoftwareInc {

class ProcessData;

class ProcessInfoBase
{
	public:
		ProcessInfoBase()
		{}

		virtual bool GetData(vecuc& vData, DWORD dwPid, ProcessData* pProcessData) = 0;
};

class ProcessInfoIoCounter : public ProcessInfoBase
{
	public:
		ProcessInfoIoCounter()
			:
		ProcessInfoBase()
		{}

		virtual bool GetData(vecuc& vData, DWORD dwPid, ProcessData* pProcessData);

};

///////////////////////////////////////////////////////////////////////
class ProcessInfoTimes : public ProcessInfoBase
{
	public:
		ProcessInfoTimes()
			:
		ProcessInfoBase()
		{}

		virtual bool GetData(vecuc& vData, DWORD dwPid, ProcessData* pProcessData);

};


///////////////////////////////////////////////////////////////////////
class ProcessInfoMemory : public ProcessInfoBase
{
	public:
		ProcessInfoMemory()
			:
		ProcessInfoBase()
		{}

		virtual bool GetData(vecuc& vData, DWORD dwPid, ProcessData* pProcessData);

};


///////////////////////////////////////////////////////////////////////
class ProcessInfoPerformance : public ProcessInfoBase
{
	public:
		ProcessInfoPerformance()
			:
		ProcessInfoBase()
		{}

		virtual bool GetData(vecuc& vData, DWORD dwPid, ProcessData* pProcessData);

};


///////////////////////////////////////////////////////////////////////
class ProcessInfoQueryWorkingSet : public ProcessInfoBase
{
	public:
		ProcessInfoQueryWorkingSet()
			:
		ProcessInfoBase()
		{}

		virtual bool GetData(vecuc& vData, DWORD dwPid, ProcessData* pProcessData);

};


}	// end namespace CitadelSoftwareInc