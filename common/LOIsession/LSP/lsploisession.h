#pragma once
#include "LOISessionContainer.h"
enum CONNECT_TYPE{
		DEFAULT,
		DIRECT,
		PROXY,
		BROADCAST
	};
class CLSPLOISession:public ISessionContainer
{
private:
	CLSPLOISession(void);
	~CLSPLOISession(void);
	void			CloneSessionViaIPC();

public:
	static CLSPLOISession* GetInstance();
	static void			DestroyInstance();
	CONNECT_TYPE	IsConnectDivertRequired(LPCTSTR lpszAppName,DWORD dwIPV4AddressToConnect,USHORT usPortToConnect,in_addr6 &IPv6TeredoAddress,USHORT &usDivertedPort);
	CONNECT_TYPE    IsSentoDivertRequired(LPCTSTR lpszAppName,DWORD dwIPV4SentToAddress,USHORT usSentToPort,in_addr6 &IPv6TeredoAddress);
	BOOL			IsBindToIPv6Required(USHORT usPort,LPCTSTR lpszAppname);
	BOOL			IsValidClient(in_addr6 IPv6TeredoAddr)	;
	in_addr6		GetBindTeredoAddress();

	void			AddLocalBoundInfo(DWORD dwBoundAddress,USHORT usBoundPort,in_addr6 IPv6BoundAddress);

private:
	static	CLSPLOISession*				s_pInstance;			
	static VOID CALLBACK				OnUIToLSPUpdate(PVOID lpParameter,BOOLEAN TimerOrWaitFired);

};
