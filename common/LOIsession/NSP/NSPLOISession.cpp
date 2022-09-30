#include "StdAfx.h"
#include "NSPLOISession.h"

CNSPLOISession::CNSPLOISession(void)
{
	//serialise the values from the maps
	CIPCFileMap				SessionListIPC(m_IpcInfo[SESSION_ITEM].szIPCName,CIPCFileMap::load);

	if(SessionListIPC.Open(0))
	{
		Serialize(SessionListIPC);
	}
}

CNSPLOISession::~CNSPLOISession(void)
{
}
/****************************************
gets the name & converts it into ipv4 address
****************************************/
BOOL CNSPLOISession::GetPeerIPv4Address(LPCTSTR lpszPeername,DWORD & dwIpV4Address)
{
	BOOL												bRet=FALSE;
	multimap<std::string,CLOISession*>::iterator		items;

	//traverse thro the session to find the name 
	ObjectCreationLock(); 
	m_StringToSessionMapList.ThreadSafeLock();
	for(items=m_StringToSessionMapList.begin();items!=m_StringToSessionMapList.end();items++)
	{
		bRet=items->second->IsPeername(lpszPeername,dwIpV4Address); 
		if(bRet)
		{
			break;
		}

	}
	m_StringToSessionMapList.ThreadSafeUnLock();
	ObjectCreationUnLock(); 	

	return bRet;
}