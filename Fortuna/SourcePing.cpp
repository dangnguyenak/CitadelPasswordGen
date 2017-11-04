/*
	(c) Citadel Software Inc, 2003, www.citadelsoftware.ca
*/

/*!  \file SourcePing.cpp
*    \brief Registry HKCR/Ping source event data.
*
*
*/


#include "stdafx.h"
#include "SourcePing.h"
#include "Fortuna.h"
#include "Timer.h"
#include "FortunaUtils.h"

namespace CitadelSoftwareInc {



SourcePing::SourcePing(const std::string& sName, Fortuna* pFortuna, SourceMgr* pOwner, std::vector<Pool*>& Pools, unsigned char ucSourceNum)
	:
	Source(sName, pFortuna, pOwner, Pools, ucSourceNum),
	m_wsaerror(0),
	m_nRetries(4),
	m_numGoodPings(0),
	m_numBadPings(0)
{
#ifdef LOGPINGDATA
	const size_t logsize = 1024*1024;
	DWORD dwWriteFreqSec = 15 * 60;
	m_SocketLog      = new DelayedWriteFile(std::string("Ping-SocketLog.bin"), logsize, dwWriteFreqSec);
	m_PingTimeoutLog = new DelayedWriteFile(std::string("Ping-TimoutLog.bin"), logsize, dwWriteFreqSec);
	m_PingLog        = new DelayedWriteFile(std::string("Ping-Log.bin"), logsize, dwWriteFreqSec);
#endif

}


SourcePing::~SourcePing()
{
#ifdef LOGPINGDATA
	delete m_SocketLog;
	delete m_PingTimeoutLog;
	delete m_PingLog;
#endif
}


// return the chaotic deterministic data to be entered into the pools
// This class will enter all of the bytes from the guids
bool SourcePing::GetChaoticData(std::vector<unsigned char>& vData)
{
	bool bRetVal = true;

	SOCKET	  rawSocket;
	UINT	  nLoop;
	int       nRet;
	struct    sockaddr_in saDest;
	struct    sockaddr_in saSrc;
	DWORD	  dwTimeSent;
	DWORD	  dwElapsed;
	u_char    cTTL;

	Timer hptimer;

	hptimer.Start();
	rawSocket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	hptimer.Stop();
	hptimer.AddElapsedTimeDifference(vData);
#ifdef LOGPINGDATA
	{
		unsigned char byte = (unsigned char)(hptimer.GetLowByteTimeDiff() & 0xff);
		m_SocketLog->Add(byte);
	}
#endif

	if (rawSocket == SOCKET_ERROR) 
	{
		m_wsaerror = WSAGetLastError();
		return false;
	}

	// TODO - use Fortuna to get these random values
	// generate a new ip address
	int i1 = rand() % 256;
	int i2 = rand() % 256;
	int i3 = rand() % 256;
	int i4 = rand() % 256;

	char ipaddr[32] = {'\0'};
	sprintf(ipaddr,"%d.%d.%d.%d",i1, i2, i3, i4);
//	printf("Ping: %s\n", ipaddr);

	unsigned long addr = inet_addr(ipaddr);

	// Setup destination socket address
	saDest.sin_addr.s_addr = addr;

	saDest.sin_family = AF_INET;
	saDest.sin_port = 0;

	// Ping multiple times
	for (nLoop = 0; nLoop < (UINT)m_nRetries; nLoop++)
	{
		if (ShuttingDown())
			return false;

		// Send ICMP echo request
		hptimer.Start();
		SendEchoRequest(rawSocket, &saDest);

		nRet = WaitForEchoReply(rawSocket);
		hptimer.Stop();
		hptimer.AddElapsedTimeDifference(vData);
		if (ShuttingDown())
			return false;

		if (nRet == SOCKET_ERROR)
		{
			bRetVal = false;
			break;
		}
		if (!nRet)
		{
//			str.Format("Request Timed Out");
//			::PostMessage(m_hWnd,WM_MSG_STATUS, 3, (LPARAM) AllocBuffer(str));

			if (nLoop == 0)
				m_numBadPings++;

//			unsigned int uc = hptimer.GetLowByteTimeDiff();
//			printf("request timed out, low byte time diff = %x\n", uc);
#ifdef FORTUNAMONITOR
		{
			if (nLoop == 0)
			{
				char buffer[256] = {'\0'};
				sprintf(buffer,"%s : timed out : ", ipaddr);
				size_t size = vData.size();
				std::string sText = BinaryToString(&vData[0], (int)size);
				std::string sPeek = std::string(buffer) + sText;
				SetPeekString(sPeek);
			}
		}
#endif

#ifdef LOGPINGDATA
	{
		unsigned char byte = (unsigned char)(hptimer.GetLowByteTimeDiff() & 0xff);
		m_PingTimeoutLog->Add(byte);
	}
#endif
		}
		else
		{
			if (nLoop == 0)
				m_numGoodPings++;

			// Receive reply
			hptimer.Start();
			dwTimeSent = RecvEchoReply(rawSocket, &saSrc, &cTTL);

			// Calculate elapsed time
			dwElapsed = GetTickCount() - dwTimeSent;
//			printf("Ping %s - elapsed time is %u ms\n", ipaddr, dwElapsed);

#ifdef FORTUNAMONITOR
		{
			if (nLoop == 0)
			{
				char buffer[256] = {'\0'};
				sprintf(buffer,"%s : %d ms", ipaddr, dwElapsed );
				SetPeekString(std::string(buffer));
			}
		}
#endif

			hptimer.Stop();
			hptimer.AddElapsedTimeDifference(vData);
#ifdef LOGPINGDATA
	{
		unsigned char byte = (unsigned char)(hptimer.GetLowByteTimeDiff() & 0xff);
		m_PingLog->Add(byte);
	}
#endif
			AddBinaryData3(vData, (const unsigned char*)&dwElapsed, sizeof(DWORD));
			if (ShuttingDown())
				return false;
		}
	}
	
	hptimer.Start();
	nRet = closesocket(rawSocket);
	hptimer.Stop();
	hptimer.AddElapsedTime(vData);

	if (nRet == SOCKET_ERROR)
		bRetVal = false;

	return bRetVal;
}


int SourcePing::SendEchoRequest(SOCKET s,LPSOCKADDR_IN lpstToAddr) 
{
	static ECHOREQUEST echoReq;
	static nId = 1;
	static nSeq = 1;
	int nRet=0;

#define ICMP_ECHOREQ 8

	// Fill in echo request
	echoReq.icmpHdr.Type		= ICMP_ECHOREQ;
	echoReq.icmpHdr.Code		= 0;
	echoReq.icmpHdr.Checksum	= 0;
	echoReq.icmpHdr.ID			= nId++;
	echoReq.icmpHdr.Seq			= nSeq++;

	// Fill in some data to send
	for (nRet = 0; nRet < REQ_DATASIZE; nRet++)
		echoReq.cData[nRet] = ' '+nRet;

	// Save tick count when sent
	echoReq.dwTime				= GetTickCount();

	// Put data in packet and compute checksum
	echoReq.icmpHdr.Checksum = in_cksum((u_short *)&echoReq, sizeof(ECHOREQUEST));

	// Send the echo request  								  
	nRet = sendto(s,						/* socket */
				 (LPSTR)&echoReq,			/* buffer */
				 sizeof(ECHOREQUEST),
				 0,							/* flags */
				 (LPSOCKADDR)lpstToAddr, /* destination */
				 sizeof(SOCKADDR_IN));   /* address length */

//	if (nRet == SOCKET_ERROR) 
//		WSAError("sendto()");


	return (nRet);
}


DWORD SourcePing::RecvEchoReply(SOCKET s, LPSOCKADDR_IN lpsaFrom, u_char *pTTL) 
{
	ECHOREPLY echoReply;
	int nRet;
	int nAddrLen = sizeof(struct sockaddr_in);

	// Receive the echo reply	
	nRet = recvfrom(s,					// socket
					(LPSTR)&echoReply,	// buffer
					sizeof(ECHOREPLY),	// size of buffer
					0,					// flags
					(LPSOCKADDR)lpsaFrom,	// From address
					&nAddrLen);			// pointer to address len

	// Check return value
//	if (nRet == SOCKET_ERROR) 
//		WSAError("recvfrom()");

	// return time sent and IP TTL
	*pTTL = echoReply.ipHdr.TTL;

	return(echoReply.echoRequest.dwTime);   		
}


int SourcePing::WaitForEchoReply(SOCKET s)
{
	struct timeval Timeout;
	fd_set readfds;

	readfds.fd_count = 1;
	readfds.fd_array[0] = s;
	Timeout.tv_sec = 1;
    Timeout.tv_usec = 0;

	return(select(1, &readfds, NULL, NULL, &Timeout));
}



//
// Mike Muuss' in_cksum() function
// and his comments from the original
// ping program
//
// * Author -
// *	Mike Muuss
// *	U. S. Army Ballistic Research Laboratory
// *	December, 1983

/*
 *			I N _ C K S U M
 *
 * Checksum routine for Internet Protocol family headers (C Version)
 *
 */
u_short SourcePing::in_cksum(u_short *addr, int len)
{
	register int nleft = len;
	register u_short *w = addr;
	register u_short answer;
	register int sum = 0;

	/*
	 *  Our algorithm is simple, using a 32 bit accumulator (sum),
	 *  we add sequential 16 bit words to it, and at the end, fold
	 *  back all the carry bits from the top 16 bits into the lower
	 *  16 bits.
	 */
	while( nleft > 1 )  {
		sum += *w++;
		nleft -= 2;
	}

	/* mop up an odd byte, if necessary */
	if( nleft == 1 ) {
		u_short	u = 0;

		*(u_char *)(&u) = *(u_char *)w ;
		sum += u;
	}

	/*
	 * add back carry outs from top 16 bits to low 16 bits
	 */
	sum = (sum >> 16) + (sum & 0xffff);	/* add hi 16 to low 16 */
	sum += (sum >> 16);			/* add carry */
	answer = ~sum;				/* truncate to 16 bits */
	return (answer);
}


}	// end namespace CitadelSoftwareInc