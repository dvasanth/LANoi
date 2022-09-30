#pragma once
#include <winsock2.h>
#include <in6addr.h>
#include "../resource.h"
#include "P2PGroupServer.h"
#include "P2PGroupClient.h"
#include "LOIsession/LOISessionContainer.h"
#include "invmanager/p2pinvitation.h"
#include "../DDNS/DynamicDNS.h"
#include "../reverseproxy/TCPReverseProxy.h"
#include "IPC/IPCCommandHandler.h"
#include "regoperations.h"
#include "../firewall/Firewall.h"
typedef enum ___UI_NOTIFY_CHANGES
{
	P2P_STOPPED,//lparam contains error string in lptstr
	CONNECTED_INVITER,
	RECV_SESSION_FROM_INVITER,
	RECV_SESSION_FROM_LOOPBACK,
	NEW_PEER_ONLINE,//lparam contains peername in lptstr
	NEWER_VERSION_CLIENT //lparam contains peer name in lptstr
}UI_NOTIFY_CHANGES;
///event notifier -- notifies about client status & command received from server peer

class CP2PManagerDefaultRPC:public IRPCExecute
{
private:
	IRPCProxyStub *m_pUINotify;
public:
	CP2PManagerDefaultRPC(IRPCProxyStub *pUIExecute)
	{
		m_pUINotify=pUIExecute;
	}
	~CP2PManagerDefaultRPC()
	{
	}
	virtual BOOL		GetP2PStatus()
	{
		m_pUINotify->PostP2PStatusChange(FALSE); 
		return FALSE;
	}
	virtual BOOL CreateInvitation(CString sNickName,CString sPassword,CString sSessionName,CString sSessionXMLString,FILETIME ftNotValidAfterTime,CString sFilepath,BYTE *pbyAppdata,DWORD dwAppDataLen)
	{
		SetLastErrorMessage(IDS_P2P_FAILED_TO_START); 
		m_pUINotify->PostCreateInvitationStatus(NULL,FALSE,GetLastErrorMessage());  
		return FALSE;
	}

	virtual BOOL JoinInvitation(CString sPeerName,CString sFilePath,CString sPassword)
	{
		SetLastErrorMessage(IDS_P2P_FAILED_TO_START); 
		m_pUINotify->PostJoinInvitationStatus(FALSE,GetLastErrorMessage(),NULL,NULL);  
		return FALSE;
	}

};

class CP2PManager:public IRPCExecute,
				  public IP2PGroupClientNotify,
				  public IP2PGroupServerNotify,
				  public IP2PReverseProxyNotify,
				  public ITimerNotify
{
public:
	enum STATE{
		RUNNING,
		STOPPED
	};
	CP2PManager();
	~CP2PManager(void);
	BOOL				Init(in_addr6 IPv6TeredoAddr,IRPCProxyStub *pUIExecute);
	/***************************************
	rpc call related
	***************************************/
	virtual BOOL		CreateInvitation(CString sNickName,CString sPassword,CString sSessionName,CString sSessionString,FILETIME ftNotValidAfterTime,CString sFilepath,BYTE *pbyAppdata,DWORD dwAppDataLen);
	virtual BOOL		JoinInvitation(CString sPeerName,CString sFilePath,CString sPassword);
	virtual BOOL		FirewallAdd(DWORD dwProcessID);
	virtual BOOL		GetP2PStatus();

	BOOL				GetConnectedPeersList(PEER_INFORMATION **ppPeerList,USHORT &dwPeerList);
	void				FreeConnectedPeerList(PEER_INFORMATION *pPeerList);
	void				Close();
	virtual void		CloseSession(CString sSessionID);
	STATE				GetState();
	
	static void			FreeLparamData(LPARAM lParam);
private:
	CDynamicDNS				m_DDNS;
	CTCPReverseProxy		m_ReverseProxy;
	CLOISessionContainer	m_LOIContainer;
	CP2PGroupServer			m_P2PServer;
	CP2PGroupClient			m_P2PClient;
	STATE					m_State;
	IRPCProxyStub			*m_pUINotify;
	CTimer					m_ClearIdleSession;
	DWORD					m_dwMinutesForIdleTimeout;
	/******************************************************
	Server related message handling
	******************************************************/
	virtual CLOISession*    OnClientCertSubmit(PCCERT_CONTEXT pCertContext,PCCERT_CONTEXT pServerCert);
	virtual void			OnIncomingClientConnectionClose(CLOISession* pLOISession);
	virtual void			OnServerConnectionClose();
	void					CloseServerSession(CLOISession *pLOISession);
	/*******************************************************************
	Client related objects
	*******************************************************************/
	virtual void			OnClientConnecting(CLOISession	*m_pLOISession);
	virtual void			OnClientClose(CLOISession	*m_pLOISession,DWORD dwLastError);
	virtual void			OnConnectingServerFailed(CString sErrorMessage);
	virtual void			OnClientConnectedServer(CLOISession *m_pLOISession);
	/***************************************
	reverse proxy related notification
	****************************************/
	CLOISession*			OnClientConnect(in_addr6 IPV6Address,USHORT usPort);
	/*************************************
	session idle timeout
	**************************************/
	void					OnTimer();
};
