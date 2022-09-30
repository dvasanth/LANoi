#include "StdAfx.h"
#include "TunnelPortConf.h"
#include <strsafe.h>

CTunnelPortConf::CTunnelPortConf(void)
{
	InitializeCriticalSection(&m_Lock); 
}

CTunnelPortConf::~CTunnelPortConf(void)
{
	DeleteCriticalSection(&m_Lock);
}

void CTunnelPortConf::Lock()
{
	EnterCriticalSection(&m_Lock);
}

void CTunnelPortConf::UnLock()
{
	LeaveCriticalSection(&m_Lock);
}
void CTunnelPortConf::Initialise()
{
	Lock();
	m_PortInfoList.clear() ;
	UnLock();
}

void CTunnelPortConf::AddPort(USHORT usHostOrderPortNumber,BOOL bProxyTunnelPortConnect,LPCTSTR lpszPortsByApp)
{
	PORT_INFORMATION  Info ={0};

	Info.usPort = usHostOrderPortNumber;
	if(lpszPortsByApp)
	{
		StringCbCopy(Info.szPortsByAppName,sizeof(Info.szPortsByAppName),lpszPortsByApp);
		_tcslwr_s(Info.szPortsByAppName,sizeof(Info.szPortsByAppName)/sizeof(Info.szPortsByAppName[0]));
		Info.dwFlag|=IDENTIFY_BY_APPNAME;
	}
	if(bProxyTunnelPortConnect)
			Info.dwFlag|=PROXY_TUNNEL_CONNECT;
	Lock();
	m_PortInfoList.insert(m_PortInfoList.end(),Info);
	UnLock();
}

void CTunnelPortConf::AddBindedPort(DWORD dwBindedAddress,USHORT usBindedPort,DWORD dwPeerVirtualAddress)
{
	PORT_INFORMATION  Info ={0};

	Info.dwVirtualAddressOfBindedAddress  = dwPeerVirtualAddress;
	Info.dwLocalBindedAddress = dwBindedAddress;
	Info.usPort = usBindedPort;
	Info.dwFlag |=BINDED_ADDRESS;
	Lock();
	m_PortInfoList.insert(m_PortInfoList.end(),Info);
	UnLock();
}
void CTunnelPortConf::Serialize(CIPCFileMap& FileMap)
{
	FileMap.SingleUseLock();
	if(FileMap.IsStoring())
	{
		PORT_HEADER			Header={0};
		list<PORT_INFORMATION, allocator<PORT_INFORMATION> >::iterator i;
		
		
		Lock();
		Header.dwTotalCount = m_PortInfoList.size(); 
		//write the total items count in header
		FileMap.Write((LPBYTE)&Header,sizeof(Header)); 
		for (i =  m_PortInfoList.begin(); i != m_PortInfoList.end(); ++i)
		{
			FileMap.Write((LPBYTE)&i->usPort ,sizeof(PORT_INFORMATION));
		}
		UnLock();
	}
	else
	{
		PORT_INFORMATION	LoadInfo ={0};
		PORT_HEADER			Header={0};

		m_PortInfoList.clear();
		Lock();

		//read the items count from header
		FileMap.Read((LPBYTE)&Header,sizeof(Header));  

		for(int iItemCount=0;iItemCount<(int)Header.dwTotalCount;iItemCount++) 
		{
			if(FileMap.Read((LPBYTE)&LoadInfo,sizeof(LoadInfo)))//add only if read success
			{
				m_PortInfoList.insert(m_PortInfoList.end(),LoadInfo);
			}
		}
		UnLock();
	}
	FileMap.SingleUseUnLock();
}

BOOL CTunnelPortConf::IsPortNeedsTunnelExpose(USHORT usPort,LPCTSTR lpszAppName)
{
	list<PORT_INFORMATION, allocator<PORT_INFORMATION> >::iterator i;

	Lock();
	for (i =  m_PortInfoList.begin(); i != m_PortInfoList.end(); ++i)
	{
		if(i->dwFlag&IDENTIFY_BY_APPNAME && lpszAppName)
		{
			TCHAR    szLowerApp[MAX_PATH]={0};
			StringCbCopy(szLowerApp,sizeof(szLowerApp),lpszAppName);
			_tcslwr_s(szLowerApp,sizeof(szLowerApp)/sizeof(szLowerApp[0]));
			if(_tcsstr(szLowerApp,i->szPortsByAppName) )
			{
				UnLock();
				return TRUE;
			}
		}
		 //only if port is non-zero then compare with port values
		if(usPort && i->usPort==usPort)
		{
			UnLock();
			return TRUE;
		}
	}
	UnLock();
	return FALSE;
}

BOOL CTunnelPortConf::IsBindedPeerAddress(DWORD dwAddress,USHORT usPort,DWORD &dwVirtualAddress)
{
	list<PORT_INFORMATION, allocator<PORT_INFORMATION> >::iterator i;

	Lock();
	for (i =  m_PortInfoList.begin(); i != m_PortInfoList.end(); ++i)
	{
		if(i->dwFlag&BINDED_ADDRESS &&
			i->dwLocalBindedAddress ==dwAddress &&
			i->usPort ==usPort
			)
		{
			dwVirtualAddress=i->dwVirtualAddressOfBindedAddress; 
			UnLock();
			return TRUE;
		}
	
	}
	UnLock();
	return FALSE;
}
BOOL CTunnelPortConf::IsPortRequiresProxyTunnel(USHORT usPort)
{
	list<PORT_INFORMATION, allocator<PORT_INFORMATION> >::iterator i;
	Lock();
	for (i =  m_PortInfoList.begin(); i != m_PortInfoList.end(); ++i)
	{
		if(i->usPort==usPort && 
			i->dwFlag&PROXY_TUNNEL_CONNECT
			)
		{
			UnLock();
			return TRUE;
		}
	}
	UnLock();
	return FALSE;
}