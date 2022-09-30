#include "StdAfx.h"
#include "LSPSocket.h"
#include "LOIUDPSocket.h"



CLOIUDPSocket::CLOIUDPSocket(PROVIDER *Provider ):CLSPOverlappedSocket(Provider) 
{
	Init();
}

CLOIUDPSocket::CLOIUDPSocket (PROVIDER *Provider ,SOCKET ProviderSocket):CLSPOverlappedSocket(Provider,ProviderSocket) 
{
	Init();
}

void CLOIUDPSocket::Init()
{
	m_pSessionContainer= CLSPSessionContainer::GetInstance();
	ZeroMemory(&m_WSPBindInfo,sizeof(m_WSPBindInfo));
	ZeroMemory(&m_WSPConnectInfo,sizeof(m_WSPConnectInfo));
	m_bSockBoundToIpv6=FALSE;
}
void  CLOIUDPSocket::Inherit( CLSPSocket *pAssignee)
{
	//copy all existsing parameters to this object
	CLOIUDPSocket		*pUdpSocket=dynamic_cast<CLOIUDPSocket*>(pAssignee);

	if(pUdpSocket)
	{
		m_bSockBoundToIpv6=pUdpSocket->m_bSockBoundToIpv6;

		m_WSPBindInfo=pUdpSocket->m_WSPBindInfo;
		m_WSPConnectInfo=pUdpSocket->m_WSPConnectInfo;

		m_BoundIPv6Addr=pUdpSocket->m_BoundIPv6Addr;
		m_sSessionID=pUdpSocket->m_sSessionID;
	}
	CLSPOverlappedSocket::Inherit(pAssignee);
}
CLOIUDPSocket::~CLOIUDPSocket(void)
{
}

int WSPAPI CLOIUDPSocket::WSPBind(
								const struct sockaddr FAR * name,
								int                   namelen,
								LPINT                 lpErrno
								)
{
	USHORT				usPort;
	DWORD				dwLocalBindAddress;   

	if(name==NULL || ((sockaddr_in*) name)->sin_family != AF_INET)//invalid parameters
	{
		goto dodefault;
	}

	usPort= ((sockaddr_in*)name)->sin_port;
	dwLocalBindAddress=((sockaddr_in*)name)->sin_addr.S_un.S_addr;

	if(m_pSessionContainer->IsBindToIPv6Required(usPort,m_BoundIPv6Addr,m_sSessionID))
	{
		if(CreateIPv6SockAndAssociate(SOCK_DGRAM,IPPROTO_UDP,lpErrno)==NO_ERROR)
		{	
			sockaddr_in6  SockAddripv6={0};
			int			  iRet=0;
			//store the original value
			m_bSockBoundToIpv6=TRUE;
			memcpy(&m_WSPBindInfo.name,name,sizeof(m_WSPBindInfo.name));
			m_WSPBindInfo.iNameLen = sizeof(m_WSPBindInfo.name);

			SockAddripv6.sin6_family = AF_INET6;
			SockAddripv6.sin6_addr = m_BoundIPv6Addr; 
			SockAddripv6.sin6_port = usPort;
			//bind to ipv6 address	
			iRet= CLSPOverlappedSocket::WSPBind((const struct sockaddr *)&SockAddripv6,sizeof(SockAddripv6),lpErrno);
			if(iRet==NO_ERROR && dwLocalBindAddress &&
				dwLocalBindAddress!=m_pSessionContainer->GetIpv4Address(m_sSessionID, m_BoundIPv6Addr)
				)
			{
				//application purposely binds to a local address
				//notify this to all clients
				m_pSessionContainer->AddLocalBoundInfo(m_sSessionID,dwLocalBindAddress,usPort, m_BoundIPv6Addr );
			}
				
			dbgprint(_T(" CLOIUDPSocket::WSPBind %X:%X to %X -- return %X LastError : %X"),dwLocalBindAddress,usPort,m_pSessionContainer->GetIpv4Address(m_sSessionID, SockAddripv6.sin6_addr) ,iRet,*lpErrno  );
			return iRet;
		}
	}
	//dodefault
dodefault:
	return CLSPOverlappedSocket::WSPBind(name,namelen,lpErrno);
}

int WSPAPI CLOIUDPSocket::WSPConnect(
						const struct sockaddr FAR * name,
						int                   namelen,
						LPWSABUF              lpCallerData,
						LPWSABUF              lpCalleeData,
						LPQOS                 lpSQOS,
						LPQOS                 lpGQOS,
						LPINT                 lpErrno
						)

{
	DWORD			dwLocalAddr=((sockaddr_in*) name)->sin_addr.S_un.S_addr;
	USHORT			usPort=((sockaddr_in*) name)->sin_port; 
	in_addr6		IPv6MappedTeredoAddress={0};
	USHORT			usDivertedPort;
	CONNECT_TYPE	Type;

	Type=m_pSessionContainer->IsConnectDivertRequired(
											dwLocalAddr,
											usPort,
											IPv6MappedTeredoAddress,
											usDivertedPort,
											m_sSessionID
											);
	if(Type==DIRECT || Type==PROXY)
	{	
		sockaddr_in6		SockAddripv6={0};
		int					iRet;
		if(CreateIPv6SockAndAssociate(SOCK_DGRAM ,IPPROTO_UDP,lpErrno)==NO_ERROR)
		{	
			SockAddripv6.sin6_family = AF_INET6;
			SockAddripv6.sin6_addr = IPv6MappedTeredoAddress;
			SockAddripv6.sin6_port = usPort;

			iRet=CLSPOverlappedSocket::WSPConnect((sockaddr*)&SockAddripv6,sizeof(SockAddripv6),lpCallerData,lpCalleeData,lpSQOS,lpGQOS,lpErrno);

			dbgprint(_T(" CLOIUDPSocket::WSPConnect %X:%X to %X -- return %X LastError : %X Provider Socket %X "),dwLocalAddr,usPort,m_pSessionContainer->GetIpv4Address(m_sSessionID,IPv6MappedTeredoAddress) ,iRet,*lpErrno,m_NextProviderSocket   );
			//store the actual adddr
			m_bSockBoundToIpv6=TRUE;
			memcpy(&m_WSPConnectInfo,name,namelen);
			m_WSPConnectInfo.iNameLen = namelen; 
			return iRet;
		}
	}

	//do default:
	return CLSPOverlappedSocket::WSPConnect(name,namelen,lpCallerData,lpCalleeData,lpSQOS,lpGQOS,lpErrno);
}

int WSPAPI CLOIUDPSocket::WSPSend(
								LPWSABUF        lpBuffers,
								DWORD           dwBufferCount,
								LPDWORD         lpNumberOfBytesSent,
								DWORD           dwFlags,
								LPWSAOVERLAPPED lpOverlapped,                             
								LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,   
								LPWSATHREADID   lpThreadId,                                 
								LPINT           lpErrno                                             
								)
{
	DWORD			dwLocalAddr= m_WSPConnectInfo.name.sin_addr.S_un.S_addr ;
	USHORT			usPort     = m_WSPConnectInfo.name.sin_port; 

	//broadcast to all available connected clients
	if(dwLocalAddr=0xffffffff && m_bSockBoundToIpv6)
	{
		BroadCastUDPToAll(usPort,lpBuffers,dwBufferCount);
		dbgprint(_T(" CLOIUDPSocket::WSPSend broadcast to all") );
	}
	
	
	//do default
	return CLSPOverlappedSocket::WSPSend(lpBuffers,dwBufferCount,
										lpNumberOfBytesSent,dwFlags,
										lpOverlapped,lpCompletionRoutine,lpThreadId,
										lpErrno);
}
int WSPAPI CLOIUDPSocket:: WSPSendTo(
								LPWSABUF        lpBuffers,
								DWORD           dwBufferCount,
								LPDWORD         lpNumberOfBytesSent,
								DWORD           dwFlags,
								const struct sockaddr FAR * lpTo,
								int             iToLen,
								LPWSAOVERLAPPED lpOverlapped,
								LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
								LPWSATHREADID   lpThreadId,
								LPINT           lpErrno
								)
{
	DWORD			dwLocalAddr=lpTo?((sockaddr_in*) lpTo)->sin_addr.S_un.S_addr:
									m_WSPConnectInfo.name.sin_addr.S_un.S_addr ;
	USHORT			usPort=lpTo?((sockaddr_in*) lpTo)->sin_port:
								m_WSPConnectInfo.name.sin_port; 
	in_addr6		IPv6MappedTeredoAddress={0};
	CONNECT_TYPE	Type;

	
	//if needs to be send thro ipv6
	Type=m_pSessionContainer->IsSentoDivertRequired(
											dwLocalAddr,
											usPort,
											IPv6MappedTeredoAddress,
											m_sSessionID
											);
	if(Type==DIRECT )
	{
		sockaddr_in6		SockAddripv6={0};
		int					iRet=NO_ERROR;
		if(!m_bSockBoundToIpv6)//check if sock is already diverted remeber we need to create ipv6 socket to sendto teredo
						iRet=CreateIPv6SockAndAssociate(SOCK_DGRAM ,IPPROTO_UDP,lpErrno);
		if(iRet==NO_ERROR)
		{
			m_bSockBoundToIpv6=TRUE;
			SockAddripv6.sin6_family = AF_INET6;
			SockAddripv6.sin6_addr = IPv6MappedTeredoAddress;
			SockAddripv6.sin6_port = usPort;
			dbgprint(_T(" CLOIUDPSocket::WSPSendTo %X:%X diverted to %X Provider Socket %X "),dwLocalAddr,usPort,m_pSessionContainer->GetIpv4Address(m_sSessionID,IPv6MappedTeredoAddress),m_NextProviderSocket  );
	
			return CLSPOverlappedSocket::WSPSendTo(lpBuffers,dwBufferCount,lpNumberOfBytesSent,dwFlags,
												(sockaddr*)&SockAddripv6,sizeof(SockAddripv6),
												lpOverlapped,
												lpCompletionRoutine, lpThreadId,
												lpErrno
												);

		}
	}//Type==DIRECT

	if(Type==BROADCAST || 
		m_bSockBoundToIpv6///if it is bound to us but ipv4 address not found then broadcast to all
		)
	{
		sockaddr_in6		SockAddripv6={0};
		int					iRet=NO_ERROR;
		if(!m_bSockBoundToIpv6)//check if sock is already diverted remeber we need to create ipv6 socket to sendto teredo
						iRet=CreateIPv6SockAndAssociate(SOCK_DGRAM ,IPPROTO_UDP,lpErrno);

		if(iRet==NO_ERROR)
		{
			m_bSockBoundToIpv6=TRUE;

			//broadcast to all available connected clients
			BroadCastUDPToAll(usPort,lpBuffers,dwBufferCount);

			//sent to ipv6 broadcast address
			ZeroMemory(&SockAddripv6,sizeof(SockAddripv6));
			SockAddripv6.sin6_family = AF_INET6;
			memset (&SockAddripv6.sin6_addr,0xff,sizeof(SockAddripv6.sin6_addr));
			SockAddripv6.sin6_port = usPort;

			return CLSPOverlappedSocket::WSPSendTo(lpBuffers,dwBufferCount,lpNumberOfBytesSent,dwFlags,
										(sockaddr*)&SockAddripv6,sizeof(SockAddripv6),
										lpOverlapped,
										lpCompletionRoutine, lpThreadId,
										lpErrno);
		}//iRet==NO_ERROR
	}//Type==BROADCAST
	
	//do default
	return CLSPOverlappedSocket::WSPSendTo(lpBuffers,dwBufferCount,lpNumberOfBytesSent,dwFlags,
									lpTo,iToLen,
									lpOverlapped,
									lpCompletionRoutine, lpThreadId,
									lpErrno
									);
}

void CLOIUDPSocket::BroadCastUDPToAll(USHORT usPort,LPWSABUF  lpBuffers,DWORD dwBufferCount)
{
	//broadcast to all available connected clients
	USHORT					usConnectedList=0;
	PEER_INFORMATION		*pConnectClient=NULL;
	in_addr6				SelfAddress=m_BoundIPv6Addr;

	dbgprint(_T(" CLOIUDPSocket::BroadCastUDPToAll at %X Provider Socket %X "),usPort,m_NextProviderSocket  );
	pConnectClient=m_pSessionContainer->GetConnectedClientList(m_sSessionID,  usConnectedList);
	if(pConnectClient)
	{
		for(int i=0;i<usConnectedList;i++)
		{
			DWORD				dwNumberOfBytesSent=0;
			sockaddr_in6		SockAddripv6={0};
			int					iErr=0;

			//don't broadcast to self
			if(memcmp(&SelfAddress,&pConnectClient[i].IPv6TeredoAddress,sizeof(in_addr6))!=0)
			{
				SockAddripv6.sin6_family = AF_INET6;
				SockAddripv6.sin6_addr = pConnectClient[i].IPv6TeredoAddress;
				SockAddripv6.sin6_port = usPort;
				//make a blocking call
				
				
				CLSPOverlappedSocket::WSPSendTo(lpBuffers,dwBufferCount,&dwNumberOfBytesSent,0,
												(sockaddr*)&SockAddripv6,sizeof(SockAddripv6),
												NULL,NULL, NULL,
												&iErr);
			}
		}
		m_pSessionContainer->FreeConnectedClientList(pConnectClient);
	}

}

BOOL CLOIUDPSocket::ChangeRecvFromParams(LPSOCKADDR &lpFrom,LPINT &lpFromLen)
{
	if( m_bSockBoundToIpv6 )//both implicit & explicit bind makes recvfrom to ipv6 
	{
		if(lpFrom && lpFromLen ) 
		{
			lpFrom = (LPSOCKADDR)new sockaddr_in6;
			lpFromLen= new INT;
			*lpFromLen =sizeof(sockaddr_in6);
			return TRUE;
		}
	}

	return FALSE;
}
void CLOIUDPSocket::OnRecvFromComplete(DWORD dwError,BOOL blpFromParamChanged,LPSOCKADDR lpActualFrom,LPINT lpActualFromLen,LPSOCKADDR lpFrom,LPINT lpFromLen)
{
	if(blpFromParamChanged)
	{
		if(dwError==NO_ERROR && *lpFromLen>=sizeof(sockaddr_in6))//success lpfrom contains ipv6 teredo client address
		{
			in_addr6	IPv6TeredoAddr=((sockaddr_in6*)lpFrom)->sin6_addr;
			DWORD		dwLocalAddr;	

			//emulate to the application as if received from ipv4 client
			dwLocalAddr=m_pSessionContainer->GetIpv4Address(m_sSessionID, IPv6TeredoAddr);
			if(dwLocalAddr && lpFromLen && *lpFromLen>=sizeof(sockaddr_in))
			{
				((sockaddr_in*)lpActualFrom)->sin_family = AF_INET;
				((sockaddr_in*)lpActualFrom)->sin_addr.S_un.S_addr   = dwLocalAddr;
				//if user purposely bound a socket 

				((sockaddr_in*)lpActualFrom)->sin_port  = ((sockaddr_in6*)lpFrom)->sin6_port; 
				dbgprint(_T(" CLOIUDPSocket::OnRecvFromComplete changed to virtual ipv4 address %X:%X Provider Socket %X  "),dwLocalAddr,((sockaddr_in6*)lpFrom)->sin6_port,m_NextProviderSocket  );
			}

			int iErr;
			if(dwLocalAddr==0)//hacker--disable send & recv on the socket 
				m_NextProvider->NextProcTable.lpWSPShutdown(m_NextProviderSocket,SD_BOTH,&iErr);  

		}//if(dwError==NO_ERROR ..

		//now free buffers allocated by us
		delete lpFrom;
		delete lpFromLen;
	}
}


int WSPAPI CLOIUDPSocket::WSPGetSockName(
											struct sockaddr FAR * name,
											LPINT           namelen,
											LPINT           lpErrno
											)
{
	//if it is LOI bound socket 
	if(m_bSockBoundToIpv6 && namelen)
	{
		//return ipv4 address information we stored in bind call
		if(*namelen>=m_WSPBindInfo.iNameLen)
		{
			//get the ipv6 bind information
			sockaddr_in6   Ipv6BoundInformation={0};
			int			   iNameLen=sizeof(Ipv6BoundInformation);
			int			   iRet;

			iRet=CLSPOverlappedSocket::WSPGetSockName(( sockaddr*)&Ipv6BoundInformation,&iNameLen,lpErrno);
			if(iRet==NO_ERROR)
			{
				memcpy(name,&m_WSPBindInfo.name,m_WSPBindInfo.iNameLen); 
				//((sockaddr_in*)name)->sin_addr.S_un.S_addr =  m_LOISession.GetIpv4Address(Ipv6BoundInformation.sin6_addr); 
				//((sockaddr_in*)name)->sin_port = Ipv6BoundInformation.sin6_port; 
				*namelen = m_WSPBindInfo.iNameLen;
				dbgprint(_T(" CLOIUDPSocket::WSPGetSockName returned %X:%X Provider Socket %X "),((sockaddr_in*)name)->sin_addr.S_un.S_addr,((sockaddr_in*)name)->sin_port,m_NextProviderSocket  );
			}
			return iRet;
		}
	}
	//do default:
	return CLSPOverlappedSocket::WSPGetSockName(name,namelen,lpErrno);									
}