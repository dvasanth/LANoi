// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (C) 2004  Microsoft Corporation.  All Rights Reserved.
//
// Module Name: spi.cpp
//
// Description:
//
//    This sample illustrates how to develop a layered service provider that is
//    capable of counting all bytes transmitted through an IP socket. The application
//    reports when sockets are created and reports how many bytes were sent and
//    received when a socket closes. The results are reported using the OutputDebugString
//    API which will allow you to intercept the I/O by using a debugger such as cdb.exe
//    or you can monitor the I/O using dbmon.exe.
//
//    This file contains the 30 SPI functions you are required to implement in a
//    service provider. It also contains the two functions that must be exported
//    from the DLL module DllMain and WSPStartup.
//    
#include "stdafx.h"
#include "lspdef.h"

#include <stdio.h>
#include <stdlib.h>
#include "LSPModule.h"

#pragma warning(disable:4127)       // Disable "conditional expression is constant" warning

////////////////////////////////////////////////////////////////////////////////
//
// Globals used across files
//
////////////////////////////////////////////////////////////////////////////////

CRITICAL_SECTION    gCriticalSection,   // Critical section for initialization and socket list
                    gOverlappedCS;      // Critical section for overlapped manager
WSPUPCALLTABLE      gMainUpCallTable;   // Winsock upcall table
HINSTANCE           gDllInstance = NULL;// DLL instance handle
LPPROVIDER          gBaseInfo = NULL;   // Provider information for each layer under us
INT                 gLayerCount = 0;    // Number of base providers we're layered over
CLSPModule			theInstance;
COverlappedManager  gOverlapManager;
////////////////////////////////////////////////////////////////////////////////
//
// Macros and Function Prototypes
//
////////////////////////////////////////////////////////////////////////////////

// Close all open sockets and free any associated resources
void 
FreeSocketsAndMemory(
    BOOL processDetach,
    int *lpErrno
    );



////////////////////////////////////////////////////////////////////////////////
//
// Globals local to this file
//
////////////////////////////////////////////////////////////////////////////////

DWORD    gTlsIndex = 0xFFFFFFFF; // Index into thread local storage
DWORD    gEntryCount = 0;        // How many times WSPStartup has been called
DWORD    gLayerCatId = 0;        // Catalog ID of our dummy entry
WSPDATA  gWSPData;

// Fill out our proc table with our own LSP functions
WSPPROC_TABLE       gProcTable = {
    WSPAccept,
    WSPAddressToString,
    WSPAsyncSelect,
    WSPBind,
    WSPCancelBlockingCall,
    WSPCleanup,
    WSPCloseSocket,
    WSPConnect,
    WSPDuplicateSocket,
    WSPEnumNetworkEvents,
    WSPEventSelect,
    WSPGetOverlappedResult,
    WSPGetPeerName,
    WSPGetSockName,
    WSPGetSockOpt,
    WSPGetQOSByName,
    WSPIoctl,
    WSPJoinLeaf,
    WSPListen,
    WSPRecv,
    WSPRecvDisconnect,
    WSPRecvFrom,
    WSPSelect,
    WSPSend,
    WSPSendDisconnect,
    WSPSendTo,
    WSPSetSockOpt,
    WSPShutdown,
    WSPSocket,
    WSPStringToAddress
    };

////////////////////////////////////////////////////////////////////////////////
//
// Function Implementation
//
////////////////////////////////////////////////////////////////////////////////

//
// Function: PrintProcTable
//
// Description
//    Print the table of function pointers. This can be useful in tracking
//    down bugs with other LSP being layered over.
//
void 
PrintProcTable(
    LPWSPPROC_TABLE lpProcTable
    )
{
    #ifdef DBG_PRINTPROCTABLE
    dbgprint("WSPAccept              = 0x%X", lpProcTable->lpWSPAccept);
    dbgprint("WSPAddressToString     = 0x%X", lpProcTable->lpWSPAddressToString);
    dbgprint("WSPAsyncSelect         = 0x%X", lpProcTable->lpWSPAsyncSelect);
    dbgprint("WSPBind                = 0x%X", lpProcTable->lpWSPBind);
    dbgprint("WSPCancelBlockingCall  = 0x%X", lpProcTable->lpWSPCancelBlockingCall);
    dbgprint("WSPCleanup             = 0x%X", lpProcTable->lpWSPCleanup);
    dbgprint("WSPCloseSocket         = 0x%X", lpProcTable->lpWSPCloseSocket);
    dbgprint("WSPConnect             = 0x%X", lpProcTable->lpWSPConnect);
    dbgprint("WSPDuplicateSocket     = 0x%X", lpProcTable->lpWSPDuplicateSocket);
    dbgprint("WSPEnumNetworkEvents   = 0x%X", lpProcTable->lpWSPEnumNetworkEvents);
    dbgprint("WSPEventSelect         = 0x%X", lpProcTable->lpWSPEventSelect);
    dbgprint("WSPGetOverlappedResult = 0x%X", lpProcTable->lpWSPGetOverlappedResult);
    dbgprint("WSPGetPeerName         = 0x%X", lpProcTable->lpWSPGetPeerName);
    dbgprint("WSPGetSockOpt          = 0x%X", lpProcTable->lpWSPGetSockOpt);
    dbgprint("WSPGetSockName         = 0x%X", lpProcTable->lpWSPGetSockName);
    dbgprint("WSPGetQOSByName        = 0x%X", lpProcTable->lpWSPGetQOSByName);
    dbgprint("WSPIoctl               = 0x%X", lpProcTable->lpWSPIoctl);
    dbgprint("WSPJoinLeaf            = 0x%X", lpProcTable->lpWSPJoinLeaf);
    dbgprint("WSPListen              = 0x%X", lpProcTable->lpWSPListen);
    dbgprint("WSPRecv                = 0x%X", lpProcTable->lpWSPRecv);
    dbgprint("WSPRecvDisconnect      = 0x%X", lpProcTable->lpWSPRecvDisconnect);
    dbgprint("WSPRecvFrom            = 0x%X", lpProcTable->lpWSPRecvFrom);
    dbgprint("WSPSelect              = 0x%X", lpProcTable->lpWSPSelect);
    dbgprint("WSPSend                = 0x%X", lpProcTable->lpWSPSend);
    dbgprint("WSPSendDisconnect      = 0x%X", lpProcTable->lpWSPSendDisconnect);
    dbgprint("WSPSendTo              = 0x%X", lpProcTable->lpWSPSendTo);
    dbgprint("WSPSetSockOpt          = 0x%X", lpProcTable->lpWSPSetSockOpt);
    dbgprint("WSPShutdown            = 0x%X", lpProcTable->lpWSPShutdown);
    dbgprint("WSPSocket              = 0x%X", lpProcTable->lpWSPSocket);
    dbgprint("WSPStringToAddress     = 0x%X", lpProcTable->lpWSPStringToAddress);
    #else
    UNREFERENCED_PARAMETER( lpProcTable );  // For W4 compliance
    #endif
}

//
// Function: DllMain
//
// Description:
//    Provides initialization when the LSP DLL is loaded. In our case we simply,
//    initialize some critical sections used throughout the DLL.
//
BOOL WINAPI 
DllMain(
    IN HINSTANCE hinstDll, 
    IN DWORD dwReason, 
    LPVOID lpvReserved
    )
{

    switch (dwReason)
    {

        case DLL_PROCESS_ATTACH:
			if(!theInstance.OnInitInstance(hinstDll))
			{
				goto cleanup;
			}
            break;

        case DLL_THREAD_ATTACH:
            break;

        case DLL_THREAD_DETACH:
            break;

        case DLL_PROCESS_DETACH:
			theInstance.OnExitInstance(lpvReserved);
            break;
    }

    return TRUE;

cleanup:

    return FALSE;
}

//
// Function: WSPAccept
//
// Description:
//    Handle the WSAAccept function. The only special consideration here is the
//    conditional accept callback. You can choose to intercept this by substituting
//    your own callback (you'll need to keep track of the user supplied callback so
//    you can trigger that once your substituted function is triggered).
//
SOCKET WSPAPI 
WSPAccept(
    SOCKET          s,                      
    struct sockaddr FAR * addr,  
    LPINT           addrlen,                 
    LPCONDITIONPROC lpfnCondition,  
    DWORD_PTR       dwCallbackData,          
    LPINT           lpErrno
    )
{
    SOCKET				NewProviderSocket;
    SOCKET				NewSocket = INVALID_SOCKET;
    CLSPSocket			*NewSocketContext = NULL;
    CLSPSocket*			pSockObj  = NULL;

    //
    // Query for our per socket info
    //
    pSockObj = theInstance.GetSockFactoryInstance()->LookupSocketObj(s, lpErrno);
    if ( NULL == pSockObj )
    {
        dbgprint( _T("WSPAccept: LookupSocketObj failed!") );
        goto cleanup;
    }

    //
    // Note: You can subsitute your own conditional accept callback function
    //       in order to intercept this callback. You would have to keep track
    //       of the user's callback function so that you can call that when
    //       your intermediate function executes.
    //
    NewProviderSocket = pSockObj->WSPAccept(
											addr, 
											addrlen,
											lpfnCondition, 
											dwCallbackData, 
											lpErrno);
    if ( INVALID_SOCKET != NewProviderSocket )
    {
        // The underlying provider received a new connection so lets create our own
        //  socket to pass back up to the application.
        //
        NewSocketContext = theInstance.GetSockFactoryInstance()->CreateSocketObject(
																		NewProviderSocket,
																		pSockObj->GetNextProvider(),
																		pSockObj,
																		lpErrno
																	);
        if  ( NULL == NewSocketContext )
        {
            goto cleanup;
        }
		NewSocket= theInstance.GetSockFactoryInstance()->MapAndStoreSocketObject(NewSocketContext,lpErrno);

		if ( INVALID_SOCKET == NewSocket )
        {
			theInstance.GetSockFactoryInstance()->CloseProviderAndFreeLSPObject(NewSocketContext,lpErrno);
			goto cleanup;
		 }
   
    }

cleanup:

    if ( NULL != pSockObj )
		pSockObj->Release();

    return NewSocket;
}

//
// Function: WSPAdressToString
//
// Description:
//    Convert an address to string. We simply pass this to the lower provider.
//
int WSPAPI 
WSPAddressToString(
    LPSOCKADDR          lpsaAddress,            
    DWORD               dwAddressLength,               
    LPWSAPROTOCOL_INFOW lpProtocolInfo,   
    LPWSTR              lpszAddressString,            
    LPDWORD             lpdwAddressStringLength,   
    LPINT               lpErrno
    )
{
    WSAPROTOCOL_INFOW *pInfo=NULL;
    PROVIDER          *Provider=NULL;
    INT                ret = SOCKET_ERROR;

	Provider= theInstance.GetProviderInstance()->GetProvider( lpProtocolInfo,lpErrno);

	if(Provider==NULL) goto cleanup;

    pInfo = &Provider->NextProvider;

   	//
    // If we're not immediately above the base then pass the lpProtocolInfo passed
    // into us.
    //
    if ( BASE_PROTOCOL != pInfo->ProtocolChain.ChainLen )
    {
        pInfo = lpProtocolInfo;
    }

     ASSERT( Provider->NextProcTable.lpWSPAddressToString );

    SetBlockingProvider(Provider);
    ret = Provider->NextProcTable.lpWSPAddressToString(
            lpsaAddress, 
            dwAddressLength,               
            pInfo, 
            lpszAddressString, 
            lpdwAddressStringLength, 
            lpErrno
            );
    SetBlockingProvider(NULL);

cleanup:

    return ret;
}

//
// Function: WSPAsyncSelect
//
// Description:
//    Register specific Winsock events with a socket. We need to substitute
//    the app socket with the provider socket and use our own hidden window.
//
int WSPAPI 
WSPAsyncSelect(
    SOCKET       s,
    HWND         hWnd,
    unsigned int wMsg,
    long         lEvent,
    LPINT        lpErrno
    )
{
    CLSPSocket*			pSockObj  = NULL;
    INT					ret = SOCKET_ERROR;

    //
    // Make sure the window handle is valid
    //
    ret = SOCKET_ERROR;
    if ( FALSE == IsWindow( hWnd ) )
    {
        *lpErrno = WSAEINVAL;
        goto cleanup;
    }

    //
    // Verify only valid events have been set
    //
    if ( 0 != (lEvent & ~FD_ALL_EVENTS) )
    {
        *lpErrno = WSAEINVAL;
        goto cleanup;
    }

    //
    // Find our provider socket corresponding to this one
    //
    pSockObj=theInstance.GetSockFactoryInstance()->LookupSocketObj(s,lpErrno);
    if ( NULL == pSockObj )
    {
        dbgprint( _T("WSPAsyncSelect: FindAndRefSocketContext failed!" ));
        goto cleanup;
    }

    ret =pSockObj->WSPAsyncSelect(
								hWnd, 
								wMsg, 
								lEvent, 
								lpErrno
								);

cleanup:

	if ( NULL != pSockObj )
		pSockObj->Release();

    return ret;
}

//
// Function: WSPBind
//
// Description:
//    Bind the socket to a local address. We just map socket handles and
//    call the lower provider.
//
int WSPAPI 
WSPBind(
    SOCKET                s,
    const struct sockaddr FAR * name,
    int                   namelen,
    LPINT                 lpErrno
    )
{
    CLSPSocket*			pSockObj  = NULL;
    INT					ret = SOCKET_ERROR;

    //
    // Find our provider socket corresponding to this one
    //
    pSockObj=theInstance.GetSockFactoryInstance()->LookupSocketObj(s, lpErrno);
    if ( NULL == pSockObj )
    {
        dbgprint( _T("WSPBind: LookupSocketObj failed!" ));
        goto cleanup;
    }

    ret = pSockObj->WSPBind(
            name, 
            namelen, 
            lpErrno
            );

cleanup:

    if ( NULL != pSockObj )
		pSockObj->Release();

    return ret;
}

//
// Function: WSPCancelBlockingCall
//
// Description:
//    This call cancels any blocking Winsock call in the current thread only.
//    For every Winsock call that blocks we use thread local storage (TLS) to
//    store a pointer to the provider on which the blocking call was issued.
//    This is necessary since WSACancelBlockingCall takes no arguments (i.e.
//    the LSP needs to keep track of what calls are blocking).
//
int WSPAPI 
WSPCancelBlockingCall(
    LPINT lpErrno
    )
{
    PROVIDER *Provider = NULL;
    INT       ret = NO_ERROR;

    Provider = (PROVIDER *) TlsGetValue( gTlsIndex );
    if ( NULL != Provider )
    {
        ASSERT( Provider->NextProcTable.lpWSPCancelBlockingCall );

        ret = Provider->NextProcTable.lpWSPCancelBlockingCall(lpErrno);
    }
    return ret;
}

// 
// Function: WSPCleanup
//
// Description:
//    Decrement the entry count. If equal to zero then we can prepare to have us
//    unloaded. Close any outstanding sockets and free up allocated memory.
//
int WSPAPI 
WSPCleanup(
    LPINT lpErrno  
    )
{
    int        ret = SOCKET_ERROR;

    if ( theInstance.IsProcessDetached())
    {
        dbgprint(_T("WSPCleanup: DLL has already been unloaded from process!"));
        ret = NO_ERROR;
        return ret;
    }

   ret = theInstance.GetProviderInstance()->DeInitialize(lpErrno); 

    return ret;
}

//
// Function: WSPCloseSocket
//
// Description:
//    Close the socket handle of the app socket as well as the provider socket.
//    However, if there are outstanding async IO requests on the app socket
//    we only close the provider socket. Only when all the IO requests complete
//    (with error) will we then close the app socket (this will occur in
//    the overlapped manager - overlapp.cpp).
//
int WSPAPI 
WSPCloseSocket(  
    SOCKET s,        
    LPINT  lpErrno
    )
{
    CLSPSocket*			pSockObj  = NULL;
    int					ret = SOCKET_ERROR;
	BOOL				bObjectDeleted=FALSE;

    //
    // Find our provider socket corresponding to this one
    //
    pSockObj=theInstance.GetSockFactoryInstance()->LookupSocketObj(s, lpErrno);
    if ( NULL == pSockObj )
    {
        dbgprint(_T( "WSPCloseSocket: LookupSocketObj failed!" ));
        goto cleanup;
    }

	ret=pSockObj->WSPCloseSocket(lpErrno, bObjectDeleted);


cleanup:
    
    if ( !bObjectDeleted && pSockObj)
    {
		pSockObj->Release(); 
    }

    return ret;
}

//
// Function: WSPConnect
//
// Description:
//    Performs a connect call. The only thing we need to do is translate
//    the socket handle.
//
int WSPAPI 
WSPConnect(
    SOCKET                s,
    const struct sockaddr FAR * name,
    int                   namelen,
    LPWSABUF              lpCallerData,
    LPWSABUF              lpCalleeData,
    LPQOS                 lpSQOS,
    LPQOS                 lpGQOS,
    LPINT                 lpErrno
    )
{
    CLSPSocket*			pSockObj  = NULL;
    INT					ret = SOCKET_ERROR;

    //
    // Find our provider socket corresponding to this one
    //
    pSockObj=theInstance.GetSockFactoryInstance()->LookupSocketObj(s, lpErrno);
    if ( NULL == pSockObj )
    {
        dbgprint( _T("WSPConnect: LookupSocketObj failed!" ));
        goto cleanup;
    }

    ret = pSockObj->WSPConnect(
								name, 
								namelen, 
								lpCallerData, 
								lpCalleeData,
								lpSQOS, 
								lpGQOS, 
								lpErrno
								);

cleanup:

   if ( NULL != pSockObj )
		pSockObj->Release();

    return ret;
}

//
// Function: WSPDuplicateSocket
//
// Description:
//    This function provides a WSAPROTOCOL_INFOW structure which can be passed
//    to another process to open a handle to the same socket. First we need
//    to translate the user socket into the provider socket and call the underlying
//    WSPDuplicateSocket. Note that the lpProtocolInfo structure passed into us
//    is an out parameter only!
//
int WSPAPI 
WSPDuplicateSocket(
    SOCKET              s,
    DWORD               dwProcessId,                      
    LPWSAPROTOCOL_INFOW lpProtocolInfo,   
    LPINT               lpErrno
    )
{
    CLSPSocket*			pSockObj  = NULL;
    int                ret = SOCKET_ERROR;
    //
    // Find our provider socket corresponding to this one
    //
    pSockObj=theInstance.GetSockFactoryInstance()->LookupSocketObj(s, lpErrno);
    if ( NULL == pSockObj )
    {
        dbgprint( _T("WSPDuplicateSocket: LookupSocketObj failed!" ));
        goto cleanup;
    }
    //
    // Find the underlying provider
    //
    ret = pSockObj->WSPDuplicateSocket(
										dwProcessId,
										lpProtocolInfo,
										lpErrno
										);

  

cleanup:

    if ( NULL != pSockObj )
		pSockObj->Release();

    return ret;    
}

//
// Function: WSPEnumNetworkEvents
//
// Description:
//    Enumerate the network events for a socket. We only need to translate the
//    socket handle.
//
int WSPAPI 
WSPEnumNetworkEvents(  
    SOCKET             s,
    WSAEVENT           hEventObject,
    LPWSANETWORKEVENTS lpNetworkEvents,
    LPINT              lpErrno
    )
{
    CLSPSocket*			pSockObj  = NULL;
    INT					ret = SOCKET_ERROR;

    //
    // Find our provider socket corresponding to this one
    //
    pSockObj=theInstance.GetSockFactoryInstance()->LookupSocketObj(s, lpErrno);
    if ( NULL == pSockObj )
    {
        dbgprint(_T( "WSPEnumNetworkEvents: LookupSocketObj failed!") );
        goto cleanup;
    }


    ret = pSockObj->WSPEnumNetworkEvents(
										hEventObject, 
										lpNetworkEvents, 
										lpErrno
										);

cleanup:


    if ( NULL != pSockObj )
		pSockObj->Release();

    return ret;
}

//
// Function: WSPEventSelect
//
// Description:
//    Register the specified events on the socket with the given event handle.
//    All we need to do is translate the socket handle.
//
int WSPAPI 
WSPEventSelect(
    SOCKET   s,
    WSAEVENT hEventObject,
    long     lNetworkEvents,
    LPINT    lpErrno
    )
{
    CLSPSocket*			pSockObj  = NULL;
    INT					ret = SOCKET_ERROR;

    //
    // Find our provider socket corresponding to this one
    //
    pSockObj = theInstance.GetSockFactoryInstance()->LookupSocketObj(s, lpErrno);
    if ( NULL == pSockObj )
    {
        dbgprint(_T( "WSPEventSelect: LookupSocketObj failed!" ));
        goto cleanup;
    }
    

    ret = pSockObj->WSPEventSelect(
            hEventObject,
            lNetworkEvents, 
            lpErrno
            );

cleanup:


    if ( NULL != pSockObj )
		pSockObj->Release();

    return ret;
}

//
// Function: WSPGetOverlappedResult
//
// Description:
//    This function reports whether the specified overlapped call has
//    completed. If it has, return the requested information. If not,
//    and fWait is true, wait until completion. Otherwise return an
//    error immediately.
//
BOOL WSPAPI 
WSPGetOverlappedResult(
    SOCKET          s,
    LPWSAOVERLAPPED lpOverlapped,
    LPDWORD         lpcbTransfer,
    BOOL            fWait,
    LPDWORD         lpdwFlags,
    LPINT           lpErrno
    )
{
    DWORD ret = FALSE;

    UNREFERENCED_PARAMETER( s );

    __try
    {
        if ( WSS_OPERATION_IN_PROGRESS != lpOverlapped->Internal ) 
        {
            // Operation has completed, update the parameters and return 
            //
            *lpcbTransfer = (DWORD)lpOverlapped->InternalHigh;
            *lpdwFlags = (DWORD)lpOverlapped->Offset;
            *lpErrno = (INT)lpOverlapped->OffsetHigh;

            ret = (lpOverlapped->OffsetHigh == 0 ? TRUE : FALSE);
        }
        else if ( FALSE != fWait )
        {
            //
            // Operation is still in progress so wait until it completes
            //

            //
            // Wait on the app supplied event handle. Once the operation
            //  is completed the IOCP or completion routine will fire.
            //  Once that is handled, WPUCompleteOverlappedRequest will
            //  be called which will signal the app event.
            //
            ret = WaitForSingleObject(lpOverlapped->hEvent, INFINITE);
            if ( ( WAIT_OBJECT_0 == ret ) &&
                    ( WSS_OPERATION_IN_PROGRESS != lpOverlapped->Internal ) )
            {
                *lpcbTransfer = (DWORD)lpOverlapped->InternalHigh;
                *lpdwFlags = (DWORD)lpOverlapped->Offset;
                *lpErrno = (INT)lpOverlapped->OffsetHigh;

                ret = (lpOverlapped->OffsetHigh == 0 ? TRUE : FALSE);
            }
            else if ( WSS_OPERATION_IN_PROGRESS == lpOverlapped->Internal )
            {
                *lpErrno = WSA_IO_PENDING;
            }
            else 
            {
                *lpErrno = GetLastError();
            }
        }
        else 
        {
            // Operation is in progress and we aren't waiting
            *lpErrno = WSA_IO_INCOMPLETE;
        }
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
        *lpErrno = WSAEFAULT;
    }

    return ret;
}

//
// Function: WSPGetPeerName
//
// Description:
//    Returns the address of the peer. The only thing we need to do is translate
//    the socket handle.
//
int WSPAPI 
WSPGetPeerName(  
    SOCKET          s,
    struct sockaddr FAR * name,
    LPINT           namelen,
    LPINT           lpErrno
    )
{
    CLSPSocket*			pSockObj  = NULL;
    INT					ret = SOCKET_ERROR;

    //
    // Find our provider socket corresponding to this one
    //
    pSockObj = theInstance.GetSockFactoryInstance()->LookupSocketObj(s, lpErrno);
    if ( NULL == pSockObj )
    {
        dbgprint(_T( "WSPGetPeerName: FindAndRefSocketContext failed!") );
        goto cleanup;
    }


    ret = pSockObj->WSPGetPeerName(
									name,
									namelen, 
									lpErrno);

cleanup:

  if ( NULL != pSockObj )
		pSockObj->Release();

    return ret;
}

//
// Function: WSPGetSockName
//
// Description:
//    Returns the local address of a socket. All we need to do is translate
//    the socket handle.
//
int WSPAPI 
WSPGetSockName(
    SOCKET          s,
    struct sockaddr FAR * name,
    LPINT           namelen,
    LPINT           lpErrno
    )
{
    CLSPSocket*			pSockObj  = NULL;
    INT					ret = SOCKET_ERROR;

    //
    // Find our provider socket corresponding to this one
    //
    pSockObj=theInstance.GetSockFactoryInstance()->LookupSocketObj(s, lpErrno);
    if ( NULL == pSockObj )
    {
        dbgprint(_T( "WSPGetSockName: LookupSocketObj failed!") );
        goto cleanup;
    }
    ret = pSockObj->WSPGetSockName(
            name,
            namelen, 
            lpErrno
            );

cleanup:

    if ( NULL != pSockObj )
		pSockObj->Release(); 

    return ret;
}

//
// Function: WSPGetSockOpt
//
// Description:
//    Get the specified socket option. All we need to do is translate the
//    socket handle.
//
int WSPAPI 
WSPGetSockOpt(
    SOCKET     s,
    int        level,
    int        optname,
    char FAR * optval,
    LPINT      optlen,
    LPINT      lpErrno
    )
{
    CLSPSocket*			pSockObj  = NULL;
    INT					ret = NO_ERROR;


    //
    // Find our provider socket corresponding to this one
    //
    pSockObj=theInstance.GetSockFactoryInstance()->LookupSocketObj(s, lpErrno);
    if ( NULL == pSockObj )
    {
        dbgprint(_T( "WSPGetSockOpt: LookupSocketObj failed!") );
        goto cleanup;
    }


    __try
    {
        //
        // We need to capture this and return our own WSAPROTOCOL_INFO structure.
        // Otherwise, if we translate the handle and pass it to the lower provider
        // we'll return the lower provider's protocol info!
        //
        if ( ( SOL_SOCKET == level ) && ( ( SO_PROTOCOL_INFOA == optname ) ||
                                          ( SO_PROTOCOL_INFOW == optname ) )
           )
        {
            if ( ( SO_PROTOCOL_INFOW == optname ) && 
                 ( sizeof( WSAPROTOCOL_INFOW ) <= *optlen )
               )
            {

                    // No conversion necessary, just copy the data
                    memcpy(optval, 
                           &pSockObj->GetNextProvider()->LayerProvider, 
                           sizeof(WSAPROTOCOL_INFOW));
       
            }
            else if ( ( SO_PROTOCOL_INFOA == optname ) && 
                      ( sizeof( WSAPROTOCOL_INFOA ) <= *optlen )
                    )
            {
             
                // Copy everything but the string
                memcpy(optval,
                       &pSockObj->GetNextProvider()->LayerProvider,
                       sizeof(WSAPROTOCOL_INFOW)-WSAPROTOCOL_LEN+1);
                // Convert our saved UNICODE string to ANSII
                WideCharToMultiByte(
                        CP_ACP,
                        0,
                        pSockObj->GetNextProvider()->LayerProvider.szProtocol,
                        -1,
                        ((WSAPROTOCOL_INFOA *)optval)->szProtocol,
                        WSAPROTOCOL_LEN+1,
                        NULL,
                        NULL
                        );

     
            }
            else
            {
                ret = SOCKET_ERROR;
                *lpErrno = WSAEFAULT;
                goto cleanup;
            }

            goto cleanup;
        }
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
        ret = SOCKET_ERROR;
        *lpErrno = WSAEFAULT;
        goto cleanup;
    }       
   
    ret = pSockObj->WSPGetSockOpt(
    							level,
								optname, 
								optval, 
								optlen, 
								lpErrno);


cleanup:

    if ( NULL != pSockObj   )
		pSockObj->Release(); 

    return ret;
}

//
// Function: WSPGetQOSByName
//
// Description:
//    Get a QOS template by name. All we need to do is translate the socket
//    handle.
//
BOOL WSPAPI 
WSPGetQOSByName(
    SOCKET   s,
    LPWSABUF lpQOSName,
    LPQOS    lpQOS,
    LPINT    lpErrno
    )
{
    CLSPSocket*			pSockObj  = NULL;
    INT					ret = SOCKET_ERROR;

    //
    // Find our provider socket corresponding to this one
    //
    pSockObj=theInstance.GetSockFactoryInstance()->LookupSocketObj(s, lpErrno);
    if ( NULL == pSockObj )
    {
        dbgprint( _T("WSPGetQOSByName: LookupSocketObj failed!") );
        goto cleanup;
    }

    ret = pSockObj->WSPGetQOSByName(
									lpQOSName,
									lpQOS, 
									lpErrno
									);

cleanup:

    if ( NULL != pSockObj )
		pSockObj->Release(); 

    return ret;
}

//
// Function: WSPIoctl
//
// Description:
//    Invoke an ioctl. In most cases, we just need to translate the socket
//    handle. However, if the dwIoControlCode is SIO_GET_EXTENSION_FUNCTION_POINTER,
//    we'll need to intercept this and return our own function pointers when
//    they're requesting either TransmitFile or AcceptEx. This is necessary so
//    we can trap these calls. Also for PnP OS's (Win2k) we need to trap calls
//    to SIO_QUERY_TARGET_PNP_HANDLE. For this ioctl we simply have to return 
//    the provider socket.
//
int WSPAPI 
WSPIoctl(
    SOCKET          s,
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
    CLSPSocket*			pSockObj  = NULL;
    int                 ret = NO_ERROR;


    *lpErrno = NO_ERROR;

    //
    // Find our provider socket corresponding to this one
    //
    pSockObj = theInstance.GetSockFactoryInstance()->LookupSocketObj(s, lpErrno);
    if ( NULL == pSockObj )
    {
        dbgprint(_T( "WSPIoctl: LookupSocketObj failed!") );
        goto cleanup;
    }

	ret =pSockObj->WSPIoctl(
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
 

cleanup:

    if ( NULL != pSockObj )
		pSockObj->Release(); 

    return ret;
}

//
// Function: WSPJoinLeaf
//
// Description:
//    This function joins a socket to a multipoint session. For those providers
//    that support multipoint semantics there are 2 possible behaviors. First,
//    for IP, WSAJoinLeaf always returns the same socket handle which was passed
//    into it. In this case there is no new socket so we don't want to create
//    any socket context once the lower provider WSPJoinLeaf is called. In the
//    second case, for ATM, a new socket IS created when we call the lower
//    provider. In this case we do want to create a new user socket and create
//    a socket context.
//
SOCKET WSPAPI 
WSPJoinLeaf(
    SOCKET       s,
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
	CLSPSocket*			pSockObj  = NULL;
    SOCKET     NextProviderSocket = INVALID_SOCKET,
               NewSocket = INVALID_SOCKET;

    //
    // Find our provider socket corresponding to this one
    //
    pSockObj = theInstance.GetSockFactoryInstance()->LookupSocketObj(s, lpErrno);
    if ( NULL == pSockObj )
    {
        dbgprint(_T( "WSPJoinLeaf: LookupSocketObj failed!") );
        goto cleanup;
    }

     NextProviderSocket=pSockObj->WSPJoinLeaf(
							name, 
							namelen, 
							lpCallerData, 
							lpCalleeData, 
							lpSQOS, 
							lpGQOS, 
							dwFlags,                        
							lpErrno
							);


    //    
    // If the socket returned from the lower provider is the same as the socket
    //  passed into it then there really isn't a new socket - just return. 
    //  Otherwise, a new socket has been created and we need to create the socket
    //  context and create a user socket to pass back.
    //
    if ( ( INVALID_SOCKET != NextProviderSocket ) && 
         ( NextProviderSocket != pSockObj->GetProviderSocket() )
       )
    {
        CLSPSocket   *NewSocketContext = NULL;

        //
        // Create socket context for new socket
        //
        NewSocketContext = theInstance.GetSockFactoryInstance()->CreateSocketObject(
														NextProviderSocket,
														pSockObj->GetNextProvider(),
														pSockObj,
														lpErrno
														);
        if  ( NULL == NewSocketContext )
        {
            goto cleanup;
        }

        //
        // Create a socket handle to pass to the app
        //
		NewSocket=theInstance.GetSockFactoryInstance()->MapAndStoreSocketObject(NewSocketContext,lpErrno);
        if ( INVALID_SOCKET == NewSocket )
        {
           // Context is not in the list yet so we can just free it
            theInstance.GetSockFactoryInstance()->CloseProviderAndFreeLSPObject(NewSocketContext,lpErrno);
            goto cleanup;
        }
    }
    else if ( NextProviderSocket == pSockObj->GetProviderSocket() )
    {
        NewSocket = s;
    }

cleanup:

    if ( NULL != pSockObj )
		pSockObj->Release();

    return NewSocket;
}

//
// Function: WSPListen
//
// Description:
//    This function sets the backlog value on a listening socket. All we need to
//    do is translate the socket handle to the correct provider.
//
int WSPAPI 
WSPListen(
    SOCKET s,        
    int    backlog,     
    LPINT  lpErrno
    )
{
    CLSPSocket*			pSockObj  = NULL;
    INT					ret = SOCKET_ERROR;

    //
    // Find our provider socket corresponding to this one
    //
    pSockObj = theInstance.GetSockFactoryInstance()->LookupSocketObj(s, lpErrno);
    if ( NULL == pSockObj )
    {
        dbgprint(_T( "WSPListen: LookupSocketObj failed!" ));
        goto cleanup;
    }
	ret=pSockObj->WSPListen(backlog,lpErrno);
 
cleanup:

    if ( NULL != pSockObj )
		pSockObj->Release();

    return ret;
}

//
// Function: WSPRecv
//
// Description:
//    This function receives data on a given socket and also allows for asynchronous
//    (overlapped) operation. First translate the socket handle to the lower provider
//    handle and then make the receive call. If called with overlap, post the operation
//    to our IOCP or completion routine.
//
int WSPAPI 
WSPRecv(
    SOCKET          s,
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

    CLSPSocket*			pSockObj  = NULL;
    int                 ret = SOCKET_ERROR;


    *lpErrno = NO_ERROR;

    //
    // Find our provider socket corresponding to this one
    //
    pSockObj = theInstance.GetSockFactoryInstance()->LookupSocketObj(s, lpErrno);
    if ( NULL == pSockObj )
    {
        dbgprint( _T("WSPRecv: LookupSocketObj failed!") );
        goto cleanup;
    }

    ret = pSockObj->WSPRecv(
							lpBuffers, 
							dwBufferCount,
							lpNumberOfBytesRecvd, 
							lpFlags, 
							lpOverlapped, 
							lpCompletionRoutine, 
							lpThreadId,
							lpErrno);

 
cleanup:

    if ( NULL != pSockObj )
		pSockObj->Release(); 

    return ret;
}

//
// Function: WSPRecvDisconnect
//
// Description:
//    Receive data and disconnect. All we need to do is translate the socket
//    handle to the lower provider.
//
int WSPAPI 
WSPRecvDisconnect(
    SOCKET   s,
    LPWSABUF lpInboundDisconnectData,
    LPINT    lpErrno
    )
{
    CLSPSocket*			pSockObj  = NULL;
    INT					ret = SOCKET_ERROR;

    //
    // Find our provider socket corresponding to this one
    //
    pSockObj = theInstance.GetSockFactoryInstance()->LookupSocketObj(s, lpErrno);
    if ( NULL == pSockObj )
    {
        dbgprint( _T("WSPRecvDisconnect: FindAndRefSocketContext failed!") );
        goto cleanup;
    }

	ret=pSockObj->WSPRecvDisconnect(lpInboundDisconnectData,lpErrno);

cleanup:

    if ( NULL != pSockObj )
		pSockObj->Release(); 

    return ret;
}

//
// Function: WSPRecvFrom
//
// Description:
//    This function receives data on a given socket and also allows for asynchronous
//    (overlapped) operation. First translate the socket handle to the lower provider
//    handle and then make the receive call. If called with overlap, post the operation
//    to our IOCP or completion routine.
//
int WSPAPI 
WSPRecvFrom(
    SOCKET          s,
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
    CLSPSocket*			pSockObj  = NULL;
    int                 ret = SOCKET_ERROR;

    *lpErrno = NO_ERROR;

    //
    // Find our provider socket corresponding to this one
    //
    pSockObj = theInstance.GetSockFactoryInstance()->LookupSocketObj(s, lpErrno);
    if ( NULL == pSockObj )
    {
        dbgprint(_T( "WSPRecvFrom: LookupSocketObj failed!") );
        goto cleanup;
    }

	ret=pSockObj->WSPRecvFrom(
						  lpBuffers,
						  dwBufferCount,
						  lpNumberOfBytesRecvd,
						  lpFlags,
						  lpFrom,
						  lpFromLen,
						  lpOverlapped,
						  lpCompletionRoutine,
						  lpThreadId,
						  lpErrno
						  );


cleanup:

    if ( NULL != pSockObj )
		pSockObj->Release();

    return ret;
}

//
// Function: UnlockFdSets
//
// Description:
//      This is a utility function which iterates through the non-NULL, non-empty
//      fd_sets and its corresponding FD_MAP array. For each socket present in
//      the fd_set the corresponding socket context object is unlocked. This needs
//      to be performed before the application's fd_sets are modified otherwise
//      the mapping between fd_set and FD_MAP can get out of sync.
//
void
UnlockFdSets(
    fd_set  *readfds,
    FD_MAP  *readmap,
    fd_set  *writefds,
    FD_MAP  *writemap,
    fd_set  *exceptfds,
    FD_MAP  *exceptmap,
    LPINT    lpErrno
    )
{
    int     i;

    // Unlock socket contexts for the readfds sockets
    if ( NULL != readfds )
    {
        for(i=0; i < (int)readfds->fd_count ;i++)
        {
            if ( NULL != readmap[i].Context )
            {
				readmap[i].Context->Release(); 
                readmap[i].Context = NULL;
            }
        }
    }

    // Unlock socket contexts for the writefds sockets
    if ( NULL != writefds )
    {
        for(i=0; i < (int)writefds->fd_count ;i++)
        {
            if ( NULL != writemap[i].Context )
            {
				writemap[i].Context->Release(); 
                writemap[i].Context = NULL;
            }
        }
    }

    // Unlock socket contexts for the except sockets
    if ( NULL != exceptfds )
    {
        for(i=0; i < (int)exceptfds->fd_count ;i++)
        {
            if ( NULL != exceptmap[i].Context )
            {
				exceptmap[i].Context->Release(); 
                exceptmap[i].Context = NULL;
            }
        }
    }
}

//
// Function: WSPSelect
//
// Description:
//    This function tests a set of sockets for readability, writeability, and
//    exceptions. We must translate each handle in the fd_set structures to
//    their underlying provider handles before calling the next provider's
//    WSPSelect. The select API is really bad for LSPs in the sense multiple
//    provider's sockets can be passed into this provider's WSPSelect call.
//    If these unknown sockets (unknown since this LSP won't have a socket
//    context structure for it) are passed in the best we can do is pass
//    it down unmodified to the lower layer in the hopes that it knows what it
//    is. In the case where these unknown sockets belong to another LSP (which
//    isn't in our immediate chain) then the lower provider won't know about
//    that socket and will fail the call. Lastly we hold the context lock on
//    all sockets passed in until we're done.
//
int WSPAPI 
WSPSelect(
    int          nfds,
    fd_set FAR * readfds,
    fd_set FAR * writefds,
    fd_set FAR * exceptfds,
    const struct timeval FAR * timeout,
    LPINT        lpErrno
    )
{
    CLSPSocket*	pSockObj  = NULL;
    u_int      count,
               i;
    BOOL       unlocked = FALSE;
    int        HandleCount,
               ret SOCKET_ERROR;

    fd_set     ReadFds, 
               WriteFds, 
               ExceptFds;

    FD_MAP    *Read = NULL, 
              *Write = NULL, 
              *Except = NULL;

    if ( ( NULL == readfds ) && ( NULL == writefds ) && ( NULL == exceptfds ) )
    {
        *lpErrno = WSAEINVAL;
        goto cleanup;
    }

    // Allocate storage to map upper level sockets to lower provider's sockets
    if ( NULL != readfds )
    {
        Read = (FD_MAP *) LspAlloc( sizeof( FD_MAP ) * readfds->fd_count, lpErrno );
        if ( NULL == Read )
            goto cleanup;
    }

    if ( NULL != writefds )
    {
        Write = (FD_MAP *) LspAlloc( sizeof( FD_MAP ) * writefds->fd_count, lpErrno );
        if ( NULL == Write )
            goto cleanup;
    }

    if ( NULL != exceptfds )
    {
        Except = (FD_MAP *) LspAlloc( sizeof( FD_MAP ) * exceptfds->fd_count, lpErrno );
        if ( NULL == Except )
            goto cleanup;
    }

    //
    // Translate all handles contained in the fd_set structures.
    //  For each fd_set go through and build another fd_set which contains
    //  their lower provider socket handles.
    //

    // Map the upper layer sockets to lower layer sockets in the write array
    if ( NULL != readfds )
    {
        FD_ZERO( &ReadFds );

        if ( readfds->fd_count > FD_SETSIZE )
        {
            *lpErrno = WSAENOBUFS;
            goto cleanup;
        }

        for (i = 0; i < readfds->fd_count; i++)
        {
            Read[i].Context = theInstance.GetSockFactoryInstance()->LookupSocketObj(
                    (Read[i].ClientSocket = readfds->fd_array[i]),
                    lpErrno
                    );
            if ( NULL == Read[i].Context )
            {
                // This socket isn't ours -- just pass down in hopes the lower provider
                // knows about it
                Read[i].ProvSocket = readfds->fd_array[i];
                FD_SET(Read[i].ProvSocket, &ReadFds);
            }
            else
            {
                Read[i].ProvSocket = Read[i].Context->GetProviderSocket();
                FD_SET(Read[i].ProvSocket, &ReadFds);

                // Save the first valid socket context structure
                if ( NULL == pSockObj )
                    pSockObj = Read[i].Context;
            }
        }
    }

    // Map the upper layer sockets to lower layer sockets in the write array
    if ( NULL != writefds )
    {
        FD_ZERO( &WriteFds );

        if ( writefds->fd_count > FD_SETSIZE )
        {
            *lpErrno = WSAENOBUFS;
            goto cleanup;
        }
        for (i = 0; i < writefds->fd_count; i++)
        {
            Write[i].Context = theInstance.GetSockFactoryInstance()->LookupSocketObj(
                    (Write[i].ClientSocket = writefds->fd_array[i]), 
                    lpErrno
                    );
            if ( NULL == Write[i].Context )
            {
                // This socket isn't ours -- just pass down in hopes the lower provider
                // knows about it
                Write[i].ProvSocket = writefds->fd_array[i];
                FD_SET(Write[i].ProvSocket, &WriteFds);
            }
            else
            {
                Write[i].ProvSocket = Write[i].Context->GetProviderSocket();
                FD_SET(Write[i].ProvSocket, &WriteFds);

                // Save the first valid socket context structure
                if ( NULL == pSockObj )
                    pSockObj = Write[i].Context;
            }
        }
    }

    // Map the upper layer sockets to lower layer sockets in the except array
    if ( NULL != exceptfds )
    {
        FD_ZERO( &ExceptFds );

        if (exceptfds->fd_count > FD_SETSIZE)
        {
            *lpErrno = WSAENOBUFS;
            goto cleanup;
        }
        for (i = 0; i < exceptfds->fd_count; i++)
        {
            Except[i].Context = theInstance.GetSockFactoryInstance()->LookupSocketObj(
                    (Except[i].ClientSocket = exceptfds->fd_array[i]), 
                    lpErrno
                    );
            if ( NULL == Except[i].Context )
            {
                // This socket isn't ours -- just pass down in hopes the lower provider
                // knows about it
                Except[i].ProvSocket = exceptfds->fd_array[i];
                FD_SET(Except[i].ProvSocket, &ExceptFds);
            }
            else
            {
                Except[i].ProvSocket = Except[i].Context->GetProviderSocket();
                FD_SET(Except[i].ProvSocket, &ExceptFds);

                // Save the first valid socket context structure
                if ( NULL == pSockObj )
                    pSockObj = Except[i].Context;
            }
        }
    }

    //
    // Now call the lower provider's WSPSelect with the fd_set structures we built
    //  containing the lower provider's socket handles.
    //
    if ( NULL == pSockObj )
    {
        *lpErrno = WSAEINVAL;
        goto cleanup;
    }

    ret = pSockObj->WSPSelect(
            nfds, 
            (readfds ? &ReadFds : NULL), 
            (writefds ? &WriteFds : NULL), 
            (exceptfds ? &ExceptFds : NULL), 
            timeout, 
            lpErrno
            );

    // Need to unlock the contexts before the original fd_sets are modified
    UnlockFdSets( readfds, Read, writefds, Write, exceptfds, Except, lpErrno );
    unlocked = TRUE;

    if ( SOCKET_ERROR != ret )
    {
        // Once we complete we now have to go through the fd_sets we passed and
        //  map them BACK to the application socket handles. Fun!
        //
        HandleCount = ret;

        if ( NULL != readfds )
        {
            count = readfds->fd_count;
            FD_ZERO(readfds);

            for(i = 0; (i < count) && HandleCount; i++)
            {
                if ( gMainUpCallTable.lpWPUFDIsSet(Read[i].ProvSocket, &ReadFds) )
                {
                    FD_SET(Read[i].ClientSocket, readfds);
                    HandleCount--;
                }
            }
        }

        if ( NULL != writefds )
        {
            count = writefds->fd_count;
            FD_ZERO(writefds);

            for(i = 0; (i < count) && HandleCount; i++)
            {
                if ( gMainUpCallTable.lpWPUFDIsSet(Write[i].ProvSocket, &WriteFds) )
                {
                    FD_SET(Write[i].ClientSocket, writefds);
                    HandleCount--;
                }
            }
        }

        if ( NULL != exceptfds )
        {
            count = exceptfds->fd_count;
            FD_ZERO(exceptfds);

            for(i = 0; (i < count) && HandleCount; i++)
            {
                if ( gMainUpCallTable.lpWPUFDIsSet(Except[i].ProvSocket, &ExceptFds) )
                {
                    FD_SET(Except[i].ClientSocket, exceptfds);
                    HandleCount--;
                }
            }
        }
    }

cleanup:

    // In case of error, ensure the socket contexts get unlocked
    if ( FALSE == unlocked )
        UnlockFdSets( readfds, Read, writefds, Write, exceptfds, Except, lpErrno );

    // Unlock socket context here in case an error occurs
    if ( NULL != Read )
        LspFree( Read );

    if ( NULL != Write )
        LspFree( Write );

    if ( NULL != Except )
        LspFree( Except );

    return ret;
}

//
// Function: WSPSend
//
// Description:
//    This function sends data on a given socket and also allows for asynchronous
//    (overlapped) operation. First translate the socket handle to the lower provider
//    handle and then make the send call. If called with overlap, post the operation
//    to our IOCP or completion routine.
//
int WSPAPI 
WSPSend(
    SOCKET          s,
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
    CLSPSocket*			pSockObj  = NULL;


    *lpErrno = NO_ERROR;

    //
    // Find our provider socket corresponding to this one
    //
	pSockObj=theInstance.GetSockFactoryInstance()->LookupSocketObj(s,lpErrno);
    if ( NULL == pSockObj )
    {
        dbgprint(_T( "WSPSend: LookupSocketObj failed!") );
        goto cleanup;
    }

    ret = pSockObj->WSPSend(
								lpBuffers, 
								dwBufferCount,
								lpNumberOfBytesSent, 
								dwFlags, 
								lpOverlapped, 
								lpCompletionRoutine, 
								lpThreadId, 
								lpErrno
								);


cleanup:

    if ( NULL != pSockObj )
		pSockObj->Release();

    return ret;
}

//
// Function: WSPSendDisconnect
//
// Description:
//    Send data and disconnect. All we need to do is translate the socket
//    handle to the lower provider.
//
int WSPAPI 
WSPSendDisconnect(
    SOCKET   s,
    LPWSABUF lpOutboundDisconnectData,
    LPINT    lpErrno
    )
{
    CLSPSocket*			pSockObj  = NULL;
    INT					ret = SOCKET_ERROR;

    //
    // Find our provider socket corresponding to this one
    //
	pSockObj=theInstance.GetSockFactoryInstance()->LookupSocketObj(s,lpErrno);
    if ( NULL == pSockObj )
    {
        dbgprint( _T("WSPSendDisconnect: LookupSocketObj failed!") );
        goto cleanup;
    }

     ret = pSockObj->WSPSendDisconnect(
										lpOutboundDisconnectData, 
										lpErrno
										);

cleanup:

    if ( NULL != pSockObj )
		pSockObj->Release();


    return ret;
}

//
// Function: WSPSendTo
//
// Description:
//    This function sends data on a given socket and also allows for asynchronous
//    (overlapped) operation. First translate the socket handle to the lower provider
//    handle and then make the send call. If called with overlap, post the operation
//    to our IOCP or completion routine.
//
int WSPAPI 
WSPSendTo(
    SOCKET          s,
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
    CLSPSocket*			pSockObj  = NULL;


    //
    // Find our provider socket corresponding to this one
    //
	pSockObj=theInstance.GetSockFactoryInstance()->LookupSocketObj(s,lpErrno);
    if ( NULL == pSockObj )
    {
        dbgprint(_T( "WSPSendTo: LookupSocketObj failed!") );
        goto cleanup;
    }

     ret = pSockObj->WSPSendTo(
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

cleanup:

    if ( NULL != pSockObj )
		pSockObj->Release(); 

    return ret;
}

//
// Function: WSPSetSockOpt
//
// Description:
//    Set a socket option. For most all options we just have to translate the
//    socket option and call the lower provider. The only special case is for
//    SO_UPDATE_ACCEPT_CONTEXT in which case a socket handle is passed as the
//    argument which we need to translate before calling the lower provider.
//
int WSPAPI WSPSetSockOpt(
    SOCKET     s,
    int        level,
    int        optname,
    const char FAR * optval,   
    int        optlen,
    LPINT      lpErrno
    )
{
    CLSPSocket*		pSockObj  = NULL,
					*pAcceptSockObj = NULL;
    INT        ret = SOCKET_ERROR;

	pSockObj=theInstance.GetSockFactoryInstance()->LookupSocketObj(s,lpErrno);
    if ( NULL == pSockObj )
    {
        dbgprint(_T( "WSPSetSockOpt: LookupSocketObj failed!") );
        goto cleanup;
    }


    if ( SO_UPDATE_ACCEPT_CONTEXT == optname )
    {
        // We need to intercept this (and any other options) that pass
        //  a socket handle as an argument so we can replace it with the
        //  correct underlying provider's socket handle.
        //
        pAcceptSockObj = theInstance.GetSockFactoryInstance()->LookupSocketObj( *((SOCKET *)optval), lpErrno);
        if ( NULL == pAcceptSockObj )
        {
            dbgprint( _T("WSPSetSockOpt: SO_UPDATE_ACCEPT_CONTEXT: LookupSocketObj failed!") );
            goto cleanup;
        }
		SOCKET ProviderSocket=pAcceptSockObj->GetProviderSocket();
		ret=pSockObj->WSPSetSockOpt(
											level,
											optname, 
											(char *)&ProviderSocket, 
											optlen, 
											lpErrno
											);
    }
    else
    {
		ret=pSockObj->WSPSetSockOpt(level,optname,optval,optlen,lpErrno);
    }

cleanup:

    if ( NULL != pSockObj )
		pSockObj->Release(); 

    if ( NULL != pAcceptSockObj )
		pAcceptSockObj->Release(); 

    return ret;
}

//
// Function: WSPShutdown
//
// Description:
//    This function performs a shutdown on the socket. All we need to do is 
//    translate the socket handle to the lower provider.
//
int WSPAPI 
WSPShutdown (
    SOCKET s,
    int    how,
    LPINT  lpErrno
    )
{
    CLSPSocket*		pSockObj = NULL;
    INT				ret = SOCKET_ERROR;

	pSockObj=theInstance.GetSockFactoryInstance()->LookupSocketObj(s,lpErrno);
    if ( NULL == pSockObj )
    {
        dbgprint(_T( "WSPShutdown: LookupSocketObj failed!") );
        goto cleanup;
    }

    ret=pSockObj->WSPShutdown(how,lpErrno);

cleanup:

    if ( NULL != pSockObj )
        pSockObj->Release() ;

    return ret;
}

//
// Function: WSPStringToAddress
//
// Description:
//    Convert a string to an address (SOCKADDR structure).  We need to translate
//    the socket handle as well as possibly substitute the lpProtocolInfo structure
//    passed to the next provider. 
//
int WSPAPI 
WSPStringToAddress(
    LPWSTR              AddressString,
    INT                 AddressFamily,
    LPWSAPROTOCOL_INFOW lpProtocolInfo,   
    LPSOCKADDR          lpAddress,
    LPINT               lpAddressLength,
    LPINT               lpErrno
    )
{
    WSAPROTOCOL_INFOW   *pInfo = NULL;
    PROVIDER            *Provider = NULL;
    INT                  ret = SOCKET_ERROR;

	Provider= theInstance.GetProviderInstance()->GetProvider( lpProtocolInfo,lpErrno);

	if(Provider==NULL) goto cleanup;

    pInfo = &Provider->NextProvider;

   	//
    // If we're not immediately above the base then pass the lpProtocolInfo passed
    // into us.
    //
    if ( BASE_PROTOCOL != pInfo->ProtocolChain.ChainLen )
    {
        pInfo = lpProtocolInfo;
    }

    ASSERT( Provider->NextProcTable.lpWSPStringToAddress );

    SetBlockingProvider(Provider);
    ret = Provider->NextProcTable.lpWSPStringToAddress(
            AddressString, 
            AddressFamily,
            pInfo, 
            lpAddress, 
            lpAddressLength, 
            lpErrno
            );
    SetBlockingProvider(NULL);

cleanup:

    return ret;
}

//
// Function: WSPSocket
//
// Description:
//    This function creates a socket. There are two sockets created. The first
//    socket is created by calling the lower providers WSPSocket. This is the
//    handle that we use internally within our LSP. We then create a second
//    socket with WPUCreateSocketHandle which will be returned to the calling
//    application. We will also create a socket context structure which will
//    maintain information on each socket. This context is associated with the
//    socket handle passed to the application.
//
SOCKET WSPAPI 
WSPSocket(
    int                 af,
    int                 type,
    int                 protocol,
    __in LPWSAPROTOCOL_INFOW lpProtocolInfo,
    GROUP               g,
    DWORD               dwFlags,
    LPINT               lpErrno
    )
{
	CLSPSocket		  *pSockObj=NULL;
	SOCKET			   ProviderSocket,CallerSocket=INVALID_SOCKET;
	PROVIDER		   *NextProvider;
	WSAPROTOCOL_INFOW  InfoCopy={0};



	NextProvider=theInstance.GetProviderInstance()->GetProvider(af,
														type,
														protocol,
														lpProtocolInfo,
														g,
														dwFlags,
														InfoCopy,
														lpErrno
														);
	if(!NextProvider)
	{
		goto cleanup;
	}

	//create sock object with the provider
	pSockObj=theInstance.GetSockFactoryInstance()->CreateSocketObject(NextProvider,af,type,protocol,lpErrno);
	if(!pSockObj)
	{
		goto cleanup;
	}

	ProviderSocket=pSockObj->WSPSocket(af,type,protocol,&InfoCopy,g,dwFlags,lpErrno);

	if(ProviderSocket==INVALID_SOCKET)
	{
		dbgprint(_T("WSPSocket: NextProcTable.WSPSocket() failed: %d"), *lpErrno);
		goto cleanup;
	}

	CallerSocket=theInstance.GetSockFactoryInstance()->MapAndStoreSocketObject(pSockObj,lpErrno);

	if(CallerSocket==INVALID_SOCKET)
	{
		goto cleanup;
	}
	 return CallerSocket;

cleanup:
	if(pSockObj)
	{
		theInstance.GetSockFactoryInstance()->CloseProviderAndFreeLSPObject(pSockObj,lpErrno);
	}

    return INVALID_SOCKET;
}

//
// Function: WSPStartup
//
// Description:
//    This function intializes our LSP. We maintain a ref count to keep track
//    of how many times this function has been called. On the first call we'll
//    look at the Winsock catalog to find our catalog ID and find which entries
//    we are layered over. We'll create a number of structures to keep this 
//    information handy.
//
//    NOTE: There are two basic methods of finding an LSPs position in the chain.
//          First, it may look at the protocol chain of the lpProtocolInfo passed
//          in. If it does this an LSP SHOULD find its position by matching ANY
//          of the provider IDs beloging to the LSP. This includes the ID of the
//          dummy entry as well as the IDs of any of the layered providers belonging
//          to the LSP!
//
//          The second option is to enumerate the Winsock catalog and find find
//          all the entries belonging to this LSP. Then from the layered provider 
//          entries, take position 1 in the protocol chain to find out who is next
//          in your layer and load them.
//
//          This LSP takes the second approach -- mainly because if another LSP
//          layers on top of us and dorks up their protocol chain, if this LSP is
//          called it can reliably load the lower provider (unless the bad LSP
//          modified our LSPs entries and goofed up our chains as well).
//
int WSPAPI 
WSPStartup(
    WORD                wVersion,
    LPWSPDATA           lpWSPData,
    LPWSAPROTOCOL_INFOW lpProtocolInfo,
    WSPUPCALLTABLE      UpCallTable,
    LPWSPPROC_TABLE     lpProcTable
    )
{

		
    int   Error = WSAEPROVIDERFAILEDINIT;
    EnterCriticalSection( &gCriticalSection );

	Error = theInstance.GetProviderInstance()->Initialize(wVersion,lpWSPData,lpProtocolInfo,UpCallTable,lpProcTable);
  
	theInstance.LoadRegistryValues(); 
    LeaveCriticalSection( &gCriticalSection );

    return Error;
}
