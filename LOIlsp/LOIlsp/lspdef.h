// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (C) 2004  Microsoft Corporation.  All Rights Reserved.
// 
// Module Name: provider.h
//
// Description:
//
//    This sample illustrates how to develop a layered service provider that is
//    capable of counting all bytes transmitted through a TCP/IP socket.
//
//    This file contains all datatypes and function prototypes used
//    throughout this project.
//
#ifndef _PROVIDER_H_
#define _PROVIDER_H_ 


#include <ws2spi.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#include <mstcpip.h>
#include "lspcommon.h"

#include "LSPSocket.h"
#include "OverlappedManager.h"
#include "LSPModule.h"
#define WM_SOCKET ( WM_USER + 321 )     // Message for our async window events

//
// This is the socket context data that we associate with each socket
//  that is passed back to the user app. That way when another API
//  is called we can query for this context information and find out
//  the socket handle to the lower provider.
//
typedef struct _SOCK_INFO
{
	 CLSPSocket       *Context;      
    LIST_ENTRY         Link;

} SOCK_INFO;

//
// Structure for mapping upper layer sockets to lower provider sockets in WSPSelect
//
typedef struct _FD_MAP
{
    SOCKET      ClientSocket;       // Upper layer socket handle
    SOCKET      ProvSocket;         // Lower layer socket handle
    CLSPSocket  *Context;            // Pointer to the socket context
} FD_MAP;


////////////////////////////////////////////////////////////////////////////////
//
// AsyncSelect.cpp prototypes
//
////////////////////////////////////////////////////////////////////////////////

// Retrieves the async worker window and creates it if it hasn't already been
HWND 
GetWorkerWindow(
    );

// Issues the shutdown command to thread handling window messages
int 
StopAsyncWindowManager(
    );

//
// Need to keep track of which PROVIDERs that are currently executing
//  a blocking Winsock call on a per thread basis.
//
inline void SetBlockingProvider(PROVIDER *Provider)
{
	extern DWORD    gTlsIndex;
    ( gTlsIndex!=0xFFFFFFFF )                   
        ? TlsSetValue ( gTlsIndex, Provider )   
        : NULL;
}
 
////////////////////////////////////////////////////////////////////////////////
//
// Spi.cpp prototypes
//
////////////////////////////////////////////////////////////////////////////////

//
// These are the Winsock functions an LSP must implement if it is non-IFS
//

SOCKET WSPAPI 
WSPAccept(
    SOCKET          s,                      
    struct sockaddr FAR * addr,  
    LPINT           addrlen,                 
    LPCONDITIONPROC lpfnCondition,  
    DWORD_PTR       dwCallbackData,          
    LPINT           lpErrno
    );

int WSPAPI 
WSPAddressToString(
    LPSOCKADDR          lpsaAddress,            
    DWORD               dwAddressLength,               
    LPWSAPROTOCOL_INFOW lpProtocolInfo,   
    LPWSTR              lpszAddressString,            
    LPDWORD             lpdwAddressStringLength,   
    LPINT               lpErrno
    );

int WSPAPI 
WSPAsyncSelect(
    SOCKET       s,
    HWND         hWnd,
    unsigned int wMsg,
    long         lEvent,
    LPINT        lpErrno
    );

int WSPAPI 
WSPBind(
    SOCKET                s,
    const struct sockaddr FAR * name,
    int                   namelen,
    LPINT                 lpErrno
    );

int WSPAPI 
WSPCancelBlockingCall(
    LPINT lpErrno
    );

int WSPAPI 
WSPCleanup(
    LPINT lpErrno  
    );

int WSPAPI 
WSPCloseSocket(  
    SOCKET s,        
    LPINT  lpErrno
    );

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
    );

int WSPAPI 
WSPDuplicateSocket(
    SOCKET              s,
    DWORD               dwProcessId,                      
    LPWSAPROTOCOL_INFOW lpProtocolInfo,   
    LPINT               lpErrno
    );

int WSPAPI 
WSPEnumNetworkEvents(  
    SOCKET             s,
    WSAEVENT           hEventObject,
    LPWSANETWORKEVENTS lpNetworkEvents,
    LPINT              lpErrno
    );

int WSPAPI 
WSPEventSelect(
    SOCKET   s,
    WSAEVENT hEventObject,
    long     lNetworkEvents,
    LPINT    lpErrno
    );

BOOL WSPAPI 
WSPGetOverlappedResult(
    SOCKET          s,
    LPWSAOVERLAPPED lpOverlapped,
    LPDWORD         lpcbTransfer,
    BOOL            fWait,
    LPDWORD         lpdwFlags,
    LPINT           lpErrno
    );

int WSPAPI 
WSPGetPeerName(  
    SOCKET          s,
    struct sockaddr FAR * name,
    LPINT           namelen,
    LPINT           lpErrno
    );

int WSPAPI 
WSPGetSockName(
    SOCKET          s,
    struct sockaddr FAR * name,
    LPINT           namelen,
    LPINT           lpErrno
    );

int WSPAPI 
WSPGetSockOpt(
    SOCKET     s,
    int        level,
    int        optname,
    char FAR * optval,
    LPINT      optlen,
    LPINT      lpErrno
    );

BOOL WSPAPI 
WSPGetQOSByName(
    SOCKET   s,
    LPWSABUF lpQOSName,
    LPQOS    lpQOS,
    LPINT    lpErrno
    );

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
    );

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
    );

int WSPAPI 
WSPListen(
    SOCKET s,        
    int    backlog,     
    LPINT  lpErrno
    );

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
    );

int WSPAPI 
WSPRecvDisconnect(
    SOCKET   s,
    LPWSABUF lpInboundDisconnectData,
    LPINT    lpErrno
    );

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
    );

int WSPAPI 
WSPSelect(
    int          nfds,
    fd_set FAR * readfds,
    fd_set FAR * writefds,
    fd_set FAR * exceptfds,
    const struct timeval FAR * timeout,
    LPINT        lpErrno
    );

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
    );

int WSPAPI 
WSPSendDisconnect(
    SOCKET   s,
    LPWSABUF lpOutboundDisconnectData,
    LPINT    lpErrno
    );

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
    );

int WSPAPI 
WSPSetSockOpt(
    SOCKET     s,
    int        level,
    int        optname,
    const char FAR * optval,   
    int        optlen,
    LPINT      lpErrno
    );

int WSPAPI 
WSPShutdown (
    SOCKET s,
    int    how,
    LPINT  lpErrno
    );

int WSPAPI 
WSPStringToAddress(
    LPWSTR              AddressString,
    INT                 AddressFamily,
    LPWSAPROTOCOL_INFOW lpProtocolInfo,   
    LPSOCKADDR          lpAddress,
    LPINT               lpAddressLength,
    LPINT               lpErrno
    );

SOCKET WSPAPI 
WSPSocket(
    int                 af,
    int                 type,
    int                 protocol,
    __in LPWSAPROTOCOL_INFOW lpProtocolInfo,
    GROUP               g,
    DWORD               dwFlags,
    LPINT               lpErrno
    );


////////////////////////////////////////////////////////////////////////////////
//
// Extension.cpp prototypes
//
////////////////////////////////////////////////////////////////////////////////

BOOL PASCAL FAR 
ExtTransmitFile (
    IN SOCKET hSocket,
    IN HANDLE hFile,
    IN DWORD nNumberOfBytesToWrite,
    IN DWORD nNumberOfBytesPerSend,
    IN LPOVERLAPPED lpOverlapped,
    IN LPTRANSMIT_FILE_BUFFERS lpTransmitBuffers,
    IN DWORD dwReserved
    );

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
    );

BOOL PASCAL FAR 
ExtConnectEx(
    IN SOCKET s,
    IN const struct sockaddr FAR *name,
    IN int namelen,
    IN PVOID lpSendBuffer OPTIONAL,
    IN DWORD dwSendDataLength,
    OUT LPDWORD lpdwBytesSent,
    IN LPOVERLAPPED lpOverlapped
    );

BOOL PASCAL FAR 
ExtTransmitPackets(
    SOCKET hSocket,
    LPTRANSMIT_PACKETS_ELEMENT lpPacketArray,
    DWORD nElementCount,
    DWORD nSendSize,
    LPOVERLAPPED lpOverlapped,
    DWORD dwFlags
    );

BOOL PASCAL FAR 
ExtDisconnectEx(
    IN SOCKET s,
    IN LPOVERLAPPED lpOverlapped,
    IN DWORD  dwFlags,
    IN DWORD  dwReserved
    );

INT PASCAL FAR 
ExtWSARecvMsg(
    IN SOCKET s,
    IN OUT LPWSAMSG lpMsg,
    OUT LPDWORD lpdwNumberOfBytesRecvd,
    IN LPWSAOVERLAPPED lpOverlapped,
    IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    );

INT PASCAL FAR 
ExtWSASendMsg(
			  IN   SOCKET s,
			  IN   LPWSAMSG lpMsg,
			  IN   DWORD dwFlags,
			  OUT  LPDWORD lpNumberOfBytesSent,
			  IN   LPWSAOVERLAPPED lpOverlapped,
			  IN   LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
			);
// Loads the given extension function from the lower provider
BOOL
LoadExtensionFunction(
    FARPROC   **func,
    GUID        ExtensionGuid,
    LPWSPIOCTL  fnIoctl,
    SOCKET      s
    );

////////////////////////////////////////////////////////////////////////////////
//
// External variable definitions
//
////////////////////////////////////////////////////////////////////////////////

extern CRITICAL_SECTION gCriticalSection,   // Critical section for initialization and 
                                            //    socket list manipulation
                        gOverlappedCS;      // Used in overlapped IO handling
     

extern HANDLE            gIocp;              // Completion port handle
extern WSPUPCALLTABLE   gMainUpCallTable;   // Upcall functions given to us by Winsock
extern WSPPROC_TABLE       gProcTable;
extern GUID             gProviderGuid;      // GUID of our dummy hidden entry
extern COverlappedManager gOverlapManager;
extern CLSPModule			theInstance;
extern DWORD    gTlsIndex ; // Index into thread local storage
extern DWORD    gEntryCount;        // How many times WSPStartup has been called
extern DWORD    gLayerCatId;        // Catalog ID of our dummy entry
extern WSPDATA  gWSPData;


#endif
