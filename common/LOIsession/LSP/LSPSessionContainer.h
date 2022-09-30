#pragma once
#include "../LOISessioncontainer.h"
#include "../../thread.h"


class CLSPSessionContainer:public ISessionContainer,
						   public IThread
{

public:
	static CLSPSessionContainer* GetInstance();
	static void				DestroyInstance();
	void					AddLocalBoundInfo(const std::string sSessionID,DWORD dwBoundAddress,USHORT usBoundPort,in_addr6 IPv6BoundAddress);
	CONNECT_TYPE			IsConnectDivertRequired(DWORD dwIPV4AddressToConnect,USHORT usPortToConnect,in_addr6 &IPv6TeredoAddress,USHORT &usDivertedPort,std::string &sSessionID);
	CONNECT_TYPE			IsSentoDivertRequired(DWORD dwIPV4SentToAddress,USHORT usSentToPort,in_addr6 &IPv6TeredoAddress,std::string &sSessionID);
	BOOL					IsBindToIPv6Required(USHORT usPort,in_addr6 &BindIpv6Addr,std::string &sSessionID);
	PEER_INFORMATION*		GetConnectedClientList(const std::string &sSessionID,USHORT &usTotalClients);
	void					FreeConnectedClientList(PEER_INFORMATION* pConnectList);
	DWORD					GetIpv4Address(const std::string &sSessionID,in_addr6 IPv6TeredoAddr);

private:
	static	CLSPSessionContainer*		s_pInstance	;
	TCHAR					m_szAppName[MAX_PATH];
	CLSPSessionContainer(void);
	~CLSPSessionContainer(void);
	DWORD					OnThreadRun();
	VOID					RefreshTheValuesFromService();
};
