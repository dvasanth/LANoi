#pragma once
#include <atlcoll.h>
#include <map>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include "error.h"
 using namespace std;
typedef struct __STREAM_BUFFER {
		DWORD				dwBytesSendPos;
		DWORD				dwByteOffset;
		DWORD				dwTotalLen;
		BYTE			    *buf;
} STREAM_BUFFER ,*LPSTREAM_BUFFER ;

#define SOCKIO_TIMEOUT_BIT		12
#define SOCKIO_TIMEOUT			(1 << SOCKIO_TIMEOUT_BIT)

#define SOCKIO_ERROR_BIT		13
#define SOCKIO_ERROR			(1 << SOCKIO_ERROR_BIT)

#define USER_CANCEL_BIT			14
#define USER_CANCEL			   (1 << USER_CANCEL_BIT)
class ISocket;
/*******************************************
Caller has to inherit the function to get about the notification about sockets
********************************************/
class ISockNotify{
public:
	virtual   void OnRequestedReceive(ISocket *pSocket, BYTE *pbyReceived,DWORD dwRecvLen){}
	virtual   void OnAccept(ISocket *pSocket){}
	virtual   void OnConnectComplete(ISocket *pSocket,DWORD dwLastError){}
	virtual	  void OnClose(ISocket *pSocket,DWORD dwLastError){}

};
class ISocket:public CError
{
public:
	ISocket(SOCKET socket,ISockNotify *pSockNotify);
	ISocket(ISockNotify *pSockNotify);
	void ConstrutInit(ISockNotify *pSockNotify);
	~ISocket(void);
	///initialisation/termination routines as  a whole for all objects
	static void	Startup()
	{
		WSADATA					wsaData={0};
		WORD					wVersionRequested = MAKEWORD( 2, 2 );
		
		WSAStartup( wVersionRequested, &wsaData );
	}
	static void Cleanup()
	{
		WSACleanup(); 
	}
	////////////socket initilsation
	void			Init(SOCKET socket);
	void			Clear();
	operator		HANDLE();
private:
	SOCKET					m_Socket;
	PVOID					m_pSockParam;
	DWORD					m_dwLastError;
	static const DWORD      SOCKET_BUF_SIZE;
	/********************
	read related variables
	********************/
	DWORD					m_dwDataToRecv;
	WSABUF					m_RecvBuf;
	DWORD					m_dwDataReadSoFar;
	/*************************
	send related variables
	**************************/
	CAtlList<STREAM_BUFFER> m_SendStreamList;	

	/************************
	Sock event & unique handle value
	***********************/
	HANDLE					m_hSocketEvent;
	LONG					m_lRaisedEvents;


	/**************************************
	Event related last operation error code
	**************************************/
	BOOL					IsInternalEvent(long lNetworkEvents);
	/**********************
	socket last error code
	***********************/
	DWORD					m_dwErrorCode[FD_MAX_EVENTS];
public:
	/*********************************
	server & client winsock call related fucntions
	*********************************/
	BOOL			CreateServer(in_addr6 Ipv6BindAddress,USHORT &usListeningPort);
	BOOL			CreateServer(DWORD ipv4addr,USHORT &usListeningPort);
	SOCKET			AcceptIncomingIpv6Client(in_addr6 &ClientIpv6Addr);
	BOOL			ConnectServer(in_addr6 Ipv6ConnectAddress,USHORT usPort);
	BOOL			ConnectServerAsync(DWORD dwNetorderAddr,USHORT usPort);
	void			ShutDown();
	void			CloseSocket();
	/******************************************
	socket helper functions
	******************************************/
	in_addr6		GetIPv6SockAddress();
	in_addr6		GetIPv6PeerAddress();
	int				SetSockOpt(int optname,char *optval,int ioptval);
	LONG			EnumNetworkEvents();
	/*****************************************
	sock read & write related fucntions
	*****************************************/
	void			NotifyOnReadData(DWORD dwDataLengthReq);
	BYTE*			GetReadData(DWORD &dwReadLen);
	virtual void	SendData(BYTE *bySendBuffer,DWORD dwDataLength);
	BOOL			SendPendingData();
	void			MarkSocketCloseOnPendingSendComplete();
	/***********************************************
	buffer handling routines
	***********************************************/
	BYTE *			GetSendBuffer(DWORD dwLen){	return new BYTE[dwLen];}
	void			ReleaseSendBuffer(BYTE *pSendBuffer){delete pSendBuffer;}
	
	/**************************************************
	Socket can store Parameters for socket specific operation
	**************************************************/
	inline PVOID& SetSockParam(){return m_pSockParam;}
	inline PVOID  GetSockParam(){return m_pSockParam;} 
	/****************************************************
	overridable functions
	****************************************************/
	virtual void			OnSockEvent(LONG lNetworkEvents);
protected:
	/*********************
	Notification object to invoke
	*********************/
	ISockNotify				*m_pSockNotify;
	BOOL					m_bSocketCloseOnSendComplete;

	DWORD					m_dwErrcode[FD_MAX_EVENTS];
	virtual INT				Recv(BYTE *pByBuffer,DWORD dwReqRecvData,DWORD &dwBytesReceived,DWORD &dwLastError);
	void					RaiseEvents(LONG lNetworkEvent);

};
