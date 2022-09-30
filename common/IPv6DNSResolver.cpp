#include "StdAfx.h"
#include "IPv6DNSResolver.h"

#include <strsafe.h>
/********************
consts
***********************/
const CHAR	CIPv6DNSResolver::GOOGLE_DNS_SERVER[]="8.8.8.8";


CIPv6DNSResolver::CIPv6DNSResolver(void)
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
}

CIPv6DNSResolver::~CIPv6DNSResolver(void)
{
	 WSACleanup();
}
/*******************************
Resolves the ipv6 address
*********************************/
BOOL CIPv6DNSResolver::IsResolvable(LPCTSTR lpszDNSName)
{
	struct addrinfo		*result = NULL;
    struct addrinfo		*ptr = NULL;
	struct addrinfo		hints={0};
	DWORD				dwRetVal=0;
	CHAR				szDNSNameA[MAX_PATH]={0};

	StringCbPrintfA(szDNSNameA,sizeof(szDNSNameA),"%S",lpszDNSName);
	//--------------------------------
    // Setup the hints address info structure
    // which is passed to the getaddrinfo() function
    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
	//--------------------------------
	// Call getaddrinfo(). If the call succeeds,
	// the result variable will hold a linked list
	// of addrinfo structures containing response
	// information
    dwRetVal = getaddrinfo(szDNSNameA, NULL, &hints, &result);
    if ( dwRetVal != 0 ) 
	{
		DEBUGLOG(LOI_ERROR,_T("getaddrinfo failed to resolve %s GetLastError %d"),lpszDNSName,dwRetVal);
        return FALSE;
    }
   
    freeaddrinfo(result);
	return TRUE;
}
/***************************************
Manually send UDP packet to resolve address
***************************************/
BOOL CIPv6DNSResolver::ManualResolve(LPCTSTR lpszDNSName,in_addr6 &IPv6Addr)
{
	SOCKET					DNSSocket=INVALID_SOCKET;
	struct sockaddr_in		DNSServerDes={0};
	BYTE					byDNSPacket[8048]={0};
	DWORD					dwRequestSize=0;
	BOOL					bRet=FALSE;
	HANDLE					hRecvEvent=WSACreateEvent(); 
	int						iRetVal=0;

	DNSSocket = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP); //UDP packet for DNS queries
	if(DNSSocket==INVALID_SOCKET)
	{
		goto cleanup;
	}

	DNSServerDes.sin_family=AF_INET;
	DNSServerDes.sin_port=htons(53);
	DNSServerDes.sin_addr.s_addr=inet_addr(GOOGLE_DNS_SERVER); //dns servers

	//form the request & send it
	if(dwRequestSize=FillDNSRequest(lpszDNSName,byDNSPacket,sizeof(byDNSPacket)))
	{
		int iRetVal=0;
		//send the packet
		iRetVal=sendto(DNSSocket,(char*)byDNSPacket,dwRequestSize,0,(sockaddr*)&DNSServerDes,sizeof(DNSServerDes));   
		if(iRetVal==SOCKET_ERROR)
		{
			DEBUGLOG(LOI_ERROR,_T("ManualResolve sendto failed %s GetLastError %d"),lpszDNSName,WSAGetLastError());
			goto cleanup;
		}
	}

	//wait for recv data
	WSAEventSelect(DNSSocket,hRecvEvent,FD_READ);  
	if(WaitForSingleObject(hRecvEvent,10000)!=WAIT_OBJECT_0)
	{
		DEBUGLOG(LOI_ERROR,_T("ManualResolve recvfrom failed %s"),lpszDNSName);
		goto cleanup;
	}

	//recv the data
	iRetVal=recvfrom(DNSSocket,(char*)byDNSPacket,sizeof(byDNSPacket),0,NULL,NULL);
	if(iRetVal==SOCKET_ERROR)
	{
		DEBUGLOG(LOI_ERROR,_T("ManualResolve recvfrom failed %s GetLastError %d"),lpszDNSName,WSAGetLastError());
		goto cleanup;
	}

	//process the reply
	if(ProcessReply(byDNSPacket,(DWORD)iRetVal,dwRequestSize,IPv6Addr))
	{
		bRet=TRUE;
	}
cleanup:
	if(DNSSocket!=INVALID_SOCKET)
	{
		WSAEventSelect(DNSSocket,hRecvEvent,0); 
		closesocket(DNSSocket);
		DNSSocket=INVALID_SOCKET;
	}
	if(hRecvEvent)
	{
		WSACloseEvent(hRecvEvent);
		hRecvEvent=0;
	}
	return bRet;
}
/******************************
process the dns reply unpack the data
********************************/
BOOL CIPv6DNSResolver::ProcessReply(BYTE *byReply,DWORD dwReplySize,DWORD dwRequestSize,in_addr6 &IPv6Addr)
{
	BYTE					*pAddresses=NULL;	
	struct R_DATA			*pResponseData=NULL;
	BOOL					 bRet=FALSE;

	try{
		//move ahead of the dns header and the query field
		pResponseData=(struct R_DATA*)(byReply+dwRequestSize+2);

		//take only the first reponse
		if(ntohs(pResponseData->type) == 0x1c) //if its an ipv6 address
		{
			if(ntohs(pResponseData->data_len)==sizeof(in_addr6))
			{	
				memcpy(&IPv6Addr, ((BYTE*)pResponseData) + sizeof(struct R_DATA),sizeof(in_addr6));
				bRet=TRUE;
			}
		}
	}//try
	catch(...)
	{
	}
	return bRet;
}

/**********************
Fill the DNS request
**************************/
DWORD CIPv6DNSResolver::FillDNSRequest(LPCTSTR lpszDNSName,BYTE *byPacket,DWORD dwLen)
{
	DWORD					dwRequestSize=0;
	struct DNS_HEADER		*pDNSRequest = NULL;
	CHAR					 szDnsNameA[MAX_PATH]={0};
	PCHAR					 pRequestName=NULL;
	struct QUESTION			*pQinfo = NULL;

	dwRequestSize=(sizeof(struct DNS_HEADER)+_tcslen(lpszDNSName)+2+sizeof(struct QUESTION));
	if(dwLen< dwRequestSize)
	{
		DEBUGLOG(LOI_ERROR,_T("FillDNSRequest size not enough"));
		return 0;
	}

	StringCbPrintfA(szDnsNameA,sizeof(szDnsNameA),"%S",lpszDNSName);
	pDNSRequest=(DNS_HEADER*)byPacket;

	pDNSRequest->id = (unsigned short) htons((SHORT)GetCurrentProcessId());
	pDNSRequest->qr = 0; //This is a query
	pDNSRequest->opcode = 0; //This is a standard query
	pDNSRequest->aa = 0; //Not Authoritative
	pDNSRequest->tc = 0; //This message is not truncated
	pDNSRequest->rd = 1; //Recursion Desired
	pDNSRequest->ra = 0; //Recursion not available! hey we dont have it (lol)
	pDNSRequest->z = 0;
	pDNSRequest->ad = 0;
	pDNSRequest->cd = 0;
	pDNSRequest->rcode = 0;
	pDNSRequest->q_count = htons(1); //we have only 1 question
	pDNSRequest->ans_count = 0;
	pDNSRequest->auth_count = 0;
	pDNSRequest->add_count = 0;

	//point to the query portion
	pRequestName =(PCHAR)(byPacket+sizeof(struct DNS_HEADER));

	ChangetoDnsNameFormat(pRequestName,szDnsNameA);

	pQinfo =(struct QUESTION*)&byPacket[ (sizeof(struct DNS_HEADER) + (strlen((const char*)pRequestName) + 1))]; //fill it

	pQinfo->qtype = htons(0x1c); //we are requesting the ipv6 address
	pQinfo->qclass = htons(1); //its internet (lol)

	return dwRequestSize;
}
///
//this will convert www.google.com to 3www6google3com ;got it <img src="http://www.binarytides.com/blog/wp-includes/images/smilies/icon_smile.gif" alt=":)" class="wp-smiley">
///
void CIPv6DNSResolver::ChangetoDnsNameFormat(char* dns, char* host)
{
	int lock=0 , i;

	StringCbCatA(host,MAX_PATH,".");

	for(i=0;i<(int)strlen((char*)host);i++)
	{
		if(host[i]=='.')
		{
			*dns++=i-lock;
			for(;lock<i;lock++)
			{
				*dns++=host[lock];
			}
			lock++;
		}
	}
	*dns++=0;
}