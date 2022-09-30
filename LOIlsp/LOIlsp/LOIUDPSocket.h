#pragma once
#include "lspoverlappedsocket.h"
#include "LOIsession/LSP/LSPSessionContainer.h"
class CLOIUDPSocket :
	public CLSPOverlappedSocket
{
public:
	CLOIUDPSocket(PROVIDER *Provider );
	CLOIUDPSocket (PROVIDER *Provider ,SOCKET ProviderSocket);
	virtual ~CLOIUDPSocket(void);
	virtual BOOL IsKindOf(LPCTSTR lpszClassName)
	{
		return _tcsicmp(lpszClassName,_T("CLOIUDPSocket"))==0;
	}
	virtual void Inherit(CLSPSocket *pAssignee);
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
	virtual   int WSPAPI WSPSendTo(
									LPWSABUF        lpBuffers,
									DWORD           dwBufferCount,
									LPDWORD         lpNumberOfBytesSent,
									DWORD           dwFlags,
									const struct sockaddr FAR * lpTo,
									int             iToLen,
									LPWSAOVERLAPPED lpOverlapped,
									LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
									LPWSATHREADID   lpThreadId,
									LPINT           lpErrno
									);
	virtual  int WSPAPI WSPSend(
								LPWSABUF        lpBuffers,
								DWORD           dwBufferCount,
								LPDWORD         lpNumberOfBytesSent,
								DWORD           dwFlags,
								LPWSAOVERLAPPED lpOverlapped,                             
								LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,   
								LPWSATHREADID   lpThreadId,                                 
								LPINT           lpErrno                                             
								);
	virtual int WSPAPI WSPGetSockName(
										struct sockaddr FAR * name,
										LPINT           namelen,
										LPINT           lpErrno
										);
	virtual BOOL ChangeRecvFromParams(LPSOCKADDR &lpFrom,LPINT &lpFromLen);
	virtual void OnRecvFromComplete(DWORD dwError,BOOL blpFromParamChanged,LPSOCKADDR lpActualFrom,LPINT lpActualFromLen,LPSOCKADDR lpFrom,LPINT lpFromLen);

private:
	CLSPSessionContainer		*m_pSessionContainer;
	WSPBindInfo					m_WSPBindInfo;
	WSPConnectInfo				m_WSPConnectInfo;
	BOOL						m_bSockBoundToIpv6;
	std::string					m_sSessionID;
	in_addr6					m_BoundIPv6Addr;
	void						Init();
	void						BroadCastUDPToAll(USHORT usPort,LPWSABUF  lpBuffers,DWORD dwBufferCount);
};
