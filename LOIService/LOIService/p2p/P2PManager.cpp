#include "StdAfx.h"
#include "P2PManager.h"

#include <psapi.h>
//#include ".\appintegrate\AppIntegrate.h"
#include <strsafe.h>



CP2PManager::CP2PManager():m_P2PClient(this) 
{
	CRegOperations  Config;

	m_State=STOPPED;
	m_pUINotify=NULL;
	m_dwMinutesForIdleTimeout=Config.GetIdleTimeout();
	m_ClearIdleSession.StartTimer(this,m_dwMinutesForIdleTimeout*60); 
}

CP2PManager::~CP2PManager(void)
{
	m_ClearIdleSession.StopTimer(); 
	Close();
}
/***********************************************
this initailises the server
***********************************************/
BOOL CP2PManager::Init(in_addr6 IPv6TeredoAddr,IRPCProxyStub *pUIExecute)
{
	BOOL				bRet;
	CRegOperations		Reg;
	CP2PInvitation      InvitataionCACert;
	PCCERT_CONTEXT		pCACert=NULL;

	pCACert= InvitataionCACert.GetCACert(); 
	if(pCACert==NULL)
	{
		SetLastErrorMessage(InvitataionCACert.GetLastErrorMessage());
		return FALSE;
	}

	bRet=m_P2PServer.StartServer(this,
								pCACert,//after use server needs to free the context
								Reg.GetStartVirtualAddress(_T("")),
								IPv6TeredoAddr
								);
	if(!bRet)
	{
		SetLastErrorMessage(m_P2PServer.GetLastErrorMessage());
		return FALSE;
	}
	
	//intitialise the PNRP
	m_DDNS.InitPNRP();

	//ask the ddns to update the information
	bRet=m_DDNS.RegisterDNSName(m_P2PServer.GetListeningAddr(),
				        		m_P2PServer.GetListeningPort(),
								NULL,
								0
								);
	if(!bRet)
	{
		////
		SetLastErrorMessage(m_DDNS.GetLastErrorMessage());
		return FALSE;
	}

	//initialse the reverse proxy 
	bRet=m_ReverseProxy.StartServer(m_P2PServer.GetListeningAddr(),
									this
									);
	if(!bRet)
	{
		SetLastErrorMessage(m_ReverseProxy.GetLastErrorMessage());
		return FALSE;
	}

	//client initialise
	m_P2PClient.Init(m_DDNS.GetDNSName());

	//initialse the session container
	m_LOIContainer.Init();
	m_pUINotify =pUIExecute;
	m_State=RUNNING;			
	m_pUINotify->PostP2PStatusChange(TRUE); //currently one way communication
	return TRUE;

}
/**************************************
Release all p2p related objects
**************************************/
void CP2PManager::Close()
{	
	m_ReverseProxy.StopServer();
	m_P2PServer.StopServer();
	m_P2PClient.CloseAllClient();  

	if(m_pUINotify)//sometimes close happens before intitalisation gettting success
		m_pUINotify->PostP2PStatusChange(FALSE); //currently one way communication
	m_LOIContainer.Close(); 
	m_pUINotify=NULL;
	m_State=STOPPED;

}
/*******************************
Returns the current 
********************************/
CP2PManager::STATE CP2PManager::GetState()
{
	return m_State;
}
/*********************************************************
creates the invitation & saves the file in the path
then creates a session with server peer information added
*********************************************************/
BOOL CP2PManager::CreateInvitation(CString sNickName,CString sPassword,CString sSessionName,CString sSessionXMLString,FILETIME ftNotValidAfterTime,CString sFilepath,BYTE *pbyAppdata,DWORD dwAppDataLen)
{
	CP2PInvitation				Invitation;
	CString						sSessionID;
	CLOISession					*pLOISession=NULL;

	//set the parameters of the invitation
	Invitation.SetAddrInfo(m_P2PServer.GetListeningAddr(),m_P2PServer.GetListeningPort());
	Invitation.SetNickname(sNickName);
	Invitation.SetSessionXMLString(sSessionXMLString);
	Invitation.SetSessionName(sSessionName);  
	Invitation.SetP2PDDNSName(m_DDNS.GetDNSName()  );
	Invitation.SetSessionName(  sSessionName);
	Invitation.SetSessionXMLString (sSessionXMLString); 
	Invitation.SetAppData(pbyAppdata,dwAppDataLen); 

	//invitation file 
	if(!Invitation.CreateSecureInvitation(sFilepath,sPassword,ftNotValidAfterTime))
	{
		SetLastErrorMessage(Invitation.GetLastErrorMessage());
		m_pUINotify->PostCreateInvitationStatus(sSessionName,FALSE,Invitation.GetLastErrorMessage());  
		return FALSE;
	}
	
	sSessionID = Invitation.GetInvitationObjectID(); 

	//ask the session container to create a session
	pLOISession=m_LOIContainer.GetSession(sSessionID,&m_P2PServer);

	//tell the server abt the session information
	m_P2PServer.InitialiseSession(pLOISession,sNickName,m_DDNS.GetDNSName(), 
								 m_ReverseProxy.GetListeningPort(),
								 sSessionName,sSessionXMLString);   

	m_pUINotify->PostCreateInvitationStatus(sSessionName,TRUE,NULL);  
	return TRUE;
}

/********************************************************
Joins  the session with the invitation file
The session is joined with the mentioned peer name
********************************************************/
BOOL CP2PManager::JoinInvitation(CString sPeerName,CString sFilePath,CString sPassword)
{
	CP2PInvitation				Invitation;
	PCCERT_CONTEXT				pClientCert;
	CString						sSessionID;
	CLOISession					*pLOISession=NULL;
	in_addr6					IPv6addr={0};
	USHORT						usServerPort=0;
	PEER_INFORMATION			ClientInfo={0};
	BYTE						*pbyAppData=NULL;
	DWORD						dwAppDataLen=0;

	//load  the parameters from the invitation file
	pClientCert=Invitation.LoadSecureInvitation(sFilePath,sPassword);
	if(pClientCert==NULL)
	{
		SetLastErrorMessage(Invitation.GetLastErrorMessage());
		m_pUINotify->PostJoinInvitationStatus(FALSE, Invitation.GetLastErrorMessage(),NULL,NULL);
		return FALSE;
	}
 

	//check if certicate is expired
	if(Invitation.IsInvitationExpired())
	{
		CString sFormat,sMsg;

		sFormat.LoadString( IDS_NETWORK_TIME_EXPIRED);
		sMsg.Format((LPCTSTR)sFormat,Invitation.GetInvitationExpiryTime()); 
		SetLastErrorMessage(sMsg);
		m_pUINotify->PostJoinInvitationStatus(FALSE, GetLastErrorMessage(),NULL,NULL);
		return FALSE;
	}

	//if same ddns then don't connect
	if(Invitation.GetP2PDDNSName()==m_DDNS.GetDNSName() ||
		m_LOIContainer.IsSessionExists(  Invitation.GetInvitationObjectID())//if already joined
		)
	{
		SetLastErrorMessage(IDS_CONNECTING_SAME_SYSTEM); 
		m_pUINotify->PostJoinInvitationStatus(FALSE, GetLastErrorMessage(),Invitation.GetSessionName(),Invitation.GetNickname()   );
		return FALSE;
	}

	
	//notify about app data received
	pbyAppData=Invitation.GetAppData(dwAppDataLen);
	if(pbyAppData)
	{
		m_pUINotify->PostInvitationAppData(pbyAppData,dwAppDataLen); 
	}

	//resolve the DDNS name to ipv6 address & port
	if(!m_DDNS.ResolveDNSName(Invitation.GetP2PDDNSName(),
						   IPv6addr,
						   usServerPort)
						   )
	{
		SetLastErrorMessage(m_DDNS.GetLastErrorMessage());
		m_pUINotify->PostJoinInvitationStatus(FALSE, GetLastErrorMessage(),Invitation.GetSessionName(),Invitation.GetNickname()  );
		return FALSE;
	}
						  
	//ask the session container to create a session
	pLOISession=m_LOIContainer.GetSession(Invitation.GetInvitationObjectID(),&m_P2PClient);

	pLOISession->SetSessionName( (LPCTSTR)Invitation.GetSessionName()); 

	//join the session with the client auth
	if(!m_P2PClient.JoinServer( pLOISession,
								sPeerName,
								m_ReverseProxy.GetListeningPort(), 
								pClientCert,//has to be freed after use
								IPv6addr,
								usServerPort)
								)
	{
		SetLastErrorMessage(m_P2PClient.GetLastErrorMessage());
		m_pUINotify->PostJoinInvitationStatus(FALSE, GetLastErrorMessage(),Invitation.GetSessionName(),Invitation.GetNickname() );
		return FALSE;
	}
	//caller will be notified status later
	return TRUE;
}
/*******************************************************
Handling server related events
********************************************************/
/**********************************************************************
Invoked when incoming client sends a valid client cert
***********************************************************************/
CLOISession*  CP2PManager::OnClientCertSubmit(PCCERT_CONTEXT pCertContext,PCCERT_CONTEXT pServerCert)
{
	CP2PInvitation			CachedInvite;
	CString					sSessionID;

	//check if its a valid client invitation
	if(!CachedInvite.IsValidClientCert(pCertContext,pServerCert))
	{
		return NULL;
	}

	//load the invitation details from cache
	if(!CachedInvite.LoadCachedInvitation(pCertContext))
	{
		return NULL;//failed to get from store
	}

	sSessionID = CachedInvite.GetInvitationObjectID ();


	//check if the session already exists
	if(!m_LOIContainer.IsSessionExists(sSessionID))
	{
		CLOISession			*pLOISession=NULL;

		//ask the session container to create a session
		pLOISession=m_LOIContainer.GetSession(sSessionID,&m_P2PServer);

		//tell the server abt the session information so its adds the server peername also
		m_P2PServer.InitialiseSession(pLOISession,
									  CachedInvite.GetNickname(),
									  m_DDNS.GetDNSName(), 
									  m_ReverseProxy.GetListeningPort(),
									  CachedInvite.GetSessionName(), 
									  CachedInvite.GetSessionXMLString()
									  );   
	}

	//return the already exists -- increment count for each client connected
	return m_LOIContainer.GetExistingSession(sSessionID);
}
/****************************************
Client conneced to the server closed
****************************************/
void	CP2PManager::OnIncomingClientConnectionClose(CLOISession* pLOISession)
{
	//one client disconnected decrement it once
	if(m_LOIContainer.Release(pLOISession)==1)//last object count probably server alone exists
	{
		///check if session needs to be closed
		if(pLOISession->GetState()==CLOISession::USER_CLOSE)
		{
			PEER_INFORMATION		*pPeerList;
			USHORT					usPeerListCount=0;
			pPeerList=pLOISession->GetConnectedClientList ( usPeerListCount);
			if(usPeerListCount)
			{
				pLOISession->FreeConnectedClientList(pPeerList); 
				if(usPeerListCount==1)//last one means only server peer is online destroy session
				{
					m_ReverseProxy.CloseSession( pLOISession);
					m_LOIContainer.Release(pLOISession);
				}
			}
		}
	}

}

/*******************************
Check for any idle sessions & remove it
*******************************/
void CP2PManager::OnTimer()
{
	CLOISession		**ppIdleSessions=NULL;
	DWORD			dwItemCount=0;

	ppIdleSessions=m_LOIContainer.GetIdleSessions(m_dwMinutesForIdleTimeout,dwItemCount);
	if(dwItemCount)
	{
		for(DWORD dwCount=0; dwCount<dwItemCount;dwCount++)
		{
			//if the session doesn't below to client then ask server peer to close
			if(!m_P2PClient.CloseClient( ppIdleSessions[dwCount]))
			{
				//its a server session
				CloseServerSession(ppIdleSessions[dwCount]);
			}
			ppIdleSessions[dwCount]->SetState(CLOISession ::USER_CLOSE ); 
		}
		m_LOIContainer.FreeSessionList(ppIdleSessions,dwItemCount);
	}
}
/**********************************
close the clients connected & destroy session if no client connected
***********************************/
void CP2PManager::CloseServerSession(CLOISession *pLOISession)
{
	if(m_P2PServer.CloseClients(pLOISession)==0)
	{
		//no peer connected
		m_LOIContainer.Release(pLOISession);
	}
}
/***************************************
Invoked by the caller to close a particular session identified by string
****************************************/
void CP2PManager::CloseSession(CString sSessionID)
{
	CLOISession  *pLOISession=NULL;

	pLOISession = m_LOIContainer.GetExistingSession(sSessionID);
	if(pLOISession)
	{
		pLOISession->SetState(CLOISession ::USER_CLOSE ); 

		//we don't session is in client or server
		//so invoke close on both
		CloseServerSession(pLOISession);
		m_P2PClient.CloseClient( pLOISession);
		m_LOIContainer.Release(pLOISession);//release the temporary created object
	}
}
/*****************************************************************
Server failed to listen on the socket- most probably adapter down
*****************************************************************/
void 	CP2PManager::OnServerConnectionClose()
{
	///server connection close
	//m_pUINotify->OnAbruptFailure(); 
}




/****************************************************
Client event handling
****************************************************/
/****************************************************
Client Updation status
****************************************************/
void CP2PManager::OnClientConnecting(CLOISession	*m_pLOISession)
{
	//notify the status
	//m_pUINotify->OnJoiningInviter(); 
}
/*************************************
Connection to the server failed 
Server is not reachable or shutdown
*************************************/
void CP2PManager::OnConnectingServerFailed(CString sErrorMessage)
{
	//notify the status abt connection failure
	//m_pUINotify->OnJoinInvitationFailed(sErrorMessage); 
}
/********************************
Connection to the server completed
********************************/
void CP2PManager::OnClientConnectedServer(CLOISession *pLOISession)
{
	TCHAR		szSessionName[MAX_PATH]={0};
	TCHAR		szInviterName[MAX_PATH]={0};

	pLOISession->GetSessionName(szSessionName,sizeof(szSessionName));
	pLOISession->GetInviterName(szInviterName,sizeof(szInviterName));
	//notify status abt connection complete
	m_pUINotify->PostJoinInvitationStatus(TRUE,_T(""),szSessionName,szInviterName);
}
/***********************************
Connection ended with the server ended 
No more session is needed
***********************************/
void CP2PManager::OnClientClose(CLOISession	*pLOISession,DWORD dwLastError)
{

	TCHAR		szSessionName[MAX_PATH]={0};
	CError		Err;
	BOOL		bUserClose=(pLOISession->GetState()==CLOISession::USER_CLOSE);

	pLOISession->GetSessionName(szSessionName,sizeof(szSessionName));
	Err.SetLastErrorMessage( IDS_ERR_CLOSE_FAILED,dwLastError?dwLastError:ERROR_UNEXP_NET_ERR);
	//notify status abt connection failure
	m_pUINotify->PostJoinInvitationStatus(FALSE,bUserClose?NULL:Err.GetLastErrorMessage() ,szSessionName,NULL);
	//close any reverse proxy connection active
	m_ReverseProxy.CloseSession( pLOISession);
	//client connection closed
	//destroy the session
	m_LOIContainer.Release(pLOISession);
}




/****************************************
reverse proxy notifications
*****************************************/
/**************************************
Reverse proxy asking whether the client can be allowed o proxy
*************************************/
CLOISession* CP2PManager::OnClientConnect(in_addr6 IPV6Address,USHORT usPort)
{
	//ask container whether is this ipv6 address exists
	return m_LOIContainer.IsProxyAllowed(IPV6Address,usPort);  
}


/*********************************
Firewall rule addition for ipv6 enabled application
**********************************/
BOOL CP2PManager::FirewallAdd(DWORD dwProcessID)
{
	TCHAR			szProcessName[MAX_PATH] = TEXT("<unknown>");
	CFirewall		AddRule;


	HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION |PROCESS_VM_READ,
                                   FALSE, dwProcessID );

	// Get the process name.
	if (NULL != hProcess )
	{
		if(GetModuleFileNameEx ( hProcess,NULL, szProcessName,sizeof(szProcessName)/sizeof(TCHAR) ))
		{
			DEBUGLOG(LOI_INFO,_T("AddFirewallRule %s"),szProcessName);
			AddRule.AddFirewallRule((LPCTSTR)szProcessName);  
		}
		CloseHandle( hProcess );
	}
	return TRUE;
}
/*********************
always true if the instance exits
************************/
BOOL CP2PManager::GetP2PStatus()
{
	m_pUINotify->PostP2PStatusChange(TRUE);  
	return TRUE;
}