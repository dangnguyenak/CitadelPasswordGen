/*
	(c) Citadel Software Inc, 2003, www.citadelsoftware.ca
*/

// This is a test source to send data to the pools
#pragma once

//#define LOGPINGDATA
#include "Source.h"

#ifdef LOGPINGDATA
#include "DelayedWriteFile.h"
#endif

namespace CitadelSoftwareInc {

class Fortuna;
class SourceMgr;
class Pool;



typedef unsigned short u_short;

// Ping random ip addresses and time the results
class SourcePing : public Source
{
	public:
		SourcePing(	const std::string& sName, 
					Fortuna* pFortuna, 
					SourceMgr* pOwner, 
					std::vector<Pool*>& Pools, 
					unsigned char ucSourceNum);

		virtual ~SourcePing();

		// derived classes must implement this to return their data
		virtual bool GetChaoticData(std::vector<unsigned char>& vData);

	private:

	private:
		SourcePing();									// don't allow default ctor
		SourcePing(const SourcePing& x);				// don't allow copy ctor
		SourcePing& operator=(const SourcePing& x);		// don't allow operator =

		int m_wsaerror;									// Socket Error from WSAGetLastError
		int m_nRetries;									// number of times to ping
		int m_numGoodPings;
		int m_numBadPings;

		DWORD RecvEchoReply(SOCKET s, LPSOCKADDR_IN lpsaFrom, u_char *pTTL);
		int   SendEchoRequest(SOCKET s,LPSOCKADDR_IN lpstToAddr);
		int   WaitForEchoReply(SOCKET s);

		u_short in_cksum(u_short *addr, int len);

#ifdef LOGPINGDATA
		DelayedWriteFile* m_SocketLog;
		DelayedWriteFile* m_PingTimeoutLog;
		DelayedWriteFile* m_PingLog;
#endif

};


// IP Header -- RFC 791
typedef struct tagIPHDR
{
	u_char  VIHL;			// Version and IHL
	u_char	TOS;			// Type Of Service
	short	TotLen;			// Total Length
	short	ID;				// Identification
	short	FlagOff;		// Flags and Fragment Offset
	u_char	TTL;			// Time To Live
	u_char	Protocol;		// Protocol
	u_short	Checksum;		// Checksum
	struct	in_addr iaSrc;	// Internet Address - Source
	struct	in_addr iaDst;	// Internet Address - Destination
}IPHDR, *PIPHDR;


// ICMP Header - RFC 792
typedef struct tagICMPHDR
{
	u_char	Type;			// Type
	u_char	Code;			// Code
	u_short	Checksum;		// Checksum
	u_short	ID;				// Identification
	u_short	Seq;			// Sequence
	char	Data;			// Data
}ICMPHDR, *PICMPHDR;


#define REQ_DATASIZE 32		// Echo Request Data size

// ICMP Echo Request
typedef struct tagECHOREQUEST
{
	ICMPHDR icmpHdr;
	DWORD	dwTime;
	char	cData[REQ_DATASIZE];
}ECHOREQUEST, *PECHOREQUEST;


// ICMP Echo Reply
typedef struct tagECHOREPLY
{
	IPHDR	ipHdr;
	ECHOREQUEST	echoRequest;
	char    cFiller[256];
}ECHOREPLY, *PECHOREPLY;


}	// end namespace CitadelSoftwareInc