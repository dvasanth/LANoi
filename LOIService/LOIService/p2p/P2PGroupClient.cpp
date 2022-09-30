#include "StdAfx.h"
#include "P2PGroupClient.h"
#include <strsafe.h>

const DWORD	CP2PGroupClient::KEEP_ALIVE_INTERVAL=99;


CP2PGroupClient::CP2PGroupClient(IP2PGroupClientNotify* pNotify):m_ProtocolHandle(this)
{
	m_pGroupClientNotify=pNotify;
	m_KeepAliveTimer.StartTimer(this,KEEP_ALIVE_INTERVAL); 
}

CP2PGroupClient::~CP2PGroupClient(void)
{
	m_KeepAliveTimer.StopTimer();
	CloseAllClient();


}
/********************
intialises the peer
***********************/
BOOL CP2PGroupClient::Init(CString sPeerUniqueID)
{
	m_sPeerID=sPeerUniqueID;
	return TRUE;
}

/**********************************************************
Main function exposed to the caller to connect a server hosted session
**********************************************************/
BOOL CP2PGroupClient::JoinServer(CLOISession *pLOISession ,CString sPeerName,USHORT usTCPReverseProxyPort,PCCERT_CONTEXT  pCertContext,in_addr6 ServerAddr,USHORT usPort)
{
	ISSLSocket		*pSSLSocket=NULL;	

	pSSLSocket= m_SSLSockIO.CreateSSLSocket(NULL,pCertContext,this); 

	if(!pSSLSocket->ConnectServer(ServerAddr,usPort))
	{
		SetLastErrorMessage(pSSLSocket->GetLastErrorMessage());
		m_SSLSockIO.Close( pSSLSocket);
		return FALSE;
	}
	//store the context parameters
	pSSLSocket->SetSockParam()=new CClientContext;
	((CClientContext*)pSSLSocket->GetSockParam())->m_sPeerName = sPeerName;
	((CClientContext*)pSSLSocket->GetSockParam())->m_usTCPReverseProxyPort=usTCPReverseProxyPort;
	((CClientContext*)pSSLSocket->GetSockParam())->m_pCertContext = pCertContext;
	((CClientContext*)pSSLSocket->GetSockParam())->m_pLOISession=pLOISession;
	m_SessionhSockMap.Insert(pLOISession,(HANDLE)*pSSLSocket);
	return TRUE;
	
}
/**************************************
clears all connected clients 
**************************************/
void CP2PGroupClient::CloseAllClient()
{
	//make sure no socket notification functions are invoked
	m_SSLSockIO.CancellAll();  

}
/************************
initiate a close on a single client connection
************************/
BOOL CP2PGroupClient::CloseClient(CLOISession *pLOISession)
{
	HANDLE		hSock;
	if(!m_SessionhSockMap.Get (pLOISession,hSock))
	{
		return FALSE;
	}
	//intitate a close for the connect client
	m_SSLSockIO.InvokeShutdown(hSock); 

	return TRUE;
}


/*******************************************************************
Invoked when the client socket is succcessfully connected to the server
********************************************************************/
void CP2PGroupClient::OnConnectComplete(ISSLSocket *pSSLSocket,DWORD dwLastError)
{
	if(dwLastError!=NO_ERROR)
	{
		//fill  the error code & notify about connection fail
		SetLastErrorMessage(IDS_ERR_FAILED_CONNECT_SERVER,dwLastError); 
		m_pGroupClientNotify->OnConnectingServerFailed(GetLastErrorMessage()); 
		m_pGroupClientNotify->OnClientClose( ((CClientContext*)pSSLSocket->GetSockParam())->m_pLOISession,dwLastError ) ;

		m_SessionhSockMap.Erase(((CClientContext*)pSSLSocket->GetSockParam())->m_pLOISession ); 
		delete ((CClientContext*)pSSLSocket->GetSockParam());
		m_SSLSockIO.Close( pSSLSocket);
		return;
	}
	//tell the sock about how much bytes ur interested to read from connected socket
	pSSLSocket->NotifyOnReadData ( sizeof(PROTO_CMD));
	//notify the caller about the connection status
	m_pGroupClientNotify->OnClientConnecting(((CClientContext*)pSSLSocket->GetSockParam())->m_pLOISession );
}

/******************************************************************
Notify the caller about connection close, disable sock notify & destroy context
******************************************************************/
void CP2PGroupClient::OnClose(ISSLSocket *pSSLSocket,DWORD dwLastError)
{
	//this means invoked due to abrupt connection close
	m_pGroupClientNotify->OnClientClose( ((CClientContext*)pSSLSocket->GetSockParam())->m_pLOISession,dwLastError );
	m_SessionhSockMap.Erase(((CClientContext*)pSSLSocket->GetSockParam())->m_pLOISession); 
	CertFreeCertificateContext(((CClientContext*)pSSLSocket->GetSockParam())->m_pCertContext );
	delete ((CClientContext*)pSSLSocket->GetSockParam());
	m_SSLSockIO.Close( pSSLSocket);
}

/***************************************************************
This is invoked when ssl server has validated the client certificate
***************************************************************/
void CP2PGroupClient::OnOutboundHandshakeComplete(ISSLSocket *pSSLSocket)
{
	DWORD				dwPacketSize=0;
	BYTE				*pProtocolData=NULL;
	PEER_INFORMATION    ClientInfo={0};

	StringCbCopy(ClientInfo.szPeerName,sizeof( ClientInfo.szPeerName),(LPCTSTR)((CClientContext*)pSSLSocket->GetSockParam())->m_sPeerName);
	StringCbCopy(ClientInfo.szPeerID,sizeof(ClientInfo.szPeerID),(LPCTSTR)m_sPeerID);
	ClientInfo.usReverseProxyPort= ((CClientContext*)pSSLSocket->GetSockParam())->m_usTCPReverseProxyPort;
	//fill the source ipv6 address for authentication
	ClientInfo.IPv6TeredoAddress = pSSLSocket->GetIPv6SockAddress();
	//get auth packet size required
	m_ProtocolHandle.FormClientAuthPacket(ClientInfo,pProtocolData,dwPacketSize);
	pProtocolData= pSSLSocket->GetSendBuffer(dwPacketSize);
	//now the protocol is formed & filled in the buffer
	m_ProtocolHandle.FormClientAuthPacket(ClientInfo,pProtocolData,dwPacketSize);
	//send the data to the server
	pSSLSocket->SendData(pProtocolData,dwPacketSize);
}
/*********************************************************************
Requested size is received just pass the receievd bytes to command dispatcher 
it will invoke the protocol function depending on the command
*********************************************************************/
void CP2PGroupClient::OnRequestedReceive(ISSLSocket *pSSLSocket, BYTE *pbyReceived,DWORD dwRecvLen)
{

	if(dwRecvLen)
	{
		DWORD		 dwPendingDataTobeRead;
		dwPendingDataTobeRead=m_ProtocolHandle.HandleCmd(pbyReceived,dwRecvLen,((CClientContext*)pSSLSocket->GetSockParam())->CmdType,*pSSLSocket); 
		pSSLSocket->NotifyOnReadData(dwPendingDataTobeRead);
	}
}
/*************************************************************
dispatched when a peer joins the group
*************************************************************/
void CP2PGroupClient::OnPeerJoin(ISSLSocket &SSLSocket,PEER_INFORMATION PeerInformation)
{
	((CClientContext*)SSLSocket.GetSockParam())->m_pLOISession->UpdateSessionPeerInfo(TRUE,&PeerInformation,NULL);   
}
/************************************************************
Dispatched when a peer leaves the group
************************************************************/
void CP2PGroupClient::OnPeerLeave(ISSLSocket &SSLSocket,in_addr6 PeerIPv6Addr)
{
	((CClientContext*)SSLSocket.GetSockParam())->m_pLOISession->UpdateSessionPeerInfo(FALSE,NULL,&PeerIPv6Addr);  
}

/***********************************************************
Dispatched when the session string is received
***********************************************************/
void CP2PGroupClient::OnSessionInfoRecv(ISSLSocket &SSLSocket,LPCTSTR lpszPortList)
{
	((CClientContext*)SSLSocket.GetSockParam())->m_pLOISession->ParseSessionXML(lpszPortList);   
}
/***************************************************
dispatched when the all information is pushed to the client
***************************************************/
void CP2PGroupClient::OnServerHandshakeComplete(ISSLSocket &SSLSocket)
{
	m_pGroupClientNotify->OnClientConnectedServer( ((CClientContext*)SSLSocket.GetSockParam())->m_pLOISession); 
}
/***********************************************
this is notification from session about dynamic port change in session
sends the dynamic port string to the server
***********************************************/
void CP2PGroupClient::OnDynamicUpdateChange(CLOISession *pLOISession,LPCTSTR lpszDynamicPortString)
{

	DWORD			dwPacketSize=0;
	BYTE			*pProtocolData=NULL;
	HANDLE			hSock;

	//get port update packet size required
	m_ProtocolHandle.FormDynamicSessionPacket((LPCTSTR)lpszDynamicPortString,pProtocolData,dwPacketSize);
	pProtocolData= new BYTE [dwPacketSize];//calle will free the memory
	//now the protocol is formed & filled in the buffer
	m_ProtocolHandle.FormDynamicSessionPacket((LPCTSTR)lpszDynamicPortString,pProtocolData,dwPacketSize);
	//send the data to the server

	if(!m_SessionhSockMap.Get(pLOISession,hSock))
	{
		delete pProtocolData;
		return;
	}
	m_SSLSockIO.SendData(hSock,pProtocolData,dwPacketSize);

}
/********************************************************************
Sends keepalive packet to the server so the peer will be condidered offline if not recevived within that
server recv time
********************************************************************/
void CP2PGroupClient::SendKeepAlive()
{

	DWORD			dwPacketSize=0;
	BYTE			*pProtocolData=NULL;
	HANDLE			*phSock=NULL;
	DWORD			dwItemCount=0;

	phSock = m_SessionhSockMap.GetItems(dwItemCount); 
	if(phSock==NULL)
	{
		return;
	}

	//get port update packet size required
	m_ProtocolHandle.FormKeepAlivePacket(KEEP_ALIVE_INTERVAL,pProtocolData,dwPacketSize);
	pProtocolData= new BYTE [dwPacketSize];//calle will free the memory
	//now the protocol is formed & filled in the buffer
	m_ProtocolHandle.FormKeepAlivePacket(KEEP_ALIVE_INTERVAL,pProtocolData,dwPacketSize);
	
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
	m_SessionhSockMap.FreeItems(phSock);
}

/****************************************************
timer to send keepalive packets to the server
****************************************************/
void CP2PGroupClient::OnTimer()
{
	SendKeepAlive();
}

