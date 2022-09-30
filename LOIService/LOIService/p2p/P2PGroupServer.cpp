#include "StdAfx.h"
#include "P2PGroupServer.h"
#include "P2PGroupClient.h"

#include <strsafe.h>
#include "loisession\LOISessionStringFormat.h"


CP2PGroupServer::CP2PGroupServer(void):m_ProtocolHandle((IProtocolCommandNotify*)this)
{
	m_dwIPV4AddressStart=0x7f010000;//default value 127.1.0.1 series
	m_dwPeerAddressCount=0;
	m_pP2PGroupNotify=NULL;
	m_pServerSSLCert=NULL;
	m_KeepAliveTimer.StartTimer(this,CP2PGroupClient::KEEP_ALIVE_INTERVAL); 
}

CP2PGroupServer::~CP2PGroupServer(void)
{
	m_KeepAliveTimer.StopTimer();
	StopServer();

}
/************************************************
puts the server in listening mode with the cert provider for ssl handshake
************************************************/
BOOL CP2PGroupServer::StartServer(IP2PGroupServerNotify *pP2PGroupNotify,PCCERT_CONTEXT  pCertContext,DWORD dwVirtualAddressStart,in_addr6 Ipv6BindAddr)
{
	ISSLSocket				 *pSSLSocket=NULL;


	m_pP2PGroupNotify=pP2PGroupNotify;
	InitStartAddress(dwVirtualAddressStart);
	
	
	//fetch the teredo address first
	pSSLSocket = m_SSLSockIO.CreateSSLSocket(_T(""), pCertContext,this);
	if(!pSSLSocket->CreateServer(Ipv6BindAddr,m_usListeningPort))
	{
		//failed to start the server
		m_SSLSockIO.Close(pSSLSocket);
		SetLastErrorMessage(pSSLSocket->GetLastErrorMessage()); 
		return FALSE;
	}
	m_IPv6ListeningAddr=Ipv6BindAddr;
	pSSLSocket->SetSockParam()=new CServerContext(TRUE);  
	m_pServerSSLCert=pCertContext;
	return TRUE;
}
/***************************
Server paramters get routines
***************************/
in_addr6 CP2PGroupServer::GetListeningAddr()
{
	return m_IPv6ListeningAddr;
}
USHORT CP2PGroupServer::GetListeningPort()
{
	return m_usListeningPort;
}

/************************
user can forcilly call this
************************/
void CP2PGroupServer::StopServer()
{
	m_SSLSockIO.CancellAll(); 
	if(m_pServerSSLCert)
	{
		CertFreeCertificateContext(m_pServerSSLCert);
		m_pServerSSLCert=NULL;
	}
}
/*************************************************************************
This called to initialse a new session on the server & also adds the self peer information to
the session
*************************************************************************/
void CP2PGroupServer::InitialiseSession(CLOISession *pLOISession,CString sPeerName,CString sPeerUniqueID,USHORT usReverseProxyPort,CString sSessionName,CString sSessionXMLString)
{
	PEER_INFORMATION  SelfInfo={0};//server needs to add its own peer information & send it to others

	SelfInfo.dwIPv4Address = GetServerAddress();
	SelfInfo.dwFlag |= PEER_FLAG_ADMIN;
	SelfInfo.IPv6TeredoAddress = GetListeningAddr();
	SelfInfo.usReverseProxyPort = usReverseProxyPort;
	StringCbCopy(SelfInfo.szPeerName,sizeof(SelfInfo.szPeerName),(LPCTSTR)sPeerName);
	StringCbCopy(SelfInfo.szPeerID,sizeof(SelfInfo.szPeerID),(LPCTSTR)sPeerUniqueID);
	SelfInfo.usVersion = CProtocolRequestHandler<ISSLSocket> ::PROTOCOL_VERSION;

	//notify the peerinformation to the session
	pLOISession->AddSelfTeredoAddress(SelfInfo.IPv6TeredoAddress);
	pLOISession->SetSessionName((LPCTSTR)sSessionName);  

	pLOISession->UpdateSessionPeerInfo(TRUE,&SelfInfo,NULL);  
	pLOISession->ParseSessionXML((LPCTSTR) sSessionXMLString);

}
/*******************************************************************
Close the clients associated with session
*******************************************************************/
DWORD CP2PGroupServer::CloseClients(CLOISession *pLOISession)
{
	HANDLE		*phSock=NULL;
	DWORD		dwClients=0;

	///just pend all the clients for connection close
	phSock=m_SessionsToSockMap.GetItems(dwClients);
	if(dwClients)
	{
		for(int iClients=0;iClients<(int)dwClients;iClients++)
				m_SSLSockIO.InvokeShutdown(phSock[iClients]); 
	}
	return dwClients;
}
/*************************************************************************
Use the context to identify whether listening socket context or accepted client
socket
*************************************************************************/
void	CP2PGroupServer::OnClose(ISSLSocket  *pSSLSocket,DWORD dwLastError)
{
	if(GetContext(pSSLSocket)->IsListeningContext()   )
	{
		if(dwLastError)
		{
			//notify about server close
			m_pP2PGroupNotify->OnServerConnectionClose(); 
		}
		delete GetContext(pSSLSocket);
		m_SSLSockIO.Close(pSSLSocket);
		return;
	}
	//client close related operation
	CLOISession			*pLOISession=NULL;
	PEER_INFORMATION	 ClientInfo={0};

	
	pLOISession= GetContext(pSSLSocket)->m_pLOISession;

	
	//remove the sslsocket from the list
	m_SessionsToSockMap.EraseByValue(HANDLE(*pSSLSocket));

	//fill only the ipv6 address should be sufficient to identify the peer
	ClientInfo.IPv6TeredoAddress = pSSLSocket->GetIPv6PeerAddress(); 
	if(pLOISession)//sometimes connection slosed before handshake complete
	{
		//update the session information in server peer
		pLOISession->UpdateSessionPeerInfo(FALSE,&ClientInfo,&ClientInfo.IPv6TeredoAddress); 
		//notify all clients abt the leaving peer
		BroadCastUserStateChange(pLOISession,FALSE,ClientInfo);
		//notify the caller about the client connection close
		m_pP2PGroupNotify->OnIncomingClientConnectionClose(pLOISession);  
	}
	//remove the context created on accept
	delete GetContext(pSSLSocket);
	m_SSLSockIO.Close(pSSLSocket);
}
/****************************************************************
Create a context for the accepted socket & store the ipv6 address
****************************************************************/
void	CP2PGroupServer::OnAccept(ISSLSocket *pSSLSocket)
{
	in_addr6			ClientAddr={0};
	ISSLSocket          *pAcceptSocket=NULL;

	pAcceptSocket=m_SSLSockIO.Accept(pSSLSocket, ClientAddr,_T(""),m_pServerSSLCert,this);
	if(pAcceptSocket==NULL)
	{
		return ;//accepting the socket failed
	}
	pAcceptSocket->SetSockParam()=  new CServerContext   (FALSE);
	
	GetContext(pAcceptSocket)->CmdType = WAIT_FOR_COMMAND;	
	GetContext(pAcceptSocket)->ClientIPv6Addr = ClientAddr;
	GetContext(pAcceptSocket)->m_pLOISession=NULL;
	pAcceptSocket->NotifyOnReadData(sizeof(PROTO_CMD));

}
/***************************************
After handshake complete, we can get the cert to identify the session
***************************************/
void	CP2PGroupServer::OnInboundHandshakeComplete(ISSLSocket *pSSLSocket,PCCERT_CONTEXT pCertContext)
{
	CLOISession			*pLOISession=NULL;

	pLOISession=m_pP2PGroupNotify->OnClientCertSubmit(pCertContext,m_pServerSSLCert );
	if(pLOISession==NULL)
	{
		pSSLSocket->ShutDown();
		return;
	}
	GetContext(pSSLSocket)->m_pLOISession =  pLOISession;


}
/********************************************************************
Onreciving client data dispatch it to the protocol handler
********************************************************************/
void CP2PGroupServer::OnRequestedReceive(ISSLSocket *pSSLSocket, BYTE *pbyReceived,DWORD dwRecvLen)
{
	if(dwRecvLen)
	{
		DWORD		 dwPendingDataTobeRead;
		dwPendingDataTobeRead=m_ProtocolHandle.HandleCmd(pbyReceived,dwRecvLen,GetContext(pSSLSocket)->CmdType,*pSSLSocket); 
		pSSLSocket->NotifyOnReadData(dwPendingDataTobeRead);
	}
}
/**************************************************************
Validates client auth request
**************************************************************/
void CP2PGroupServer::OnClientAuthenticate(ISSLSocket &SSLSocket,PEER_INFORMATION PeerInformation)
{
	in_addr6			ClientGivenIPv6=PeerInformation.IPv6TeredoAddress;
	in_addr6			ConnectedIpv6= GetContext(&SSLSocket)->ClientIPv6Addr;

	if(memcmp(&ConnectedIpv6,&ClientGivenIPv6,sizeof(in_addr6))==0)
	{
		int			iTimeout=CP2PGroupClient::KEEP_ALIVE_INTERVAL+40;//allowance
		//user valid
		//assign ipv4 address

		PeerInformation.dwIPv4Address = GetNextAddress();
		
		//add the new peer inforamation to the server session
		GetContext(&SSLSocket)->m_pLOISession->UpdateSessionPeerInfo(TRUE,&PeerInformation,NULL); 

		//send new peer information to all connected client
		BroadCastUserStateChange(GetContext(&SSLSocket)->m_pLOISession,TRUE,PeerInformation);

		//map the session with socket handle
		m_SessionsToSockMap.Insert(GetContext(&SSLSocket)->m_pLOISession,SSLSocket);

		//send the session information to the client
		SendSessionData(&SSLSocket);

		//timeout set
		SSLSocket.SetSockOpt(SO_RCVTIMEO,(char*)&iTimeout,sizeof(iTimeout) );
		return;
	}
	//someone else
	SSLSocket.ShutDown( );

}
/***********************************************
Dynamic port updates sent by a client
************************************************/
void CP2PGroupServer::OnDynamicSessionUpdate(ISSLSocket &SSLSocket,LPCTSTR lpszDynamicPortList)
{
	OnDynamicUpdateChange(GetContext(&SSLSocket)->m_pLOISession,lpszDynamicPortList);
}

/*****************************************
sends the session information to the joined client
*****************************************/
void CP2PGroupServer::SendSessionData(ISSLSocket *pSSLSocket)
{
	PEER_INFORMATION		*pPeerList;
	USHORT					usPeerListCount=0;
	

	pPeerList=GetContext(pSSLSocket)->m_pLOISession->GetConnectedClientList(usPeerListCount);
	//send the connected peer listfirst
	UpdateSessionPeerList( HANDLE(*pSSLSocket),TRUE,pPeerList,(DWORD)usPeerListCount);
	GetContext(pSSLSocket)->m_pLOISession->FreeConnectedClientList(pPeerList);

	//send about the port information
	UpdateSessionInformation(HANDLE(*pSSLSocket),(LPCTSTR)GetContext(pSSLSocket)->m_pLOISession->GetSessionString());

	//send handshake completed command
	SendHandShakeCompleted(HANDLE(*pSSLSocket));
	
}

/********************************************************************
broadcast new user joined to all clients connected for this session
********************************************************************/
void CP2PGroupServer::BroadCastUserStateChange(CLOISession *pLOISession, BOOL bAddOrRemove,PEER_INFORMATION PeerData)
{
	DWORD				dwConnectedClients;
	HANDLE				*pClientSocks=NULL;


	pClientSocks=m_SessionsToSockMap.GetItemsByKey(pLOISession,dwConnectedClients); 
	for (int iCount =0;iCount < (int)dwConnectedClients;iCount++)
	{
		//update one by one the state change
		UpdateSessionPeerList(pClientSocks[iCount], bAddOrRemove,&PeerData,1);
	}
}
/********************************************************************
broadcast session information to all clients connected for this session
********************************************************************/
void CP2PGroupServer::BroadCastSessionInformation(CLOISession *pLOISession, LPCTSTR lpszSessionString)
{
	DWORD				dwConnectedClients;
	HANDLE				*pClientSocks=NULL;


	pClientSocks=m_SessionsToSockMap.GetItemsByKey(pLOISession,dwConnectedClients); 
	for (int iCount =0;iCount < (int)dwConnectedClients;iCount++)
	{
		//update one by one the state change
		UpdateSessionInformation(pClientSocks[iCount],lpszSessionString);
	}
}
/************************************************
updates the session information to the client 
*************************************************/
void CP2PGroupServer::UpdateSessionInformation(HANDLE hSocket,LPCTSTR lpszPortList)
{
	DWORD			dwPacketSize=0;
	BYTE			*pProtocolData=NULL;

	if(lpszPortList==NULL) return;
	//get port update packet size required
	m_ProtocolHandle.FormSessionPacket(lpszPortList,pProtocolData,dwPacketSize);
	pProtocolData= new BYTE [dwPacketSize];//calle will free the memory
	//now the protocol is formed & filled in the buffer
	m_ProtocolHandle.FormSessionPacket(lpszPortList,pProtocolData,dwPacketSize);
	DEBUGLOG(LOI_INFO,_T("Sending UpdateSessionInformation len %d"),dwPacketSize);
	m_SSLSockIO.SendData(hSocket,pProtocolData,dwPacketSize);
	
}
/**************************************************
updates the peer information to the client
***************************************************/
void CP2PGroupServer::UpdateSessionPeerList(HANDLE hSock,BOOL bAddOrRemove,PEER_INFORMATION *pPeerList,DWORD dwPeerListCount)
{
	DWORD			dwPacketSize=0;
	BYTE			*pProtocolData=NULL;

	//get port update packet size required
	m_ProtocolHandle.FormPeerAddorRemovePacket(bAddOrRemove,pPeerList,dwPeerListCount,pProtocolData,dwPacketSize);
	pProtocolData= new BYTE [dwPacketSize];//calle will free the memory
	//now the protocol is formed & filled in the buffer
	m_ProtocolHandle.FormPeerAddorRemovePacket(bAddOrRemove,pPeerList,dwPeerListCount,pProtocolData,dwPacketSize);
	DEBUGLOG(LOI_INFO,_T("Sending UpdateSessionPeerList len %d"),dwPacketSize);
	m_SSLSockIO.SendData(hSock,pProtocolData,dwPacketSize);
}

/********************************************************
Sends the handshake completed to the client 
********************************************************/
void CP2PGroupServer::SendHandShakeCompleted(HANDLE hSock)
{
	DWORD			dwPacketSize=0;
	BYTE			*pProtocolData=NULL;

	//get port update packet size required
	m_ProtocolHandle.FormHandshakePacket(pProtocolData,dwPacketSize);
	pProtocolData= new BYTE [dwPacketSize];//calle will free the memory
	//now the protocol is formed & filled in the buffer
	m_ProtocolHandle.FormHandshakePacket(pProtocolData,dwPacketSize);
	DEBUGLOG(LOI_INFO,_T("Sending HandShake complete len %d"),dwPacketSize);
	m_SSLSockIO.SendData(hSock,pProtocolData,dwPacketSize);

}
/**********************************
this is notification send from session abt new port changes in session
*********************************/
void CP2PGroupServer::OnDynamicUpdateChange(CLOISession *pLOISession,LPCTSTR lpszDynamicPortString)
{
	//get the session inf
	CString						sOldSessionString;
	CLOISessionStringFormat		LOIDynamicSessionFormat(lpszDynamicPortString);
	CLOISessionStringFormat     LOIExistingSessionXML(pLOISession->GetSessionString());
	//dynamic port information
	DWORD						dwBoundAddress=0,dwVirtualAddress=0;
	USHORT						usBoundPort=0;

	//add dynamic values to the old XML
	while(LOIDynamicSessionFormat.GetNextDynamicBoundValues(dwBoundAddress,usBoundPort,dwVirtualAddress))
	{
		LOIExistingSessionXML.AddDynamicBoundedAddress(dwBoundAddress,usBoundPort,dwVirtualAddress);
	}
	

	pLOISession->ParseSessionXML((LPCTSTR)LOIExistingSessionXML.GetXMLString());//add the new dynamic session iD

	 //update to all the connected clients the new port string
	BroadCastSessionInformation(pLOISession,(LPCTSTR)pLOISession->GetSessionString()); 
}

/********************************************************************
Sends keepalive packet to the client so the peer will be condidered offline if not able to get ack
from that peer
********************************************************************/
void CP2PGroupServer::SendKeepAlive()
{

	DWORD			dwPacketSize=0;
	BYTE			*pProtocolData=NULL;
	HANDLE			*phSock=NULL;
	DWORD			dwItemCount=0;

	phSock = m_SessionsToSockMap.GetItems(dwItemCount); 
	if(phSock==NULL)
	{
		return;
	}

	//get port update packet size required
	m_ProtocolHandle.FormKeepAlivePacket(CP2PGroupClient::KEEP_ALIVE_INTERVAL,pProtocolData,dwPacketSize);
	pProtocolData= new BYTE [dwPacketSize];//calle will free the memory
	//now the protocol is formed & filled in the buffer
	m_ProtocolHandle.FormKeepAlivePacket(CP2PGroupClient::KEEP_ALIVE_INTERVAL,pProtocolData,dwPacketSize);
	
	//send to all connected clients
	for(int iItems=0;iItems<(int)dwItemCount;iItems++)
	{
		BYTE			*pTemp=NULL;
		//allocate new buffer & copy the data
		pTemp= new BYTE[dwPacketSize];
		memcpy(pTemp,pProtocolData,dwPacketSize);
		m_SSLSockIO.SendData(phSock[iItems],pTemp,dwPacketSize);
	}
	delete pProtocolData;
	m_SessionsToSockMap.FreeItems(phSock);
}

/****************************************************
timer to send keepalive packets to the server
****************************************************/
void CP2PGroupServer::OnTimer()
{
	SendKeepAlive();
}

