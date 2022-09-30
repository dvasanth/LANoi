#pragma once
#include "../LOISessioncontainer.h"
class CNSPLOISession:public ISessionContainer
{
public:
	CNSPLOISession(void);
	virtual ~CNSPLOISession(void);
	BOOL	GetPeerIPv4Address(LPCTSTR lpszPeername,DWORD & dwIpV4Address);
};
