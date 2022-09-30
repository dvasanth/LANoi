#include "StdAfx.h"
#include "ConnectedPeerListConf.h"


CConnectedPeerListConf::CConnectedPeerListConf(void)
{
	InitializeCriticalSection(&m_Lock); 
	ZeroMemory(&m_SelfPeerIPv6Address,sizeof(m_SelfPeerIPv6Address));
}

CConnectedPeerListConf::~CConnectedPeerListConf(void)
{
	DeleteCriticalSection(&m_Lock);
}

void CConnectedPeerListConf::Lock()
{
	EnterCriticalSection(&m_Lock);
}

void CConnectedPeerListConf::UnLock()
{
	LeaveCriticalSection(&m_Lock);
}

void CConnectedPeerListConf::AddSelfPeerAddress(in_addr6 IPv6Addr)
{
	m_SelfPeerIPv6Address=IPv6Addr;
}

void CConnectedPeerListConf::AddPeer(PEER_INFORMATION Info)
{
	if(UpdatePeer(Info))
	{
		//ignore since already added
		return;
	}
	//otherwise add the element
	Lock();
	m_PeerList.insert(m_PeerList.end(),Info);
	UnLock();
}
BOOL CConnectedPeerListConf::UpdatePeer(PEER_INFORMATION UpdateInfo)
{
	//if already exits then just update it
	list<PEER_INFORMATION, allocator<PEER_INFORMATION> >::iterator i;
	Lock();
	for (i =  m_PeerList.begin(); i != m_PeerList.end(); ++i)
	{
		//if(memcmp(&i->IPv6TeredoAddress,&UpdateInfo.IPv6TeredoAddress,sizeof(in_addr6))==0)
		if(_tcsicmp(i->szPeerID,UpdateInfo.szPeerID)==0)   //unique value for each peer
		{
			*i= UpdateInfo;
			UnLock();
			return TRUE;
		}
	}
	UnLock();
	return FALSE;
}

void CConnectedPeerListConf::RemovePeer(in_addr6 IPv6Addr)
{
	list<PEER_INFORMATION, allocator<PEER_INFORMATION> >::iterator i;
	Lock();
	for (i =  m_PeerList.begin(); i != m_PeerList.end(); ++i)
	{
		if(memcmp(&i->IPv6TeredoAddress,&IPv6Addr,sizeof(in_addr6))==0)
		{
			m_PeerList.erase( i);
			break;
		}
	}
	UnLock();
}
void CConnectedPeerListConf::Serialize(CIPCFileMap& FileMap)
{
	FileMap.SingleUseLock();
	if(FileMap.IsStoring())
	{
		Lock();
		list<PEER_INFORMATION, allocator<PEER_INFORMATION> >::iterator i;
		P2P_LIST_HEADER  ListHeader={0};

		//write the total items count in header
		ListHeader.dwTotalCount = m_PeerList.size(); 
		FileMap.Write((LPBYTE)&ListHeader,sizeof(ListHeader)); 

		FileMap.Write((LPBYTE)&m_SelfPeerIPv6Address,sizeof(m_SelfPeerIPv6Address));
		for (i =  m_PeerList.begin(); i != m_PeerList.end(); ++i)
		{
			PEER_INFORMATION  SaveInfo ={i->dwIPv4Address,i->IPv6TeredoAddress,i->usReverseProxyPort ,_T(""),i->dwFlag   };
						
			StringCbCopy(SaveInfo.szPeerName,sizeof(SaveInfo.szPeerName),i->szPeerName);   
			StringCbCopy(SaveInfo.szPeerID ,sizeof(SaveInfo.szPeerID ),i->szPeerID );   
			FileMap.Write((LPBYTE)&SaveInfo,sizeof(SaveInfo));
		}
		UnLock();
	}
	else
	{
		PEER_INFORMATION  LoadInfo ={0};
		P2P_LIST_HEADER   ListHeader={0};
		Lock();
		m_PeerList.clear();
		//read the items count from header
		FileMap.Read((LPBYTE)&ListHeader,sizeof(ListHeader));  

		FileMap.Read((LPBYTE)&m_SelfPeerIPv6Address,sizeof(m_SelfPeerIPv6Address));
		for(int iItemCount=0;iItemCount<(int)ListHeader.dwTotalCount;iItemCount++) 
		{
			if(FileMap.Read((LPBYTE)&LoadInfo,sizeof(LoadInfo)) && //add only valid entries
				LoadInfo.dwIPv4Address
				)
			{
				m_PeerList.insert(m_PeerList.end(),LoadInfo);
			}
		}

		UnLock();
	}
	FileMap.SingleUseUnLock();
}

BOOL CConnectedPeerListConf::IsTeredoMappedIpv4Address(DWORD dwIPv4Address,in_addr6 &IPv6TeredoAddress,USHORT &usTeredoProxyPort)
{
	list<PEER_INFORMATION, allocator<PEER_INFORMATION> >::iterator i;
	Lock();
	for (i =  m_PeerList.begin(); i != m_PeerList.end(); ++i)
	{
		if(i->dwIPv4Address==dwIPv4Address)
		{
			IPv6TeredoAddress=i->IPv6TeredoAddress;
			usTeredoProxyPort=i->usReverseProxyPort ;
			UnLock();
			return TRUE;
		}
	}
	UnLock();
	return FALSE;
}

BOOL CConnectedPeerListConf::IsPeername(LPCTSTR lpszPeername,DWORD &dwIPv4Address)
{
	list<PEER_INFORMATION, allocator<PEER_INFORMATION> >::iterator i;

	Lock();
	for (i =  m_PeerList.begin(); i != m_PeerList.end(); ++i)
	{
		if(_tcsicmp(i->szPeerName,lpszPeername)==0)
		{
			dwIPv4Address=i->dwIPv4Address;
			UnLock();
			return TRUE;
		}
	}
	UnLock();
	return FALSE;
}
DWORD CConnectedPeerListConf::GetIpv4Address(in_addr6 Ipv6Address)
{
	list<PEER_INFORMATION, allocator<PEER_INFORMATION> >::iterator i;

	Lock();
	for (i =  m_PeerList.begin(); i != m_PeerList.end(); i++)
	{
		if(memcmp(&Ipv6Address,&i->IPv6TeredoAddress,sizeof(in_addr6))==0)
		{
			DWORD dwIPv4Address;
			dwIPv4Address=i->dwIPv4Address;
			UnLock();
			return dwIPv4Address;
		}
	}
	UnLock();
	return 0;
}
in_addr6 CConnectedPeerListConf::GetSelfTeredoAddress()
{

	return m_SelfPeerIPv6Address;
}

PEER_INFORMATION* CConnectedPeerListConf::GetConnectedClientList(USHORT &usTotalClients)
{
	list<PEER_INFORMATION, allocator<PEER_INFORMATION> >::iterator i;
	PEER_INFORMATION								*pConnectList=NULL;

	if(m_PeerList.empty())
	{
		usTotalClients=0;
		return pConnectList;
	}

	Lock();
	pConnectList= new PEER_INFORMATION[(int)m_PeerList.size()];
	usTotalClients=0;
	for (i =  m_PeerList.begin(); i != m_PeerList.end(); ++i)
	{
		pConnectList[usTotalClients]=*i;
		usTotalClients++;
	}
	UnLock();
	return pConnectList;
}
void	CConnectedPeerListConf::FreeConnectedClientList(PEER_INFORMATION* pConnectList)
{
	if(pConnectList)
		delete pConnectList;
}