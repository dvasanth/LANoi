#pragma once
#include "ConnectedPeerListConf.h"
#include "TunnelPortConf.h"
#include "DynamicSessionInfoUpdate.h"
#ifdef CSTRING_DEFINED
	#include "loisession\LOISessionStringFormat.h"
#endif
class CLOISession;
/*****************
Notifies abt session change nottification
*****************/
class ISessionChangeNotify{
public:
	virtual void OnSessionValueChange(){}
};
/***********************
Notifies abt Dynamic session change notification
***********************/
class IDynamicSessionUpdateNotify{
public:
	virtual void OnDynamicUpdateChange(CLOISession *pLOISession,LPCTSTR lpszDynamicPortString){}
};
enum CONNECT_TYPE{
		DEFAULT,
		DIRECT,
		PROXY,
		BROADCAST
	};
class CLOISession
{
public:
	enum SESSION_STATE{
		ACTIVE,
		INACTIVE,
		USER_CLOSE
	};
	CLOISession(ISessionChangeNotify *pSessionChange,IDynamicSessionUpdateNotify *pDynamicUpdate);

	virtual ~CLOISession(void);
	DWORD AddRef();
	DWORD Release();

	//session information update
//	void			AddSessionInfo(LPCTSTR lpszPortList);
#ifdef CSTRING_DEFINED
	void			ParseSessionXML(LPCTSTR lpszSessionXML);
	CString			GetSessionString();
#endif
	void			UpdateSessionPeerInfo(BOOL bAddPeer,LPPEER_INFORMATION pPeerInfo,in_addr6	*pIPv6ClosedTeredoAddress);


	
	
	void			SetState(SESSION_STATE State);
	CLOISession::SESSION_STATE	GetState();

	void			AddSelfTeredoAddress(in_addr6 IPv6Addr);

	PEER_INFORMATION*	GetConnectedClientList(USHORT &usTotalClients);
	static void			FreeConnectedClientList(PEER_INFORMATION* pConnectList);
	void				GetInviterName(LPTSTR lpszServerName,DWORD dwSizeInBytes);
	DWORD				GetIpv4Address(in_addr6 IPv6TeredoAddr);
	BOOL				IsPortRequireProxy(USHORT usPort);
	//dynamic update hlprs
	BOOL			GetDynamicBoundStr(LPTSTR lpszStr,DWORD dwSizeInBytes);

	//serializing routine
	void			Serialize(CIPCFileMap& FileMap);
	void			AddDynamicPortString(LPCTSTR szPortString);
	//doesn't require instance
	static BOOL								IsValidPortEntries(LPCTSTR lpszPortList);
	/*********************************************
	sock related validation related calls
	*******************************************/
	CONNECT_TYPE				IsConnectDivertRequired(LPCTSTR lpszAppName,DWORD dwIPV4AddressToConnect,USHORT usPortToConnect,in_addr6 &IPv6TeredoAddress,USHORT &usDivertedPort);
	CONNECT_TYPE				IsSentoDivertRequired(LPCTSTR lpszAppName,DWORD dwIPV4SentToAddress,USHORT usSentToPort,in_addr6 &IPv6TeredoAddress);
	BOOL						IsBindToIPv6Required(USHORT usPort,LPCTSTR lpszAppName);
	in_addr6					GetBindTeredoAddress();
	DWORD						IsPeername(LPCTSTR lpszPeername,DWORD & dwIpV4Address);
	/**************
	Get & sets the session name
	***************/
	void						SetSessionName(LPCTSTR lpszSessionName);
	void						GetSessionName(LPTSTR lpszSessioName,DWORD dwSizeInBytes);

	DWORD						GetLastActivity();
private:
#ifdef CSTRING_DEFINED
	CString								m_sSessionXML;
#endif
	CConnectedPeerListConf				m_PeerConf;
	CTunnelPortConf						m_PortConf;
	LPTSTR								m_pszSessionString;
	TCHAR								m_szSessionName[MAX_PATH];
	SESSION_STATE						m_State;
	volatile LONG						m_lReferenceCount;
	ISessionChangeNotify				*m_PSessionChangeNotify;
	IDynamicSessionUpdateNotify			*m_pDynamicSessionNotify;
	DWORD								m_dwLastSessionActivityTime;
	//string hlprs
	BOOL					ParseDynamicBoundAddress(LPCTSTR lpszAddressStr,DWORD &dwBoundAddress,USHORT &usBoundPort,DWORD &dwVirtualAddress);
};
