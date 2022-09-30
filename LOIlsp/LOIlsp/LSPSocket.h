#pragma once
typedef struct _WSPEventSelectInfo
{
	WSAEVENT			 hCallerEventObject;
	WSANETWORKEVENTS     ProviderEvents;
	long				 lEventMask;
}WSPEventSelectInfo,*LPWSPEventSelectInfo;

typedef struct _WSPAsyncSelectInfo
{
	HWND					 hWnd;
	UINT					 uMsg;
	long					 lEventmask;
}WSPAsyncSelectInfo,*LPWSPAsyncSelectInfo;

typedef struct _WSPBindInfo
{
	sockaddr_in				 name;
	int						 iNameLen;
}WSPBindInfo,*LPWSPBindInfo;


typedef struct _WSPConnectInfo
{
	sockaddr_in				 name;
	int						 iNameLen;
}WSPConnectInfo,*LPWSPConnectInfo;


class CLSPSocket
{
public:
	CLSPSocket(PROVIDER *Provider );
	CLSPSocket(PROVIDER *Provider ,SOCKET ProviderSocket);
	virtual void Inherit(CLSPSocket *pAssignee);
	virtual ~CLSPSocket(void);
    DWORD_PTR		*Link;
	DWORD			AddRef();
	DWORD			Release();
	DWORD			AddAsyncRef();
	DWORD			ReleaseAsync();
	DWORD			ReleaseAtomicAsync();
	PROVIDER *		GetNextProvider();
	SOCKET			GetProviderSocket();
	void			SetCallerSocket(SOCKET CallerSocket);
	SOCKET			GetCallerSocket();
	BOOL			IsLastSocketOperation();
	void			Close();
	int			    CreateIPv6SockAndAssociate(int iSockType,int iProtocol,LPINT lpErrno);

	virtual BOOL IsKindOf(LPCTSTR lpszClassName)
	{
		return _tcsicmp(lpszClassName,_T("CLSPSocket"))==0;
	}
	virtual SOCKET WSPAPI WSPSocket(
								int                 af,
								int                 type,
								int                 protocol,
								__in LPWSAPROTOCOL_INFOW lpProtocolInfo,
								GROUP               g,
								DWORD               dwFlags,
								LPINT               lpErrno
								);

	virtual int WSPAPI WSPShutdown (
							int    how,
							LPINT  lpErrno
							);

	virtual int WSPAPI WSPSetSockOpt(
							int        level,
							int        optname,
							const char FAR * optval,   
							int        optlen,
							LPINT      lpErrno
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
	virtual int WSPAPI 	WSPSendDisconnect(
					LPWSABUF lpOutboundDisconnectData,
					LPINT    lpErrno
					);

	virtual int WSPAPI WSPSend(
						LPWSABUF        lpBuffers,
						DWORD           dwBufferCount,
						LPDWORD         lpNumberOfBytesSent,
						DWORD           dwFlags,
						LPWSAOVERLAPPED lpOverlapped,                             
						LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,   
						LPWSATHREADID   lpThreadId,                                 
						LPINT           lpErrno                                             
						);
	virtual int WSPAPI WSPSelect(
								int          nfds,
								fd_set FAR * readfds,
								fd_set FAR * writefds,
								fd_set FAR * exceptfds,
								const struct timeval FAR * timeout,
								LPINT        lpErrno
								);
	virtual int WSPAPI 	WSPRecvFrom(
					LPWSABUF        lpBuffers,
					DWORD           dwBufferCount,
					LPDWORD         lpNumberOfBytesRecvd,
					LPDWORD         lpFlags,
					struct sockaddr FAR * lpFrom,
					LPINT           lpFromLen,
					LPWSAOVERLAPPED lpOverlapped,
					LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
					LPWSATHREADID   lpThreadId,
					LPINT           lpErrno
					);

	virtual int WSPAPI WSPRecvDisconnect(
										LPWSABUF lpInboundDisconnectData,
										LPINT    lpErrno
										);

	virtual int WSPAPI WSPRecv(
							LPWSABUF        lpBuffers,
							DWORD           dwBufferCount,
							LPDWORD         lpNumberOfBytesRecvd,
							LPDWORD         lpFlags,
							LPWSAOVERLAPPED lpOverlapped,
							LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
							LPWSATHREADID   lpThreadId,
							LPINT           lpErrno
							);
	virtual int WSPAPI WSPListen(
								int    backlog,     
								LPINT  lpErrno
								);
	virtual	SOCKET WSPAPI WSPJoinLeaf(
						const struct sockaddr FAR * name,
						int          namelen,
						LPWSABUF     lpCallerData,
						LPWSABUF     lpCalleeData,
						LPQOS        lpSQOS,
						LPQOS        lpGQOS,
						DWORD        dwFlags,
						LPINT        lpErrno
						);
	virtual int WSPAPI WSPIoctl(
								DWORD           dwIoControlCode,
								LPVOID          lpvInBuffer,
								DWORD           cbInBuffer,
								LPVOID          lpvOutBuffer,
								DWORD           cbOutBuffer,
								LPDWORD         lpcbBytesReturned,
								LPWSAOVERLAPPED lpOverlapped,
								LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
								LPWSATHREADID   lpThreadId,
								LPINT           lpErrno
								);
	virtual BOOL WSPAPI WSPGetQOSByName(
										LPWSABUF lpQOSName,
										LPQOS    lpQOS,
										LPINT    lpErrno
										);
	virtual int WSPAPI WSPGetSockOpt(
									int        level,
									int        optname,
									char FAR * optval,
									LPINT      optlen,
									LPINT      lpErrno
									);

	virtual int WSPAPI WSPGetSockName(
										struct sockaddr FAR * name,
										LPINT           namelen,
										LPINT           lpErrno
										);

	virtual int WSPAPI WSPGetPeerName(  
									struct sockaddr FAR * name,
									LPINT           namelen,
									LPINT           lpErrno
									);
	virtual	int WSPAPI WSPEventSelect(
									WSAEVENT hEventObject,
									long     lNetworkEvents,
									LPINT    lpErrno
									);
	virtual int WSPAPI WSPEnumNetworkEvents(  
											WSAEVENT           hEventObject,
											LPWSANETWORKEVENTS lpNetworkEvents,
											LPINT              lpErrno
											);
	virtual int WSPAPI WSPDuplicateSocket(
										DWORD               dwProcessId,                      
										LPWSAPROTOCOL_INFOW lpProtocolInfo,   
										LPINT               lpErrno
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
	virtual int WSPAPI WSPCloseSocket(  
								LPINT  lpErrno,
								BOOL &bObjectDeleted
								);
	virtual	int WSPAPI WSPBind(
								const struct sockaddr FAR * name,
								int                   namelen,
								LPINT                 lpErrno
								);
	virtual int WSPAPI WSPAsyncSelect(
									HWND         hWnd,
									unsigned int wMsg,
									long         lEvent,
									LPINT        lpErrno
									);
	virtual SOCKET WSPAPI WSPAccept(
								struct sockaddr FAR * addr,  
								LPINT           addrlen,                 
								LPCONDITIONPROC lpfnCondition,  
								DWORD_PTR       dwCallbackData,          
								LPINT           lpErrno
								);
	virtual BOOL PASCAL FAR ExtTransmitFile(
											IN HANDLE hFile,
											IN DWORD nNumberOfBytesToWrite,
											IN DWORD nNumberOfBytesPerSend,
											IN LPOVERLAPPED lpOverlapped,
											IN LPTRANSMIT_FILE_BUFFERS lpTransmitBuffers,
											IN DWORD dwFlags,
											INT   &Errno
										    );
	virtual BOOL PASCAL FAR ExtAcceptEx(
										CLSPSocket *AcceptSocketContext,
										IN SOCKET sNextProviderAcceptSocket,
										IN PVOID lpOutputBuffer,
										IN DWORD dwReceiveDataLength,
										IN DWORD dwLocalAddressLength,
										IN DWORD dwRemoteAddressLength,
										OUT LPDWORD lpdwBytesReceived,
										IN LPOVERLAPPED lpOverlapped,
										INT   &Errno
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
	virtual BOOL PASCAL FAR ExtTransmitPackets(
										LPTRANSMIT_PACKETS_ELEMENT lpPacketArray,
										DWORD nElementCount,
										DWORD nSendSize,
										LPOVERLAPPED lpOverlapped,
										DWORD dwFlags,
										INT   &Errno);
	virtual BOOL PASCAL FAR ExtDisconnectEx(
										IN LPOVERLAPPED lpOverlapped,
										IN DWORD  dwFlags,
										IN DWORD  dwReserved,
										INT   &Errno
										);
	virtual INT PASCAL FAR ExtWSARecvMsg(
									IN OUT LPWSAMSG lpMsg,
									OUT LPDWORD lpdwNumberOfBytesRecvd,
									IN LPWSAOVERLAPPED lpOverlapped,
									IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
									INT   &Errno
									);
	virtual INT PASCAL FAR ExtWSASendMsg(
									  IN   LPWSAMSG lpMsg,
									  IN   DWORD dwFlags,
									  OUT  LPDWORD lpNumberOfBytesSent,
									  IN   LPWSAOVERLAPPED lpOverlapped,
									  IN   LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
									  INT   &Errno
									  );
	void PostAsyncWindowMsg(LPARAM lParam);
	void CheckFirewallOperations();
	void AddFirewallRule();
protected:

	volatile LONG			 m_dwRefCount;
	DWORD					 m_dwOutstandingAsync;
	BOOL					 m_bClosing;
	CRITICAL_SECTION         m_SockCritSec;
	PROVIDER				*m_NextProvider;
	SOCKET					 m_NextProviderSocket;
	SOCKET					 m_CallerSocket;
	HANDLE					 m_hIocp;           // associated with an IOCP?
    int						 m_LastError;           // Last error that occured on this socket
	BOOL					 m_bNonBlockingSocket;
	WSPAsyncSelectInfo		 m_WSPAsyncSelectInfo;
	WSPEventSelectInfo       m_WSPEventSelectInfo;
	DWORD					 m_dwUpdateAcceptSockContext;
	DWORD					 m_dwUpdateConnectSockContext;
	TCHAR					 m_szAppName[MAX_PATH];

    ULONGLONG				 BytesSent;       // Byte counts
    ULONGLONG				 BytesRecv;
	void					 AcquireSocketLock();
	void					 ReleaseSocketLock();
	void					 Init();

	///ipv6 related

	SOCKET					 CreateNextProviderSocket(int iAddressFamily,int iProtocol,int iSockType,PROVIDER **NewProvider,LPINT lpErrno);
	void					 CopySocketOpt(SOCKET OldSock,SOCKET NewSock,PROVIDER *OldProvider,PROVIDER *NewProvider,BOOL bEnableNonBlockingSocket);
	void					 EnableIPv6UnsoliciatedTraffic();
	void					 DisableSocketnotifications();
	void					 ReEnableSocketnotifications();

};
