#include "StdAfx.h"
#include "TCPReverseProxy.h"
#include <iphlpapi.h>

CTCPReverseProxy::CTCPReverseProxy(void):m_ProtocolHandle(this)
{
}

CTCPReverseProxy::~CTCPReverseProxy(void)
{
}
/**********************************
this initialises the server
***********************************/
BOOL CTCPReverseProxy::StartServer(in_addr6 IPv6Address,IP2PReverseProxyNotify *pProxyNotify)
{
	CReverseTCPContext		*pServerContext=new CReverseTCPContext(TRUE,FALSE);
	ISocket					*pSocket=NULL;
	//fetch the teredo address first
	pSocket = m_SockIO.CreateSocket(this);
	if(!pSocket->CreateServer(IPv6Address,m_usListeningPort))
	{
		//failed to start the server
		m_SockIO.Close(pSocket);
		SetLastErrorMessage(pSocket->GetLastErrorMessage()); 
		DEBUGLOG(LOI_ERROR,_T(" CTCPReverseProxy::StartServer failed %s"),(LPCTSTR)m_SockIO.GetLastErrorMessage());
		return FALSE;
	}

	m_pP2PReverseProxyNotify=pProxyNotify;
	pSocket->SetSockParam()=pServerContext;  
	return TRUE;
}
/**********************************
returns the listening port of the server
**********************************/
USHORT CTCPReverseProxy::GetListeningPort()
{
	return m_usListeningPort;
}
/********************************
stops all socket connections
********************************/
void  CTCPReverseProxy::StopServer()
{
	m_SockIO.CancellAll(); 
}
/*************************************************************************
Use the context to identify whether listening socket context or accepted client
socket
*************************************************************************/
void	CTCPReverseProxy::OnClose(ISocket  *pSocket,DWORD dwLastError)
{
	if(GetContext(pSocket)->IsListeningContext() )
	{
		//notify about server close
		delete GetContext(pSocket);
		m_SockIO.Close(pSocket);
		return;
	}

	if(GetContext(pSocket)->IsRemoteContext())
	{
		//remove from the session id map
		m_SessionhRemoteSockMap.Erase( (HANDLE)*pSocket);
	}

	//remove the context created on accept
	if(GetContext(pSocket)->m_hPairedSocket!=INVALID_HANDLE_VALUE)
	{
		DWORD		dwLastBytesOnSock=0;
		PBYTE		pByReceived=NULL;

		m_SockIO.InitiateSocketClose (GetContext(pSocket)->m_hPairedSocket);
		GetContext(pSocket)->m_hPairedSocket=INVALID_HANDLE_VALUE;
	}

	delete GetContext(pSocket);
	m_SockIO.Close(pSocket);
}
/************************************************
close all socket associated session id
**************************************************/
void CTCPReverseProxy::CloseSession(CLOISession* pLOISession)
{
	HANDLE		*pSockhandles=NULL;
	DWORD		 dwRemoteSockItems=0;

	pSockhandles=m_SessionhRemoteSockMap.GetKeys(dwRemoteSockItems);
	if(dwRemoteSockItems)
	{
		for(DWORD dwItems=0;dwItems<dwRemoteSockItems;dwItems++)
		{
			CLOISession* pSockLOISession=NULL;
			if(m_SessionhRemoteSockMap.Get(pSockhandles[dwItems],pSockLOISession))
			{
				if(pLOISession == pSockLOISession)
				{
					//close this socket as the session closed
					m_SockIO.InitiateSocketClose (pSockhandles[dwItems]);
				}
			}
		}
		m_SessionhRemoteSockMap.FreeItems(pSockhandles);
	}

}
/****************************************************************
Create a context for the accepted socket 
****************************************************************/
void	CTCPReverseProxy::OnAccept(ISocket *pSocket)
{
	in_addr6			ClientAddr={0};
	ISocket				*pAcceptSocket=NULL;

	pAcceptSocket=m_SockIO.Accept(pSocket, ClientAddr,this);
	if(pAcceptSocket==NULL)
	{
		return ;//accepting the socket failed
	}
	pAcceptSocket->SetSockParam()=  new CReverseTCPContext(FALSE,TRUE);
	GetContext(pAcceptSocket)->m_PeerIpv6Addr=ClientAddr;
	GetContext(pAcceptSocket)->CmdType = WAIT_FOR_COMMAND;	
	pAcceptSocket->NotifyOnReadData(sizeof(PROTO_CMD));
}
/**********************************
Process the cmd received
**********************************/
void	CTCPReverseProxy::OnRequestedReceive(ISocket *pSocket, BYTE *pbyReceived,DWORD dwRecvLen)
{
	if(dwRecvLen)
	{	
		DWORD		 dwPendingDataTobeRead=0;
		dwPendingDataTobeRead=m_ProtocolHandle.HandleCmd(pbyReceived,dwRecvLen,GetContext(pSocket)->CmdType,*pSocket);
		pSocket->NotifyOnReadData(dwPendingDataTobeRead);
	}
}
/*********************************************************
Remote peer sends the port to which he wishes to connect
*********************************************************/
DWORD	CTCPReverseProxy::OnReverseProxyConnect(ISocket &SocketObj,USHORT usPort,PROTO_CMD_TYPE &NextState)
{
	ISocket 				*pLocalClientSock=NULL;
	BOOL					bRet;
	CLOISession*			pLOISession=NULL;

	//check first the user is authenticated & allowed to proxy this port
	pLOISession=m_pP2PReverseProxyNotify->OnClientConnect(GetContext(&SocketObj)->m_PeerIpv6Addr,usPort);
	if(pLOISession==NULL )
	{
		//user is not authenticated he can't proxy data
		m_SockIO.InvokeShutdown( (HANDLE)SocketObj);//shutdown ipv6 peer
		DEBUGLOG(LOI_ERROR,_T(" CTCPReverseProxy::OnClientConnect return false"));
		return 0;
	}

	//do connect to local host ipaddress
	pLocalClientSock=m_SockIO.CreateSocket (this); 
	if(pLocalClientSock == NULL)
	{
		//user is not authenticated he can't proxy data
		m_SockIO.InvokeShutdown( (HANDLE)SocketObj);//shutdown ipv6 peer
		DEBUGLOG(LOI_ERROR,_T(" CTCPReverseProxy::OnReverseProxyConnect failed to create local sock"));
		return 0;
	}
	DEBUGLOG(LOI_INFO,_T("Connecting local port %d"),ntohs(usPort));
	bRet= pLocalClientSock->ConnectServerAsync(GetFirstLocalIPAddresses(),usPort);
	if(!bRet)
	{
		m_SockIO.InvokeShutdown( (HANDLE)SocketObj);//shutdown ipv6 peer
		DEBUGLOG(LOI_ERROR,_T(" CTCPReverseProxy::OnReverseProxyConnect failed to connect local sock %s"),(LPCTSTR)m_SockIO.GetLastErrorMessage() );
		return 0;
	}

	int iEnable = 1,iRecvBufsize=0,iSendBufSize=60*1024;
	pLocalClientSock->SetSockOpt( TCP_NODELAY,(char *) &iEnable,sizeof(int));
	SocketObj.SetSockOpt( TCP_NODELAY,(char *) &iEnable,sizeof(int));
//	SocketObj.SetSockOpt(SO_RCVBUF,(char*)&iRecvBufsize,sizeof(iRecvBufsize));
//	SocketObj.SetSockOpt(SO_SNDBUF, (char*)&iSendBufSize,sizeof(iSendBufSize));
//	pLocalClientSock->SetSockOpt(SO_RCVBUF,(char*)&iRecvBufsize,sizeof(iRecvBufsize));

	//succedded
	pLocalClientSock->SetSockParam()=new CReverseTCPContext(FALSE,FALSE);

	//insert associated remote sock with session id
	m_SessionhRemoteSockMap.Insert((HANDLE)SocketObj,pLOISession);

	//initial states
	GetContext(pLocalClientSock) ->CmdType = REVERSE_PROXY_CLIENT_RECV;
	GetContext(&SocketObj)->CmdType =REVERSE_PROXY_REMOTE_RECV;
	//pair the sockets 
	GetContext(pLocalClientSock)->m_hPairedSocket = (HANDLE)SocketObj;
	GetContext(&SocketObj)->m_hPairedSocket = (HANDLE)*pLocalClientSock;
	return 0;// means read as much as possible
	
}
/************************************************
Invoked when any data is recevied from local sock
************************************************/
DWORD	CTCPReverseProxy::OnLocalClientRecv(ISocket &SocketObj,BYTE *ByValidateData,DWORD dwBytesReceived,PROTO_CMD_TYPE &NextState)
{
	BYTE							*pRelayData= SocketObj.GetSendBuffer (dwBytesReceived);

	//just relay the data to the ipv6 client peer
	memcpy(pRelayData,ByValidateData,dwBytesReceived);
	m_SockIO.SendData(GetContext(&SocketObj)->m_hPairedSocket,//send the data to assoicted ipv6 context
						pRelayData,
						dwBytesReceived
						);

	return 0;// means read as much as possible
}

/***********************************
Invoked when data is received from remote client receive
***********************************/
DWORD	CTCPReverseProxy::OnRemoteClientRecv(ISocket &SocketObj,BYTE *ByValidateData,DWORD dwBytesReceived,PROTO_CMD_TYPE &NextState)
{
	BYTE							*pRelayData= SocketObj.GetSendBuffer(dwBytesReceived);
	
	//just relay the data to the ipv6 client peer
	memcpy(pRelayData,ByValidateData,dwBytesReceived);
	m_SockIO.SendData(GetContext(&SocketObj)->m_hPairedSocket,//send the data to assoicted local connected sock context
					  pRelayData,
					  dwBytesReceived
					 );
	return 0;// means read as much as possible
}


/******************************************
Find the best interface in the system for ip address to connect
******************************************/
//find the best adapter to local ipv4 connect
DWORD CTCPReverseProxy::GetFirstLocalIPAddresses(void)
{
	PIP_ADAPTER_INFO		pAdapterInfo;
	PIP_ADAPTER_INFO		pAdapter = NULL;
	DWORD					dwRetVal = 0;
	ULONG					ulOutBufLen;
	DWORD					dwFirstLocalAddr=0;

	pAdapterInfo = (IP_ADAPTER_INFO *) malloc( sizeof(IP_ADAPTER_INFO) );
	ulOutBufLen = sizeof(IP_ADAPTER_INFO);
	// Make an initial call to GetAdaptersInfo to get
	// the necessary size into the ulOutBufLen variable
	if (GetAdaptersInfo( pAdapterInfo, &ulOutBufLen) != ERROR_SUCCESS) 
	{
		free (pAdapterInfo);
	    pAdapterInfo = (IP_ADAPTER_INFO *) malloc (ulOutBufLen);
	}

	if ((dwRetVal = GetAdaptersInfo( pAdapterInfo, &ulOutBufLen)) == NO_ERROR) 
	{
		pAdapter = pAdapterInfo;
		while (pAdapter) 
		{
			TCHAR   szAddString[MAX_PATH]={0};
			StringCbPrintf(szAddString,sizeof(szAddString),_T("%hs"),pAdapter->IpAddressList.IpAddress.String); 
			if(CString(szAddString) != _T("0.0.0.0") )
			{
				dwFirstLocalAddr= inet_addr(pAdapter->IpAddressList.IpAddress.String);
				DEBUGLOG(LOI_ERROR,_T(" CTCPReverseProxy::GetFirstLocalIPAddresses found addr %X"),dwFirstLocalAddr);
				break;
			}
			pAdapter = pAdapter->Next;
		}
	}

	if(pAdapterInfo)
			free (pAdapterInfo);
	return dwFirstLocalAddr;
}