#pragma once
#include "isocket.h"
#include "SSPISchannel.h"
#include <atlstr.h>

class ISSLSocketNotify;

class ISSLSocket :	public ISocket
{
public:
	ISSLSocket(LPTSTR lpszServername,PCCERT_CONTEXT  pCertContext,SOCKET socket,ISSLSocketNotify *pSockNotify);
	ISSLSocket(LPTSTR lpszServername,PCCERT_CONTEXT  pCertContext,ISSLSocketNotify *pSockNotify);
	virtual ~ISSLSocket(void);
	virtual void		SendData(BYTE *bySendBuffer,DWORD dwDataLength);
private:
	void		ConstructInit(LPTSTR lpszServername,PCCERT_CONTEXT  pCertContext,ISSLSocketNotify *pSSLNotify);
private:
	/**********************************
	SSL auth specific
	***********************************/
	CString					m_sServerName;
	PCCERT_CONTEXT			m_pCertContext;
	BOOL					m_bInboundsocket;				
	SSPI_STATE				m_SSLState;
	STREAM_BUFFER			m_DecryptedBuffer;
	STREAM_BUFFER			m_EncryptedBuffer;
	static const DWORD		STREAM_CONST_LEN;
	/********************************
	class wrapped to perform ssl specific task
	********************************/
	CSSPISchannel			m_SSPI;
	ISSLSocketNotify		*m_pSSLNotify;
	/**********************************
	ssl recv streaming fucntions
	***********************************/
	PBYTE					GetEncryptStream(DWORD &dwEncryptSteamSize);
	DWORD					GetDecryptStreamQueueDataSize();
	void					QueueDecryptStream(PBYTE pDecryptedStream,DWORD dwLen);
	BOOL					DequeueDecryptStream(PBYTE pDecryptBuffer,DWORD dwGetByteLen);
	void					FreeStreamData();
	/**************************************
	SSL send related function
	**************************************/

	BOOL					AllocateAndSend(BYTE *byEncBuffer,DWORD dwEncDataLength);
	/********************************************
	Overridden functions to handle ssl specfic operations
	*********************************************/

protected:
	virtual void		OnSockEvent(LONG lNetworkEvents);
	virtual INT			Recv(BYTE *pByBuffer,DWORD dwReqRecvData,DWORD &dwBytesReceived,DWORD &dwLastError);

};

#define INBOUND_HANDSHAKE_COMPLETE_BIT			15
#define INBOUND_HANDSHAKE_COMPLETE				(1 << INBOUND_HANDSHAKE_COMPLETE_BIT)

#define OUTBOUND_HANDSHAKE_COMPLETE_BIT			16
#define OUTBOUND_HANDSHAKE_COMPLETE				(1 << OUTBOUND_HANDSHAKE_COMPLETE_BIT)


 class ISSLSocketNotify:public ISockNotify{
private:
	//translate iocontext to ssliocontext
	virtual   void OnConnectComplete(ISocket *pSocket,DWORD dwLastError)
	{
		OnConnectComplete(dynamic_cast<ISSLSocket*>(pSocket),dwLastError);
	}
	virtual   void OnRequestedReceive(ISocket *pSocket, BYTE *pbyReceived,DWORD dwRecvLen)
	{
		OnRequestedReceive(dynamic_cast<ISSLSocket*>(pSocket),pbyReceived,dwRecvLen);
	}
	virtual   void OnAccept(ISocket *pSocket)
	{
		OnAccept(dynamic_cast<ISSLSocket*>(pSocket));
	}
	virtual	  void OnClose(ISocket *pSocket,DWORD dwLastError)
	{
		OnClose(dynamic_cast<ISSLSocket*>(pSocket),dwLastError);
	}
public:
	virtual   void OnAccept(ISSLSocket *pSSLSocket){}
	virtual	  void OnClose(ISSLSocket *pSSLSocket,DWORD dwLastError){}
	virtual   void OnConnectComplete(ISSLSocket *pSSLSocket,DWORD dwLastError){}
	virtual   void OnRequestedReceive(ISSLSocket *pSSLSocket, BYTE *pbyReceived,DWORD dwRecvLen){}
	//ssl related
	virtual   void OnInboundHandshakeComplete(ISSLSocket *pSSLSocket,PCCERT_CONTEXT pCertContext){}
	virtual   void OnOutboundHandshakeComplete(ISSLSocket *pSSLSocket){}

};