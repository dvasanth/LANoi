#include "StdAfx.h"
#include "LSPLOISession.h"

CLSPLOISession*	CLSPLOISession::s_pInstance=NULL;

CLSPLOISession* CLSPLOISession::GetInstance()
{
	if(s_pInstance==NULL)
	{
		s_pInstance=new CLSPLOISession();

	}
	return s_pInstance;
}

void CLSPLOISession::DestroyInstance()
{
	if(s_pInstance)
	{
		delete s_pInstance;
		s_pInstance=NULL;
	}
}

CLSPLOISession::CLSPLOISession(void)
{
	CloneSessionViaIPC();
}

CLSPLOISession::~CLSPLOISession(void)
{
	UnRegisterEventNotifiers();
}
void CLSPLOISession::CloneSessionViaIPC()
{
	RefreshValuesFromUI();
	//update the peer list in real time
	RegisterUIToLSPNotifier(OnUIToLSPUpdate,this);
	//just OPEn the  lsp to UI event to set & reset
	RegisterLSPToUINotifier(NULL,NULL); 
}

void CALLBACK CLSPLOISession::OnUIToLSPUpdate(PVOID lpParameter,BOOLEAN TimerOrWaitFired)
{
	CLSPLOISession *pThis =(CLSPLOISession *)lpParameter;
	pThis->RefreshValuesFromUI();
}




void CLSPLOISession::AddLocalBoundInfo(DWORD dwBoundAddress,USHORT usBoundPort,in_addr6 IPv6BoundAddress)
{
	CIPCFileMap		DynamicUpdateMap(m_IpcInfo[DYNAMIC_UPDATE_ITEM].szIPCName ,CIPCFileMap::store);

	dbgprint(_T(" CLSPLOISession::AddLocalBoundInfo %X:%X"), dwBoundAddress,usBoundPort);
	DynamicUpdateMap.Open(m_IpcInfo[DYNAMIC_UPDATE_ITEM].dwMaxSize );
	m_DynamicUpdate.UpdateDynamicSessionInfo(DynamicUpdateMap,dwBoundAddress,usBoundPort,GetIpv4Address(IPv6BoundAddress));
	NotifyUIInstance();

}

CONNECT_TYPE CLSPLOISession::IsConnectDivertRequired(LPCTSTR lpszAppName,DWORD dwIPV4AddressToConnect,USHORT usPortToConnect,in_addr6 &IPv6TeredoAddress,USHORT &usDivertedPort)
{
	USHORT          usProxyServerPort;
	DWORD			dwVirtualPeerAddressToConnect=0;

	//check if this a teredo mapped address
	if( m_PortConf.IsPortNeedsTunnelExpose(usPortToConnect,lpszAppName) && //perform divertion only for app & port we are insterested
		m_PeerConf.IsTeredoMappedIpv4Address(dwIPV4AddressToConnect,
											IPv6TeredoAddress,
											usProxyServerPort))
	{
		//is this port require proxy connection
		if(m_PortConf.IsPortRequiresProxyTunnel(usPortToConnect))
		{
			usDivertedPort=usProxyServerPort;
			return PROXY;
		}
		usDivertedPort=usPortToConnect;
		return DIRECT;
	}
	//check if it is a dynamic binded port information
	if(m_PortConf.IsBindedPeerAddress(dwIPV4AddressToConnect,usPortToConnect, dwVirtualPeerAddressToConnect ))
	{
		//chcek whether the client still exists
		if(m_PeerConf.IsTeredoMappedIpv4Address(dwVirtualPeerAddressToConnect, IPv6TeredoAddress ,usProxyServerPort))
		{
			return DIRECT;
		}
	}

	//no change just pass as it is
	return DEFAULT;
}
CONNECT_TYPE CLSPLOISession::IsSentoDivertRequired(LPCTSTR lpszAppName,DWORD dwIPV4SentToAddress,USHORT usSentToPort,in_addr6 &IPv6TeredoAddress)
{
	USHORT          usProxyServerPort;
	DWORD			dwVirtualPeerAddressToConnect=0;
	//otherwise check if it is teredo mapped adddres
	//if it is our ipv4 virtual address
	if(m_PortConf.IsPortNeedsTunnelExpose(usSentToPort,lpszAppName) && //perform divertion only for app & port we are insterested
		m_PeerConf.IsTeredoMappedIpv4Address(dwIPV4SentToAddress,
											IPv6TeredoAddress,
											usProxyServerPort))
											return DIRECT;
	//check if it is broadcast address
	if(dwIPV4SentToAddress == 0xffffffff)
	{
		return BROADCAST;
	}
	//check if it is a dynamic binded port information
	if(m_PortConf.IsBindedPeerAddress(dwIPV4SentToAddress,usSentToPort, dwVirtualPeerAddressToConnect ))
	{
		//chcek whether the client still exists
		if(m_PeerConf.IsTeredoMappedIpv4Address(dwVirtualPeerAddressToConnect, IPv6TeredoAddress ,usProxyServerPort))
		{
			return DIRECT;
		}
	}
	return DEFAULT;
}

BOOL CLSPLOISession::IsBindToIPv6Required(USHORT usPort,LPCTSTR lpszAppName)
{
	return m_PortConf.IsPortNeedsTunnelExpose(usPort,lpszAppName);
}

BOOL CLSPLOISession::IsValidClient(in_addr6 IPv6TeredoAddr)
{
	//has ipv4 address only if authenticated
	return m_PeerConf.GetIpv4Address(IPv6TeredoAddr);
}

in_addr6 CLSPLOISession::GetBindTeredoAddress()
{
	return m_PeerConf.GetSelfTeredoAddress();
}
