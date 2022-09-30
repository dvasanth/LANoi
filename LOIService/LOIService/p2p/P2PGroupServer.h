#pragma once

#include "P2PGroupClient.h"
#include <time.h>
#include "../sockio/SocketIO.h"

class CServerContext:public CClientContext
{
private:
	BOOL		m_bServer;
public :
	CServerContext(BOOL bServer)
	{
		m_bServer=bServer;
		ZeroMemory(&ClientIPv6Addr,sizeof(ClientIPv6Addr));
	}
	virtual ~CServerContext(){}
	BOOL IsListeningContext()
	{
		return m_bServer; 
	}
	in_addr6	ClientIPv6Addr;


};

///event notifier -- notifies about client status & command received from server peer
class IP2PGroupServerNotify{
public:
	virtual CLOISession*    OnClientCertSubmit(PCCERT_CONTEXT pCertContext,PCCERT_CONTEXT pServerCertContext){return NULL;}
	virtual void			OnServerConnectionClose(){}
	virtual void			OnIncomingClientConnectionClose(CLOISession* pLOISession){}
	
};

class CP2PGroupServer:public CError,
					  public ISSLSocketNotify,
					  public IProtocolCommandNotify<ISSLSocket> ,
					   public IDynamicSessionUpdateNotify,
					   public ITimerNotify

{
public:
	CP2PGroupServer(void);
	~CP2PGroupServer(void);

	//
	BOOL				StartServer(IP2PGroupServerNotify *pP2PGroupNotify,PCCERT_CONTEXT  pCertContext,DWORD dwVirtualAddressStart,in_addr6 Ipv6BindAddr);
	void				StopServer();
	in_addr6			GetListeningAddr();
	USHORT				GetListeningPort();
	void				InitialiseSession(CLOISession *pLOISession,CString sPeerName,CString sPeerUniqueID,USHORT usReverseProxyPort,CString sSessionName,CString sSessionXMLString);
	DWORD				CloseClients(CLOISession *pLOISession);
private:
	CProtocolRequestHandler	<ISSLSocket> m_ProtocolHandle;
	IP2PGroupServerNotify				*m_pP2PGroupNotify;
	CSocketIO							m_SSLSockIO;
	CMTSMap <CLOISession*,HANDLE>		m_SessionsToSockMap;
	in_addr6							m_IPv6ListeningAddr;
	CTimer								m_KeepAliveTimer;	
	USHORT								m_usListeningPort;
	volatile DWORD						m_dwIPV4AddressStart;
	volatile DWORD						m_dwPeerAddressCount;
	DWORD								m_dwServerAddress;
	PCCERT_CONTEXT						m_pServerSSLCert;


	
	// socket notification fucntions
	virtual	  void			OnClose(ISSLSocket  *pSSLSocket,DWORD dwLastError);
	virtual   void			OnAccept(ISSLSocket *pSSLSocket);
	virtual   void			OnRequestedReceive(ISSLSocket *pSSLSocket, BYTE *pbyReceived,DWORD dwRecvLen);
//ssl socket notification
	virtual   void			OnInboundHandshakeComplete(ISSLSocket *pSSLSocket,PCCERT_CONTEXT pClientCertContext);
// protocol disptach fucntions
	virtual void			OnClientAuthenticate(ISSLSocket &SSLSocket,PEER_INFORMATION PeerInformation);
	virtual void			OnDynamicSessionUpdate(ISSLSocket &SSLSocket,LPCTSTR lpszPortList);
//peer proto/session hlprs
	void							UpdateSessionInformation(HANDLE hSocket,LPCTSTR lpszPortList);
	void							UpdateSessionPeerList(HANDLE hSock,BOOL bAddOrRemove,PEER_INFORMATION *pPeerList,DWORD dwPeerListCount);
	void							SendHandShakeCompleted(HANDLE hSock);
	void							SendSessionData(ISSLSocket *pSSLSocket);
	void							BroadCastUserStateChange(CLOISession *pLOISession, BOOL bAddOrRemove,PEER_INFORMATION PeerData);
	void							BroadCastSessionInformation(CLOISession *pLOISession, LPCTSTR lpszSessionString);
	void							SendKeepAlive();
	void							OnTimer();
	/*******************
	notitcation from session
	*******************/
	virtual void					OnDynamicUpdateChange(CLOISession *pLOISession,LPCTSTR lpszsDynamicPortString);
	///Socket param macro
	inline CServerContext*  GetContext(ISSLSocket *pSocket)
	{
		return ((CServerContext*)pSocket->GetSockParam()) ;
	}


	inline void InitStartAddress(DWORD dwVirtualAddressStart )
	{
		srand ( (UINT)time(NULL) );
		USHORT					  usRandStartAddressRange=rand()%64000;
		m_dwIPV4AddressStart=(dwVirtualAddressStart&0xffff0000)|(usRandStartAddressRange);
		m_dwServerAddress=m_dwIPV4AddressStart;//first address is for server
	}
	///ipaddress asignment related
	inline DWORD GetServerAddress()
	{
		return htonl(m_dwServerAddress);
	}
	inline DWORD GetNextAddress()
	{
		BYTE				byHostID;
		volatile DWORD		dwSubNet=m_dwIPV4AddressStart&0x0000ffff;//xy <- 192.168 .x.y

		InterlockedIncrement((LONG*)&dwSubNet);
		byHostID=(BYTE)dwSubNet&0x000000ff;
		if(byHostID==0 || byHostID==255)
		{
			InterlockedIncrement((LONG*)&dwSubNet);
		}
		dwSubNet&=0x0000ffff;
		m_dwIPV4AddressStart=(dwSubNet|(m_dwIPV4AddressStart&0xffff0000));//new start address
		return htonl(m_dwIPV4AddressStart);
	}
};


