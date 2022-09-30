#pragma once
#include "ipcfilemap.h"

typedef struct _DYNAMIC_SESSION_UPDATE_INFO{
	TCHAR    szSessionID[MAX_PATH];
	DWORD    dwBindedLocalAddress;
	USHORT   usBoundPort;
	DWORD    dwVirtualPeerAddress;
	
  } DYNAMIC_SESSION_UPDATE_INFO, *LPDYNAMIC_SESSION_UPDATE_INFO;

class CDynamicSessionInfoUpdate 
{
public:
	CDynamicSessionInfoUpdate(void);
	~CDynamicSessionInfoUpdate(void);
	void							Initialise(CIPCFileMap& FileMap);
	void							UpdateDynamicSessionInfo(CIPCFileMap& FileMap,LPCTSTR lpszSessionID,DWORD dwNetOrderIPAddress,USHORT usNetOrderPort,DWORD dwVirtualAddress);
	DYNAMIC_SESSION_UPDATE_INFO*	ClearDynamicUpdatedItems(CIPCFileMap& FileMap,DWORD &dwItemCount);
	void							FreeDynamicUpdatedItems(DYNAMIC_SESSION_UPDATE_INFO* pItems);

};
