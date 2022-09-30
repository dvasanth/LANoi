#pragma once

#include "ProtocolRequestHandler.h"
#include "../sockio/SocketIO.h"

typedef enum peer_group_event_tag
{
	ENCRYPT_DATA,
	DECRYPT_DATA,
	NEW_PEER_JOINED,
	PEER_LOGGED_OFF,
	PORT_STRING,
	SERVER_STOPPED,
	PEER_CLOSED,
	CLIENT_CONNECTING,
	CLIENT_CONNECTED_SERVER,
	CLIENT_RECEIVED_SESSION_INFO,
	CLIENT_CLOSED,
	IS_AUTHENTICATED_PEER,
	DYNAMIC_UPDATED_PORT_STRING,
}P2P_GROUP_EVENT_TYPE;


typedef DWORD (CALLBACK * P2PEVENTCALLBACK) (PEER_GROUP_EVENT_DATA,PVOID );  

class IGroup{
protected:
	P2PEVENTCALLBACK	m_EventCallback;
	PVOID				m_pParam;

	virtual void		SocketEventHandler()=0;

	DWORD NotifyEvent(P2P_GROUP_EVENT_TYPE   EventType,PVOID pEventData,DWORD	 dwDataLen)
	{
		PEER_GROUP_EVENT_DATA   PeerEvent={EventType,pEventData,dwDataLen};
		return m_EventCallback(PeerEvent,m_pParam);
	}

	HANDLE LaunchHandler(LPVOID lpParam)
	{
		return CreateThread(0,0,EventProc,this,0,0);
	}
	static DWORD WINAPI EventProc(LPVOID lpParam)
	{
		((IGroup*)lpParam)->SocketEventHandler();
		return 0;
	}
};


