#pragma once

#include "lspoverlappedsocket.h"
#include "LOIsession/LSP/LSPSessionContainer.h"

///tdi related
typedef struct _TA_ADDRESS { 
  USHORT  AddressLength; 
  USHORT  AddressType; 
  UCHAR   Address[1]; 
} TA_ADDRESS, *PTA_ADDRESS; 
typedef struct _TRANSPORT_ADDRESS { 
  LONG  TAAddressCount; 
  TA_ADDRESS  Address[1]; 
} TRANSPORT_ADDRESS, *PTRANSPORT_ADDRESS; 
typedef struct _TDI_ADDRESS_INFO { 
  ULONG  ActivityCount; 
  TRANSPORT_ADDRESS  Address; 
} TDI_ADDRESS_INFO, *PTDI_ADDRESS_INFO; 

class CLOITCPsocket :public CLSPOverlappedSocket
{
public:
	CLOITCPsocket(PROVIDER *Provider );
	CLOITCPsocket (PROVIDER *Provider ,SOCKET ProviderSocket);
	virtual ~CLOITCPsocket(void);
	virtual void Inherit(CLSPSocket *pAssignee);
	virtual BOOL IsKindOf(LPCTSTR lpszClassName)
	{
		return _tcsicmp(lpszClassName,_T("CLOITCPsocket"))==0;
	}
protected:
	virtual	int WSPAPI WSPBind(
								const struct sockaddr FAR * name,
								int                   namelen,
								LPINT                 lpErrno
								);
	virtual int WSPAPI WSPConnect(
						const struct sockaddr FAR * name,
						int                   namelen,
						LPWSABUF              lpCallerData,
						LPWSABUF              lpCalleeData,
						LPQOS                 lpSQOS,
						LPQOS                 lpGQOS,
						LPINT                 lpErrno
						);
	virtual SOCKET WSPAPI WSPAccept(
								struct sockaddr FAR * addr,  
								LPINT           addrlen,                 
								LPCONDITIONPROC lpfnCondition,  
								DWORD_PTR       dwCallbackData,          
								LPINT           lpErrno
								);

	virtual int WSPAPI WSPGetPeerName(  
										struct sockaddr FAR * name,
										LPINT           namelen,
										LPINT           lpErrno
										);
	virtual int WSPAPI WSPGetSockName(
										struct sockaddr FAR * name,
										LPINT           namelen,
										LPINT           lpErrno
										);
	virtual BOOL PASCAL FAR ExtConnectEx(
										IN const struct sockaddr FAR *name,
										IN int namelen,
										IN PVOID lpSendBuffer OPTIONAL,
										IN DWORD dwSendDataLength,
										OUT LPDWORD lpdwBytesSent,
										IN LPOVERLAPPED lpOverlapped,
										INT   &Errno
										);

	
private:
	static const DWORD			ACCEPTEX_SOCK_ADDR_SIZE;
	CLSPSessionContainer		*m_pSessionContainer;
	WSPBindInfo					m_WSPBindInfo;
	WSPConnectInfo				m_WSPConnectInfo;
	BOOL						m_bSockBoundToIpv6;
	BOOL						m_bSockDiverted;
	std::string					m_sSessionID;
	in_addr6					m_BoundIPv6Addr;
	void						Init();
	void						SendProxyConnect(USHORT usPort);
	void						WaitForConnectComplete();
	void						GetAcceptedPeerName(in_addr6 IPv6PeerAddr,USHORT usPeerPort,struct sockaddr FAR * name,LPINT namelen);
	void						GetAcceptedSockName(in_addr6 IPv6PeerAddr,struct sockaddr FAR * name,LPINT namelen);
	virtual  BOOL				ChangeAcceptExParam(
											  CLSPSocket *AcceptSocketContext,	
											  SOCKET &AcceptProviderSocket,
											  PVOID &lpOutputBuffer,
											   DWORD &dwReceiveDataLength,
											   DWORD &dwLocalAddressLength,
											   DWORD &dwRemoteAddressLength);
	virtual void				OnChangeAcceptExParamComplete(
											  SOCKET  AcceptProviderSocket,
											  DWORD dwError,
											  DWORD dwBytesTransferred,
											  PVOID lpActualOutputBuffer,
											  PVOID lpAppReturnedBuffer,
											  DWORD dwActualRecvLen,
											  DWORD dwActualLocalAddrLen,
											  DWORD dwActualRemoteAddrLen
											  );
			BOOL				GetAcceptExSockaddrsIPv6(
										PVOID lpOutputBuffer,
										DWORD dwReceiveDataLength,
										LPSOCKADDR *LocalSockaddr,
										LPINT LocalSockaddrLength,
										LPSOCKADDR *RemoteSockaddr,
										LPINT RemoteSockaddrLength
										);

};
