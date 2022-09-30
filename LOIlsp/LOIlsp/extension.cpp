// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (C) 2004  Microsoft Corporation.  All Rights Reserved.
//
// Module Name: extension.cpp
//
// Description:
//
//    This sample illustrates how to develop a layered service provider that is
//    capable of counting all bytes transmitted through a TCP/IP socket.
//
//    This file contains all of the Winsock extension functions that can
//    be monitored by a service provider. This is done by intercepting any
//    WSAIoctl calls with the SIO_GET_EXTENSION_FUNCTION_POINTER (see spi.cpp
//    and WSPIoctl for more info). We substitute our own function pointer so
//    that an application calls into us. Currently we intercept only TransmitFile
//    and AcceptEx.
//
#include "stdafx.h"
#include "lspdef.h"
#include "SocketFactory.h"
//
// Function: LoadExtensionFunction
//
// Description:
//    This function dynamically loads the given extension function from the
//    underlying provider. Each extension function checks to see if the 
//    corresponding extension function for the lower provider is loaded
//    before calling. If not, it will load it as needed. This is necessary
//    if the app loads the extension function for say TCP and then calls
//    that extension function on a UDP socket. Normally this isn't the case
//    but we're being defensive here.
//
BOOL
LoadExtensionFunction(
    FARPROC   **func,
    GUID        ExtensionGuid,
    LPWSPIOCTL  fnIoctl,
    SOCKET      s
    )
{
    DWORD   dwBytes;
    int     rc, 
            error,
            ret = TRUE;

    // Use the lower provider's WSPIoctl to load the extension function
    rc = fnIoctl(
            s,
            SIO_GET_EXTENSION_FUNCTION_POINTER,
           &ExtensionGuid,
            sizeof(GUID),
            func,
            sizeof(FARPROC),
           &dwBytes,
            NULL,
            NULL,
            NULL,
           &error
            );
    

    if ( SOCKET_ERROR == rc )
    {
        dbgprint(L"LoadExtensionFunction: WSAIoctl (SIO_GET_EXTENSION_FUNCTION) failed: %d",
            error);
        ret = FALSE;
    }
    else if( NULL == *func )
    {
        // Some providers won't throw an error even when
        // they return a NULL function pointer    

        dbgprint(_T("LoadExtensionFunction: WSAIoctl (SIO_GET_EXTENSION_FUNCTION) returned a NULL function pointer"));
        ret = FALSE;
    }

    return ret;
}

//
// Function: ExtGetAcceptExSockaddrs
//
// Description:
//      There is no need to intercept the GetAcceptExSockaddrs function since
//      it doesn't take any socket handles as parameters. If the LSP is proxying
//      connections (or something similar) where the LSP "hides" either the local
//      or remote addresses, it may be required to intercept this function and
//      change the addresses. Another option is to make this change in the
//      ExtAcceptEx function (but the addresses there are encoded).
//

//
// Function: ExtTransmitFile
//
// Description:
//    This is our provider's TransmitFile function. When an app calls WSAIoctl
//    to request the function pointer to TransmitFile, we intercept the call
//    and return a pointer to our extension function instead.
//
BOOL PASCAL FAR 
ExtTransmitFile(
    IN SOCKET hSocket,
    IN HANDLE hFile,
    IN DWORD nNumberOfBytesToWrite,
    IN DWORD nNumberOfBytesPerSend,
    IN LPOVERLAPPED lpOverlapped,
    IN LPTRANSMIT_FILE_BUFFERS lpTransmitBuffers,
    IN DWORD dwFlags
    )
{
    CLSPSocket*			pSockObj  = NULL;
    int                 Errno = 0,
                        ret = FALSE;

    // Get the context
    pSockObj=theInstance.GetSockFactoryInstance()->LookupSocketObj( hSocket, &Errno );
    if ( NULL == pSockObj )
    {
        dbgprint(_T("ExtTransmitFile: LookupSocketObj failed!") );
        goto cleanup;
    }


	ret = pSockObj->ExtTransmitFile(
            hFile,
            nNumberOfBytesToWrite,
            nNumberOfBytesPerSend,
            lpOverlapped,
            lpTransmitBuffers,
            dwFlags,
			Errno
            );

cleanup:

	if ( NULL != pSockObj )
		pSockObj->Release();

    if ( FALSE == ret )
        WSASetLastError( Errno );

    return ret;
}

//
// Function: ExtAcceptEx
//
// Description:
//    This is our provider's AcceptEx function. When an app calls WSAIoctl
//    to request the function pointer to AcceptEx, we intercept the call
//    and return a pointer to our extension function instead.
//
BOOL PASCAL FAR 
ExtAcceptEx(
    IN SOCKET sListenSocket,
    IN SOCKET sAcceptSocket,
    IN PVOID lpOutputBuffer,
    IN DWORD dwReceiveDataLength,
    IN DWORD dwLocalAddressLength,
    IN DWORD dwRemoteAddressLength,
    OUT LPDWORD lpdwBytesReceived,
    IN LPOVERLAPPED lpOverlapped
    )
{
    LPWSAOVERLAPPEDPLUS ProviderOverlapped = NULL;
    CLSPSocket         *ListenSocketContext = NULL,
                       *AcceptSocketContext = NULL;
    int                 Errno = 0,
                        ret = FALSE;


    //
    // Query the socket context for the listening socket
    //
    ListenSocketContext = theInstance.GetSockFactoryInstance()->LookupSocketObj( sListenSocket, &Errno );
    if ( NULL == ListenSocketContext )
    {
        dbgprint(_T( "ExtAcceptEx: LookupSocketObj failed! (listen socket)") );
        goto cleanup;
    }
    //
    // Also need to query the socket context for the accept socket
    //
    AcceptSocketContext = theInstance.GetSockFactoryInstance()->LookupSocketObj( sAcceptSocket, &Errno );
    if ( NULL == AcceptSocketContext )
    {
        dbgprint( _T("ExtAcceptEx: LookupSocketObj failed! (accept socket)" ));
        goto cleanup;
    }


	ret=ListenSocketContext->ExtAcceptEx(
				AcceptSocketContext,
                sAcceptSocket,
                lpOutputBuffer,
                dwReceiveDataLength,
                dwLocalAddressLength,
                dwRemoteAddressLength,
                lpdwBytesReceived,
                lpOverlapped,
				Errno
					);

cleanup:

    if ( NULL != ListenSocketContext )
		ListenSocketContext->Release(); 

    if ( NULL != AcceptSocketContext )
		AcceptSocketContext->Release(); 

    if ( FALSE == ret )
        WSASetLastError( Errno );

    return ret;
}






//
// Function: ExtConnectEx
//
// Description:
//    This is our provider's ConnectEx function. When an app calls WSAIoctl
//    to request the function pointer to ConnectEx, we intercept the call
//    and return a pointer to our extension function instead.
//
BOOL PASCAL FAR 
ExtConnectEx(
    IN SOCKET s,
    IN const struct sockaddr FAR *name,
    IN int namelen,
    IN PVOID lpSendBuffer OPTIONAL,
    IN DWORD dwSendDataLength,
    OUT LPDWORD lpdwBytesSent,
    IN LPOVERLAPPED lpOverlapped
    )
{
    CLSPSocket           *SocketContext = NULL;
    int                  Errno = NO_ERROR,
                         ret = FALSE;

    // Get the context
    SocketContext = theInstance.GetSockFactoryInstance()->LookupSocketObj( s, &Errno );
    if ( NULL == SocketContext )
    {
        dbgprint(_T( "ExtConnectEx: FindAndRefSocketContext failed!" ));
        goto cleanup;
    }

 
        ret = SocketContext->ExtConnectEx(
                name,
                namelen,
                lpSendBuffer,
                dwSendDataLength,
                lpdwBytesSent,
                lpOverlapped,
				Errno
                );

cleanup:

    if ( NULL != SocketContext )
		SocketContext->Release();

    if ( FALSE == ret )
        WSASetLastError( Errno );

    return ret;
}

//
// Function: ExtTransmitPackets
//
// Description:
//    This is our provider's TransmitPackets function. When an app calls WSAIoctl
//    to request the function pointer to TransmitPackets, we intercept the call
//    and return a pointer to our extension function instead.
//
BOOL PASCAL FAR 
ExtTransmitPackets(
    SOCKET hSocket,
    LPTRANSMIT_PACKETS_ELEMENT lpPacketArray,
    DWORD nElementCount,
    DWORD nSendSize,
    LPOVERLAPPED lpOverlapped,
    DWORD dwFlags)
{
    CLSPSocket           *SocketContext = NULL;
    int                  Errno = NO_ERROR,
                         ret = FALSE;

    // Get the context
    SocketContext = theInstance.GetSockFactoryInstance()->LookupSocketObj( hSocket, &Errno );
    if ( NULL == SocketContext )
    {
        dbgprint(_T( "ExtTransmitPackets: FindAndRefSocketContext failed!") );
        goto cleanup;
    }

    
        ret = SocketContext->ExtTransmitPackets(
                lpPacketArray,
                nElementCount,
                nSendSize,
                lpOverlapped,
                dwFlags,
				Errno
                );
  


cleanup:

    if ( NULL != SocketContext )
		SocketContext->Release(); 
     

    if ( FALSE == ret )
        WSASetLastError( Errno );

    return ret;
}

//
// Function: ExtDisconnectEx
//
// Description:
//    This is our provider's DisconnectEx function. When an app calls WSAIoctl
//    to request the function pointer to DisconnectEx, we intercept the call
//    and return a pointer to our extension function instead.
//
BOOL PASCAL FAR 
ExtDisconnectEx(
    IN SOCKET s,
    IN LPOVERLAPPED lpOverlapped,
    IN DWORD  dwFlags,
    IN DWORD  dwReserved
    )
{
    CLSPSocket*			 pSockObj  = NULL;
    int                  Errno = NO_ERROR,
                         ret = FALSE;

    // Get the context
    pSockObj=theInstance.GetSockFactoryInstance()->LookupSocketObj( s, &Errno );
    if ( NULL == pSockObj )
    {
        dbgprint( L"ExtDisconnectEx: LookupSocketObj failed!" );
        goto cleanup;
    }


        ret = pSockObj->ExtDisconnectEx(
                lpOverlapped,
                dwFlags,
                dwReserved,
				Errno
                );
  
cleanup:

    if ( NULL != pSockObj )
		pSockObj->Release();

    if ( FALSE == ret )
        WSASetLastError( Errno );

    return ret;
}

//
// Function: ExtWSARecvMsg
//
// Description:
//    This is our provider's WSARecvMsg function. When an app calls WSAIoctl
//    to request the function pointer to WSARecvMsg, we intercept the call
//    and return a pointer to our extension function instead.
//
INT PASCAL FAR 
ExtWSARecvMsg(
    IN SOCKET s,
    IN OUT LPWSAMSG lpMsg,
    OUT LPDWORD lpdwNumberOfBytesRecvd,
    IN LPWSAOVERLAPPED lpOverlapped,
    IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    )
{
    CLSPSocket*			 pSockObj  = NULL;
    int                  Errno = NO_ERROR,
                         ret = SOCKET_ERROR;

    // Get the context
    pSockObj = theInstance.GetSockFactoryInstance()->LookupSocketObj( s, &Errno );
    if ( NULL == pSockObj )
    {
        dbgprint( L"ExtWSARecvMsg: LookupSocketObj failed!" );
        goto cleanup;
    }

    ret = pSockObj->ExtWSARecvMsg(
                lpMsg,
                lpdwNumberOfBytesRecvd,
                lpOverlapped,
                lpCompletionRoutine,
				Errno
                );
     

cleanup:

    if ( NULL != pSockObj )
		pSockObj->Release();

    if ( SOCKET_ERROR == ret )
        WSASetLastError( Errno );

    return ret;
}

//
//Function: ExtWSASendMsg
//
// Description:
//    This is our provider's WSASendMsg function. When an app calls WSAIoctl
//    to request the function pointer to WSASendMsg, we intercept the call
//    and return a pointer to our extension function instead.
//
INT PASCAL FAR 
ExtWSASendMsg(
			  IN   SOCKET s,
			  IN   LPWSAMSG lpMsg,
			  IN   DWORD dwFlags,
			  OUT  LPDWORD lpNumberOfBytesSent,
			  IN   LPWSAOVERLAPPED lpOverlapped,
			  IN   LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
			)
{
   CLSPSocket*			 pSockObj  = NULL;
    int                  Errno = NO_ERROR,
                         ret = SOCKET_ERROR;

    // Get the context
    pSockObj = theInstance.GetSockFactoryInstance()->LookupSocketObj( s, &Errno );
    if ( NULL == pSockObj )
    {
        dbgprint( L"ExtWSASendMsg: LookupSocketObj failed!" );
        goto cleanup;
    }

    ret = pSockObj->ExtWSASendMsg(
                lpMsg,
				dwFlags,
                lpNumberOfBytesSent,
                lpOverlapped,
                lpCompletionRoutine,
				Errno
                );
     

cleanup:

    if ( NULL != pSockObj )
		pSockObj->Release();

    if ( SOCKET_ERROR == ret )
        WSASetLastError( Errno );

    return ret;
}