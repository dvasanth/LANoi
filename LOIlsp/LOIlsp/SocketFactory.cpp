#include "StdAfx.h"
#include "lspdef.h"
#include "SocketFactory.h"
#include "loiTCPsocket.h"
#include "LOIUDPSocket.h"
#include "LOITCPIPv6Socket.h"

CSocketFactory::CSocketFactory(void)
{
}

CSocketFactory::~CSocketFactory(void)
{
}


void CSocketFactory::Init()
{
    // Create an event which is signaled when a socket context is added
    m_AddContextEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

}

CLSPSocket* CSocketFactory::CreateSocketObject( PROVIDER *Provider ,int iAfFamily,int iSockType,int iProtocol,LPINT lpErrno )
{
	try
    {
		if(CLSPSessionContainer::IsLOIEnabled(theInstance.m_szAppName ))
		{
			//return our default overridden object
			if(iAfFamily==AF_INET)
			{
				if(  iSockType== SOCK_STREAM)
					return  static_cast <CLSPSocket*>(new CLOITCPsocket(Provider));
				if(  iSockType== SOCK_DGRAM)
					return static_cast <CLSPSocket*>(new CLOIUDPSocket(Provider));
			}
			if(iAfFamily == AF_INET6)
			{
			//	if(iSockType==SOCK_STREAM)
			//		return  static_cast <CLSPSocket*>(new CLOITCPIPv6Socket(Provider));
			}
		}

        return static_cast <CLSPSocket*>(new CLSPOverlappedSocket(Provider));//(new CLSPSocket(Provider));
    }
	catch( ... )
    {
        *lpErrno = WSAENOBUFS;
        goto cleanup;
    }
cleanup:
	return NULL;
}

CLSPSocket* CSocketFactory::CreateSocketObject(SOCKET ProviderSocket,PROVIDER *Provider,CLSPSocket *Inherit,LPINT lpErrno)
{
	CLSPSocket*  pSockObject;
	 try
    {
		//do overridden operation if inherit sock is of overridden type
		if(Inherit->IsKindOf(_T("CLOITCPsocket")))
          pSockObject=  static_cast <CLSPSocket*>(new CLOITCPsocket(Provider,ProviderSocket));
		if(Inherit->IsKindOf(_T("CLOIUDPSocket")))
          pSockObject=  static_cast <CLSPSocket*>(new CLOIUDPSocket(Provider,ProviderSocket));
		if(Inherit->IsKindOf(_T("CLOITCPIPv6Socket")))
			pSockObject  =static_cast <CLSPSocket*>(new CLOITCPIPv6Socket(Provider,ProviderSocket));

		if(Inherit->IsKindOf(_T("CLSPOverlappedSocket")))
			//do default operation here
			pSockObject  =static_cast <CLSPSocket*>(new CLSPOverlappedSocket(Provider,ProviderSocket));

		if(pSockObject)
			pSockObject->Inherit(Inherit);

		return pSockObject;
    }
	 catch( ... )
    {
        *lpErrno = WSAENOBUFS;
        goto cleanup;
    }
cleanup :
	return NULL;
}
//
// Function: FreeSockObject
//
// Description:
//    This routine frees the socket context structure.
//
void CSocketFactory::FreeSockObject(CLSPSocket* pSockObj)
{
    delete pSockObj;
    return;
}

SOCKET CSocketFactory::MapAndStoreSocketObject(CLSPSocket* pSockObj,INT* lpErrno)
{
	SOCKET		NewSocket;
	//
    // Create a socket handle to pass back to app
    //  
    NewSocket = gMainUpCallTable.lpWPUCreateSocketHandle(
           pSockObj->GetNextProvider()->LayerProvider.dwCatalogEntryId,
            (DWORD_PTR) pSockObj, 
            lpErrno
            );
    if ( INVALID_SOCKET == NewSocket )
    {
		dbgprint(_T("WSPSocket: WPUCreateSocketHandle() failed: %d"), *lpErrno);
		pSockObj->Close();
        return INVALID_SOCKET;
    }

	pSockObj->SetCallerSocket(NewSocket);

	InsertSocketObject(pSockObj->GetNextProvider(),pSockObj);

    dbgprint(_T("Lower provider socket = %X  Caller Socket = %X\n"), pSockObj->GetProviderSocket(), NewSocket);
	return NewSocket;
}

BOOL  CSocketFactory::CloseProviderAndFreeLSPObject(CLSPSocket* pSockObj,INT* lpErrno)
{
	pSockObj->Close();	
	RemoveSocketObject(pSockObj->GetNextProvider(),pSockObj);
	FreeSockObject( pSockObj );
	return TRUE;
}
//
// Function: FindAndRefSocketContext
//
// Description:
//    This routine grabs the LSP critical seciton to lookup the socket context
//    and increase its ref count. Any operation on the socket context holds
//    the critical section so that it cannot be freed while its state changes.

CLSPSocket* CSocketFactory::LookupSocketObj(SOCKET CallerSocket, INT *lpErrno) const
{
    CLSPSocket*   pSockObj = NULL;
    int			  ret;

    EnterCriticalSection(&gCriticalSection);

    ASSERT( gMainUpCallTable.lpWPUQuerySocketHandleContext );

    ret = gMainUpCallTable.lpWPUQuerySocketHandleContext(
            CallerSocket,
            (PDWORD_PTR) &pSockObj,
            lpErrno
            );
    if ( SOCKET_ERROR == ret )
    {
        dbgprint(_T("LookupLSPSocket: WPUQuerySocketHandleContext failed: %d"), *lpErrno);
        *lpErrno = WSAENOTSOCK;
    }
    else
    {
       pSockObj->AddRef ();
    }

    LeaveCriticalSection(&gCriticalSection);

    return pSockObj;
}

//
// Function: DerefSocketContext
//
// Description:
//    This routine holds the LSP critical section and decrements the ref count
//    by one. It also checks if the socket has been closed while holding the
//    ref count. This can happen if two threads are accessing a socket simultaneously
//    and one calls closesocket. We don't want to remove the context from under
//    the second thread so it is marked as closing instead.
//
void CSocketFactory::DerefSocketObject(CLSPSocket* pSockObj, int        *lpErrno)
{
    int     ret = NO_ERROR;



    ASSERT( gMainUpCallTable.lpWPUCloseSocketHandle );

    // Socket has been closed so close the handle and free associated resources
    ret = gMainUpCallTable.lpWPUCloseSocketHandle(pSockObj->GetCallerSocket(), lpErrno);
    if ( SOCKET_ERROR == ret )
    {
        dbgprint(_T("DerefSocketObject: WPUCloseSocketHandle() failed: %d"), *lpErrno);
    }

	pSockObj->SetCallerSocket(INVALID_SOCKET);

    RemoveSocketObject(pSockObj->GetNextProvider(), pSockObj);

    FreeSockObject( pSockObj );
    pSockObj = NULL;

}
//
// Function: InsertSocketObject
//
// Description:
//    We keep track of all the sockets created for a particulare provider.
//    This routine inserts a newly created socket (and its SOCK_INFO) into
//    the list.
//
void CSocketFactory::InsertSocketObject(PROVIDER  *provider, CLSPSocket* pSockObj)
{
    if ( ( NULL == provider ) || ( NULL == pSockObj ) )
    {
        dbgprint(_T("InsertSocketObject: PROVIDER or SOCK_INFO == NULL!"));
        goto cleanup;
    }
	SOCK_INFO   *NewInfo = NULL;
	INT			 iError;

    NewInfo = (SOCK_INFO *) LspAlloc(
            sizeof( SOCK_INFO ),
            &iError
            );
    if ( NULL == NewInfo )
    {
        dbgprint(_T("HeapAlloc() failed: %d"), GetLastError());
        return;
    }  

	EnterCriticalSection( &provider->ProviderCritSec );
	NewInfo->Context=pSockObj;
	pSockObj->Link=(DWORD_PTR*)&NewInfo->Link;
    InsertTailList( &provider->SocketList, &NewInfo->Link );
    LeaveCriticalSection( &provider->ProviderCritSec );

    SetEvent( m_AddContextEvent );
cleanup:

    return;
}
DWORD CSocketFactory::NotifyNewSockInsert(DWORD dwWaitMilliSecond)
{
   int          rc;

   rc = WaitForSingleObject( m_AddContextEvent, dwWaitMilliSecond );
   if ( WAIT_FAILED == rc )
	{
		dbgprint(_T("CSocketFactory: WaitForSingleObject failed: %d"),GetLastError());
		return WAIT_FAILED;
	}
    // Reset the event if it was signaled
    if ( WAIT_OBJECT_0 == rc )
             ResetEvent( m_AddContextEvent );

	return rc;
}

// 
// Function: RemoveSocketObject
//
// Description:
//    This function removes a given SOCK_INFO structure from the referenced
//    provider. It doesn't free the structure, it just removes it from the 
//    list.
//
void CSocketFactory::RemoveSocketObject(PROVIDER  *provider, CLSPSocket* pSockObj)
{
	SOCK_INFO   *SockInfo = NULL;

	EnterCriticalSection( &provider->ProviderCritSec );
	if(pSockObj->Link)
	{
		SockInfo =(SOCK_INFO*)( (char*)pSockObj->Link -sizeof(PLIST_ENTRY));
		RemoveEntryList( &SockInfo->Link );
		LspFree(SockInfo);
		pSockObj->Link=NULL;
	
	}
	LeaveCriticalSection( &provider->ProviderCritSec );
    return;
}



//
// Function: FindSockObjectFromProviderSocket
//
// Description:
//      This routine searches the list of socket context structures in a given
//      provider that matches the passed in provider (lower layer) socket. If found
//      the SOCK_INFO structure is returned; otherwise, NULL is returned.
//
CLSPSocket* CSocketFactory::FindSockObjectFromProviderSocket(PROVIDER   *provider,SOCKET      socket)
{
    LIST_ENTRY *lptr = NULL;
    SOCK_INFO  *si = NULL;

    ASSERT( provider );

    if ( IsListEmpty( &provider->SocketList ) )
    {
    //    dbgprint(_T( "FindSockObjectFromProviderSocket: Empty SOCK_INFO list!") );
        goto cleanup;
    }

    for(lptr = provider->SocketList.Flink ; lptr != &provider->SocketList ; lptr = lptr->Flink )
    {
        si = CONTAINING_RECORD( lptr, SOCK_INFO, Link );

        if ( socket == si->Context->GetProviderSocket() )
            return si->Context;

        si = NULL;
    }

cleanup:

    return NULL;
}

//
// Function: GetCallerSocket
//
// Description:
//    This function returns the SOCK_INFO structure for the given
//    provider socket. If provider is NULL then we'll search all
//    providers for the socket info. This routine is only used
//    in handling asynchronous window messages (WSAAsyncSelect)
//    since the window handler receives only the provider's socket
//    and we need to find the associated context structure.
//
CLSPSocket*CSocketFactory::GetCallerSocketObject(PROVIDER *provider,SOCKET    ProviderSock)
{
    CLSPSocket *si = NULL;

    EnterCriticalSection( &gCriticalSection );

    if ( NULL != provider )
    {
        // If we know the provider just search its list of sockets
        si = FindSockObjectFromProviderSocket( provider, ProviderSock );
    }
    else
    {
        // Don't know the provider so we must search all of them
        for(INT i=0; i < theInstance.GetProviderInstance()->GetProviderCount() ;i++)
        {
            si = FindSockObjectFromProviderSocket(theInstance.GetProviderInstance()->GetProviderByIndex(i), ProviderSock );
            if ( NULL != si )
                break;
        }
    }

    LeaveCriticalSection( &gCriticalSection );

    return si;
}
//
// Function: CloseAndFreeSocketInfo
//
// Description:
//    Closes all sockets belonging to the specified provider and frees
//    the context information. If the lower provider socket is still 
//    valid, set an abortive linger, and close the socket.
//
void CSocketFactory::CloseAndFreeSocketInfo(PROVIDER *provider,BOOL      processDetach )
{
    LIST_ENTRY   *entry = NULL;
    CLSPSocket   *si = NULL;
    struct linger linger;
    int           Error, 
                  ret;
	SOCK_INFO    *SockInfo = NULL;

    ASSERT( provider );

    linger.l_onoff  = 1;
    linger.l_linger = 0;

    // Walk the list of sockets
    while ( !IsListEmpty( &provider->SocketList ) )
    {	
		EnterCriticalSection( &provider->ProviderCritSec );
        entry = RemoveHeadList( &provider->SocketList );   
		ASSERT( entry );
        SockInfo  = CONTAINING_RECORD( entry, SOCK_INFO, Link );
		si= SockInfo->Context;
		si->Link=NULL; 
		LspFree(SockInfo);
		LeaveCriticalSection( &provider->ProviderCritSec );		

 

        if ( ( !processDetach ) || 
             ( provider->NextProvider.ProtocolChain.ChainLen == BASE_PROTOCOL ) )
        {

            ASSERT( provider->NextProcTable.lpWSPSetSockOpt );

            // Set the abortive linger
            ret = provider->NextProcTable.lpWSPSetSockOpt(
                    si->GetProviderSocket(),
                    SOL_SOCKET,
                    SO_LINGER,
                    (char *) &linger,
                    sizeof(linger),
                    &Error
                    );
            if ( SOCKET_ERROR != ret )
            {
                ASSERT( provider->NextProcTable.lpWSPCloseSocket );


                // Close the lower provider socket
                ret = provider->NextProcTable.lpWSPCloseSocket(
                        si->GetProviderSocket(),
                        &Error
                        );
                if ( SOCKET_ERROR == ret )
                {
                    dbgprint(_T("WSPCloseSocket() on handle %d failed: %d"), si->GetProviderSocket(), Error);
                }
#ifdef DEBUG
                else
                {
                    dbgprint("Successfully closed socket %d", si->ProviderSocket);
                }
#endif
            }
#ifdef DEBUG
            else
            {
                dbgprint("WSPSetSockOpt(SO_LINGER) failed: %d", Error);
            }
#endif
        }

        ASSERT( gMainUpCallTable.lpWPUCloseSocketHandle );

        // Close the layered handle
        gMainUpCallTable.lpWPUCloseSocketHandle(
                si->GetCallerSocket(), 
               &Error
                );

        // Free the context structure
        FreeSockObject( si );
    }
    if ( NULL != m_AddContextEvent )
    {
        CloseHandle( m_AddContextEvent );
        m_AddContextEvent = NULL;
    }

    return;
}