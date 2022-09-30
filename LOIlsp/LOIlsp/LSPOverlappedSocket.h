#pragma once

////////////////////////////////////////////////////////////////////////////////
//
// Structures for each overlapped enabled API containing the arguments to that
//    function.
//
////////////////////////////////////////////////////////////////////////////////

// Argument list for the AcceptEx API
typedef struct _ACCEPTEXARGS
{
    SOCKET       sAcceptSocket,
                 sProviderAcceptSocket;
    PVOID        lpOutputBuffer;
    DWORD        dwReceiveDataLength,
                 dwLocalAddressLength,
                 dwRemoteAddressLength;
    DWORD        dwBytesReceived;
	PVOID		 lpActualOutputBuffer;
	DWORD		 dwActualReceiveDataLength;
	DWORD		 dwActualLocalAddresslength;
	DWORD		 dwActualRemoteAddressLength;
	BOOL		 bParamChanged;
} ACCEPTEXARGS;

// Argument list for the TransmitFile API
typedef struct _TRANSMITFILEARGS
{
    HANDLE        hFile;
    DWORD         nNumberOfBytesToWrite,
                  nNumberOfBytesPerSend;
    LPTRANSMIT_FILE_BUFFERS lpTransmitBuffers;
    DWORD         dwFlags;
} TRANSMITFILEARGS;

// Argument list for the ConnectEx API
typedef struct _CONNECTEXARGS
{
    SOCKET           s;
    SOCKADDR_STORAGE name;
    int              namelen;
    PVOID            lpSendBuffer;
    DWORD            dwSendDataLength;
    DWORD            dwBytesSent;
} CONNECTEXARGS;

// Argument list for TransmitPackets API
typedef struct _TRANSMITPACKETSARGS
{
    SOCKET          s;
    LPTRANSMIT_PACKETS_ELEMENT lpPacketArray;
    DWORD           nElementCount;
    DWORD           nSendSize;
    DWORD           dwFlags;
} TRANSMITPACKETSARGS;

// Argument list for DisconnectEx API
typedef struct _DISCONNECTEXARGS
{
    SOCKET          s;
    DWORD           dwFlags;
    DWORD           dwReserved;
} DISCONNECTEXARGS;

// Argument list for WSARecvMsg API
typedef struct _WSARECVMSGARGS
{
    SOCKET          s;
    LPWSAMSG        lpMsg;
    DWORD           dwNumberOfBytesRecvd;
} WSARECVMSGARGS;

//argument list for WSASendMsg
typedef struct _WSASENDMSGARGS
{
    SOCKET          s;
    LPWSAMSG        lpMsg;
	DWORD			dwFlags;
    DWORD           dwNumberOfBytesSent;
} WSASENDMSGARGS;

// Argument list for the WSARecv API
typedef struct _RECVARGS
{
    LPWSABUF       lpBuffers;
    DWORD          dwBufferCount;
    DWORD          dwNumberOfBytesRecvd,
                   dwFlags;
} RECVARGS;

// Argument list for the WSARecvFrom API
typedef struct _RECVFROMARGS
{
    LPWSABUF       lpBuffers;
    DWORD          dwBufferCount;
    DWORD          dwNumberOfBytesRecvd,
                   dwFlags;
    LPSOCKADDR     lpFrom;
    LPINT          lpFromLen;
    LPSOCKADDR     lpActualFrom;
    LPINT          lpActualFromLen;
	BOOL		   blpFromParamChanged;
} RECVFROMARGS;

// Argument list for the WSASend API
typedef struct _SENDARGS
{
    LPWSABUF       lpBuffers;
    DWORD          dwBufferCount;
    DWORD          dwNumberOfBytesSent;
    DWORD          dwFlags;
} SENDARGS;

// Argument list for the WSASendTo API
typedef struct _SENDTOARGS
{
    LPWSABUF         lpBuffers;
    DWORD            dwBufferCount;
    DWORD            dwNumberOfBytesSent;
    DWORD            dwFlags;
    SOCKADDR_STORAGE To;
    int              iToLen;
} SENDTOARGS;

// Argument list for the WSAIoctl API
typedef struct _IOCTLARGS
{
    DWORD          dwIoControlCode;
    LPVOID         lpvInBuffer;
    DWORD          cbInBuffer;
    LPVOID         lpvOutBuffer;
    DWORD          cbOutBuffer;
    DWORD          cbBytesReturned;
} IOCTLARGS;

// Enumerated type of all asynchronous Winsock operations
typedef enum
{
    LSP_OP_IOCTL         = 1,
    LSP_OP_RECV,
    LSP_OP_RECVFROM,
    LSP_OP_SEND,
    LSP_OP_SENDTO,
    LSP_OP_TRANSMITFILE,
    LSP_OP_ACCEPTEX,
    LSP_OP_CONNECTEX,
    LSP_OP_DISCONNECTEX,
    LSP_OP_TRANSMITPACKETS,
    LSP_OP_WSARECVMSG,
	LSP_OP_WSASENDMSG
} LspOperation;

//
// Our OVERLAPPED structure that includes state and argument
//  information. This structure is used for all overlapped IO
//  operations. Whenever the user app requests overlapped IO
//  we intercept this and make our own call with our own 
//  structure. This way we will receive notification first
//  at which time we can perform post processesing. When we
//  are done we can then post the completion to the user app.
//
typedef struct _WSAOVERLAPPEDPLUS
{
    WSAOVERLAPPED   ProviderOverlapped;     // Overlap to pass to lower layer
    PROVIDER       *Provider;               // Reference to provider who owns this socket
    CLSPSocket     *SockInfo;               // Socket initiating this operation
    SOCKET          CallerSocket;           // Upper layer's socket handle
    SOCKET          ProviderSocket;         // Lower layer's socket handle
    HANDLE          Iocp;                   
    int             Error;                  // Error that operation completed with

    BOOL            CloseThread;            // Indicates whether we need to close thread we opened

    union 
    {
        // Various arguments to the call being made
        ACCEPTEXARGS        AcceptExArgs;
        TRANSMITFILEARGS    TransmitFileArgs;
        CONNECTEXARGS       ConnectExArgs;
        TRANSMITPACKETSARGS TransmitPacketsArgs;
        DISCONNECTEXARGS    DisconnectExArgs;
        WSARECVMSGARGS      WSARecvMsgArgs;
		WSASENDMSGARGS      WSASendMsgArgs;
        RECVARGS            RecvArgs;
        RECVFROMARGS        RecvFromArgs;
        SENDARGS            SendArgs;
        SENDTOARGS          SendToArgs;
        IOCTLARGS           IoctlArgs;
    };

    LspOperation            Operation;              // Type of operation posted
    WSATHREADID             CallerThreadId;         // Which thread initiated operation
    LPWSAOVERLAPPED         lpCallerOverlapped;     // Upper layer's overlapped structure
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCallerCompletionRoutine;   // APC to queue upon completion

    LIST_ENTRY              Link;           // Linked list entry

} WSAOVERLAPPEDPLUS, * LPWSAOVERLAPPEDPLUS;

class CLSPOverlappedSocket :
	public CLSPSocket
{
public:
	CLSPOverlappedSocket(void);
	virtual ~CLSPOverlappedSocket(void);
	CLSPOverlappedSocket(PROVIDER *Provider );
	CLSPOverlappedSocket(PROVIDER *Provider ,SOCKET ProviderSocket);
	
	virtual BOOL IsKindOf(LPCTSTR lpszClassName)
	{
		return _tcsicmp(lpszClassName,_T("CLSPOverlappedSocket"))==0;
	}
	WSAOVERLAPPEDPLUS *	PrepareOverlappedOperation(LspOperation operation,WSABUF *lpBuffers,DWORD dwBufferCount,LPWSAOVERLAPPED lpOverlapped,LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,LPWSATHREADID  lpThreadId,int *lpErrno);
	int					ExecuteOverlappedOperation(WSAOVERLAPPEDPLUS *ol,BOOL bSynchronous,LPINT lpErrno);
	void CALLBACK		IntermediateCompletionRoutine(DWORD dwError,DWORD cbTransferred,LPWSAOVERLAPPEDPLUS lpOverlapped,DWORD dwFlags);
	////////////////////////////////////////////////////////////////////////////////
	//
	// Function prototypes
	//
	////////////////////////////////////////////////////////////////////////////////


	// Executes an overlapped operation
	int ExecuteOverlappedOperation(WSAOVERLAPPEDPLUS  *lpOverlapped,LPINT lpErrno);


	// Invokes the APC associated with an overlapped operation to signal completion
	static VOID CALLBACK CallUserApcProc(ULONG_PTR           Context);
	// Checks whether a given socket has been closed in which case all associated resrouces
	// can be freed
	void CheckForContextCleanup(WSAOVERLAPPEDPLUS  *ol);
protected:

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
										IN SOCKET sCallerAcceptSocket,
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
	void			OnOverlappedIoComplete(DWORD dwError,WSAOVERLAPPEDPLUS *olp);
	virtual BOOL	ChangeRecvFromParams(LPSOCKADDR &lpFrom,LPINT &lpFromLen);
	virtual void	OnRecvFromComplete(DWORD dwError,BOOL blpFromParamChanged,LPSOCKADDR lpActualFrom,LPINT lpActualFromLen,LPSOCKADDR lpFrom,LPINT lpFromLen);
	virtual  BOOL   ChangeAcceptExParam(  CLSPSocket *AcceptSocketContext,	
											  SOCKET &AcceptProviderSocket,
										PVOID &lpOutputBuffer,
										DWORD &dwReceiveDataLength,
										DWORD &dwLocalAddressLength,
										DWORD &dwRemoteAddressLength);
	virtual void	OnChangeAcceptExParamComplete(
											 SOCKET AcceptProviderSocket,
											  DWORD dwError,
											  DWORD dwBytesTransferred,
											  PVOID lpActualOutputBuffer,
											  PVOID lpAppReturnedBuffer,
											  DWORD dwActualRecvLen,
											  DWORD dwActualLocalAddrLen,
											  DWORD dwActualRemoteAddrLen);
private:
	BOOL				m_bSynchronous;
	void				Init();
	void				InitSockOverlap();
	void				CopyOffset(WSAOVERLAPPED  *ProviderOverlapped,WSAOVERLAPPED  *UserOverlapped);

};
