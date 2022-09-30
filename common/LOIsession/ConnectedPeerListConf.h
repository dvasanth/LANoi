#pragma once
#include "../../LOIService/LOIService/p2p/ProtocolRequestHandler.h"
#include "IPCFileMap.h"
#include <list>
#include <string>
#include <iostream>
#include <strsafe.h>
#if _MSC_VER > 1020   // if VC++ version is > 4.2
   using namespace std;  // std c++ libs implemented in std
#endif

  typedef struct _P2P_LIST_HEADER {
	DWORD dwTotalCount;
  } P2P_LIST_HEADER, *LPP2P_LIST_HEADER;

class CConnectedPeerListConf:public ISerialize 
{
public:
	CConnectedPeerListConf(void);
	~CConnectedPeerListConf(void);
	void				AddPeer(PEER_INFORMATION Info);
	void				RemovePeer(in_addr6 IPv6Addr);
	virtual void		Serialize(CIPCFileMap& FileMap);
	BOOL				IsTeredoMappedIpv4Address(DWORD dwIPv4Address,in_addr6 &IPv6TeredoAddress,USHORT &usTeredoProxyPort);
	BOOL				IsPeername(LPCTSTR lpszPeername,DWORD &dwIPv4Address);
	DWORD				GetIpv4Address(in_addr6 Ipv6Address);
	in_addr6			GetSelfTeredoAddress();
	PEER_INFORMATION*	GetConnectedClientList(USHORT &usTotalClients);
	static void			FreeConnectedClientList(PEER_INFORMATION* pConnectList);
	void				AddSelfPeerAddress(in_addr6 IPv6Addr);
private:
		list<PEER_INFORMATION, allocator<PEER_INFORMATION> > m_PeerList;
		CRITICAL_SECTION   m_Lock;
		void				Lock();
		void				UnLock();
		in_addr6			m_SelfPeerIPv6Address;

		BOOL				UpdatePeer(PEER_INFORMATION UpdateInfo);
	
};
