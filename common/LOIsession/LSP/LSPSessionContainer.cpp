#include "StdAfx.h"
#include "LSPSessionContainer.h"
#include <strsafe.h>

CLSPSessionContainer*		CLSPSessionContainer::s_pInstance=NULL;
/*********************************
singleton class always returns the same object
*********************************/
CLSPSessionContainer* CLSPSessionContainer::GetInstance()
{
	if(s_pInstance==NULL)
	{
		s_pInstance=new CLSPSessionContainer();

	}
	return s_pInstance;
}
/******************************
Destroys the singleton object created during unload most probably
******************************/
void CLSPSessionContainer::DestroyInstance()
{
	if(s_pInstance)
	{
		delete s_pInstance;
		s_pInstance=NULL;
	}
}
/**************************************
construtor needs to be private here
Initialise the thread to wait for event notification from service . IN case event set then serialising the items
**************************************/
CLSPSessionContainer::CLSPSessionContainer(void)
{
	//serialize the session information from service
	RefreshTheValuesFromService();
	CreateThread(); 
	TCHAR   szAppPath[MAX_PATH]={0};
	GetModuleFileName (NULL,szAppPath,sizeof(szAppPath)/sizeof(szAppPath[0]));
	if(_tcsrchr(szAppPath,'\\'))
	StringCbCopy(m_szAppName,sizeof(m_szAppName),_tcsrchr(szAppPath,'\\')+1);
}
/***********************************
destructor 
*********************************/
CLSPSessionContainer::~CLSPSessionContainer(void)
{
	WaitForThreadExit(3000);
}

/*********************
thread to wait for event notification from LOI service
**********************/
DWORD CLSPSessionContainer::OnThreadRun()
{
	DWORD_PTR					dwRetVal;
	HANDLE						hServiceSessionSyncEvent;

	hServiceSessionSyncEvent=OpenEvent(SYNCHRONIZE ,FALSE, UI_TO_LSP_NOTIFY_NAME);
	if(hServiceSessionSyncEvent==NULL)
	{
		return 0;
	}


	do{
		dwRetVal=MsgWaitForEvents(1,&hServiceSessionSyncEvent);
		if(dwRetVal == WAIT_OBJECT_0 )
		{
			BOOL bExit;
			//we have received a message
			//dispatch message
			DispatchMessage( bExit);
			if(bExit)break;//if exit then return
			continue;
		}	
		if(dwRetVal==WAIT_OBJECT_0+1  )
		{
			//sesssion information changed
			RefreshTheValuesFromService();

		}
		else
		{
			break;//any error case
		}
	}while(1);

	return 0;
}
/**********************************************
Serialise the values from the service
*********************************************/
VOID CLSPSessionContainer::RefreshTheValuesFromService()
{
	CIPCFileMap				SessionListIPC(m_IpcInfo[SESSION_ITEM].szIPCName,CIPCFileMap::load);

	if(SessionListIPC.Open(0))
	{
		Serialize(SessionListIPC);
	}
}
/****************************************************
Add the information about dynamically added session parameters
****************************************************/
void CLSPSessionContainer::AddLocalBoundInfo(const std::string sSessionID, DWORD dwBoundAddress,USHORT usBoundPort,in_addr6 IPv6BoundAddress)
{
	CIPCFileMap							DynamicUpdateMap(m_IpcInfo[DYNAMIC_SESSION_UPDATE_ITEM].szIPCName ,CIPCFileMap::store);
	CDynamicSessionInfoUpdate			DynamicUpdate;
	TCHAR								szSessionID[MAX_PATH]={0};

	//dbgprint(_T(" CLSPLOISession::AddLocalBoundInfo %X:%X"), dwBoundAddress,usBoundPort);
	if(DynamicUpdateMap.Open(m_IpcInfo[DYNAMIC_SESSION_UPDATE_ITEM].dwMaxSize ))
	{
		StringCbPrintf(szSessionID,sizeof(szSessionID),_T("%hs"), sSessionID.c_str());
		DynamicUpdate.UpdateDynamicSessionInfo(DynamicUpdateMap,szSessionID,dwBoundAddress,usBoundPort,GetIpv4Address(sSessionID,IPv6BoundAddress));
		NotifyUIInstance();
	}
}
/*****************************
Winsock connect validation routine
traverse the session & find anybody is interested in divertion
*****************************/
CONNECT_TYPE CLSPSessionContainer::IsConnectDivertRequired(DWORD dwIPV4AddressToConnect,USHORT usPortToConnect,in_addr6 &IPv6TeredoAddress,USHORT &usDivertedPort,std::string &sSessionID)
{
	multimap<std::string,CLOISession*>::iterator	items;
	CONNECT_TYPE								Ret=DEFAULT;

	ObjectCreationLock(); 
	m_StringToSessionMapList.ThreadSafeLock();
	for(items=m_StringToSessionMapList.begin();items!=m_StringToSessionMapList.end();items++)
	{
		Ret=items->second-> IsConnectDivertRequired(m_szAppName,
													dwIPV4AddressToConnect,
													usPortToConnect,
													IPv6TeredoAddress,
													usDivertedPort); 
		if(Ret!=DEFAULT)
		{
			//found session which is interested in this connection
			sSessionID=items->first; 
			break;
		}

	}
	m_StringToSessionMapList.ThreadSafeUnLock();
	ObjectCreationUnLock(); 	


	return Ret;
}
/***************************************************
handling winsock UDP sento validation
***************************************************/
CONNECT_TYPE CLSPSessionContainer::IsSentoDivertRequired(DWORD dwIPV4SentToAddress,USHORT usSentToPort,in_addr6 &IPv6TeredoAddress,std::string &sSessionID)
{
	multimap<std::string,CLOISession*>::iterator	items;
	CONNECT_TYPE									Ret=DEFAULT;

	ObjectCreationLock(); 
	m_StringToSessionMapList.ThreadSafeLock();
	for(items=m_StringToSessionMapList.begin();items!=m_StringToSessionMapList.end();items++)
	{
		Ret=items->second->IsSentoDivertRequired(m_szAppName,
													dwIPV4SentToAddress,
													usSentToPort,
													IPv6TeredoAddress
													); 
		if(Ret!=DEFAULT)
		{
			//found session which is interested in this connection
			sSessionID=items->first; 
			break;
		}

	}
	m_StringToSessionMapList.ThreadSafeUnLock();
	ObjectCreationUnLock(); 	

	return Ret;

}
/***********************
checks if any session interested in this bind call
************************/
BOOL CLSPSessionContainer::IsBindToIPv6Required(USHORT usPort,in_addr6 &BindIpv6Addr,std::string &sSessionID)
{
	multimap<std::string,CLOISession*>::iterator	items;
	BOOL											bRet=FALSE;

	ObjectCreationLock(); 
	m_StringToSessionMapList.ThreadSafeLock();
	for(items=m_StringToSessionMapList.begin();items!=m_StringToSessionMapList.end();items++)
	{
		bRet=items->second->IsBindToIPv6Required(usPort,m_szAppName); 
		if(bRet)
		{
			//found session which is interested in this connection
			sSessionID=items->first; 
			BindIpv6Addr=items->second->GetBindTeredoAddress(); 
			break;
		}

	}
	m_StringToSessionMapList.ThreadSafeUnLock();
	ObjectCreationUnLock(); 	

	return bRet;
}
/**************************************************
gets all the connected peers in a session
**************************************************/
PEER_INFORMATION* CLSPSessionContainer::GetConnectedClientList(const std::string &sSessionID,USHORT &usTotalClients)
{
	CLOISession			*pSession=NULL;
	PEER_INFORMATION	*pPeerList=NULL;

	usTotalClients=0;
	ObjectCreationLock(); 
	
	if(m_StringToSessionMapList.IsKeyExists(sSessionID))
	{
		pSession=m_StringToSessionMapList[sSessionID];
		pPeerList=pSession->GetConnectedClientList(usTotalClients); 
	}
	ObjectCreationUnLock(); 	
	return pPeerList;
}
/***********************************************
frees the memory allocated
***********************************************/
void CLSPSessionContainer::FreeConnectedClientList(PEER_INFORMATION* pConnectList)
{
	if(pConnectList)
		delete pConnectList;
}
/***********************************
gets the ipv4 address of a teredo address
************************************/
DWORD CLSPSessionContainer::GetIpv4Address(const std::string &sSessionID,in_addr6 IPv6TeredoAddr)
{
	CLOISession			*pSession=NULL;
	DWORD				 dwIpv4Addr=0;

	ObjectCreationLock(); 
	
	if(m_StringToSessionMapList.IsKeyExists(sSessionID))
	{
		pSession=m_StringToSessionMapList[sSessionID];
		dwIpv4Addr=pSession->GetIpv4Address(IPv6TeredoAddr); 
	}

	ObjectCreationUnLock(); 	
	return dwIpv4Addr;
}