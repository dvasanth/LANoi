#include "StdAfx.h"
#include "lspdef.h"
#include "LSPSocket.h"
#include <strsafe.h>

CLSPSocket::CLSPSocket(PROVIDER *Provider )
{
	Init();
	m_NextProvider=Provider;
}
CLSPSocket::CLSPSocket(PROVIDER *Provider ,SOCKET ProviderSocket)
{
	Init();
	m_NextProvider=Provider;
	m_NextProviderSocket=ProviderSocket;
}

void CLSPSocket::Init()
{

	m_CallerSocket=m_NextProviderSocket=INVALID_SOCKET;
	m_dwOutstandingAsync=0;

    m_bClosing          = FALSE;

	m_hIocp=NULL;
	m_LastError=0;
	Link=NULL;
	BytesSent=BytesRecv=0;
    InitializeCriticalSection( &m_SockCritSec );
	m_dwRefCount=0;
	m_bNonBlockingSocket=FALSE;

	ZeroMemory(&m_WSPAsyncSelectInfo,sizeof(m_WSPAsyncSelectInfo));
	ZeroMemory(&m_WSPEventSelectInfo,sizeof(m_WSPEventSelectInfo));
	ZeroMemory(m_szAppName,sizeof(m_szAppName));

	TCHAR   szAppPath[MAX_PATH]={0};
	GetModuleFileName (NULL,szAppPath,sizeof(szAppPath)/sizeof(szAppPath[0]));
	if(_tcsrchr(szAppPath,'\\'))
	StringCbCopy(m_szAppName,sizeof(m_szAppName),_tcsrchr(szAppPath,'\\')+1);
	
}
CLSPSocket::~CLSPSocket(void)
{
	DeleteCriticalSection( &m_SockCritSec );
}

void CLSPSocket::Inherit(CLSPSocket *pAssignee)
{
	m_WSPAsyncSelectInfo=pAssignee->m_WSPAsyncSelectInfo ;
}

DWORD CLSPSocket::AddRef()
{
	InterlockedIncrement(&m_dwRefCount);
	return m_dwRefCount;
}

DWORD CLSPSocket::Release()
{
	EnterCriticalSection(&gCriticalSection);
	// Decrement the ref count and see if someone closed this socket (from another thread)
	InterlockedDecrement(&m_dwRefCount);
        
	if ( IsLastSocketOperation())
    {
		INT iError;
		theInstance.GetSockFactoryInstance()->DerefSocketObject( this, &iError );
		LeaveCriticalSection( &gCriticalSection );
		return 0;
	}
	LeaveCriticalSection( &gCriticalSection );
	return m_dwRefCount;
}
DWORD CLSPSocket::AddAsyncRef()
{
	m_dwOutstandingAsync++;
	return m_dwOutstandingAsync;
}

DWORD CLSPSocket::ReleaseAsync()
{
	m_dwOutstandingAsync--;
	return m_dwOutstandingAsync;
}
DWORD CLSPSocket::ReleaseAtomicAsync()
{
	InterlockedIncrement( (volatile LONG *)&m_dwOutstandingAsync );
	return m_dwOutstandingAsync;
}

BOOL CLSPSocket::IsLastSocketOperation()
{
	return  ( 0 == m_dwRefCount ) && 
            ( 0 == m_dwOutstandingAsync ) && 
            ( TRUE == m_bClosing ); 
}
//
// Function: AcquireSocketLock
// 
// Description:
//    This routine acquires the critical section which is a member of the 
//    socket's context structure. This is held when modifying the socket
//    context outside of looking up the context (which is performed by
//    FindAndRefSocketContext).
//
void CLSPSocket::AcquireSocketLock()
{
    EnterCriticalSection( &m_SockCritSec );
}

//
// Function: ReleaseSocketLock
//
// Description:
//    This routine releases the socket context critical section.
//
void CLSPSocket::ReleaseSocketLock()
{
    LeaveCriticalSection( &m_SockCritSec );
}


SOCKET WSPAPI CLSPSocket::WSPSocket(
									int                 af,
									int                 type,
									int                 protocol,
									__in LPWSAPROTOCOL_INFOW lpProtocolInfo,
									GROUP               g,
									DWORD               dwFlags,
									LPINT               lpErrno
									)
{

    //
    // Create the underlying provider's socket.
    //

    dbgprint(_T("Calling the lower provider WSPSocket: '%s'"), m_NextProvider->NextProvider.szProtocol);

    ASSERT( m_NextProvider->NextProcTable.lpWSPSocket );

    SetBlockingProvider(m_NextProvider);
    m_NextProviderSocket = m_NextProvider->NextProcTable.lpWSPSocket(
            af, 
            type, 
            protocol, 
            lpProtocolInfo,
            g, 
            dwFlags | WSA_FLAG_OVERLAPPED, // Always Or in the overlapped flag
            lpErrno
            );
    SetBlockingProvider(NULL);


	return m_NextProviderSocket;
}

SOCKET CLSPSocket::GetProviderSocket()
{
	return m_NextProviderSocket;
}

void CLSPSocket::Close()
{
	int tempErr,ret;

  	if(m_NextProviderSocket!=INVALID_SOCKET)
	{
		ret=m_NextProvider->NextProcTable.lpWSPCloseSocket(
					m_NextProviderSocket, 
				   &tempErr
					);
		m_NextProviderSocket=INVALID_SOCKET;
        if ( SOCKET_ERROR == ret )
        {
           dbgprint(_T("WSPCloseSocket() on handle %d failed: %d"), m_NextProviderSocket, ret);
        }
	}
}

PROVIDER *CLSPSocket::GetNextProvider()
{
	return m_NextProvider;
}

void   CLSPSocket::SetCallerSocket(SOCKET CallerSocket)
{
	m_CallerSocket=CallerSocket;
}

SOCKET   CLSPSocket::GetCallerSocket()
{

	return m_CallerSocket;
}
int WSPAPI CLSPSocket::WSPShutdown (
											int    how,
											LPINT  lpErrno
											)
{
	INT        ret = SOCKET_ERROR;
	SetBlockingProvider(m_NextProvider);
    ret = m_NextProvider->NextProcTable.lpWSPShutdown(
														m_NextProviderSocket, 
														how, 
														lpErrno
														);
    SetBlockingProvider(NULL);

	return ret;
}


int WSPAPI CLSPSocket::WSPSetSockOpt(
											int        level,
											int        optname,
											const char FAR * optval,   
											int        optlen,
											LPINT      lpErrno
											)
{
	 INT        ret = SOCKET_ERROR;

     SetBlockingProvider(m_NextProvider);
     ret = m_NextProvider->NextProcTable.lpWSPSetSockOpt(
													   m_NextProviderSocket, 
														level,                 
														optname, 
														optval, 
														optlen, 
														lpErrno
														);
     SetBlockingProvider(NULL);

	return ret;
}


int WSPAPI CLSPSocket::WSPSendTo(
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
								)
{
    int                 ret = SOCKET_ERROR;
	ASSERT( m_NextProvider->NextProcTable.lpWSPSendTo );

    SetBlockingProvider(m_NextProvider);
    ret = m_NextProvider->NextProcTable.lpWSPSendTo(
													m_NextProviderSocket, 
													lpBuffers, 
													dwBufferCount,
													lpNumberOfBytesSent, 
													dwFlags, 
													lpTo, 
													iToLen, 
													lpOverlapped, 
													lpCompletionRoutine, 
													lpThreadId, 
													lpErrno
													);
    SetBlockingProvider(NULL);
    if ( SOCKET_ERROR != ret )
    {
        
    }
	return ret;
}

int WSPAPI CLSPSocket::WSPSendDisconnect(
										LPWSABUF lpOutboundDisconnectData,
										LPINT    lpErrno
										)
{
   INT        ret = SOCKET_ERROR;
   ASSERT( m_NextProvider->NextProcTable.lpWSPSendDisconnect );

    SetBlockingProvider(m_NextProvider);
    ret = m_NextProvider->NextProcTable.lpWSPSendDisconnect(
            m_NextProviderSocket,
            lpOutboundDisconnectData, 
            lpErrno
            );
    SetBlockingProvider(NULL);

	return ret;
}


 int WSPAPI CLSPSocket::WSPSend(
								LPWSABUF        lpBuffers,
								DWORD           dwBufferCount,
								LPDWORD         lpNumberOfBytesSent,
								DWORD           dwFlags,
								LPWSAOVERLAPPED lpOverlapped,                             
								LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,   
								LPWSATHREADID   lpThreadId,                                 
								LPINT           lpErrno                                             
								)
 {
    INT                 ret = SOCKET_ERROR;

	ASSERT( m_NextProvider->NextProcTable.lpWSPSend );

    // Make a blocking send call
    SetBlockingProvider(m_NextProvider);
    ret = m_NextProvider->NextProcTable.lpWSPSend(
												m_NextProviderSocket, 
												lpBuffers, 
												dwBufferCount,
												lpNumberOfBytesSent, 
												dwFlags, 
												lpOverlapped, 
												lpCompletionRoutine, 
												lpThreadId, 
												lpErrno
												);
    SetBlockingProvider(NULL);
    if ( SOCKET_ERROR != ret )
    {
        BytesSent += *lpNumberOfBytesSent;
    }
	dbgprint(_T(" CLSPSocket::WSPSend bytes sent %d"),*lpNumberOfBytesSent);
	return ret;
 }

 int WSPAPI CLSPSocket::WSPSelect(
								int          nfds,
								fd_set FAR * readfds,
								fd_set FAR * writefds,
								fd_set FAR * exceptfds,
								const struct timeval FAR * timeout,
								LPINT        lpErrno
								)
 {
	INT                 ret = SOCKET_ERROR;
   
	ASSERT( m_NextProvider->NextProcTable.lpWSPSelect );

    SetBlockingProvider(m_NextProvider);
    ret = m_NextProvider->NextProcTable.lpWSPSelect(
													nfds, 
													readfds , 
													writefds , 
													exceptfds , 
													timeout, 
													lpErrno
													);
    SetBlockingProvider(NULL);

	return ret;
 }

 int WSPAPI 	CLSPSocket::WSPRecvFrom(
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
										)
{
    int                 ret = SOCKET_ERROR;

    ASSERT( m_NextProvider->NextProcTable.lpWSPRecvFrom );

    // Make a blocking WSPRecvFrom call
    SetBlockingProvider(m_NextProvider);
    ret = m_NextProvider->NextProcTable.lpWSPRecvFrom(
													m_NextProviderSocket, 
													lpBuffers, 
													dwBufferCount,
													lpNumberOfBytesRecvd, 
													lpFlags, 
													lpFrom, 
													lpFromLen, 
													lpOverlapped, 
													lpCompletionRoutine, 
													lpThreadId, 
													lpErrno);
     SetBlockingProvider(NULL);
     if ( SOCKET_ERROR != ret )
     {
        BytesRecv += *lpNumberOfBytesRecvd;
     }
	 return ret;
}
										

int WSPAPI CLSPSocket::WSPRecvDisconnect(
										LPWSABUF lpInboundDisconnectData,
										LPINT    lpErrno
										)
{
    INT        ret = SOCKET_ERROR;
    ASSERT( m_NextProvider->NextProcTable.lpWSPRecvDisconnect );

    SetBlockingProvider(m_NextProvider);
    ret = m_NextProvider->NextProcTable.lpWSPRecvDisconnect(
															m_NextProviderSocket,                           
															lpInboundDisconnectData, 
															lpErrno
															);
    SetBlockingProvider(NULL);

	return ret;
}

int WSPAPI CLSPSocket::WSPRecv(
							LPWSABUF        lpBuffers,
							DWORD           dwBufferCount,
							LPDWORD         lpNumberOfBytesRecvd,
							LPDWORD         lpFlags,
							LPWSAOVERLAPPED lpOverlapped,
							LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
							LPWSATHREADID   lpThreadId,
							LPINT           lpErrno
							)
{
    int                 ret = SOCKET_ERROR;
    ASSERT( m_NextProvider->NextProcTable.lpWSPRecv );

    SetBlockingProvider(m_NextProvider);
    ret = m_NextProvider->NextProcTable.lpWSPRecv(
													m_NextProviderSocket, 
													lpBuffers, 
													dwBufferCount,
													lpNumberOfBytesRecvd, 
													lpFlags, 
													lpOverlapped, 
													lpCompletionRoutine, 
													lpThreadId,
													lpErrno);
    SetBlockingProvider(NULL);
    if ( SOCKET_ERROR != ret )
    {
        BytesRecv += *lpNumberOfBytesRecvd;
    }
	return ret;
}

int WSPAPI CLSPSocket::WSPListen(
								int    backlog,     
								LPINT  lpErrno
								)
{
    INT        ret = SOCKET_ERROR;
    ASSERT( m_NextProvider->NextProcTable.lpWSPListen );

    SetBlockingProvider(m_NextProvider);
    ret = m_NextProvider->NextProcTable.lpWSPListen(
													m_NextProviderSocket, 
													backlog, 
													lpErrno
													);
    SetBlockingProvider(NULL);

	return ret;
}
SOCKET WSPAPI CLSPSocket::WSPJoinLeaf(
						const struct sockaddr FAR * name,
						int          namelen,
						LPWSABUF     lpCallerData,
						LPWSABUF     lpCalleeData,
						LPQOS        lpSQOS,
						LPQOS        lpGQOS,
						DWORD        dwFlags,
						LPINT        lpErrno
						)
{
   SOCKET     NextProviderSocket = INVALID_SOCKET;


   ASSERT( m_NextProvider->NextProcTable.lpWSPJoinLeaf );

    SetBlockingProvider(m_NextProvider);
    NextProviderSocket = m_NextProvider->NextProcTable.lpWSPJoinLeaf(
																	m_NextProviderSocket,                           
																	name, 
																	namelen, 
																	lpCallerData, 
																	lpCalleeData, 
																	lpSQOS, 
																	lpGQOS, 
																	dwFlags,                        
																	lpErrno
																	);
    SetBlockingProvider(NULL);
	return NextProviderSocket;
}

int WSPAPI CLSPSocket::WSPIoctl(
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
								)
{
	int                 ret = NO_ERROR;

	ASSERT( m_NextProvider->NextProcTable.lpWSPIoctl );

    SetBlockingProvider(m_NextProvider);
    ret = m_NextProvider->NextProcTable.lpWSPIoctl(
												m_NextProviderSocket, 
												dwIoControlCode, 
												lpvInBuffer,
												cbInBuffer, 
												lpvOutBuffer, 
												cbOutBuffer, 
												lpcbBytesReturned, 
												lpOverlapped, 
												lpCompletionRoutine, 
												lpThreadId, 
												lpErrno);
    SetBlockingProvider(NULL);

	if (ret==NO_ERROR && FIONBIO  == (int)dwIoControlCode  )
	{
		//store the nonblocking  information in sockinfo
		m_bNonBlockingSocket = *(BOOL*)lpvInBuffer;

	}
	return ret;
}

BOOL WSPAPI CLSPSocket::WSPGetQOSByName(
										LPWSABUF lpQOSName,
										LPQOS    lpQOS,
										LPINT    lpErrno
										)
{
    INT					ret = SOCKET_ERROR;

	ASSERT( m_NextProvider->NextProcTable.lpWSPGetQOSByName );

    SetBlockingProvider(m_NextProvider);
    ret = m_NextProvider->NextProcTable.lpWSPGetQOSByName(
            m_NextProviderSocket, 
            lpQOSName,
            lpQOS, 
            lpErrno
            );
    SetBlockingProvider(NULL);

	return ret;
}

int WSPAPI CLSPSocket::WSPGetSockOpt(
									int        level,
									int        optname,
									char FAR * optval,
									LPINT      optlen,
									LPINT      lpErrno
									)
{
	INT					ret = NO_ERROR;
	ASSERT( m_NextProvider->NextProcTable.lpWSPGetSockOpt );

    SetBlockingProvider(m_NextProvider);
    ret = m_NextProvider->NextProcTable.lpWSPGetSockOpt(
                m_NextProviderSocket, 
                level,
                optname, 
                optval, 
                optlen, 
                lpErrno);
    SetBlockingProvider(NULL);
	return ret;
}

int WSPAPI CLSPSocket::WSPGetSockName(
										struct sockaddr FAR * name,
										LPINT           namelen,
										LPINT           lpErrno
										)
{
    INT					ret = SOCKET_ERROR;

    ASSERT( m_NextProvider->NextProcTable.lpWSPGetSockName );

    SetBlockingProvider(m_NextProvider);
    ret = m_NextProvider->NextProcTable.lpWSPGetSockName(
														m_NextProviderSocket, 
														name,
														namelen, 
														lpErrno
														);
    SetBlockingProvider(NULL);
	return ret;
}

int WSPAPI CLSPSocket::WSPGetPeerName(  
									struct sockaddr FAR * name,
									LPINT           namelen,
									LPINT           lpErrno
									)
{
	INT					ret = SOCKET_ERROR;
	ASSERT(m_NextProvider->NextProcTable.lpWSPGetPeerName );

    SetBlockingProvider(m_NextProvider);
    ret = m_NextProvider->NextProcTable.lpWSPGetPeerName(
														m_NextProviderSocket, 
														name,
														namelen, 
														lpErrno);
    SetBlockingProvider(NULL);

	return ret;
}

int WSPAPI CLSPSocket::WSPEventSelect(
									WSAEVENT hEventObject,
									long     lNetworkEvents,
									LPINT    lpErrno
									)
{
    INT					ret = SOCKET_ERROR;    
    ASSERT( m_NextProvider->NextProcTable.lpWSPEventSelect );

    SetBlockingProvider(m_NextProvider);
    ret = m_NextProvider->NextProcTable.lpWSPEventSelect(
            m_NextProviderSocket, 
            hEventObject,
            lNetworkEvents, 
            lpErrno
            );
    SetBlockingProvider(NULL);

	if(ret==NO_ERROR)
	{
		m_WSPEventSelectInfo.hCallerEventObject =  hEventObject;
		m_WSPEventSelectInfo.lEventMask = lNetworkEvents;
		m_bNonBlockingSocket=TRUE;
	}
	return ret;
}

int WSPAPI CLSPSocket::WSPEnumNetworkEvents(  
											WSAEVENT           hEventObject,
											LPWSANETWORKEVENTS lpNetworkEvents,
											LPINT              lpErrno
											)
{
	INT					ret = SOCKET_ERROR;
	ASSERT( m_NextProvider->NextProcTable.lpWSPEnumNetworkEvents );

    SetBlockingProvider(m_NextProvider);
    ret = m_NextProvider->NextProcTable.lpWSPEnumNetworkEvents(
															m_NextProviderSocket,                             
															hEventObject, 
															lpNetworkEvents, 
															lpErrno
															);
    SetBlockingProvider(NULL);


	return ret;
}

int WSPAPI CLSPSocket::WSPDuplicateSocket(
										DWORD               dwProcessId,                      
										LPWSAPROTOCOL_INFOW lpProtocolInfo,   
										LPINT               lpErrno
										)
{
    DWORD              dwReserved;
    int                ret = SOCKET_ERROR;
    PROVIDER          *Provider = NULL;

   //
    // Find the underlying provider
    //

    ASSERT( m_NextProvider->NextProcTable.lpWSPDuplicateSocket );

    SetBlockingProvider(m_NextProvider);
    ret = m_NextProvider->NextProcTable.lpWSPDuplicateSocket(
            m_NextProviderSocket,
            dwProcessId,
            lpProtocolInfo,
            lpErrno
            );
    SetBlockingProvider(NULL);

    if ( NO_ERROR == ret )
    {
        //
        // We want to return the WSAPROTOCOL_INFOW structure of the underlying
        // provider but we need to preserve the reserved info returned by the
        // WSPDuplicateSocket call.
        //
        dwReserved = lpProtocolInfo->dwProviderReserved;
        memcpy(lpProtocolInfo, &m_NextProvider->LayerProvider, sizeof(WSAPROTOCOL_INFOW));
        lpProtocolInfo->dwProviderReserved = dwReserved;

        dbgprint(_T("WSPDuplicateSocket: Returning %S provider with reserved %d"),
                lpProtocolInfo->szProtocol, dwReserved );
    }

	return ret;
}

int WSPAPI CLSPSocket::WSPConnect(
						const struct sockaddr FAR * name,
						int                   namelen,
						LPWSABUF              lpCallerData,
						LPWSABUF              lpCalleeData,
						LPQOS                 lpSQOS,
						LPQOS                 lpGQOS,
						LPINT                 lpErrno
						)
{
    INT					ret = SOCKET_ERROR;
    ASSERT( m_NextProvider->NextProcTable.lpWSPConnect );

    SetBlockingProvider(m_NextProvider);
    ret = m_NextProvider->NextProcTable.lpWSPConnect(
													m_NextProviderSocket, 
													name, 
													namelen, 
													lpCallerData, 
													lpCalleeData,
													lpSQOS, 
													lpGQOS, 
													lpErrno
													);
    SetBlockingProvider(NULL);

	return ret;
}

int WSPAPI CLSPSocket::WSPCloseSocket(  
								LPINT  lpErrno,
								BOOL &bObjectDeleted
								)
{
    int					ret = SOCKET_ERROR;

	bObjectDeleted=FALSE;

	AcquireSocketLock(  );

    dbgprint(_T("WSPCloseSocket: Closing layered socket 0x%p (provider 0x%p)"),
        m_CallerSocket, m_NextProviderSocket);

    //
    // If we there are outstanding async calls on this handle don't close the app
    //  socket handle...only close the provider's handle.  Therefore any errors
    //  incurred can be propogated back to the app socket. Also verify closesocket
    //  hasn't already been called on this socket
    //

    dbgprint(_T("dwOutstanding = %d; RefCount = %d"), m_dwOutstandingAsync, 
        m_dwRefCount);

    ASSERT( m_NextProvider->NextProcTable.lpWSPCloseSocket );

    if ( ( ( 0 != m_dwOutstandingAsync ) || 
           ( 1 != m_dwRefCount ) ) &&
         ( TRUE != m_bClosing )
       )
    {
        //
        // Either there are outstanding asynchronous operations or some other thread
        // is performing an operation AND close has not already been called on this
        // socket
        //
        m_bClosing = TRUE;

        ret = m_NextProvider->NextProcTable.lpWSPCloseSocket(
                m_NextProviderSocket, 
                lpErrno
                );
        if ( SOCKET_ERROR == ret )
        {
            goto cleanup;
        }
       
        dbgprint(_T("Closed lower provider socket: %X"), m_NextProviderSocket );

       m_NextProviderSocket = INVALID_SOCKET;

    }
    else if ( ( 0 == m_dwOutstandingAsync ) &&
              ( 1 == m_dwRefCount )
            )
    {
        //
        // No one else is referencing this socket so we can close and free all
        //  objects associated with it
        //
        SetBlockingProvider(m_NextProvider);
        ret = m_NextProvider->NextProcTable.lpWSPCloseSocket(
                m_NextProviderSocket, 
                lpErrno
                );
        
        SetBlockingProvider(NULL);
        
        if ( SOCKET_ERROR == ret )
        {
            dbgprint(_T("WSPCloseSocket: Provider close failed"));
            goto cleanup;
        }
        

        m_NextProviderSocket = INVALID_SOCKET;

        //
        // Remove the socket info
        //
        theInstance.GetSockFactoryInstance()->RemoveSocketObject(m_NextProvider, this);

        //
        // Close the app socket
        //
		ret = gMainUpCallTable.lpWPUCloseSocketHandle(m_CallerSocket, lpErrno);
        if ( SOCKET_ERROR == ret )
        {
            dbgprint(_T("WPUCloseSocketHandle failed: %d"), *lpErrno);
        }

        dbgprint(_T("Closing socket %X Bytes Sent [%I64d] Bytes Recv [%I64d]"), 
                m_CallerSocket, BytesSent, BytesRecv);

        ReleaseSocketLock(  );

        // Don't need to 'DerefSocketContext' as we're deleting the object now

        theInstance.GetSockFactoryInstance()->FreeSockObject( this );

		bObjectDeleted=TRUE;
    }
cleanup:
	if(!bObjectDeleted)
	ReleaseSocketLock(  );
	return ret;
}

int WSPAPI CLSPSocket::WSPBind(
								const struct sockaddr FAR * name,
								int                   namelen,
								LPINT                 lpErrno
								)
{
    INT					ret = SOCKET_ERROR;
    ASSERT( m_NextProvider->NextProcTable.lpWSPBind );

    SetBlockingProvider(m_NextProvider);
    ret = m_NextProvider->NextProcTable.lpWSPBind(
            m_NextProviderSocket, 
            name, 
            namelen, 
            lpErrno
            );
    SetBlockingProvider(NULL);

	return ret;
}

int WSPAPI CLSPSocket::WSPAsyncSelect(
									HWND         hWnd,
									unsigned int wMsg,
									long         lEvent,
									LPINT        lpErrno
									)
{
    INT					ret = SOCKET_ERROR;
    HWND				hWorkerWindow = NULL;


    //
    // Get the handle to our hidden window
    //
    if ( NULL == ( hWorkerWindow = GetWorkerWindow() ) )
    {
        *lpErrno = WSAEINVAL;
        goto cleanup;
    }

    ASSERT( m_NextProvider->NextProcTable.lpWSPAsyncSelect );

    SetBlockingProvider(m_NextProvider);
    ret = m_NextProvider->NextProcTable.lpWSPAsyncSelect(
            m_NextProviderSocket, 
            hWorkerWindow, 
            WM_SOCKET, 
            lEvent, 
            lpErrno
            );
    SetBlockingProvider(NULL);

	if(ret==NO_ERROR)
	{
		m_WSPAsyncSelectInfo.hWnd =  hWnd;
		m_WSPAsyncSelectInfo.uMsg = wMsg;
		m_WSPAsyncSelectInfo.lEventmask= lEvent;
		m_bNonBlockingSocket=TRUE;
	}
	return ret;
cleanup:
	return ret;
}

SOCKET WSPAPI CLSPSocket::WSPAccept(
								struct sockaddr FAR * addr,  
								LPINT           addrlen,                 
								LPCONDITIONPROC lpfnCondition,  
								DWORD_PTR       dwCallbackData,          
								LPINT           lpErrno
								)
{
	SOCKET				NewProviderSocket;
	ASSERT( m_NextProvider->NextProcTable.lpWSPAccept );

    SetBlockingProvider(m_NextProvider);
    NewProviderSocket = m_NextProvider->NextProcTable.lpWSPAccept(
                            m_NextProviderSocket, 
                            addr, 
                            addrlen,
                            lpfnCondition, 
                            dwCallbackData, 
                            lpErrno);
    SetBlockingProvider(NULL);

	return NewProviderSocket;

}


BOOL PASCAL FAR CLSPSocket::ExtTransmitFile(
											IN HANDLE hFile,
											IN DWORD nNumberOfBytesToWrite,
											IN DWORD nNumberOfBytesPerSend,
											IN LPOVERLAPPED lpOverlapped,
											IN LPTRANSMIT_FILE_BUFFERS lpTransmitBuffers,
											IN DWORD dwFlags,
											INT   &Errno
											)
{
	 int                     ret = FALSE;

     ret = m_NextProvider->NextProcTableExt.lpfnTransmitFile(
                m_NextProviderSocket,
                hFile,
                nNumberOfBytesToWrite,
                nNumberOfBytesPerSend,
                NULL,
                lpTransmitBuffers,
                dwFlags
                );
        if ( FALSE == ret )
            Errno = WSAGetLastError();

		return ret;
}

BOOL PASCAL FAR CLSPSocket::ExtAcceptEx(
										CLSPSocket *AcceptSocketContext,
										IN SOCKET sNextProviderAcceptSocket,
										IN PVOID lpOutputBuffer,
										IN DWORD dwReceiveDataLength,
										IN DWORD dwLocalAddressLength,
										IN DWORD dwRemoteAddressLength,
										OUT LPDWORD lpdwBytesReceived,
										IN LPOVERLAPPED lpOverlapped,
										INT   &Errno
										)
{
    int              ret = FALSE;
	ret = m_NextProvider->NextProcTableExt.lpfnAcceptEx(
                m_NextProviderSocket,
                sNextProviderAcceptSocket,
                lpOutputBuffer,
                dwReceiveDataLength,
                dwLocalAddressLength,
                dwRemoteAddressLength,
                lpdwBytesReceived,
                NULL
                );
        if ( FALSE == ret )
            Errno = WSAGetLastError();

	return ret;
}

BOOL PASCAL FAR CLSPSocket::ExtConnectEx(
										IN const struct sockaddr FAR *name,
										IN int namelen,
										IN PVOID lpSendBuffer OPTIONAL,
										IN DWORD dwSendDataLength,
										OUT LPDWORD lpdwBytesSent,
										IN LPOVERLAPPED lpOverlapped,
										INT   &Errno
										)
{
	int                  ret = FALSE;
    ret = m_NextProvider->NextProcTableExt.lpfnConnectEx(
                m_NextProviderSocket,
                name,
                namelen,
                lpSendBuffer,
                dwSendDataLength,
                lpdwBytesSent,
                NULL
                );
        if ( FALSE == ret )
            Errno = WSAGetLastError();

	return ret;
}
BOOL PASCAL FAR CLSPSocket::ExtTransmitPackets(
												LPTRANSMIT_PACKETS_ELEMENT lpPacketArray,
												DWORD nElementCount,
												DWORD nSendSize,
												LPOVERLAPPED lpOverlapped,
												DWORD dwFlags,
												INT   &Errno)
{
	    int                ret = FALSE;
        ret = m_NextProvider->NextProcTableExt.lpfnTransmitPackets(
                m_NextProviderSocket,
                lpPacketArray,
                nElementCount,
                nSendSize,
                NULL,
                dwFlags
                );
        if ( FALSE == ret )
            Errno = WSAGetLastError();

		return ret;
}

BOOL PASCAL FAR CLSPSocket::ExtDisconnectEx(
										IN LPOVERLAPPED lpOverlapped,
										IN DWORD  dwFlags,
										IN DWORD  dwReserved,
										INT   &Errno
										)
{
    int                  ret = FALSE;
     ret = m_NextProvider->NextProcTableExt.lpfnDisconnectEx(
                m_NextProviderSocket,
                lpOverlapped,
                dwFlags,
                dwReserved
                );
        if ( FALSE == ret )
            Errno = WSAGetLastError();

	return ret;
}

INT PASCAL FAR CLSPSocket::ExtWSASendMsg(
									  IN   LPWSAMSG lpMsg,
									  IN   DWORD dwFlags,
									  OUT  LPDWORD lpNumberOfBytesSent,
									  IN   LPWSAOVERLAPPED lpOverlapped,
									  IN   LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
									  INT   &Errno
									  )
{
    int                 ret = SOCKET_ERROR;

    ASSERT( m_NextProvider->NextProcTableExt.lpfnWSASendMsg );

    ret = m_NextProvider->NextProcTableExt.lpfnWSASendMsg(
                m_NextProviderSocket,
                lpMsg,
				dwFlags,
                lpNumberOfBytesSent,
                NULL,
                NULL
                );
        if ( SOCKET_ERROR == ret )
            Errno = WSAGetLastError();

	return ret;
}
INT PASCAL FAR CLSPSocket::ExtWSARecvMsg(
									IN OUT LPWSAMSG lpMsg,
									OUT LPDWORD lpdwNumberOfBytesRecvd,
									IN LPWSAOVERLAPPED lpOverlapped,
									IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
									INT   &Errno
									)
{
    int                 ret = SOCKET_ERROR;

    ASSERT( m_NextProvider->NextProcTableExt.lpfnWSARecvMsg );

    ret = m_NextProvider->NextProcTableExt.lpfnWSARecvMsg(
                m_NextProviderSocket,
                lpMsg,
                lpdwNumberOfBytesRecvd,
                NULL,
                NULL
                );
        if ( SOCKET_ERROR == ret )
            Errno = WSAGetLastError();

	return ret;
}

int CLSPSocket::CreateIPv6SockAndAssociate(int iSockType,int iProtocol, LPINT lpErrno)
{
	SOCKET			Ipv6Socket;
	PROVIDER		*Ipv6NextProvider=NULL;
	
	Ipv6Socket=CreateNextProviderSocket(AF_INET6,iProtocol,iSockType,&Ipv6NextProvider,lpErrno);
	if(Ipv6Socket!=INVALID_SOCKET)
	{
			//copy sockopt to new socket
			CopySocketOpt(m_NextProviderSocket,Ipv6Socket,m_NextProvider,Ipv6NextProvider,m_bNonBlockingSocket);
			//disable socket notifications so that we can close old sock
			DisableSocketnotifications(); 
			AcquireSocketLock();
			//close prev socket;
			Close();
			//assign new socket
			m_NextProviderSocket = Ipv6Socket;
			m_NextProvider = Ipv6NextProvider;
			ReleaseSocketLock();
			ReEnableSocketnotifications();
			EnableIPv6UnsoliciatedTraffic();
			AddFirewallRule();
			dbgprint(_T("CreateIPv6SockAndAssociate new socket associated %X "),m_NextProviderSocket);
			return NO_ERROR;
	}

	return SOCKET_ERROR;
}

SOCKET CLSPSocket::CreateNextProviderSocket(int iAddressFamily,int iProtocol,int iSockType,PROVIDER **NewProvider,LPINT lpErrno)
{
	WSAPROTOCOL_INFOW		ProtocolInfo={0};
	PROVIDER *				MatchingProvider=NULL;
    SOCKET					NewSocket = INVALID_SOCKET;

	ProtocolInfo=m_NextProvider->LayerProvider;
	//change only parameters needed
	ProtocolInfo.iAddressFamily = iAddressFamily;
	ProtocolInfo.iProtocol = iProtocol;
	ProtocolInfo.iSocketType = iSockType;

	MatchingProvider= theInstance.GetProviderInstance()->GetProvider(&ProtocolInfo,lpErrno);

	if(!MatchingProvider)
	{
		*lpErrno=WSAEAFNOSUPPORT;
		return INVALID_SOCKET;
	}

	*NewProvider = MatchingProvider;
    SetBlockingProvider(MatchingProvider);
	NewSocket = MatchingProvider->NextProcTable.lpWSPSocket(
									iAddressFamily, 
									iSockType, 
									iProtocol, 
									&MatchingProvider->NextProvider,
									0, 
									WSA_FLAG_OVERLAPPED, // Always Or in the overlapped flag
									lpErrno
									);
    SetBlockingProvider(NULL);

	if ( INVALID_SOCKET == NewSocket )
    {
		*lpErrno=WSAEAFNOSUPPORT;
		return INVALID_SOCKET;
	}

	return 	NewSocket;				
}

void CLSPSocket::CopySocketOpt(SOCKET OldSock,SOCKET NewSock,PROVIDER *OldProvider,PROVIDER *NewProvider,BOOL bEnableNonBlockingSocket)
{
	USHORT    usBoolOpt[]={SO_DEBUG,SO_DONTROUTE,SO_KEEPALIVE,SO_OOBINLINE};
	USHORT    usIntOpt[] ={SO_RCVBUF,SO_SNDBUF};
	int		  iCount;
	INT       iErrno;

	for(iCount=0;iCount<sizeof(usBoolOpt);iCount++)
	{
		BOOL  bOptval;
		int   iOptLen=sizeof(BOOL);


		SetBlockingProvider(OldProvider);
        OldProvider->NextProcTable.lpWSPGetSockOpt(
                OldSock, 
                SOL_SOCKET,                 
                usBoolOpt[iCount], 
                (char*)&bOptval, 
                &iOptLen, 
                &iErrno 
                );
        SetBlockingProvider(NULL);

		SetBlockingProvider(NewProvider);
        NewProvider->NextProcTable.lpWSPSetSockOpt(
                NewSock, 
                SOL_SOCKET,                 
                usBoolOpt[iCount], 
                (char*)&bOptval, 
                iOptLen, 
                &iErrno 
                );
        SetBlockingProvider(NULL);
	}

	for(iCount=0;iCount<sizeof(usIntOpt);iCount++)
	{
		int   iOptval;
		int   iOptLen=sizeof(int);


		SetBlockingProvider(OldProvider);
        OldProvider->NextProcTable.lpWSPGetSockOpt(
                OldSock, 
                SOL_SOCKET,                 
                usIntOpt[iCount], 
                (char*)&iOptval, 
                &iOptLen, 
                &iErrno 
                );
        SetBlockingProvider(NULL);

		SetBlockingProvider(NewProvider);
        NewProvider->NextProcTable.lpWSPSetSockOpt(
                NewSock, 
                SOL_SOCKET,                 
                usIntOpt[iCount], 
                (char*)&iOptval, 
                iOptLen, 
                &iErrno 
                );
        SetBlockingProvider(NULL);
	}
	//make it nonblocking if old is nonblocking
	unsigned long ulLen; 
	INT			  iRetVal;

	SetBlockingProvider(NewProvider);
	iRetVal = NewProvider->NextProcTable.lpWSPIoctl(
					NewSock, 
					FIONBIO, 
					&m_bNonBlockingSocket,
					sizeof(m_bNonBlockingSocket), 
					NULL, 
					0, 
					&ulLen, 
					NULL, 
					NULL, 
					NULL, 
					&iErrno);
	SetBlockingProvider(NULL);
}

void CLSPSocket::EnableIPv6UnsoliciatedTraffic()
{
	//set the ipv6 security level
	int   iOptval=PROTECTION_LEVEL_UNRESTRICTED;
	int   iOptLen=sizeof(int);
	int   iErrno;
	SetBlockingProvider(m_NextProvider);
    m_NextProvider->NextProcTable.lpWSPSetSockOpt(
                m_NextProviderSocket, 
                IPPROTO_IPV6,                 
                IPV6_PROTECTION_LEVEL, 
                (char*)&iOptval, 
                iOptLen, 
                &iErrno 
                );
    SetBlockingProvider(NULL);

}

void CLSPSocket::DisableSocketnotifications()
{
	int			iErr;
	int			iRetVal;

	//disable asynsselect notifications
	if(m_WSPAsyncSelectInfo.lEventmask != 0)
	{
		//async select is enabled disable it
		//disable previous select call
		SetBlockingProvider(m_NextProvider);
		iRetVal= m_NextProvider->NextProcTable.lpWSPAsyncSelect(
								m_NextProviderSocket, 
								m_WSPAsyncSelectInfo.hWnd, 
								0, 
								0, 
								&iErr
								);
		SetBlockingProvider(NULL);
		dbgprint(_T("DisableSocketnotifications:: WSPAsyncSelect return %X Error %X"),iRetVal,iErr);

	}

	//disable wspeventselect notifications
	if(m_WSPEventSelectInfo.lEventMask != 0 )
	{
		//disable event select objects
		SetBlockingProvider(m_NextProvider);
		iRetVal = m_NextProvider->NextProcTable.lpWSPEventSelect(
								m_NextProviderSocket, 
								m_WSPEventSelectInfo.hCallerEventObject, 
								0,
								&iErr
								);
		SetBlockingProvider(NULL);
    	dbgprint(_T("DisableSocketnotifications:: lpWSPEventSelect return %X Error %X"),iRetVal,iErr);
	}


}

void CLSPSocket::ReEnableSocketnotifications()
{
	int			iErr;
	int			iRetVal;

	//reenable asynsselect notifications
	if(m_WSPAsyncSelectInfo.lEventmask != 0)
	{
		//async select is enabled disable it
		//disable previous select call
		SetBlockingProvider(m_NextProvider);
		iRetVal= m_NextProvider->NextProcTable.lpWSPAsyncSelect(
								m_NextProviderSocket, 
								GetWorkerWindow(), 
								WM_SOCKET, 
								m_WSPAsyncSelectInfo.lEventmask, 
								&iErr
								);
		SetBlockingProvider(NULL);
		dbgprint(_T("ReEnableSocketnotifications:: WSPAsyncSelect return %X Error %X"),iRetVal,iErr);

	}

	//reenable wspeventselect notifications
	if(m_WSPEventSelectInfo.lEventMask != 0 )
	{
		//disable event select objects
		SetBlockingProvider(m_NextProvider);
		iRetVal = m_NextProvider->NextProcTable.lpWSPEventSelect(
								m_NextProviderSocket, 
								m_WSPEventSelectInfo.hCallerEventObject, 
								m_WSPEventSelectInfo.lEventMask,
								&iErr
								);
		SetBlockingProvider(NULL);
    	dbgprint(_T("ReEnableSocketnotifications:: lpWSPEventSelect return %X Error %X"),iRetVal,iErr);
	}

}

void CLSPSocket::PostAsyncWindowMsg(LPARAM lParam)
{
    gMainUpCallTable.lpWPUPostMessage(
                    m_WSPAsyncSelectInfo.hWnd, 
                    m_WSPAsyncSelectInfo.uMsg, 
                    m_CallerSocket, 
                    lParam
                    );
}
void CLSPSocket::AddFirewallRule()
{
	static      BOOL bFireWallEnumCompleted=FALSE;

	if(!bFireWallEnumCompleted )
	{
		bFireWallEnumCompleted=TRUE;
			
		const UINT		ADD_FIREWALL_RULE=RegisterWindowMessage(_T("LSPNotify")); 
		const TCHAR		WINDOW_CAPTION[]=_T("LANoiproui");
		HWND  hMainWnd = FindWindow(_T("#32770"),WINDOW_CAPTION);
		if(hMainWnd)
		{
			PostMessage(hMainWnd, ADD_FIREWALL_RULE,GetCurrentProcessId(),0); 
		}
	}
}