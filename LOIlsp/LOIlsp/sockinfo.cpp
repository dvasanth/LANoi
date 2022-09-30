// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (C) 2004  Microsoft Corporation.  All Rights Reserved.
//
// Module Name: sockinfo.cpp
//
// Description:
//
//    This sample illustrates how to develop a layered service provider that is
//    capable of counting all bytes transmitted through a TCP/IP socket.
//
//    This file contains routines associated with the SOCK_INFO structure. This
//    structure maintains the mapping between the upper layer's socket and the
//    corresponding lower layer's socket. It also keeps track of the current
//    state and what operations are pending on the socket. The routines in this
//    file are for allocating, linked list management, etc.
//    

#include "lspdef.h"

// Lookup a socket context structure given the lower provider's socket
SOCK_INFO *
FindSockInfoFromProviderSocket(
    PROVIDER   *provider,
    SOCKET      socket
    );




//
// Function: CreateSockInfo
//
// Description:
//    Allocates a new socket info context structure and initializes the fields
//    except for the LayeredSocket field. The context must be allocated first,
//    then the layered socket is created (with the SOCK_INFO structure as the
//    context information), and then the LayeredSocket field is set. If
//    the Inherit context is provided, information is copied to the new socket
//    context structure (such as with WSPAccept). If the Insert flag is TRUE
//    then the context is automatically inserted into the list of sockets
//    for the given provider. If not then the caller must insert the context
//    (WSPAccept does this to ensure all fields of the context are valid
//    including LayeredSocket before insertion so that the async thread
//    handler will work properly).
//
SOCK_INFO *
CreateSockInfo(
    PROVIDER  *Provider, 
    SOCKET     ProviderSocket, 
    SOCK_INFO *Inherit, 
    BOOL       Insert,
    int       *lpErrno
    )
{
    SOCK_INFO   *NewInfo = NULL;

    NewInfo = (SOCK_INFO *) LspAlloc(
            sizeof( SOCK_INFO ),
            lpErrno
            );
    if ( NULL == NewInfo )
    {
        dbgprint("HeapAlloc() failed: %d", GetLastError());
       *lpErrno = WSAENOBUFS;
        goto cleanup;
    }

    //
    // Initialize the fields to default values
    //
    NewInfo->ProviderSocket     = ProviderSocket;
    NewInfo->bClosing           = FALSE;
    NewInfo->dwOutstandingAsync = 0;
    NewInfo->BytesRecv          = 0;
    NewInfo->BytesSent          = 0;
    NewInfo->Provider           = Provider;
    NewInfo->hWnd               = (Inherit ? Inherit->hWnd : 0);
    NewInfo->uMsg               = (Inherit ? Inherit->uMsg : 0);

    __try
    {
        InitializeCriticalSection( &NewInfo->SockCritSec );
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
        *lpErrno = WSAENOBUFS;
        goto cleanup;
    }

    if ( TRUE == Insert )
        InsertSocketInfo(Provider, NewInfo);

    return NewInfo;

cleanup:

    if ( NULL != NewInfo )
        LspFree( NewInfo );

    return NULL;
}





//
// Function: FindSockInfoFromProviderSocket
//
// Description:
//      This routine searches the list of socket context structures in a given
//      provider that matches the passed in provider (lower layer) socket. If found
//      the SOCK_INFO structure is returned; otherwise, NULL is returned.
//
SOCK_INFO *
FindSockInfoFromProviderSocket(
    PROVIDER   *provider,
    SOCKET      socket
    )
{
    LIST_ENTRY *lptr = NULL;
    SOCK_INFO  *si = NULL;

    ASSERT( provider );

    if ( IsListEmpty( &provider->SocketList ) )
    {
        dbgprint( "FindSockInfoFromProviderSocket: Empty SOCK_INFO list!" );
        goto cleanup;
    }

    for(lptr = provider->SocketList.Flink ; lptr != &provider->SocketList ; lptr = lptr->Flink )
    {
        si = CONTAINING_RECORD( lptr, SOCK_INFO, Link );

        if ( socket == si->ProviderSocket )
            break;

        si = NULL;
    }

cleanup:

    return si;
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
SOCK_INFO *
GetCallerSocket(
    PROVIDER *provider, 
    SOCKET    ProviderSock
    )
{
    SOCK_INFO *si = NULL;

    EnterCriticalSection( &gCriticalSection );

    if ( NULL != provider )
    {
        // If we know the provider just search its list of sockets
        si = FindSockInfoFromProviderSocket( provider, ProviderSock );
    }
    else
    {
        // Don't know the provider so we must search all of them
        for(INT i=0; i < gLayerCount ;i++)
        {
            si = FindSockInfoFromProviderSocket( &gBaseInfo[ i ], ProviderSock );
            if ( NULL != si )
                break;
        }
    }

    LeaveCriticalSection( &gCriticalSection );

    return si;
}
