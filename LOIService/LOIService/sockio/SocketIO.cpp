#include "StdAfx.h"
#include "SocketIO.h"


const DWORD  CSocketIO::SOCKET_BUF_SIZE=30*1460;

CSocketIO::CSocketIO(void)
{
	CreateThread(); 
	ISocket::Startup();
	
}

CSocketIO::~CSocketIO(void)
{
	WaitForThreadExit();
	ISocket::Cleanup();
}
/****************************************************
Creates the socket interface which exposes all winsock functions
****************************************************/
ISocket *CSocketIO::CreateSocket(ISockNotify *pSockNotify)
{
	ISocket		*pSocket=NULL;

	pSocket=new ISocket(pSockNotify);

	//map for fast search on event notify
	AddToSocketNotify(pSocket);

	return pSocket;
}
/**********************************************************
Create the ssl socket interface
**********************************************************/
ISSLSocket *CSocketIO::CreateSSLSocket(LPTSTR lpszServername,PCCERT_CONTEXT  pCertContext,ISSLSocketNotify *pSSLSockNotify)
{
	ISSLSocket   *pSocket=NULL;


	pSocket=new ISSLSocket(lpszServername,pCertContext, pSSLSockNotify);
	//map for fast search on event notify
	AddToSocketNotify(pSocket);

	return pSocket;
}

/********************************************
Accepts returns the socket we need create the object with socket
Use overloaded construtor for this
********************************************/
ISocket *CSocketIO::Accept(ISocket *pArgSocket,in_addr6 &ClientIpv6Addr,ISockNotify *pSockNotify)
{
	SOCKET		ClientSock;
	ISocket		*pSocket=NULL;
	
	ClientSock=pArgSocket->AcceptIncomingIpv6Client(ClientIpv6Addr);
	if(ClientSock==INVALID_SOCKET)
		return NULL;
	pSocket=new ISocket(ClientSock,pSockNotify);
	//map for fast search on event notify
	AddToSocketNotify(pSocket);
	return pSocket;
}
/************************************************
above routine for ssl  socket creation
************************************************/
ISSLSocket *CSocketIO::Accept(ISSLSocket *pArgSocket,in_addr6 &ClientIpv6Addr,LPTSTR lpszServername,PCCERT_CONTEXT  pCertContext,ISSLSocketNotify *pSSLSockNotify)
{
	SOCKET		ClientSock;
	ISSLSocket	*pSocket=NULL;

	ClientSock=pArgSocket->AcceptIncomingIpv6Client(ClientIpv6Addr);
	if(ClientSock==INVALID_SOCKET)
							return NULL;
	pSocket=new ISSLSocket(lpszServername,pCertContext,ClientSock,pSSLSockNotify);
	//map for fast search on event notify
	AddToSocketNotify(pSocket);

	return pSocket;
}
/***************************
Clear the object invoked only from the sock thread context
***************************/
void CSocketIO::Close(ISocket *pSocket)
{
	m_EventHandleMap.erase(HANDLE(*pSocket));
	DEBUGLOG(LOI_INFO,_T("CSocketIO::Close %X HANDLE count %d"),HANDLE(*pSocket),m_EventHandleMap.size() );
	delete pSocket;
}
/******************************************************
Main single thread which handles the sock events or dispatch message posted
to the thread message
******************************************************/
DWORD CSocketIO::OnThreadRun()
{
	LONG									lNetworkEvents;
	BOOL									bExit=FALSE;
	ISocket								   *pEventSocket;

	while(1)//keep dispatching events still exit
	{
		pEventSocket=NULL;
		//wait for the sock events
		lNetworkEvents=GetSockEvents(pEventSocket,bExit);
		//if exit 
		if(bExit)
			break;
	
		//dispatch socket message
		if(pEventSocket)
			pEventSocket->OnSockEvent(lNetworkEvents);
	}

	return 0;
	
}
/********************************************************
Waits for the socket handle events in the map or diapatch message if received
********************************************************/
LONG CSocketIO::GetSockEvents(ISocket* &ppISocket,BOOL &bExit)
{
	HANDLE				 *pEvents=NULL;
	DWORD				  dwEventCount=2;
	DWORD_PTR			  dwRetVal;
    void				  *phEventHandle=NULL;
	UINT				   iCount=0;
	LONG				   lNetworkEvents;
	map<HANDLE, ISocket*> ::iterator iteratorEvents;


	do{
		lNetworkEvents=0;
		iCount=0;
		pEvents=NULL;

		if(m_EventHandleMap.size())
		{
			pEvents= new HANDLE[m_EventHandleMap.size()];

			for (iteratorEvents = m_EventHandleMap.begin();iteratorEvents != m_EventHandleMap.end();iteratorEvents++ )
			{
				//initalise events
				pEvents[iCount++]=(*iteratorEvents).first;

			}
		}

		//if count is greter than max then time slice and wait
		dwRetVal= MsgWaitForEvents(iCount,pEvents);
		if(dwRetVal == WAIT_OBJECT_0  )
		{
			//we have received a message
			//dispatch message
			DispatchMessage( bExit);
			if(bExit)break;//if exit then return
			delete pEvents;//free memory so to check handles to listen
			continue;
		}	
		if(dwRetVal>=WAIT_OBJECT_0+1 && dwRetVal<(WAIT_OBJECT_0+iCount+1))
		{
			ppISocket = m_EventHandleMap[pEvents[dwRetVal-WAIT_OBJECT_0-1]];
			lNetworkEvents =ppISocket->EnumNetworkEvents(); 
		}
		break;//error case
	}while(1);

	delete pEvents;
	return lNetworkEvents;//notify the event
}

/****************************************************************
//handle the message posted to the thread
*****************************************************************/
DWORD CSocketIO::OnThreadMsg(UINT uiMsg,WPARAM wParam,LPARAM lParam)
{
	DWORD					dwRet=0;
	CSocketIO::THREAD_MSG	ThreadMsg=(THREAD_MSG)uiMsg;

	switch(ThreadMsg)
	{
		case SEND_DATA:
			OnSendDataMsg((HANDLE)wParam,(PBYTE)((WSABUF*)lParam)->buf,((WSABUF*)lParam)->len);
			delete PVOID(lParam);
			break;
		case ADD_TO_SOCKET_NOTIFY:
			OnAddToSocketNotify((HANDLE)lParam,(ISocket *)wParam);
			break;
		case CANCEL_SOCKET_NOTIFY:
			OnCancelSocketNotify((ISocket *)wParam);
			break;
		case SEND_SHUTDOWN:
			OnInitiateSocketClose((HANDLE)wParam);
			break;
		case CANCEL_ALL:
			OnCancelAll();
			break;
	}

	return dwRet;
}
/*********************************
Shutdown send on the socket 
**********************************/
void CSocketIO::InitiateSocketClose(HANDLE hSock)
{
	PostThreadMessage(SEND_SHUTDOWN,(WPARAM)hSock,0); 
}
/*************************
call shutdown with SD_SEND so it will invoke FD_CLOSE event
*************************/
void CSocketIO::OnInitiateSocketClose(HANDLE hSock)
{
	ISocket				*pSocket=NULL;

	
	if(m_EventHandleMap.find(hSock)!= m_EventHandleMap.end() )
	{
		pSocket = m_EventHandleMap[hSock];
		DEBUGLOG(LOI_INFO,_T("CSocketIO::OnInitiateSocketClose %X"),hSock);
		//complete all send data
		if(pSocket->SendPendingData())
		{
			pSocket->ShutDown(); 
		}
		else
		{
			//still pending data just mark for close
			pSocket->MarkSocketCloseOnPendingSendComplete(); 
		}
		return;
	}
}
/******************************************
this send can be invoked from any thread
******************************************/
BOOL CSocketIO::SendData(HANDLE hSock,BYTE *bySendBuffer,DWORD dwDataLength)
{
	//add to buffer list
	WSABUF			  *pWsaBuf=new 	WSABUF;

	
	pWsaBuf->buf =(CHAR*)bySendBuffer;
	pWsaBuf->len = dwDataLength;
	PostThreadMessage(SEND_DATA,(WPARAM)hSock,(LPARAM)pWsaBuf); 
	return TRUE;
}
/***********************
Send message handleer
***********************/
void CSocketIO::OnSendDataMsg(HANDLE hSock,BYTE *bySendBuffer,DWORD dwDataLength)
{
	STREAM_BUFFER		QueueItem={0,0,dwDataLength,bySendBuffer};
	ISocket				*pSocket=NULL;

	
	if(m_EventHandleMap.find(hSock)!= m_EventHandleMap.end() )
	{
		pSocket = m_EventHandleMap[hSock];
		//DEBUGLOG(LOI_INFO,_T("CSocketIO::OnSendDataMsg %d"),dwDataLength);
		pSocket->SendData(bySendBuffer,dwDataLength); 
		return;
	}
	DEBUGLOG(LOI_ERROR,_T("CSocketIO::OnSendDataMsg can't find associated socket %d"),hSock);
	//can't find associated socket so free memory
	delete bySendBuffer;
}



/********************************
this queues a close to a socket & can be invoked from any thread
********************************/
void CSocketIO::InvokeShutdown(HANDLE hSocket)
{
	PostThreadMessage(CANCEL_SOCKET_NOTIFY,(WPARAM)hSocket,0);
}
/**************************************
Just invoke shutdown so it initiate FD_CLOSE event on the socket
**************************************/
void CSocketIO::OnCancelSocketNotify(HANDLE hSocket)
{
	ISocket				*pSocket=NULL;


	if(m_EventHandleMap.find(hSocket)!= m_EventHandleMap.end() )
	{	
		pSocket = m_EventHandleMap[hSocket];
		DEBUGLOG(LOI_INFO,_T("CSocketIO::OnCancelSocketNotify %X"),hSocket);
		//we invoke the close on the socket
		pSocket->OnSockEvent(USER_CANCEL); 
	}
}
/****************************
add can be invoked from any thread
****************************/
void CSocketIO::AddToSocketNotify(ISocket *pIoContext)
{
	HANDLE hSocket=HANDLE(*pIoContext);
	PostThreadMessage(ADD_TO_SOCKET_NOTIFY,(WPARAM)pIoContext,(LPARAM)hSocket);
}
/******************
add message handler 
(*******************/
void CSocketIO::OnAddToSocketNotify(HANDLE hSocket,ISocket *pIoContext)
{
	//map for fast search on event notify
	pair <HANDLE, ISocket*> MapItem(hSocket,pIoContext);

	m_EventHandleMap.insert(MapItem);
	DEBUGLOG(LOI_INFO,_T("CSocketIO::OnAddToSocketNotify %X handle count %d"),hSocket,m_EventHandleMap.size());
}
/***************************************
clears all the objects- if caller failed to release it
***************************************/
void CSocketIO::CancellAll()
{
	PostThreadMessage(CANCEL_ALL,0,0);
	WaitForThreadExit(); 
}
/***************************************
messagen handler for cancel all
***************************************/
void CSocketIO::OnCancelAll()
{
	map<HANDLE, ISocket*> ::iterator		iteratorEvents;
	//invoke onclose for sock context & clear all sock memory allocated
	while( m_EventHandleMap.begin() != m_EventHandleMap.end() )
	{
		iteratorEvents =  m_EventHandleMap.begin();
		DEBUGLOG(LOI_INFO,_T("CSocketIO::OnCancelAll %X"),(*iteratorEvents).first);
		//invoke close on the socket
		//tell the user about socket close caller will do the cleanup
		(*iteratorEvents).second->OnSockEvent(USER_CANCEL);  
    }
}
