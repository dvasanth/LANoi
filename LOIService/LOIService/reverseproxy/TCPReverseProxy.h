#pragma once
#include "error.h"
#include "../p2p/p2peventnotify.h"
#include "../p2p/ProtocolRequestHandler.h"
#include "../sockio/socketio.h"
#include "../p2p/P2PGroupServer.h"


/*******************
stores info abt the server contexts
*******************/
class CReverseTCPContext{
private:
	BOOL		m_bServer;
	BOOL		m_bRemoteContext;
public :
	CReverseTCPContext(BOOL bServer,BOOL bRemoteContext)
	{
		m_bServer=bServer;
		CmdType=WAIT_FOR_COMMAND;
		m_hPairedSocket=INVALID_HANDLE_VALUE;
		m_bRemoteContext=bRemoteContext;
	}
	virtual ~CReverseTCPContext(){}
	BOOL IsListeningContext()
	{
		return m_bServer; 
	}	
	BOOL IsRemoteContext()
	{
		return m_bRemoteContext;
	}
	PROTO_CMD_TYPE			CmdType;
	HANDLE					m_hPairedSocket;
	in_addr6				m_PeerIpv6Addr;
};
///event notifier -- notifies about connected client status
class IP2PReverseProxyNotify{
public:
	virtual CLOISession*	OnClientConnect(in_addr6 IPV6Address,USHORT usPort){return FALSE;}
};

class CTCPReverseProxy:public IProtocolCommandNotify<ISocket> ,
					   public ISockNotify,
					   public CError
{
public:
	CTCPReverseProxy(void);
	~CTCPReverseProxy(void);
	BOOL				StartServer(in_addr6 IPv6Address,IP2PReverseProxyNotify *pEventNotify);
	void				StopServer();
	USHORT				GetListeningPort();
	void				CloseSession(CLOISession* pLOISession);
private:
	CSocketIO							m_SockIO;
	USHORT								m_usListeningPort;
	IP2PReverseProxyNotify				*m_pP2PReverseProxyNotify;
	CProtocolRequestHandler<ISocket>	m_ProtocolHandle;
	CMTSMap	<HANDLE,CLOISession*>		m_SessionhRemoteSockMap;
	virtual   void		OnRequestedReceive(ISocket *pSocket, BYTE *pbyReceived,DWORD dwRecvLen);
	virtual	  void		OnClose(ISocket  *pSocket,DWORD dwLastError);
	virtual   void		OnAccept(ISocket *pSocket);

	//protocol handlers
	virtual	DWORD		OnReverseProxyConnect(ISocket &SocketObj,USHORT usPort,PROTO_CMD_TYPE &NextState);
	virtual DWORD		OnLocalClientRecv(ISocket &SocketObj,BYTE *ByValidateData,DWORD dwBytesReceived,PROTO_CMD_TYPE &NextState);
	virtual DWORD		OnRemoteClientRecv(ISocket &SocketObj,BYTE *ByValidateData,DWORD dwBytesReceived,PROTO_CMD_TYPE &NextState);
	DWORD				GetFirstLocalIPAddresses(void);
	///Socket param macro
	inline CReverseTCPContext*		GetContext(ISocket *pSocket)
	{
		return ((CReverseTCPContext*)pSocket->GetSockParam()) ;
	}
	
};
