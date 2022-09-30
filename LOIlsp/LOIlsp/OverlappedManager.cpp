#include "StdAfx.h"
#include "lspdef.h"
#include "OverlappedManager.h"

COverlappedManager::COverlappedManager(void)
{
	m_WorkerThread = NULL;
	m_Iocp = NULL;
	m_ThreadCount = (DWORD)-1;
}

COverlappedManager::~COverlappedManager(void)
{
}




////////////////////////////////////////////////////////////////////////////////
//
// Function Implementation
//
////////////////////////////////////////////////////////////////////////////////

//
// Function: InitOverlappedManager
//
// Description:
//    This function is called once and determines whether we're running
//    on NT or Win9x. If on NT, create an IO completion port as the
//    intermediary between the upper and lower layer. All overlapped IO
//    will be posted using this IOCP. When IOCP are available we'll create
//    a number of threads (equal to the number of procs) to service the
//    completion of the IO. If on Win9x, we'll create a single thread which
//    will post overlapped IO operations using APCs.
//
int COverlappedManager::InitOverlappedManager()
{
    DWORD   i;
    int     ret = NO_ERROR;

    EnterCriticalSection( &gOverlappedCS );

    //
    // Make sure we're not already initialized -- we'll always have at least
    // one worker thread running
    //
    if ( NULL != m_WorkerThread )
        goto cleanup;

    InitializeListHead( &m_FreeOverlappedPlus );

    //
    // See if we're on NT by trying to create the completion port. If it
    //  fails then we're on Win9x.
    //
    m_Iocp = CreateIoCompletionPort(
            INVALID_HANDLE_VALUE,
            NULL,
            (ULONG_PTR)0,
            0
            );
    if ( NULL != m_Iocp )
    {
        SYSTEM_INFO     sinfo;

        //
        // We're on NT so figure out how many processors we have
        //
        GetSystemInfo( &sinfo );
        m_ThreadCount = sinfo.dwNumberOfProcessors;
    }
  

    dbgprint(_T("Creating %d threads"), m_ThreadCount);

    m_WorkerThread = (HANDLE *) LspAlloc(
            sizeof( HANDLE ) * m_ThreadCount,
           &ret         // if fails, ret will be WSAENOBUFS
            );
    if ( NULL == m_WorkerThread )
    {
        goto cleanup;
    }
//	m_ThreadCount=1;
    //
    // Create our worker threads
    //
    for(i=0; i < m_ThreadCount ;i++)
    {
        m_WorkerThread[i] = CreateThread(
                NULL, 
                0, 
                OverlappedManagerThread, 
                (LPVOID)m_Iocp, 
                0, 
                NULL
                );
        if ( NULL == m_WorkerThread[ i ] )
        {
            dbgprint(_T("InitOverlappedManager: CreateThread() failed: %d"), GetLastError());
            ret = WSAEPROVIDERFAILEDINIT;
            goto cleanup;
        }
    }

cleanup:

    LeaveCriticalSection( &gOverlappedCS );

    return ret;
}

//
// Function: StopOverlappedManager
//
// Description:
//    This function is called before the DLL gets unloaded. It tries to
//    shutdown the worker threads gracefully before exiting. It also
//    frees/closes allocated memory and handles.
//
int COverlappedManager::StopOverlappedManager()
{
    DWORD     i;
    int       ret = NO_ERROR;

    EnterCriticalSection( &gOverlappedCS );

    //
    // Post a completion packet to the IOCP (one for each thread)
    //
    if ( NULL != m_Iocp )
    {
        for(i=0; i < m_ThreadCount ;i++)
        {
            ret = PostQueuedCompletionStatus(
                    m_Iocp,
                    (DWORD)-1,
                    0,
                    NULL
                    );
            if ( 0 == ret )
            {
                dbgprint(_T("PostQueuedCompletionStatus() failed: %d"), GetLastError());
            }
        }

        //
        // Wait a while for the threads to get the signal and exit - if it fails or
        // times out then oh well, we need to clean up anyway
        //
    }
   
    if ( NULL != m_WorkerThread )
    {
        ret = WaitForMultipleObjectsEx( m_ThreadCount, m_WorkerThread, TRUE, 5000, TRUE );
        if ( WAIT_TIMEOUT == ret )
            dbgprint(_T("StopOverlappedManager: Timed out waiting for IOCP threads to exit!"));
        else if ( WAIT_FAILED == ret )
            dbgprint(_T("StopOverlappedManager: WaitForMultipleObjectsEx failed: %d"),
                    GetLastError());
        else
            dbgprint(_T("StopOverlappedManager: All worker threads stopped!"));

        for(i=0; i < m_ThreadCount ;i++)
        {
            CloseHandle( m_WorkerThread[ i ] );
            m_WorkerThread[ i ] = NULL;

            dbgprint(_T("Closing overlapped thread(s)"));
        }

        LspFree( m_WorkerThread );
        m_WorkerThread = NULL;
    }

    //
    // Cleanup remaining handles...
    //
    if ( NULL != m_Iocp )
    {
        CloseHandle( m_Iocp );
        m_Iocp = NULL;
    }

     LeaveCriticalSection( &gOverlappedCS );

	 Sleep(200);
	 FreeOverlappedLookasideList();
     return ret;
}

//
// Function: OverlappedManagerThread
//
// Description:
//    This thread receives the completion notifications for operations
//    posted to our IO completion port. Once we receive a completion
//    we'll complete the operation back to the calling app.
//
DWORD WINAPI COverlappedManager::OverlappedManagerThread(LPVOID lpParam)
{
    WSAOVERLAPPEDPLUS *pOverlapPlus = NULL;
    WSAOVERLAPPED     *pOverlap = NULL;
    HANDLE             hIocp = (HANDLE)lpParam;
    ULONG_PTR          key;
    DWORD              dwBytesXfered;
    int                ret;
    LPWSAOVERLAPPEDPLUS olp = NULL;

    while ( TRUE )
    {
        if ( NULL != hIocp )
        {
            ret = GetQueuedCompletionStatus(
                    hIocp,
                   &dwBytesXfered,
                   &key,
                   &pOverlap,
                    INFINITE
                    );
            if ( 0 == ret )
            {
                // Socket failures could be reported here so we still
                // call IntermediateCompletionRoutine
                dbgprint(_T("GetQueuedCompletionStatus() failed: %d"), GetLastError());
            }

            if ( -1 == dwBytesXfered )
            {
                //
                // StopOverlappedManager will send a completion packet with -1
                //    bytes transfered to indicate the completion routine
                //    should exit
                //
                dbgprint(_T("OverlappedManagerThread: Received exit message"));
                goto cleanup;
            }
			if ( NULL == pOverlap )
			{
				dbgprint(_T("IntermediateCompletionRoutine: lpOverlapped == NULL!"));

				continue;
			}

			ASSERT( pOverlap );

			olp = CONTAINING_RECORD( pOverlap, WSAOVERLAPPEDPLUS, ProviderOverlapped );

			//
			// We actually already have the socket context for this operation (its in
			//    the WSAOVERLAPPEDPLUS structure but do this anyway to make sure the
			//    socket hasn't been closed as well as to increment the ref count while
			//    we're accessing the SOCK_INFO structure.
			//
			CLSPOverlappedSocket*			pSockObj = NULL;
			int								Error;
			pSockObj=dynamic_cast<CLSPOverlappedSocket*> (theInstance.GetSockFactoryInstance()->LookupSocketObj(olp->CallerSocket, &Error));
			if ( NULL == pSockObj )
			{
				dbgprint( _T("IntermediateCompletionRoutine: LookupSocketObj failed!") );
				continue;
			}

			// Handle the IO that completed
			pSockObj->IntermediateCompletionRoutine(
														WSA_IO_PENDING,
														dwBytesXfered,
														olp,
														0
														);
		
        }
     
    }

cleanup:

    ExitThread( 0 );
}

//
// Function: QueueOverlappedOperation
//
// Description:
//    Whenever one of the overlapped enabled Winsock SPI functions are
//    called (as an overlapped operation), it calls this function which
//    determines whether it can execute it immediate (in the case of NT
//    and IOCP) or post it to a queue to be executed by the woker thread
//    via an APC (on Win9x).
//
INT COverlappedManager::QueueOverlappedOperation(LPWSAOVERLAPPED  lpCallerOverlapped,LPINT lpErrno)
{
	int     ret=NO_ERROR;

    //
    // Set the fields of the overlapped to indicate IO is not complete yet
    //

    __try
    {
        SetOverlappedInProgress( lpCallerOverlapped );
    } 
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
		*lpErrno=WSAEFAULT;
        return SOCKET_ERROR;
    }

    //
    // Simply execute the operation
    //
	return ret ;
	
}

HANDLE COverlappedManager::AssociateHandle(SOCKET ProviderSocket,SOCKET CallerSocket,BOOL& bSynchronous)
{
	HANDLE   hSockIocp=NULL;
	int		 err;

	if ( NULL != m_Iocp )
    {
          hSockIocp = CreateIoCompletionPort(
                    (HANDLE)ProviderSocket,
                    m_Iocp,
                    CallerSocket,
                    0
                    );
            if ( NULL == hSockIocp )
            {
                if ( ERROR_INVALID_PARAMETER == (err = GetLastError() ) )
                {
                    //
                    // If the socket option SO_SYNCHRONOUS_(NON)ALERT is set then 
                    // no overlapped operation can be performed on that socket and 
                    // tryiing to associate it with a completion port will fail. 
                    // The other bad news is that an LSP cannot trap this setsockopt 
                    // call. In reality we don't have to do anything. If an app sets 
                    // this option and then makes overlapped calls anyway, then they 
                    // shouldn't be expecting any overlapped notifications! This 
                    // statement is put here in case you want to mark the socket
                    // info structure as synchronous.
                    //
                    bSynchronous = TRUE;
                }
                else
                {
                    dbgprint(_T("QueueOverlappedOperation: CreateIoCompletionPort() ")
                             _T( "failed: %d (Prov %d Iocp 0x%x Caller 0x%x 0)"), 
                            err, ProviderSocket, 
                            m_Iocp, CallerSocket);
                }
            }
            dbgprint(_T("Adding provider handle %X to IOCP"), ProviderSocket);
        }

 
	return hSockIocp;

}

//
// Function: AllocOverlappedStructure
//
// Description:
//    This returns an unused WSAOVERLAPPEDPLUS structure. We maintain a list
//    of freed structures so that we don't spend too much time allocating and
//    freeing memory.
//
LPWSAOVERLAPPEDPLUS COverlappedManager::AllocOverlappedStructure(CLSPOverlappedSocket  *SocketContext)
{
    LPWSAOVERLAPPEDPLUS lpWorkerOverlappedPlus = NULL;

    if ( NULL == SocketContext )
    {
        dbgprint(_T("AllocOverlappedStructure: SocketContext is NULL!"));
        return NULL;
    }

    EnterCriticalSection( &gOverlappedCS );



    if ( IsListEmpty( &m_FreeOverlappedPlus ) )
    {
        int     err;

        lpWorkerOverlappedPlus = (WSAOVERLAPPEDPLUS *) LspAlloc( 
                sizeof(WSAOVERLAPPEDPLUS), 
               &err 
                );
    }
    else
    {
        LIST_ENTRY  *entry = NULL;

        entry = RemoveHeadList( &m_FreeOverlappedPlus );

        lpWorkerOverlappedPlus = CONTAINING_RECORD( entry, WSAOVERLAPPEDPLUS, Link );
    }

  

    LeaveCriticalSection( &gOverlappedCS );

    return lpWorkerOverlappedPlus;
}

//
// Function: SetOverlappedInProgress
//
// Description:
//    Simply set the interal fields of an OVERLAPPED structure to the
//    "in progress" state.
//
void COverlappedManager::SetOverlappedInProgress(OVERLAPPED *ol)
{
    ol->Internal = WSS_OPERATION_IN_PROGRESS;
    ol->InternalHigh = 0;
}




//
// Function: FreeOverlappedStructure
//
// Description:
//    Once we're done using a WSAOVERLAPPEDPLUS structure we return it to
//    a list of free structures for re-use later.
//
void COverlappedManager::FreeOverlappedStructure(WSAOVERLAPPEDPLUS  *olp)
{
    EnterCriticalSection( &gOverlappedCS );


    memset( olp, 0, sizeof( WSAOVERLAPPEDPLUS ) );

    InsertHeadList( &m_FreeOverlappedPlus, &olp->Link );

    LeaveCriticalSection( &gOverlappedCS );
}



//
// Function: FreeOverlappedLookasideList
// 
// Description:
//      This routine frees all WSAOVELRAPPEDPLUS structures in the lookaside list.
//      This function is called when the LSP is unloading.
//
void COverlappedManager::FreeOverlappedLookasideList()
{
    WSAOVERLAPPEDPLUS  *olp = NULL;
    LIST_ENTRY         *entry = NULL;

    EnterCriticalSection( &gOverlappedCS );

    while ( ! IsListEmpty( &m_FreeOverlappedPlus ) )
    {
        entry = RemoveHeadList( &m_FreeOverlappedPlus );

        olp = CONTAINING_RECORD( entry, WSAOVERLAPPEDPLUS, Link );

        LspFree( olp );
    }

    LeaveCriticalSection( &gOverlappedCS );
}

