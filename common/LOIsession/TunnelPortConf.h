#pragma once
#include "IPCFileMap.h"
#include <list>
#include <string>
#include <iostream>

#if _MSC_VER > 1020   // if VC++ version is > 4.2
   using namespace std;  // std c++ libs implemented in std
#endif

typedef struct _PORT_INFORMATION {
	USHORT		usPort;
	TCHAR		szPortsByAppName[MAX_PATH];
	DWORD		dwLocalBindedAddress;
	DWORD	    dwVirtualAddressOfBindedAddress;
	DWORD		dwFlag;
  } PORT_INFORMATION, *LPPORT_INFORMATION;

typedef struct _PORT_HEADER {
	DWORD dwTotalCount;
  } PORT_HEADER, *LPPORT_HEADER;

#define  BINDED_ADDRESS		   0x1
#define  PROXY_TUNNEL_CONNECT  0x2
#define  IDENTIFY_BY_APPNAME   0x4	  

class CTunnelPortConf:public ISerialize 
{
public:
	CTunnelPortConf(void);
	~CTunnelPortConf(void);
	void			AddPort(USHORT usHostOrderPortNumber,BOOL bProxyTunnelPortConnect,LPCTSTR lpszPortsByApp=NULL);
	virtual void	Serialize(CIPCFileMap& FileMap);
	BOOL			IsPortNeedsTunnelExpose(USHORT usPort,LPCTSTR lpszAppName);
	BOOL		    IsPortRequiresProxyTunnel(USHORT usPort);
	void			AddBindedPort(DWORD dwBindedAddress,USHORT usBindedPort,DWORD dwPeerVirtualAddress);
	BOOL			IsBindedPeerAddress(DWORD dwAddress,USHORT usPort,DWORD &dwVirtualAddress);
	void			Initialise();
private:
	list<PORT_INFORMATION, allocator<PORT_INFORMATION> > m_PortInfoList;
	CRITICAL_SECTION   m_Lock;
	void			Lock();
	void			UnLock();
};
