#include "StdAfx.h"
#include "ISocket.h"

/////////////////////*********
const DWORD  ISocket::SOCKET_BUF_SIZE=30*1460;
ISocket::ISocket(SOCKET socket,ISockNotify *pSockNotify)
{
   ConstrutInit(pSockNotify);
   Init(socket);
  
}
ISocket::ISocket(ISockNotify *pSockNotify)
{
	ConstrutInit(pSockNotify);

}
/**********
common init for the construtor
************/
void ISocket::ConstrutInit(ISockNotify *pSockNotify)
{
	ZeroMemory(&m_RecvBuf,sizeof(m_RecvBuf));
	m_Socket=INVALID_SOCKET;
	m_pSockNotify=pSockNotify;
	m_hSocketEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
	m_dwDataToRecv =0;//if user needs particular size then can change it in on connect
	m_dwDataReadSoFar =0;
	m_bSocketCloseOnSendComplete=FALSE;
}
ISocket::~ISocket(void)
{
	Clear();
	CloseHandle(m_hSocketEvent);
}
/********************************************
caller can use this as a unique parameter to identify the socket
can wait on this for socket event
********************************************/
ISocket::operator HANDLE()
{
	return m_hSocketEvent;
}

/*************************************************************
Initialise the parameters for the socket
*************************************************************/
void ISocket::Init(SOCKET socket)
{
	
	m_dwDataToRecv =0;//if user needs particular size then can change it in on connect
	m_dwDataReadSoFar =0;
	m_RecvBuf.buf = new CHAR[ SOCKET_BUF_SIZE];
	m_RecvBuf.len = SOCKET_BUF_SIZE;
	ZeroMemory(m_RecvBuf.buf,m_RecvBuf.len);
	m_Socket=socket;
	WSAEventSelect(m_Socket,
				   m_hSocketEvent ,
				   FD_CONNECT|FD_ACCEPT|FD_READ|FD_WRITE|FD_CLOSE);//so we can get write notificationt o write piece by piece	

	m_lRaisedEvents=0;
}
/*****************************************************************
Free resources allocated for the socket
*****************************************************************/
void ISocket::Clear()
{
	//close sock
	CloseSocket();

	if(m_RecvBuf.buf)
	{
		delete m_RecvBuf.buf;
		m_RecvBuf.buf=NULL;
	}
	
	//remove all send buffers
	POSITION pos = m_SendStreamList.GetHeadPosition();
	for (int i=0;i <(int) m_SendStreamList.GetCount();i++)
	{	
		STREAM_BUFFER     QueueItem = m_SendStreamList.GetNext(pos);
		ReleaseSendBuffer(QueueItem.buf );
	}

}
/**********************************************************************
Caller invokes to do socket calls for server intialisation & listening system assigned
dynamic port is returned. Error is set in CError fucntions
**********************************************************************/
BOOL ISocket::CreateServer(in_addr6 Ipv6BindAddress,USHORT &usListeningPort)
{
	SOCKADDR_IN6			LocalAddr={0};
	SOCKET					Socket;

	Socket = socket (AF_INET6, SOCK_STREAM, IPPROTO_TCP);
	if (Socket == INVALID_SOCKET) 
	{
		m_dwLastError=WSAGetLastError();
		SetLastErrorMessage(IDS_ERR_SOCK,m_dwLastError);
		return INVALID_SOCKET;
	}

	Init(Socket); 


	int   iOptval=PROTECTION_LEVEL_UNRESTRICTED;
	int   iOptLen=sizeof(int);
	if (setsockopt(m_Socket,   IPPROTO_IPV6,                 
            IPV6_PROTECTION_LEVEL, 
            (char*)&iOptval, 
            iOptLen
			) == SOCKET_ERROR)
	{
		m_dwLastError=WSAGetLastError();
		SetLastErrorMessage(IDS_ERR_SET_SOCK_IPV6,m_dwLastError);
		return INVALID_SOCKET;
	}

	// Bind the socket to port m_usPort
	// and begin listening for connections

	LocalAddr.sin6_family  = AF_INET6;
	LocalAddr.sin6_addr = Ipv6BindAddress;
	LocalAddr.sin6_port = 0;

	if (bind( m_Socket,(SOCKADDR*) &LocalAddr,sizeof(LocalAddr)) == SOCKET_ERROR) 
	{
		m_dwLastError=WSAGetLastError();
		SetLastErrorMessage(IDS_ERR_BIND_SOCK,m_dwLastError);
		return INVALID_SOCKET;
	}


	if (listen( m_Socket,SOMAXCONN ) == SOCKET_ERROR)
	{	
		m_dwLastError=WSAGetLastError();
		SetLastErrorMessage(IDS_ERR_BIND_SOCK,m_dwLastError);
		return INVALID_SOCKET;
	}

	int iNameLen=sizeof(LocalAddr);
	
	if(getsockname(m_Socket,(sockaddr*)&LocalAddr,&iNameLen)!=ERROR_SUCCESS)
	{
		m_dwLastError=WSAGetLastError();
		SetLastErrorMessage(IDS_ERR_BIND_SOCK,m_dwLastError);
		return INVALID_SOCKET;
	}
	
	usListeningPort=LocalAddr.sin6_port; 

	return TRUE;
}
/**********************************************************************
Caller invokes to do socket calls for server intialisation & listening system assigned
dynamic port is returned. Error is set in CError fucntions
**********************************************************************/
BOOL ISocket::CreateServer(DWORD ipv4addr,USHORT &usListeningPort)
{
	SOCKADDR_IN			LocalAddr={0};
	SOCKET					Socket;

	Socket = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (Socket == INVALID_SOCKET) 
	{
		m_dwLastError=WSAGetLastError();
		SetLastErrorMessage(IDS_ERR_SOCK,m_dwLastError);
		return INVALID_SOCKET;
	}

	Init(Socket); 

	// Bind the socket to port m_usPort
	// and begin listening for connections

	LocalAddr.sin_family  = AF_INET;
	LocalAddr.sin_addr.S_un.S_addr  = ipv4addr;
	LocalAddr.sin_port = htons(444);

	if (bind( m_Socket,(SOCKADDR*) &LocalAddr,sizeof(LocalAddr)) == SOCKET_ERROR) 
	{
		m_dwLastError=WSAGetLastError();
		SetLastErrorMessage(IDS_ERR_BIND_SOCK,m_dwLastError);
		return INVALID_SOCKET;
	}


	if (listen( m_Socket,SOMAXCONN ) == SOCKET_ERROR)
	{	
		m_dwLastError=WSAGetLastError();
		SetLastErrorMessage(IDS_ERR_BIND_SOCK,m_dwLastError);
		return INVALID_SOCKET;
	}

	int iNameLen=sizeof(LocalAddr);
	
	if(getsockname(m_Socket,(sockaddr*)&LocalAddr,&iNameLen)!=ERROR_SUCCESS)
	{
		m_dwLastError=WSAGetLastError();
		SetLastErrorMessage(IDS_ERR_BIND_SOCK,m_dwLastError);
		return INVALID_SOCKET;
	}
	
	usListeningPort=LocalAddr.sin_port; 

	return TRUE;
}

/********************************************************************
Connect does all winsock initialisation functions. Return sucess or failure
error is set in CError member functions
********************************************************************/
BOOL ISocket::ConnectServer(in_addr6 Ipv6ConnectAddress,USHORT usPort)
{
	int						iRetVal;	
	SOCKET					Socket;

	Socket = socket (AF_INET6, SOCK_STREAM, IPPROTO_TCP);
	if (Socket == INVALID_SOCKET)
	{
		return FALSE;
	}

	Init(Socket); 

	  int   iOptval=PROTECTION_LEVEL_UNRESTRICTED;
	  int   iOptLen=sizeof(int);
	  if (setsockopt(m_Socket,   IPPROTO_IPV6,                 
					IPV6_PROTECTION_LEVEL, 
					(char*)&iOptval, 
					iOptLen
					) == SOCKET_ERROR)
	  {
		m_dwLastError=WSAGetLastError();
			SetLastErrorMessage(IDS_ERR_SET_SOCK_IPV6,m_dwLastError);
		return FALSE;
	  }
 	  //----------------------
	  // The sockaddr_in structure specifies the address family,
	  // IP address, and port of the server to be connected to.
	  sockaddr_in6 clientService={0}; 
	  clientService.sin6_family  = AF_INET6;
	  clientService.sin6_addr  = Ipv6ConnectAddress;
	  clientService.sin6_port = usPort;

	  iRetVal=connect( m_Socket, (SOCKADDR*) &clientService, sizeof(clientService) );
	  //----------------------
	  // Connect to server.
	  if ( iRetVal == SOCKET_ERROR && WSAGetLastError()!=WSAEWOULDBLOCK)  
	  {
		m_dwLastError=WSAGetLastError();
		SetLastErrorMessage(IDS_ERR_CONNECT_FAILED,m_dwLastError);
		return FALSE;
	  }
	  return TRUE;
}
/********************************************************************
This overloaded fucntion for ipv4
********************************************************************/
BOOL ISocket::ConnectServerAsync(DWORD dwNetorderAddr,USHORT usPort)
{
	  int				iRetVal;	
	  SOCKET			Socket;

	  Socket = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
	  if (Socket == INVALID_SOCKET)
	 {
	    	return FALSE;
	 }

	  
	  //----------------------
	  // The sockaddr_in structure specifies the address family,
	  // IP address, and port of the server to be connected to.
	  sockaddr_in clientService={0}; 
	  clientService.sin_family  = AF_INET;
	  clientService.sin_addr.S_un.S_addr    = dwNetorderAddr;
	  clientService.sin_port = usPort;

	  iRetVal=connect( Socket, (SOCKADDR*) &clientService, sizeof(clientService) );
	  //----------------------
	  // Connect to server.
	  if ( iRetVal == SOCKET_ERROR && WSAGetLastError()!=WSAEWOULDBLOCK)  
	  {
		m_dwLastError=WSAGetLastError();
		SetLastErrorMessage(IDS_ERR_CONNECT_FAILED,m_dwLastError);
		return FALSE;
	  }
	  Init(Socket); 
	  return TRUE;	
}
/*******************************************************************************
Accepts a incoming client & returns a sock 
*******************************************************************************/
SOCKET ISocket::AcceptIncomingIpv6Client(in_addr6 &ClientIpv6Addr)
{
	sockaddr_in6    Ipv6Info={0};
	int				iaddrlen=sizeof(Ipv6Info);
	SOCKET			ClientSocket;

	ClientSocket = accept(m_Socket ,(sockaddr *) &Ipv6Info, &iaddrlen);
	ClientIpv6Addr=Ipv6Info.sin6_addr; 
	return ClientSocket;
}
/***************************************
shutdown used for gracefull connection close
***************************************/
void ISocket::ShutDown()
{
	shutdown(m_Socket,SD_BOTH);
	RaiseEvents(FD_CLOSE); 
}

/************************************************
Frees the socket resource & deinitialise the member variable
************************************************/
void ISocket::CloseSocket()
{
	if(m_Socket!=INVALID_SOCKET)
	{
		//first stop event notification
		WSAEventSelect(m_Socket,m_hSocketEvent ,0);
		closesocket(m_Socket);
		m_Socket=INVALID_SOCKET;
	}
}

/************************************************
Returns the binded ipv6 address to which the socket bound
************************************************/
in_addr6 ISocket::GetIPv6SockAddress()
{
	sockaddr_in6		SockAdd={0};
	int					iNameLen=sizeof(SockAdd);
	getsockname(m_Socket,(struct sockaddr*)&SockAdd,&iNameLen);
	return SockAdd.sin6_addr; 
}

/************************************************
Returns the remote peer ipv6 address to which the socket connected
************************************************/
in_addr6 ISocket::GetIPv6PeerAddress()
{
	sockaddr_in6		SockAdd={0};
	int					iNameLen=sizeof(SockAdd);
	getpeername(m_Socket,(struct sockaddr*)&SockAdd,&iNameLen);
	return SockAdd.sin6_addr; 
}

/*************************************************************
Sets socket option on the client
*************************************************************/
int ISocket::SetSockOpt(int optname,char *optval,int ioptval)
{
	return setsockopt(m_Socket,SOL_SOCKET,optname,(char*)&ioptval,ioptval);
}



/*************************************************************************************
this actually queues the send & initiate first send call.. Pending data will be send in 
on receiving FD_SEND
**************************************************************************************/
void ISocket::SendData(BYTE *bySendBuffer,DWORD dwDataLength)
{
	STREAM_BUFFER     QueueItem={0,0,dwDataLength,bySendBuffer};

	m_SendStreamList.AddTail(QueueItem);
	SendPendingData();
}
/****************************************************
reads a buffer from the queue & intiate send ..clears the buffer if send is complete
//true means no pending data
*****************************************************/
BOOL ISocket::SendPendingData()
{
	POSITION				pos =0,Prev=0;
	//pop the queued item
	while(pos = m_SendStreamList.GetHeadPosition())	
	{	
		WSABUF			wsasendbuf[1]={0};
		DWORD			dwBytesSent=0;
		int				iErr;
		STREAM_BUFFER   QueuedItem={0};
		int             iLastError=0;

		QueuedItem=m_SendStreamList.GetAt(pos);

		wsasendbuf[0].buf =(CHAR*)&QueuedItem.buf[QueuedItem.dwBytesSendPos]; 
		wsasendbuf[0].len = ULONG(QueuedItem.dwTotalLen- QueuedItem.dwBytesSendPos);
		iErr= WSASend( m_Socket,
				wsasendbuf,
				sizeof(wsasendbuf)/sizeof(wsasendbuf[0]),
				&dwBytesSent,
				0,NULL,NULL);
		iLastError=WSAGetLastError();
		if(iErr==SOCKET_ERROR && iLastError!=WSAEWOULDBLOCK)
		{
			DEBUGLOG(LOI_ERROR,_T("Send Failed for Socket %d Error %d"),m_Socket,iLastError);
			ShutDown();
			break;
		}

		QueuedItem.dwBytesSendPos+= dwBytesSent;

		//update if any pending data is to be written
		if(QueuedItem.dwBytesSendPos<QueuedItem.dwTotalLen)
		{
			m_SendStreamList.SetAt(pos,QueuedItem); 
			break;//no more data can be send
		}
		if(QueuedItem.dwBytesSendPos==QueuedItem.dwTotalLen)//sent fully so remove the item
		{
			ReleaseSendBuffer((BYTE*)QueuedItem.buf); 
			m_SendStreamList.RemoveAt(pos);
		}
	}
	return m_SendStreamList.IsEmpty();
}

/******************************************************************
This notifies the sock events to the caller by invoking the functions
*******************************************************************/
void ISocket::OnSockEvent(LONG lNetworkEvents)
{
	if(IsInternalEvent(lNetworkEvents))
	{
		//event generated for internal processing
		return;
	}
	if(lNetworkEvents&FD_ACCEPT)
	{
		DEBUGLOG(LOI_INFO,_T(" OnSockEvent=FD_ACCEPT"));
		m_pSockNotify->OnAccept(this);
	}

	if(lNetworkEvents&FD_CONNECT)
	{
		DEBUGLOG(LOI_INFO,_T(" OnSockEvent=FD_CONNECT"));
		m_pSockNotify->OnConnectComplete(this,m_dwErrcode[FD_CONNECT_BIT] );
		
	}


	if(lNetworkEvents&FD_READ)
	{
		DWORD		dwReadLen;
		BYTE		*pbyRecv;

		pbyRecv=GetReadData(dwReadLen);
		if(dwReadLen)//only notify if data exists
		{
			m_pSockNotify->OnRequestedReceive(this,pbyRecv,dwReadLen);
		}
		
	}
	if(lNetworkEvents&USER_CANCEL)
	{
		DEBUGLOG(LOI_INFO,_T(" OnSockEvent=FD_USER_CANCEL"));
		m_pSockNotify->OnClose( this,0);
		return;
	}
	///close always last invoked
	if(lNetworkEvents&FD_CLOSE)
	{
		DEBUGLOG(LOI_INFO,_T(" OnSockEvent=FD_CLOSE"));
		m_pSockNotify->OnClose( this,m_dwErrcode[FD_CLOSE_BIT]);
	}

}
/******************************
Mark the socket to close on pending send data complete
*******************************/
void ISocket::MarkSocketCloseOnPendingSendComplete()
{
		m_bSocketCloseOnSendComplete=TRUE;
}
/********************************************************************************
Checks whether event raised needs to notified to the caller or just required to be handled internally
1) We need not inform abt FD_WRITE just need to write the pending buffers
2) read needs to be notified only if it reaches the amount the caller requested

********************************************************************************/

BOOL ISocket::IsInternalEvent(long lNetworkEvents)
{
	if(lNetworkEvents&FD_WRITE)
	{
		if(m_dwErrcode[FD_WRITE_BIT])
		{
			//error in send operAtion just shutdown the sock it will generate close event
			ShutDown();
			if(lNetworkEvents==FD_WRITE)return TRUE;//return only if write is set we need to notify other events to p2p
		}
		//check any pending write
		if(SendPendingData()&& 
		   m_bSocketCloseOnSendComplete //pending send is complete & socket marked for close then close the socket
		   )
		{
			ShutDown();
		}
		if(lNetworkEvents==FD_WRITE)return TRUE;//return only if write is set we need to notify other events  to p2p
	}

	if(lNetworkEvents&FD_READ)
	{
		if(m_dwErrcode[FD_READ_BIT])
		{
			//error in recv operAtion just shutdown the sock it will generate close event
			DEBUGLOG(LOI_ERROR,_T("ISocket::Recv Failed for Socket %d Error %d"),m_Socket,m_dwErrcode[FD_READ_BIT]);
			ShutDown();
			return TRUE;
		}
		//check any pending recv
		DWORD			dwBytesReceived=0;
		int				iErr;
			
		iErr= Recv((BYTE*)&m_RecvBuf.buf[m_dwDataReadSoFar],
							 m_dwDataToRecv?m_dwDataToRecv - m_dwDataReadSoFar :m_RecvBuf.len,//if req len is zero then receive as much as possible
							 dwBytesReceived,
							 m_dwErrcode[FD_READ_BIT]
							 );		
		if(iErr==SOCKET_ERROR && m_dwErrcode[FD_READ_BIT]!=WSAEWOULDBLOCK)
		{
			DEBUGLOG(LOI_ERROR,_T("ISocket::Recv Failed for Socket %d"),m_Socket);
			ShutDown();
			return TRUE;
		}
		 m_dwDataReadSoFar+=dwBytesReceived;
		 if(m_dwDataReadSoFar==0 ) //no data read ..try  to get data in next read
							return TRUE;
		 if(m_dwDataReadSoFar<m_dwDataToRecv)//read data is less than what we supposed to receive
		 {
			 //DBGPRINT(_T("WSARecv %d Received on socket %d"),m_dwDataReadSoFar,m_Socket);
			 return TRUE;// still lots of data to be read before notifying the server/client
		 }
	}

	return FALSE;
}
/********************************************************
this functions recv from the socket & overridable for custom implementation
*********************************************************/
INT ISocket::Recv(BYTE *pByBuffer,DWORD dwReqRecvData,DWORD &dwBytesReceived,DWORD &dwLastError)
{
	WSABUF			wsarecvbuf[1]={0};
	int				iErr;
	DWORD			dwFlags=0;
	
	dwBytesReceived=0;
	wsarecvbuf[0].buf =(CHAR*)pByBuffer;
	wsarecvbuf[0].len =dwReqRecvData;  
	iErr=WSARecv( m_Socket,
				  wsarecvbuf,
				  1,
				  &dwBytesReceived,
				  &dwFlags,NULL,NULL);
	dwLastError=WSAGetLastError();
	return iErr ;
}
/*************************************************************
Notifies the user when the required bytes is read
*************************************************************/
void ISocket::NotifyOnReadData(DWORD dwDataLengthReq)
{
	m_dwDataToRecv=dwDataLengthReq;
	if(m_dwDataToRecv>m_RecvBuf.len)
	{
	   delete m_RecvBuf.buf;
	   m_RecvBuf.buf= new CHAR[m_dwDataToRecv];
	   m_RecvBuf.len = m_dwDataToRecv;
	   ZeroMemory(m_RecvBuf.buf,m_RecvBuf.len);
	}
}
/*****************************************************************
Reads the data from the byte stream
*****************************************************************/
BYTE* ISocket::GetReadData(DWORD &dwReadLen)
{
	dwReadLen=0;
	if(m_dwDataReadSoFar)
	{
		dwReadLen=m_dwDataReadSoFar ;  
		m_dwDataToRecv= m_dwDataReadSoFar=0;//notified received buffer.. reset the read queue items
		return (BYTE*)m_RecvBuf.buf;  
	}
	return NULL;
}
/**************************************************
raises events internally to notify the caller about sock operation
used in SSL socket
**************************************************/
void ISocket::RaiseEvents(LONG lNetworkEvent)
{
	SetEvent (m_hSocketEvent );
	m_lRaisedEvents|=lNetworkEvent;
}
/*************************************************
Gets the network events from the socket
************************************************/
LONG ISocket::EnumNetworkEvents()
{
	WSANETWORKEVENTS	wsaNetworkEvents={0};
	LONG				lNetworkEvents;

	WSAEnumNetworkEvents(m_Socket,m_hSocketEvent,&wsaNetworkEvents);
	lNetworkEvents=wsaNetworkEvents.lNetworkEvents ;
	memcpy(&m_dwErrcode[0],&wsaNetworkEvents.iErrorCode[0],sizeof(wsaNetworkEvents.iErrorCode));

	//append events generated internally
	lNetworkEvents|=m_lRaisedEvents;
	m_lRaisedEvents=0;

	return lNetworkEvents;
}