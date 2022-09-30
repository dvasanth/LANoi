#pragma once
#include "Error.h"
#include "thread.h"

#include <winsock2.h>
#include <Ws2tcpip.h>
#include <atlcoll.h>
#include <map>
#include "ISocket.h"
#include "isslsocket.h"
 using namespace std;


#define SOCKIO_TIMEOUT_BIT		12
#define SOCKIO_TIMEOUT			(1 << SOCKIO_TIMEOUT_BIT)

#define SOCKIO_ERROR_BIT		13
#define SOCKIO_ERROR			(1 << SOCKIO_ERROR_BIT)

#define USER_CANCEL_BIT			14
#define USER_CANCEL			   (1 << USER_CANCEL_BIT)

class CSocketIO:public CError,IThread 
{
public:
	CSocketIO(void);
	virtual ~CSocketIO(void);
	/****************************************************
	Socket creation related fucntions
	****************************************************/
	ISocket *		CreateSocket(ISockNotify *pSockNotify);
	ISSLSocket *	CreateSSLSocket(LPTSTR lpszServername,PCCERT_CONTEXT  pCertContext,ISSLSocketNotify *pSSLSockNotify);
	void			Close(ISocket *pSocket);
	ISocket *		Accept(ISocket *pSocket,in_addr6 &ClientIpv6Addr,ISockNotify *pSockNotify);
	ISSLSocket *	Accept(ISSLSocket *pSocket,in_addr6 &ClientIpv6Addr,LPTSTR lpszServername,PCCERT_CONTEXT  pCertContext,ISSLSocketNotify *pSSLSockNotify);
	/******************
	send/close/cancellall related caller needs to be invoked from another thread
	*****************/
	BOOL			SendData(HANDLE hSock,BYTE *bySendBuffer,DWORD dwDataLength);
	void			InvokeShutdown(HANDLE hSocket);
	void			CancellAll();
	void			InitiateSocketClose(HANDLE hSock);

	enum THREAD_MSG{
		SEND_DATA=WM_USER+10,
		SEND_SHUTDOWN,
		ADD_TO_SOCKET_NOTIFY,
		CANCEL_SOCKET_NOTIFY,
		CANCEL_ALL=WM_QUIT,

		

	};
private:
	static const DWORD        SOCKET_BUF_SIZE;
	map <HANDLE, ISocket*>	  m_EventHandleMap;
	virtual DWORD			  OnThreadRun();
	LONG					  GetSockEvents(ISocket *&ppSocket,BOOL &bExit);
	/********************************************
	thread message handlers
	********************************************/
	void	     			  OnSendDataMsg(HANDLE hSock,BYTE *bySendBuffer,DWORD dwDataLength);
	void					  OnCancelSocketNotify(HANDLE hSocket);
	void					  AddToSocketNotify(ISocket *pIoContext);
	void					  OnAddToSocketNotify(HANDLE hSocket,ISocket *pIoContext);
	void					  OnCancelAll();
	void					  OnInitiateSocketClose(HANDLE hSock);
protected:
	virtual DWORD		OnThreadMsg(UINT uiMsg,WPARAM wParam,LPARAM lParam);
};

