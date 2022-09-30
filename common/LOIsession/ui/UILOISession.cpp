#include "StdAfx.h"
#include "UILOISession.h"


CUILOISession::CUILOISession(void)
{
	m_hClientChangeEvent=NULL;
	
}

CUILOISession::~CUILOISession(void)
{
	StopChangeNotification();
}

/**********************************************
Serialise the values from the service & store all information in a list
*********************************************/
VOID CUILOISession::RefreshClientInfo()
{
	CIPCFileMap				SessionListIPC(m_IpcInfo[SESSION_ITEM].szIPCName,CIPCFileMap::load);

	if(SessionListIPC.Open(0))
	{
		Serialize(SessionListIPC);
	}
	multimap<std::string,CLOISession*>::iterator		items;
	CSingleLock											ListLock(&m_ListLock);

	//traverse thro the session to find the name 
	ObjectCreationLock(); 
	m_StringToSessionMapList.ThreadSafeLock();
	ListLock.Lock();
	m_ClientList.RemoveAll();

	for(items=m_StringToSessionMapList.begin();items!=m_StringToSessionMapList.end();items++)
	{
		PEER_INFORMATION		*pConnectClient=NULL;
		USHORT					usClientCount=0;
		pConnectClient=items->second->GetConnectedClientList(usClientCount); 
		if(pConnectClient)
		{
			for(int iIndex=0;iIndex<usClientCount;iIndex++)
			{
				CLIENT_INFORMATION  ClientInfo={0};
				USES_CONVERSION;

				ClientInfo.PeerInfo= pConnectClient[iIndex];
				items->second->GetSessionName(ClientInfo.szSessionName,sizeof(ClientInfo.szSessionName));
				StringCbPrintf(ClientInfo.szSessionID,
							sizeof(ClientInfo.szSessionID),
							_T("%s"),   
							A2T(items->first.c_str())
							);
				m_ClientList.AddHead(ClientInfo);
				
			}
			items->second->FreeConnectedClientList(pConnectClient);
		}
		
	}
	ListLock.Unlock();
	m_StringToSessionMapList.ThreadSafeUnLock();
	ObjectCreationUnLock(); 	

}
/*******************************************
returns all the connected client information 
*******************************************/
CLIENT_INFORMATION* CUILOISession::GetConnectedClientsInfo(USHORT &usClientCount)
{
	CLIENT_INFORMATION*		pClientInfo=NULL;
	CSingleLock				ListLock(&m_ListLock);

	ListLock.Lock();
	usClientCount=m_ClientList.GetCount();

	if(usClientCount)
	{
		pClientInfo = new CLIENT_INFORMATION[usClientCount];
		POSITION pos = m_ClientList.GetHeadPosition();
		for (int iIndex=0;iIndex < m_ClientList.GetCount();iIndex++)
		{
			pClientInfo[iIndex]= m_ClientList.GetNext(pos);
		}
	}

	ListLock.Unlock();
	return pClientInfo;
}
/*****************************************
free list routine
*****************************************/
void CUILOISession::FreeConnectedClientInfo(CLIENT_INFORMATION*		pClientInfo)
{
	if(pClientInfo)
		delete pClientInfo;
}

/************************************
Register for client change notification
************************************/
void CUILOISession::RegisterChangeNotification(UINT uiMsg,HWND hNotifyWnd)
{
	GetUIToLSPEvent(m_hClientChangeEvent);

	m_ClientEventNotify.RegisterEvent(m_hClientChangeEvent,hNotifyWnd,uiMsg);

}

/****************************
stop the  notification
****************************/
void CUILOISession::StopChangeNotification()
{
	if(m_hClientChangeEvent)
	{
		CloseHandle(m_hClientChangeEvent);
		m_hClientChangeEvent=NULL;
	}
	m_ClientEventNotify.UnRegisterEvent();
}