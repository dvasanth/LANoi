#pragma once
#include "mtsmap.h"
#include "LOIsession/LOISession.h"
#include "timer.h"
#include "../sockio/SocketIO.h"
#include <wincrypt.h>


class CClientContext
{
public :
	CClientContext()
	{
		CmdType=WAIT_FOR_COMMAND;
		m_pCertContext=NULL;
		m_pLOISession=NULL;
	}
	virtual ~CClientContext(){}
	CString						m_sPeerName;
	USHORT						m_usTCPReverseProxyPort;
	PROTO_CMD_TYPE				CmdType;
	PCCERT_CONTEXT				m_pCertContext;
	CLOISession					*m_pLOISession;
};	
///event notifier -- notifies about client status & command received from server peer
class IP2PGroupClientNotify{
public:
	virtual void OnClientConnecting(CLOISession	*m_pLOISession){}
	virtual void OnClientClose(CLOISession	*m_pLOISession,DWORD dwLastError){}
	virtual void OnConnectingServerFailed(CString sErrorMessage){}
	virtual void OnClientConnectedServer(CLOISession *m_pLOISession){}

};
class CP2PGroupClient:public IProtocolCommandNotify<ISSLSocket> ,
					 public CError,
					 public ISSLSocketNotify,
					 public ITimerNotify,
					 public IDynamicSessionUpdateNotify
{
public:
	CP2PGroupClient(IP2PGroupClientNotify *);
	virtual ~CP2PGroupClient(void);
	
	BOOL		Init(CString sPeerUniqueID);
	BOOL		JoinServer(CLOISession *pLOISession ,CString sPeerName,USHORT usTCPReverseProxyPort,PCCERT_CONTEXT  pCertContext,in_addr6 ServerAddr,USHORT usPort);
	BOOL		CloseClient(CLOISession *pLOISession);
	void		CloseAllClient();
	static const DWORD			KEEP_ALIVE_INTERVAL;
private:
	CSocketIO							m_SSLSockIO;
	CProtocolRequestHandler<ISSLSocket>	m_ProtocolHandle;
	CMTSMap	<CLOISession*,HANDLE>		m_SessionhSockMap;
	IP2PGroupClientNotify				*m_pGroupClientNotify;
	CTimer								m_KeepAliveTimer;		
	CString								m_sPeerID;
		virtual void OnClientAuthenticate(ISSLSocket &SSLSocket,PEER_INFORMATION PeerInformation){};
	/////////////timer notification
	virtual void OnTimer();
	// socket notification fucntions
	virtual		void OnClose(ISSLSocket  *pSSLSocket,DWORD dwLastError);
	virtual		void OnConnectComplete(ISSLSocket *pSSLSocket,DWORD dwLastError);
	virtual		void OnRequestedReceive(ISSLSocket *pSSLSocket, BYTE *pbyReceived,DWORD dwRecvLen);
	//ssl socket notification
	virtual		void OnOutboundHandshakeComplete(ISSLSocket *pSSLSocket);
	//protocol handle function
	virtual		void OnPeerJoin(ISSLSocket &SSLSocket,PEER_INFORMATION PeerInformation);
	virtual		void OnPeerLeave(ISSLSocket &SSLSocket,in_addr6 PeerIPv6Addr);
	virtual		void OnSessionInfoRecv(ISSLSocket &SSLSocket,LPCTSTR lpszPortList);
	virtual		void OnServerHandshakeComplete(ISSLSocket &SSLSocket);
	void		SendKeepAlive();
	virtual void OnDynamicUpdateChange(CLOISession *pLOISession,LPCTSTR lpszDynamicPortString);
};
