#pragma once
#include "../LOISessioncontainer.h"
#include "eventnotify.h"

 typedef struct _CLIENT_INFORMATION {
		PEER_INFORMATION  PeerInfo;
		TCHAR			  szSessionName[MAX_PATH];
		TCHAR			  szSessionID[MAX_PATH];
	}CLIENT_INFORMATION, *LPCLIENT_INFORMATION;

class CUILOISession :
	public ISessionContainer
{
public:
	CUILOISession(void);
	virtual ~CUILOISession(void);
	VOID						RefreshClientInfo();
	CLIENT_INFORMATION*			GetConnectedClientsInfo(USHORT &usClientCount);
	void						FreeConnectedClientInfo(CLIENT_INFORMATION*		pClientInfo);
	void						RegisterChangeNotification(UINT uiMsg,HWND hNotifyWnd);
	void						StopChangeNotification();
private:
	CList <CLIENT_INFORMATION,CLIENT_INFORMATION>   m_ClientList;
	CCriticalSection								m_ListLock;
	HANDLE											m_hClientChangeEvent;

	CEventNotify									m_ClientEventNotify;

};
