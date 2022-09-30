#include "StdAfx.h"
#include "LSPSocket.h"
#include "loiTCPsocket.h"

const DWORD	CLOITCPsocket::ACCEPTEX_SOCK_ADDR_SIZE=sizeof(SOCKADDR_IN6)+ 16;


CLOITCPsocket::CLOITCPsocket(PROVIDER *Provider ):CLSPOverlappedSocket(Provider) 
{
	Init();
}

CLOITCPsocket::CLOITCPsocket (PROVIDER *Provider ,SOCKET ProviderSocket):CLSPOverlappedSocket(Provider,ProviderSocket) 
{
	Init();
}

void CLOITCPsocket::Init()
{
	m_pSessionContainer= CLSPSessionContainer::GetInstance();
	ZeroMemory(&m_WSPBindInfo,sizeof(m_WSPBindInfo));
	ZeroMemory(&m_WSPConnectInfo,sizeof(m_WSPConnectInfo));
	m_bSockDiverted=FALSE;
	m_bSockBoundToIpv6=FALSE;
}

void CLOITCPsocket::Inherit(CLSPSocket *pAssignee)
{
	//copy all existsing parameters to this object happens for accepted socket
	CLOITCPsocket		*pTcpSocket=dynamic_cast<CLOITCPsocket*>(pAssignee);

	if(pTcpSocket)
	{
		m_bSockBoundToIpv6=pTcpSocket->m_bSockBoundToIpv6;
		m_bSockDiverted=pTcpSocket->m_bSockDiverted;

		m_WSPBindInfo=pTcpSocket->m_WSPBindInfo;
		m_WSPConnectInfo=pTcpSocket->m_WSPConnectInfo;

		m_BoundIPv6Addr=pTcpSocket->m_BoundIPv6Addr;
		m_sSessionID=pTcpSocket->m_sSessionID;
	}
	CLSPOverlappedSocket::Inherit(pAssignee);
}
CLOITCPsocket::~CLOITCPsocket(void)
{
}


int WSPAPI CLOITCPsocket::WSPBind(
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

	dwLocalBindAddress=((sockaddr_in*)name)->sin_addr.S_un.S_addr;
	usPort= ((sockaddr_in*)name)->sin_port;
	if(m_pSessionContainer->IsBindToIPv6Required(usPort,m_BoundIPv6Addr,m_sSessionID))
	{
		if(CLSPSocket::CreateIPv6SockAndAssociate(SOCK_STREAM,IPPROTO_TCP,lpErrno)==NO_ERROR)
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
			dbgprint(_T(" CLOITCPsocket::WSPBind %X:%X to %X -- return %X LastError : %X provider socket %X"),dwLocalBindAddress,usPort,m_pSessionContainer->GetIpv4Address(m_sSessionID,SockAddripv6.sin6_addr ),  iRet,*lpErrno,m_NextProviderSocket   );
			return iRet;
		}
	}
	//dodefault
dodefault:
	return CLSPOverlappedSocket::WSPBind(name,namelen,lpErrno);
}


int WSPAPI CLOITCPsocket::WSPConnect(
						const struct sockaddr FAR * name,
						int                   namelen,
						LPWSABUF              lpCallerData,
						LPWSABUF              lpCalleeData,
						LPQOS                 lpSQOS,
						LPQOS                 lpGQOS,
						LPINT                 lpErrno
						)

{
	DWORD			dwLocalAddr;
	USHORT			usPort; 
	in_addr6		IPv6MappedTeredoAddress={0};
	USHORT			usDivertedPort;
	CONNECT_TYPE	Type;

	if(name==NULL || ((sockaddr_in*) name)->sin_family != AF_INET)//invalid parameters
	{
		goto dodefault;
	}
	dwLocalAddr=((sockaddr_in*) name)->sin_addr.S_un.S_addr;
	usPort=((sockaddr_in*) name)->sin_port;

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
		if( m_bSockBoundToIpv6 || 
			CreateIPv6SockAndAssociate(SOCK_STREAM,IPPROTO_TCP,lpErrno)==NO_ERROR)
		{	
			SockAddripv6.sin6_family = AF_INET6;
			SockAddripv6.sin6_addr = IPv6MappedTeredoAddress;
			SockAddripv6.sin6_port = Type==DIRECT?usPort:usDivertedPort;//use direct port if the remote end is app else its service needs proxy

			iRet=CLSPOverlappedSocket::WSPConnect((sockaddr*)&SockAddripv6,sizeof(SockAddripv6),lpCallerData,lpCalleeData,lpSQOS,lpGQOS,lpErrno);

			//store the actual adddr
			m_bSockDiverted=TRUE;
			memcpy(&m_WSPConnectInfo.name ,name,sizeof(m_WSPConnectInfo.name));
			m_WSPConnectInfo.iNameLen = sizeof(m_WSPConnectInfo.name); 
			if(PROXY==Type)
			{
				if(iRet==NO_ERROR || (iRet==SOCKET_ERROR && *lpErrno == WSAEWOULDBLOCK))
				{
					if(*lpErrno== WSAEWOULDBLOCK)
									WaitForConnectComplete();
					SendProxyConnect(usPort);
				}
			}////proxy
			dbgprint(_T(" CLOITCPsocket::WSPConnect %X:%X to %X -- return %X LastError : %X provider socket %X"),dwLocalAddr,usPort,m_pSessionContainer->GetIpv4Address(m_sSessionID,IPv6MappedTeredoAddress) ,iRet,*lpErrno,m_NextProviderSocket   );
			return iRet;
		 }
	}
dodefault:
	//do default:
	return CLSPOverlappedSocket::WSPConnect(name,namelen,lpCallerData,lpCalleeData,lpSQOS,lpGQOS,lpErrno);
}

BOOL PASCAL FAR CLOITCPsocket::ExtConnectEx(
										IN const struct sockaddr FAR *name,
										IN int namelen,
										IN PVOID lpSendBuffer OPTIONAL,
										IN DWORD dwSendDataLength,
										OUT LPDWORD lpdwBytesSent,
										IN LPOVERLAPPED lpOverlapped,
										INT   &Errno
										)
{


	DWORD			dwLocalAddr;
	USHORT			usPort; 
	in_addr6		IPv6MappedTeredoAddress={0};
	USHORT			usDivertedPort;
	CONNECT_TYPE	Type;

	if(name==NULL || ((sockaddr_in*) name)->sin_family != AF_INET)//invalid parameters
	{
		goto dodefault;
	}
	dwLocalAddr=((sockaddr_in*) name)->sin_addr.S_un.S_addr;
	usPort=((sockaddr_in*) name)->sin_port;

	Type=m_pSessionContainer->IsConnectDivertRequired(
											dwLocalAddr,
											usPort,
											IPv6MappedTeredoAddress,
											usDivertedPort,
											m_sSessionID
											);
		///proxy not supported for connectex
	if(Type==DIRECT )
	{	
		sockaddr_in6		SockAddripv6={0};
		BOOL				bRet;
		if( m_bSockBoundToIpv6 || 
			CreateIPv6SockAndAssociate(SOCK_STREAM,IPPROTO_TCP,&Errno)==NO_ERROR)
		{	
			SockAddripv6.sin6_family = AF_INET6;
			SockAddripv6.sin6_addr = IPv6MappedTeredoAddress;
			SockAddripv6.sin6_port = usPort;

			bRet=CLSPOverlappedSocket::ExtConnectEx(
													(sockaddr*)&SockAddripv6,
													sizeof(SockAddripv6),
													lpSendBuffer,
													dwSendDataLength,
													lpdwBytesSent,
													lpOverlapped,
													Errno);

			//store the actual adddr
			m_bSockDiverted=TRUE;
			memcpy(&m_WSPConnectInfo.name ,name,sizeof(m_WSPConnectInfo.name));
			m_WSPConnectInfo.iNameLen = sizeof(m_WSPConnectInfo.name); 
			dbgprint(_T(" ExtConnectEx::WSPConnect %X:%X to %X -- return %X LastError : %X provider socket %X"),dwLocalAddr,usPort,m_pSessionContainer->GetIpv4Address(m_sSessionID, IPv6MappedTeredoAddress) ,bRet,Errno,m_NextProviderSocket   );
			return bRet;
		 }
	}

dodefault:
	return CLSPOverlappedSocket::ExtConnectEx(
											name,
											namelen,
											lpSendBuffer,
											dwSendDataLength,
											lpdwBytesSent,
											lpOverlapped,
											Errno
											);
}


void CLOITCPsocket::WaitForConnectComplete()
{
	fd_set				writefd={0};
	struct timeval		timeout={0};
	int					Errno=0;
	int					ret=0;
					
	timeout.tv_sec = 15;
	FD_ZERO(&writefd);
	FD_SET(m_NextProviderSocket ,&writefd);

	ret = CLSPOverlappedSocket::WSPSelect(1,
										  NULL,&writefd,NULL,
										  &timeout,&Errno);


}

void CLOITCPsocket::SendProxyConnect(USHORT usPort)
{

	DWORD				dwBytesSent;
	int					iErr,ret;
	DWORD				dwProtolen = sizeof(PROTO_CMD)+sizeof(PROXY_CONNECT);
	BYTE				*pConnectData= new BYTE[dwProtolen];
	WSABUF				 wsabuff={dwProtolen,(char*)pConnectData};
	LPPROTO_CMD			 pCmd=(LPPROTO_CMD)pConnectData;
	LPPROXY_CONNECT		 pProxyConnect=(LPPROXY_CONNECT)(pConnectData+sizeof(PROTO_CMD));

	pCmd->usCmd = REVERSE_PROXY_CONNECT;
	pCmd->dwLength = dwProtolen-sizeof(PROTO_CMD);
	pProxyConnect->usPort = usPort;



	//send the actual port to connect to remote system
    SetBlockingProvider(m_NextProvider);
	ret = m_NextProvider->NextProcTable.lpWSPSend(
												m_NextProviderSocket, 
												&wsabuff, 
												1,
												&dwBytesSent, 
												0, 
												NULL,NULL,NULL, 
												&iErr
												);
	SetBlockingProvider(NULL);
	
	if(pConnectData)
				delete pConnectData;

}
SOCKET WSPAPI CLOITCPsocket::WSPAccept(
								struct sockaddr FAR * addr,  
								LPINT           addrlen,                 
								LPCONDITIONPROC lpfnCondition,  
								DWORD_PTR       dwCallbackData,          
								LPINT           lpErrno
								)
{
	if(m_bSockBoundToIpv6)
	{
		sockaddr_in6   Ipv6Name={0};
		int			   iNameLen=sizeof(Ipv6Name);
		SOCKET		   NewSocket;

		NewSocket=CLSPOverlappedSocket::WSPAccept((sockaddr*)&Ipv6Name,&iNameLen,lpfnCondition,dwCallbackData,lpErrno);
		if(NewSocket!=INVALID_SOCKET)
		{
			//got a new teredo client connected
			dbgprint(_T(" CLOITCPsocket::WSPAccept accepted socket over teredo socket :%X"),NewSocket );
			//check if he is authneticated client
			if(!m_pSessionContainer->IsValidClient(Ipv6Name.sin6_addr))
			{
				//hacker--disable send & recv on the client 
				dbgprint(_T(" CLOITCPsocket::WSPAccept closing hacker socket over teredo socket :%X"),NewSocket );
				m_NextProvider->NextProcTable.lpWSPShutdown(NewSocket,SD_BOTH,lpErrno);
			}
			
			//fill the app  address  with virtual address
			if(addr && addrlen && *addrlen>=sizeof(sockaddr_in))
			{
				GetAcceptedPeerName(Ipv6Name.sin6_addr,Ipv6Name.sin6_port,addr,addrlen);
			}
		}
		return NewSocket;
	}
	//do default:
	return CLSPOverlappedSocket::WSPAccept(addr,addrlen,lpfnCondition,dwCallbackData,lpErrno);
}


///handle acceptex
BOOL CLOITCPsocket::ChangeAcceptExParam(
										CLSPSocket *AcceptSocketContext,	
										SOCKET &AcceptProviderSocket,
										PVOID &lpOutputBuffer,
										DWORD &dwReceiveDataLength,
										DWORD &dwLocalAddressLength,
										DWORD &dwRemoteAddressLength)
{
	int iError;
	if(m_bSockBoundToIpv6 &&
		(AcceptSocketContext->CreateIPv6SockAndAssociate (SOCK_STREAM,IPPROTO_TCP,&iError)==NO_ERROR)
		)
	{
		//create new buffer to hold ipv6 sockaddr
		DWORD     dwSockAddrReq=ACCEPTEX_SOCK_ADDR_SIZE*2;
		LPBYTE	  lpNewBuffer=new BYTE[dwReceiveDataLength+dwSockAddrReq]; 	

		lpOutputBuffer=(PVOID)lpNewBuffer;
		dwLocalAddressLength=ACCEPTEX_SOCK_ADDR_SIZE;
		dwRemoteAddressLength=ACCEPTEX_SOCK_ADDR_SIZE;

		//inherit the values from the listening socket
		//current socket instance is the listening socket
		AcceptSocketContext->Inherit(this); 
		//assign the new ipv6 provider socket
		AcceptProviderSocket=AcceptSocketContext->GetProviderSocket(); 
		return TRUE;
	}
	return FALSE;
}

void	CLOITCPsocket::OnChangeAcceptExParamComplete(
											  SOCKET  AcceptProviderSocket,
											  DWORD dwError,
											  DWORD dwBytesTransferred,
											  PVOID lpActualOutputBuffer,
											  PVOID lpAppReturnedBuffer,
											  DWORD dwActualRecvLen,
											  DWORD dwActualLocalAddrLen,
											  DWORD dwActualRemoteAddrLen)
{

	  if(dwError==NO_ERROR)
	  {
		  //operation completed successfully copy the received values to original
		  if(dwBytesTransferred)
		  {
			  //sometimes user might have freed the buffer so put it in exception handler
			  try{
				memcpy(lpActualOutputBuffer,lpAppReturnedBuffer,dwBytesTransferred); 
			  }
			  catch(...){
				  dbgprint(_T("Exception while Copying to actual acceptex buffer"));
			  }
			  
		  }
		  //get the ipv6 address values
		  SOCKADDR_IN6  *pLocalAddr=NULL,*pRemoteAddr=NULL;
		  INT			iLocalAddrLen=0,iRemoteAddrLen=0;

		  if(GetAcceptExSockaddrsIPv6(lpAppReturnedBuffer,
									 dwActualRecvLen,
									 (LPSOCKADDR* )&pLocalAddr, &iLocalAddrLen,
									 (LPSOCKADDR* )&pRemoteAddr,&iRemoteAddrLen)
									
									 )
		  {
			  //remap the address & copy the data
			  sockaddr_in  EmulatedLocal={0},EmulatedRemote={0};
			  INT		   iEnulatedLocalLen=sizeof(EmulatedLocal),iEmulatedRemoteLen=sizeof(EmulatedRemote);

			  GetAcceptedPeerName (pRemoteAddr->sin6_addr,0,(sockaddr*)&EmulatedRemote,&iEmulatedRemoteLen);
			  GetAcceptedSockName( pLocalAddr->sin6_addr,(sockaddr*)&EmulatedLocal,&iEmulatedRemoteLen);

			  DWORD   dwIPv4LocalAddr=EmulatedLocal.sin_addr.S_un.S_addr  ;
			  DWORD   dwIPv4RemoteAddr=EmulatedRemote.sin_addr.S_un.S_addr ;
			  if(!dwIPv4RemoteAddr)
			  {
				  INT iError;
				//hacker--disable send & recv on the client 
				dbgprint(_T(" CLOITCPsocket::OnChangeAcceptExParamComplete closing hacker socket over teredo socket :%X"),AcceptProviderSocket );
				m_NextProvider->NextProcTable.lpWSPShutdown(AcceptProviderSocket,SD_BOTH,&iError);
			  }

			  try{
				  //copy to the actual buffer in TDI format
				  PTDI_ADDRESS_INFO		pTdiLocalAddr=(PTDI_ADDRESS_INFO)((BYTE*)lpActualOutputBuffer+dwActualRecvLen);
				  PTRANSPORT_ADDRESS	pTdiRemoteAddr=(PTRANSPORT_ADDRESS)((BYTE*)lpActualOutputBuffer+dwActualRecvLen+dwActualLocalAddrLen);

				  pTdiLocalAddr->ActivityCount = 1;
				  pTdiLocalAddr->Address.TAAddressCount =1;
				  pTdiLocalAddr->Address.Address->AddressLength=sizeof(sockaddr_in);     
				  pTdiLocalAddr->Address.Address->AddressType = AF_INET;  
				  dbgprint(_T("OnChangeAcceptExParamComplete Copying %X IPv4 to local"),dwIPv4LocalAddr);
				  sockaddr_in *pLocalIPv4Addr = ( sockaddr_in *)&pTdiLocalAddr->Address.Address->AddressType; 
				  pLocalIPv4Addr->sin_addr.S_un.S_addr   = dwIPv4LocalAddr;
				  pLocalIPv4Addr->sin_port =  pLocalAddr->sin6_port;
				  pLocalIPv4Addr->sin_family = AF_INET; 

				  //copy remote
				  pTdiRemoteAddr->TAAddressCount  =1;
				  pTdiRemoteAddr->Address->AddressLength =  sizeof(sockaddr_in);
				  pTdiRemoteAddr->Address->AddressType = AF_INET;
				  dbgprint(_T("OnChangeAcceptExParamComplete Copying %X IPv4 to remote"),dwIPv4RemoteAddr);
				  sockaddr_in *pRemoteIPv4Addr = ( sockaddr_in *)&pTdiRemoteAddr->Address->AddressType;
				  pRemoteIPv4Addr->sin_addr.S_un.S_addr   = dwIPv4RemoteAddr;
				  pRemoteIPv4Addr->sin_family = AF_INET;
				  pRemoteIPv4Addr->sin_port = pRemoteAddr->sin6_port; 
			  }catch(...)
			  {
				  dbgprint(_T("OnChangeAcceptExParamComplete copying exception"));
			  }
		  }
		  //check if it authenticated user
	
	  }
		if(	lpAppReturnedBuffer)
	  	  delete lpAppReturnedBuffer;

}


BOOL CLOITCPsocket::GetAcceptExSockaddrsIPv6(
										PVOID lpOutputBuffer,
										DWORD dwReceiveDataLength,
										LPSOCKADDR *LocalSockaddr,
										LPINT LocalSockaddrLength,
										LPSOCKADDR *RemoteSockaddr,
										LPINT RemoteSockaddrLength
										)
{
   GUID    guidGetAcceptex= WSAID_GETACCEPTEXSOCKADDRS;
   BOOL bRet = LoadExtensionFunction(
                 (FARPROC **)&m_NextProvider->NextProcTableExt.lpfnGetAcceptExSockaddrs,
                 guidGetAcceptex,
                 m_NextProvider->NextProcTable.lpWSPIoctl,
                 m_NextProviderSocket
                 );
   if ( FALSE == bRet )
   {
       dbgprint(_T("Lower provider AcceptEx == NULL!"));
	   return FALSE;
   }

	*LocalSockaddrLength=*RemoteSockaddrLength=0;
	m_NextProvider->NextProcTableExt.lpfnGetAcceptExSockaddrs(
										 lpOutputBuffer,
										dwReceiveDataLength,
										ACCEPTEX_SOCK_ADDR_SIZE,
										ACCEPTEX_SOCK_ADDR_SIZE,
										LocalSockaddr,
										LocalSockaddrLength,
										RemoteSockaddr,
										RemoteSockaddrLength
										);
	if(*LocalSockaddrLength && *RemoteSockaddrLength) return TRUE;

   return FALSE;

}



int WSPAPI CLOITCPsocket::WSPGetPeerName(  
										struct sockaddr FAR * name,
										LPINT           namelen,
										LPINT           lpErrno
										)
{
	//if diverted socket
	if(m_bSockDiverted && namelen && name)
	{
		//return  ipv4 address information + port stored in connect call
		if(*namelen>=m_WSPConnectInfo.iNameLen)
		{
			memcpy(name,&m_WSPConnectInfo.name,m_WSPConnectInfo.iNameLen); 
			*namelen = m_WSPConnectInfo.iNameLen;
			dbgprint(_T(" CLOITCPsocket::WSPGetPeerName  %X:%X Provider Socket %X"),((sockaddr_in*)name)->sin_addr.S_un.S_addr,((sockaddr_in*)name)->sin_port,m_NextProviderSocket  );
			return NO_ERROR;
		}
	}

	//this is a accepted socket
	if(m_bSockBoundToIpv6 && namelen && name)
	{
		if( *namelen>=m_WSPBindInfo.iNameLen)
		{
			//get the ipv6 bind information
			sockaddr_in6   Ipv6BoundInformation={0};
			int			   iNameLen=sizeof(Ipv6BoundInformation);
			int			   iRet;

			iRet=CLSPOverlappedSocket::WSPGetPeerName(( sockaddr*)&Ipv6BoundInformation,&iNameLen,lpErrno);
			if(iRet==NO_ERROR)
			{
				GetAcceptedPeerName(Ipv6BoundInformation.sin6_addr,Ipv6BoundInformation.sin6_port,name,namelen); 
				
				dbgprint(_T(" CLOITCPsocket::WSPGetPeerName accepted socket  %X:%X Provider Socket %X"),((sockaddr_in*)name)->sin_addr.S_un.S_addr,((sockaddr_in*)name)->sin_port,m_NextProviderSocket   );
			}
			return iRet;
		}
	}
	//do default:
	return CLSPOverlappedSocket::WSPGetPeerName(name,namelen,lpErrno);
}

void CLOITCPsocket::GetAcceptedPeerName(in_addr6 IPv6PeerAddr,USHORT usPeerPort,struct sockaddr FAR * name,LPINT namelen)
{
	//show to the application as if the 
	DWORD dwFoolWithLocalSubnet=m_pSessionContainer->GetIpv4Address(m_sSessionID, IPv6PeerAddr);

	memcpy(name,&m_WSPBindInfo.name,m_WSPBindInfo.iNameLen); //copy all info from bind data
	if(((sockaddr_in*)&m_WSPBindInfo.name )->sin_addr.S_un.S_addr)//purposely the caller has bound a local address
	{
		//provide a address from the same subnet & host from virtual address
		if(memcmp(&IPv6PeerAddr, &m_BoundIPv6Addr,sizeof(in_addr6))==0)
		{
			//connection to the same m/c is made then just give the same BOUND IPV4 address in connection
			dwFoolWithLocalSubnet =((sockaddr_in*)&m_WSPBindInfo.name )->sin_addr.S_un.S_addr;
		}
		else
		{
			DWORD  dwBoundedSubnet=0x00ffffff&((sockaddr_in*)&m_WSPBindInfo.name )->sin_addr.S_un.S_addr;
			DWORD  dwHost=dwFoolWithLocalSubnet&0xff000000;
			dwFoolWithLocalSubnet= dwBoundedSubnet|dwHost;
		}
		
	}
	((sockaddr_in*)name)->sin_addr.S_un.S_addr = dwFoolWithLocalSubnet;   
	((sockaddr_in*)name)->sin_port = usPeerPort; 
	*namelen = m_WSPBindInfo.iNameLen;
}

void CLOITCPsocket::GetAcceptedSockName(in_addr6 IPv6PeerAddr,struct sockaddr FAR * name,LPINT namelen)
{
	memcpy(name,&m_WSPBindInfo.name,m_WSPBindInfo.iNameLen); 
	((sockaddr_in*) name)->sin_addr.S_un.S_addr=m_pSessionContainer->GetIpv4Address(m_sSessionID, IPv6PeerAddr);//if not purposely bound then give our address
	if(((sockaddr_in*)&m_WSPBindInfo.name )->sin_addr.S_un.S_addr)//purposely the caller has bound a local address
	{
		((sockaddr_in*) name)->sin_addr.S_un.S_addr=((sockaddr_in*)&m_WSPBindInfo.name )->sin_addr.S_un.S_addr;
	}
	*namelen = m_WSPBindInfo.iNameLen;
}
int WSPAPI CLOITCPsocket::WSPGetSockName(
										struct sockaddr FAR * name,
										LPINT           namelen,
										LPINT           lpErrno
										)
{
	//if it is LOI bound socket 
	if(m_bSockBoundToIpv6 && namelen && name)
	{
		//return ipv4 address information we stored in bind call
		if( *namelen>=m_WSPBindInfo.iNameLen)
		{
			sockaddr_in6   Ipv6BoundInformation={0};
			int			   iNameLen=sizeof(Ipv6BoundInformation);
			int			   iRet;

			iRet=CLSPOverlappedSocket::WSPGetSockName(( sockaddr*)&Ipv6BoundInformation,&iNameLen,lpErrno);
			if(iRet == NO_ERROR)
			{
				GetAcceptedSockName(Ipv6BoundInformation.sin6_addr,name,namelen);  
				if(!((sockaddr_in*) name)->sin_port)
				{
					((sockaddr_in*) name)->sin_port = Ipv6BoundInformation.sin6_port; 
				}
				dbgprint(_T(" CLOITCPsocket::WSPGetSockName bound socket  %X:%X Provider Socket %X"),((sockaddr_in*)name)->sin_addr.S_un.S_addr,((sockaddr_in*)name)->sin_port,m_NextProviderSocket   );
				return NO_ERROR;
			}
		}
		
	}

	//if diverted socket
	if(m_bSockDiverted && namelen && name)
	{
		//return loaclhost ipv4 address information + port stored in connect call
		if(*namelen>=m_WSPConnectInfo.iNameLen)
		{
			sockaddr_in6   Ipv6BoundInformation={0};
			int			   iNameLen=sizeof(Ipv6BoundInformation);
			int			   iRet;

			iRet=CLSPOverlappedSocket::WSPGetPeerName(( sockaddr*)&Ipv6BoundInformation,&iNameLen,lpErrno);
			if(iRet==NO_ERROR)
			{
				DWORD dwFoolWithLocalSubnet=0;
				memcpy(name,&m_WSPConnectInfo.name,m_WSPConnectInfo.iNameLen); 
				if(memcmp(&Ipv6BoundInformation.sin6_addr , &m_BoundIPv6Addr,sizeof(in_addr6))==0)
				{
					//connection to the same m/c is made then just give the same BOUND IPV4 address in connection
					((sockaddr_in*) name)->sin_addr.S_un.S_addr =((sockaddr_in*)&m_WSPConnectInfo.name )->sin_addr.S_un.S_addr;
				}
				else
				{
					//show to the application as if it is connected to same subnet 
					DWORD dwFoolWithLocalSubnet=((sockaddr_in*)&m_WSPConnectInfo.name )->sin_addr.S_un.S_addr&0x00ffffff;//take the subnet from connect information & append host from virtual address
					//some dummy address if counted to some address..If we he connected to virtual adddress,then it will point to the virtual adddress itself
					//if connected to virtual address subnet then will give same virtual address assigned for the connected virtual address
					((sockaddr_in*) name)->sin_addr.S_un.S_addr = dwFoolWithLocalSubnet|(m_pSessionContainer->GetIpv4Address(m_sSessionID,  Ipv6BoundInformation.sin6_addr)&0xff000000);   
				}
				//to get the implicit bound port
				CLSPOverlappedSocket::WSPGetSockName(( sockaddr*)&Ipv6BoundInformation,&iNameLen,lpErrno);
				((sockaddr_in*)name)->sin_port = Ipv6BoundInformation.sin6_port; 
				*namelen = m_WSPConnectInfo.iNameLen;
				dbgprint(_T(" CLOITCPsocket::WSPGetSockName connected socket  %X:%X Provider Socket %X"),((sockaddr_in*)name)->sin_addr.S_un.S_addr,((sockaddr_in*)name)->sin_port,m_NextProviderSocket   );
			}
			return iRet;
		}
	}

	//do default:
	return CLSPOverlappedSocket::WSPGetSockName(name,namelen,lpErrno);
}

