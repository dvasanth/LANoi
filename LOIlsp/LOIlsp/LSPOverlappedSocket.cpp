#include "StdAfx.h"
#include "lspdef.h"
#include "LSPOverlappedSocket.h"


#include <stdio.h>
#include <stdlib.h>
#include "lspdef.h"
#pragma warning(disable:4127)       // disable "conditional expression is constant" warning
#pragma warning(disable:4702)       // disable "unreacheable code" warning

CLSPOverlappedSocket::CLSPOverlappedSocket(PROVIDER *Provider ):CLSPSocket(Provider) 
{
	Init();
}

CLSPOverlappedSocket::CLSPOverlappedSocket(PROVIDER *Provider ,SOCKET ProviderSocket):CLSPSocket(Provider,ProviderSocket) 
{
	Init();
}

void CLSPOverlappedSocket::Init()
{
	m_bSynchronous=FALSE;
}

CLSPOverlappedSocket::~CLSPOverlappedSocket(void)
{
}



void CLSPOverlappedSocket::InitSockOverlap()
{
	m_bSynchronous=FALSE;
	AcquireSocketLock();
	if(m_hIocp==NULL)
	{
		m_hIocp=gOverlapManager.AssociateHandle(m_NextProviderSocket,m_CallerSocket,m_bSynchronous);
	}
	ReleaseSocketLock();
}

//
// Function: PrepareOverlappedOperation
//
// Description:
//    This function simply allocates and initializes the common fields of a
//    WSAOVERLAPPEDPLUS structure. These fields are required in order to handle
//    overlapped IO correctly.
//
WSAOVERLAPPEDPLUS *CLSPOverlappedSocket::PrepareOverlappedOperation(LspOperation operation,WSABUF *lpBuffers,DWORD dwBufferCount,LPWSAOVERLAPPED lpOverlapped,LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,LPWSATHREADID  lpThreadId,int *lpErrno)
{
    WSAOVERLAPPEDPLUS *ProviderOverlapped = NULL;
    int                ret = SOCKET_ERROR,
                       err = 0;

    // Allocate a WSAOVERLAPPEDPLUS structure

    //
    // We have to keep track of the number of outstanding overlapped requests 
    // an application has. Otherwise, if the app were to close a socket that 
    // had oustanding overlapped ops remaining, we'd start leaking structures 
    // in gOverlappedPool. The idea here is to force the CallerSocket to remain 
    // open until the lower provider has processed all the overlapped requests. 
    // If we closed both the lower socket and the caller socket, we would no 
    // longer be able to correlate completed requests to any apps sockets.
    //	
	AcquireSocketLock();
	AddAsyncRef();
    ReleaseSocketLock();

	ProviderOverlapped = gOverlapManager.AllocOverlappedStructure( this );


    if ( NULL == ProviderOverlapped )
    {
        *lpErrno = WSAENOBUFS;
        goto cleanup;
    }


    __try 
    {
        // Check for an event and reset if. Also, copy the offsets from the upper
        // layer's WSAOVERLAPPED structure.
        if ( ( NULL == lpCompletionRoutine ) && 
             ( NULL != lpOverlapped->hEvent ) )
        { 
            ULONG_PTR   ptr = 1;

            ret = ResetEvent( (HANDLE) ( (ULONG_PTR) lpOverlapped->hEvent & ~ptr ) );
            if (ret == 0)
            {
                *lpErrno = ERROR_INVALID_HANDLE;
                ret = SOCKET_ERROR;
                goto cleanup;
            }
        }

        // Copy any offset information from the caller's overlapped to ours
        CopyOffset( &ProviderOverlapped->ProviderOverlapped, lpOverlapped );

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        *lpErrno = WSAEFAULT;
        goto cleanup;
    }

    if ( NULL != lpThreadId )
    {
        ProviderOverlapped->CallerThreadId = *lpThreadId;
    }
    else
    {
        // If thread info wasn't passed to us, we need to open the thread context
        ret = gMainUpCallTable.lpWPUOpenCurrentThread( &ProviderOverlapped->CallerThreadId, &err );
        if ( SOCKET_ERROR == ret )
        {
            dbgprint(L"WPUOpenCurrentThread failed: %d", err);
        }
        else
        {
            // Need to remember for later to close the context since we opened it
            ProviderOverlapped->CloseThread = TRUE;
        }
    }

    // Fill in the remaining fields
    ProviderOverlapped->Provider           = m_NextProvider;
    ProviderOverlapped->lpCallerOverlapped = lpOverlapped;
    ProviderOverlapped->lpCallerCompletionRoutine = lpCompletionRoutine;
    ProviderOverlapped->SockInfo           = this;
    ProviderOverlapped->CallerSocket       = m_CallerSocket;
    ProviderOverlapped->ProviderSocket     = m_NextProviderSocket;
    ProviderOverlapped->Error              = NO_ERROR;
    ProviderOverlapped->Operation          = operation;

    if ( ( NULL != lpBuffers) && ( dwBufferCount ) )
    {
        // Depending on the underlying operation, copy some parameters specific to it
        switch ( operation )
        {
            case LSP_OP_RECV:
                ProviderOverlapped->RecvArgs.dwBufferCount = dwBufferCount;
                ProviderOverlapped->RecvArgs.lpBuffers =  lpBuffers;
                if ( NULL == ProviderOverlapped->RecvArgs.lpBuffers )
                    goto cleanup;

                break;

            case LSP_OP_RECVFROM:
                ProviderOverlapped->RecvFromArgs.dwBufferCount = dwBufferCount;
                ProviderOverlapped->RecvFromArgs.lpBuffers = lpBuffers;
                if ( NULL == ProviderOverlapped->RecvFromArgs.lpBuffers )
                    goto cleanup;

                break;

            case LSP_OP_SEND:
                ProviderOverlapped->SendArgs.dwBufferCount = dwBufferCount;
                ProviderOverlapped->SendArgs.lpBuffers = lpBuffers;
                if ( NULL == ProviderOverlapped->SendArgs.lpBuffers )
                    goto cleanup;

                break;
              
            case LSP_OP_SENDTO:
                ProviderOverlapped->SendToArgs.dwBufferCount = dwBufferCount;
                ProviderOverlapped->SendToArgs.lpBuffers = lpBuffers;
                if ( NULL == ProviderOverlapped->SendToArgs.lpBuffers )
                    goto cleanup;

                break;

            default:
                break;
        }
    }

    ret = NO_ERROR;

cleanup:

    if ( SOCKET_ERROR == ret )
    {
		AcquireSocketLock(  );
		ReleaseAsync();
		ReleaseSocketLock(  );
        gOverlapManager.FreeOverlappedStructure(ProviderOverlapped );
        ProviderOverlapped = NULL;
    }
    else
    {
        ASSERT( NO_ERROR == ret );
    }

    return ProviderOverlapped;
}
//
// Function: CopyOffset
//
// Description:
//    Any offset information passed by the application in its OVERLAPPED structure
//    needs to be copied down to the OVERLAPPED structure the LSP passes to the
//    lower layer. This function copies the offset fields.
//
void CLSPOverlappedSocket::CopyOffset(WSAOVERLAPPED  *ProviderOverlapped,WSAOVERLAPPED  *UserOverlapped)
{
    ProviderOverlapped->Offset     = UserOverlapped->Offset;
    ProviderOverlapped->OffsetHigh = UserOverlapped->OffsetHigh;
}

//
// Function: ExecuteOverlappedOperation
//
// Description:
//    This function actually executes an overlapped operation that was queued.
//    If on Win9x we substitute our own completion function in order to intercept
//    the results. If on NT we post the operation to our completion port.
//    This function either returns NO_ERROR if the operation succeeds immediately
//    or the Winsock error code upone failure (or overlapped operation).
//
int CLSPOverlappedSocket::ExecuteOverlappedOperation(WSAOVERLAPPEDPLUS *ol,LPINT lpErrno)
{
    LPWSAOVERLAPPED_COMPLETION_ROUTINE   routine = NULL;
    PROVIDER                            *Provider;
    DWORD                               *lpdwFlags = NULL,
                                        *lpdwBytes = NULL;
    int                                  ret=SOCKET_ERROR, err=0;

    Provider = ol->Provider;

    //
    // Reset the event handle if present. The handle is masked with 0xFFFFFFFE in
    //  order to zero out the last bit. If the last bit is one and the socket is
    //  associated with a compeltion port then when an overlapped operation is 
    //  called, the operation is not posted to the IO completion port.
    //
    if ( NULL != ol->lpCallerOverlapped->hEvent )
    {
        ULONG_PTR   ptr = 1;

        ResetEvent( (HANDLE) ( (ULONG_PTR) ol->lpCallerOverlapped->hEvent & ~ptr ) );
    }

    switch ( ol->Operation )
    {
        case LSP_OP_IOCTL:
            lpdwFlags = NULL;
            lpdwBytes = &ol->IoctlArgs.cbBytesReturned;
			dbgprint(_T("Queue LSP_OP_IOCTL"));
            ret = Provider->NextProcTable.lpWSPIoctl(
                    ol->ProviderSocket,
                    ol->IoctlArgs.dwIoControlCode,
                    ol->IoctlArgs.lpvInBuffer,
                    ol->IoctlArgs.cbInBuffer,
                    ol->IoctlArgs.lpvOutBuffer,
                    ol->IoctlArgs.cbOutBuffer,
                   &ol->IoctlArgs.cbBytesReturned,
                   &ol->ProviderOverlapped,
                    routine,
                   &ol->CallerThreadId,
                   &err
                   );
            break;                         

        case LSP_OP_RECV:
			dbgprint(_T("Queue LSP_OP_RECV"));
            lpdwFlags = &ol->RecvArgs.dwFlags;
            lpdwBytes = &ol->RecvArgs.dwNumberOfBytesRecvd;
            ret = Provider->NextProcTable.lpWSPRecv(
                    ol->ProviderSocket,
                    ol->RecvArgs.lpBuffers,
                    ol->RecvArgs.dwBufferCount,
                   &ol->RecvArgs.dwNumberOfBytesRecvd,
                   &ol->RecvArgs.dwFlags,
                   &ol->ProviderOverlapped,
                    routine,
                   &ol->CallerThreadId,
                   &err
                    );
            break;

        case LSP_OP_RECVFROM:
			dbgprint(_T("Queue LSP_OP_RECVFROM Provider Socket %X"),ol->ProviderSocket);
            lpdwFlags = &ol->RecvFromArgs.dwFlags;
            lpdwBytes = &ol->RecvFromArgs.dwNumberOfBytesRecvd;
            ret = Provider->NextProcTable.lpWSPRecvFrom(
                    ol->ProviderSocket,
                    ol->RecvFromArgs.lpBuffers,
                    ol->RecvFromArgs.dwBufferCount,
                   &ol->RecvFromArgs.dwNumberOfBytesRecvd,
                   &ol->RecvFromArgs.dwFlags,
                    ol->RecvFromArgs.lpFrom,
                    ol->RecvFromArgs.lpFromLen,
                   &ol->ProviderOverlapped,
                    routine,
                   &ol->CallerThreadId,
                   &err
                    );
            break;

        case LSP_OP_SEND:
			dbgprint(_T("Queue LSP_OP_SEND"));
            lpdwFlags = &ol->SendArgs.dwFlags;
            lpdwBytes = &ol->SendArgs.dwNumberOfBytesSent;
            ret = Provider->NextProcTable.lpWSPSend(
                    ol->ProviderSocket,
                    ol->SendArgs.lpBuffers,
                    ol->SendArgs.dwBufferCount,
                   &ol->SendArgs.dwNumberOfBytesSent,
                    ol->SendArgs.dwFlags,
                   &ol->ProviderOverlapped,
                    routine,
                   &ol->CallerThreadId,
                   &err
                   );
             break;

        case LSP_OP_SENDTO:
			dbgprint(_T("Queue LSP_OP_SENDTO"));
            lpdwFlags = &ol->SendToArgs.dwFlags;
            lpdwBytes = &ol->SendToArgs.dwNumberOfBytesSent;
            ret = Provider->NextProcTable.lpWSPSendTo(
                    ol->ProviderSocket,
                    ol->SendToArgs.lpBuffers,
                    ol->SendToArgs.dwBufferCount,
                   &ol->SendToArgs.dwNumberOfBytesSent,
                    ol->SendToArgs.dwFlags,
                    (SOCKADDR *)&ol->SendToArgs.To,
                    ol->SendToArgs.iToLen,
                   &ol->ProviderOverlapped,
                    routine,
                   &ol->CallerThreadId,
                   &err
                    );
            break;

        case LSP_OP_TRANSMITFILE:
			dbgprint(_T("Queue LSP_OP_TRANSMITFILE"));
            lpdwFlags = &ol->TransmitFileArgs.dwFlags;
            lpdwBytes = NULL;
            ret = Provider->NextProcTableExt.lpfnTransmitFile(
                    ol->ProviderSocket,
                    ol->TransmitFileArgs.hFile,
                    ol->TransmitFileArgs.nNumberOfBytesToWrite,
                    ol->TransmitFileArgs.nNumberOfBytesPerSend,
                   &ol->ProviderOverlapped,
                    ol->TransmitFileArgs.lpTransmitBuffers,
                    ol->TransmitFileArgs.dwFlags
                    );
            if ( FALSE == ret )
            {
                ret = SOCKET_ERROR;
                err = WSAGetLastError();
                WSASetLastError(err);
            }
            else
            {
                ret = NO_ERROR;
            }
            break;

        case LSP_OP_ACCEPTEX:
			dbgprint(_T("Queue LSP_OP_ACCEPTEX AcceptExArgs.lpOutputBuffer %X , ol->AcceptExArgs.dwReceiveDataLength %X,ol->AcceptExArgs.dwLocalAddressLength %X ol->AcceptExArgs.dwRemoteAddressLength %X"),ol->AcceptExArgs.lpOutputBuffer, ol->AcceptExArgs.dwReceiveDataLength,ol->AcceptExArgs.dwLocalAddressLength,ol->AcceptExArgs.dwRemoteAddressLength);
            lpdwFlags = NULL;
            lpdwBytes = &ol->AcceptExArgs.dwBytesReceived;
            ret = Provider->NextProcTableExt.lpfnAcceptEx(
                    ol->ProviderSocket,
                    ol->AcceptExArgs.sProviderAcceptSocket,
                    ol->AcceptExArgs.lpOutputBuffer,
                    ol->AcceptExArgs.dwReceiveDataLength,
                    ol->AcceptExArgs.dwLocalAddressLength,
                    ol->AcceptExArgs.dwRemoteAddressLength,
                   &ol->AcceptExArgs.dwBytesReceived,
                   &ol->ProviderOverlapped
                    );
            if ( FALSE == ret )
            {
                ret = SOCKET_ERROR;
                err = WSAGetLastError();
                WSASetLastError(err);
            }
            else
            {
                ret = NO_ERROR;
            }
            break;

        case LSP_OP_CONNECTEX:
			dbgprint(_T("Queue LSP_OP_CONNECTEX"));
            lpdwFlags = NULL;
            lpdwBytes = &ol->ConnectExArgs.dwBytesSent;
            ret = Provider->NextProcTableExt.lpfnConnectEx(
                    ol->ProviderSocket,
                    (SOCKADDR *)&ol->ConnectExArgs.name,
                    ol->ConnectExArgs.namelen,
                    ol->ConnectExArgs.lpSendBuffer,
                    ol->ConnectExArgs.dwSendDataLength,
                   &ol->ConnectExArgs.dwBytesSent,
                   &ol->ProviderOverlapped
                    );
            if ( FALSE == ret )
            {
                ret = SOCKET_ERROR;
                err = WSAGetLastError();
                WSASetLastError(err);
            }
            else
            {
                ret = NO_ERROR;
            }
            break;

        case LSP_OP_DISCONNECTEX:
			dbgprint(_T("Queue LSP_OP_DISCONNECTEX"));
            lpdwFlags = &ol->DisconnectExArgs.dwFlags;
            lpdwBytes = NULL;
            ret = Provider->NextProcTableExt.lpfnDisconnectEx(
                    ol->ProviderSocket,
                   &ol->ProviderOverlapped,
                    ol->DisconnectExArgs.dwFlags,
                    ol->DisconnectExArgs.dwReserved
                    );
            if ( FALSE == ret )
            {
                ret = SOCKET_ERROR;
                err = WSAGetLastError();
                WSASetLastError(err);
            }
            else
            {
                ret = NO_ERROR;
            }
            break;

        case LSP_OP_TRANSMITPACKETS:
			dbgprint(_T("Queue LSP_OP_TRANSMITPACKETS"));
            lpdwFlags = &ol->TransmitPacketsArgs.dwFlags;
            lpdwBytes = NULL;
            ret = Provider->NextProcTableExt.lpfnTransmitPackets(
                    ol->ProviderSocket,
                    ol->TransmitPacketsArgs.lpPacketArray,
                    ol->TransmitPacketsArgs.nElementCount,
                    ol->TransmitPacketsArgs.nSendSize,
                   &ol->ProviderOverlapped,
                    ol->TransmitPacketsArgs.dwFlags
                    );
            if ( FALSE == ret )
            {
                ret = SOCKET_ERROR;
                err = WSAGetLastError();
                WSASetLastError(err);
            }
            else
            {
                ret = NO_ERROR;
            }
            break;

        case LSP_OP_WSARECVMSG:
			dbgprint(_T("Queue LSP_OP_WSARECVMSG"));
            lpdwFlags = NULL;
            lpdwBytes = &ol->WSARecvMsgArgs.dwNumberOfBytesRecvd;
            ret = Provider->NextProcTableExt.lpfnWSARecvMsg(
                    ol->ProviderSocket,
                    ol->WSARecvMsgArgs.lpMsg,
                   &ol->WSARecvMsgArgs.dwNumberOfBytesRecvd,
                   &ol->ProviderOverlapped,
                    routine
                    );
            if ( SOCKET_ERROR == ret )
            {
                err = WSAGetLastError();
                WSASetLastError( err );
            }
            else
            {
                ret = NO_ERROR;
            }
            break;

		case LSP_OP_WSASENDMSG:

			dbgprint(_T("Queue LSP_OP_WSASENDMSG"));
            lpdwFlags = &ol->WSASendMsgArgs.dwFlags;
            lpdwBytes = &ol->WSASendMsgArgs.dwNumberOfBytesSent;
            ret = Provider->NextProcTableExt.lpfnWSASendMsg(
                    ol->ProviderSocket,
                    ol->WSASendMsgArgs.lpMsg,
					ol->WSASendMsgArgs.dwFlags,
                   &ol->WSASendMsgArgs.dwNumberOfBytesSent,
                   &ol->ProviderOverlapped,
                    routine
                    );
            if ( SOCKET_ERROR == ret )
            {
                err = WSAGetLastError();
                WSASetLastError( err );
            }
            else
            {
                ret = NO_ERROR;
            }
            break;

        default:
            dbgprint(L"ExecuteOverlappedOperation: Unknown operation!");
            ret = SOCKET_ERROR;
            break;
    }

    if ( ( NO_ERROR != ret ) && ( WSA_IO_PENDING != err ) )
    {
        //
        // If the call immediately fails, update the OVERLAPPED info and return
        //
        ol->lpCallerOverlapped->Offset       = (lpdwFlags ? *lpdwFlags : 0);
        ol->lpCallerOverlapped->OffsetHigh   = err;
        ol->lpCallerOverlapped->InternalHigh = (lpdwBytes ? *lpdwBytes : 0);
        ol->lpCallerOverlapped->Internal     = 0;

        dbgprint(_T("Overlap op failed immediately: ret %X Err %X"), ret,err);
		//do after io complete operations 
		OnOverlappedIoComplete(ret,ol);
		

     //   CheckForContextCleanup( ol );
		AcquireSocketLock( );
		ReleaseAsync();
		ReleaseSocketLock();
        gOverlapManager.FreeOverlappedStructure( ol );
    }
    else if ( ( NO_ERROR == ret ) && ( FALSE == m_bSynchronous ) )
    {
        // 
        // NOTE: We could return success from here, but if you want to perform
        //       some post processing on the data buffers before indicating
        //       success to the upper layer (because once success is returned, the
        //       upper layer can inspect the buffers). Because of this we return
        //       pending and in the completion processing (either via IOCP or
        //       APC), we can do post processing there.
        //
        err = WSA_IO_PENDING;
        ret = SOCKET_ERROR;
    }
    else if ( ( NO_ERROR == ret ) && ( TRUE == m_bSynchronous ) )
    {
        // The winsock call actually blocked and there will be no completion
        // notification on the IOCP.
        //
        dbgprint(_T("Succeeded without error - synchronous socket though"));

    }

	if(ret != NO_ERROR)
	{
		*lpErrno=err;
	}
    return ( ( NO_ERROR == ret ) ? ret : SOCKET_ERROR );
}
void CLSPOverlappedSocket::OnOverlappedIoComplete(DWORD dwError,WSAOVERLAPPEDPLUS *olp)
{
	switch ( olp->Operation )
    {
		case LSP_OP_RECVFROM:
			OnRecvFromComplete(dwError,
							   olp->RecvFromArgs.blpFromParamChanged,
							   olp->RecvFromArgs.lpActualFrom,
							   olp->RecvFromArgs.lpActualFromLen,
							   olp->RecvFromArgs.lpFrom,
							   olp->RecvFromArgs.lpFromLen
							   );
			break;
		case LSP_OP_ACCEPTEX:  
				if(olp->AcceptExArgs.bParamChanged)
				{
					OnChangeAcceptExParamComplete(
												olp->AcceptExArgs.sProviderAcceptSocket,  
												dwError,
												olp->lpCallerOverlapped->InternalHigh,
												olp->AcceptExArgs.lpActualOutputBuffer ,
												olp->AcceptExArgs.lpOutputBuffer ,
												olp->AcceptExArgs.dwActualReceiveDataLength,
												olp->AcceptExArgs.dwActualLocalAddresslength,
												olp->AcceptExArgs.dwActualRemoteAddressLength  
												);
				}
				break;

	}
}
//
// Function: IntermediateCompletionRoutine
//
// Description:
//    Once an overlapped operation completes we call this routine to keep
//    count of the bytes sent/received as well as to complete the operation
//    to the application.
//
void CALLBACK CLSPOverlappedSocket::IntermediateCompletionRoutine(DWORD dwError,DWORD cbTransferred, LPWSAOVERLAPPEDPLUS olp,DWORD dwFlags)
{
    CLSPSocket				*AcceptSocketContext = NULL;
    int						Error,
							ret;

  
    if ( WSA_IO_PENDING == dwError )
    {
        //
        // Get the results of the operation
        //

        ASSERT( olp->Provider );
        ASSERT( olp->Provider->NextProcTable.lpWSPGetOverlappedResult );

        dwError = NO_ERROR;
        ret = olp->Provider->NextProcTable.lpWSPGetOverlappedResult(
                olp->ProviderSocket,
                &olp->ProviderOverlapped,
               &cbTransferred,
                FALSE,
               &dwFlags,
                (int *)&dwError
                );
 
        if ( FALSE == ret )
        {
            dbgprint(_T("IntermediateCompletionRoutine: WSPGetOverlappedResult failed: %d"), dwError);
        }
        else
        {

            dbgprint(_T("Bytes transferred on socket 0x%x: %d [op=%d; err=%d]"), 
                    olp->CallerSocket, cbTransferred, olp->Operation, dwError);
        }
    }

	try{
		olp->lpCallerOverlapped->Offset       = dwFlags;
		olp->lpCallerOverlapped->OffsetHigh   = dwError;
		olp->lpCallerOverlapped->InternalHigh = cbTransferred;
	}
	catch(...)
	{
		//user has exit his thread
		dwError= WSAEFAULT;
		if ( NULL != olp )
			gOverlapManager.FreeOverlappedStructure( olp );
		return;
	}

    m_LastError = dwError;

    if ( 0 == dwError )
    {
        AcquireSocketLock(  );

        //
        // NOTE: This is where any post processing should go for overlapped operations.
        //       For example, if you wanted to inspect the data received, you would do
        //       that here for any operation that receives data (don't forget AcceptEx!).
        //       In this sample, all we do is count the bytes sent and received on the
        //       socket.
        //

        switch ( olp->Operation )
        {
            case LSP_OP_RECV:
                BytesRecv += cbTransferred;
                break;

            case LSP_OP_RECVFROM:
                BytesRecv += cbTransferred;
                break;

            case LSP_OP_SEND:
                BytesSent += cbTransferred;
                break;

            case LSP_OP_SENDTO:
                BytesSent += cbTransferred;
                break;

            case LSP_OP_TRANSMITFILE:
                BytesSent += cbTransferred;
                break;

            case LSP_OP_TRANSMITPACKETS:
                BytesSent += cbTransferred;
                break;

            case LSP_OP_ACCEPTEX:

                ReleaseSocketLock();

                AcceptSocketContext = theInstance.GetSockFactoryInstance()->LookupSocketObj(
																						olp->AcceptExArgs.sAcceptSocket,
																					   &Error
																					   );
                if (AcceptSocketContext == NULL)
                {
                    dbgprint( _T("IntermediateCompletionRoutine: LookupSocketObj failed! (LSP_OP_ACCEPTEX)") );
                    dwError = WSAENOTSOCK;
                    olp->lpCallerOverlapped->OffsetHigh = dwError;
                }
                else
                {
					AcceptSocketContext->ReleaseAtomicAsync();
					AcceptSocketContext->Release();
                }

                break;

            case LSP_OP_CONNECTEX:
                BytesSent += cbTransferred;
                break;

            case LSP_OP_WSARECVMSG:
                BytesRecv += cbTransferred;
                break;

			case LSP_OP_WSASENDMSG:
				BytesSent += cbTransferred;
				break;

            default:
                break;
        }
        // Already released for AcceptEx operations
        if ( LSP_OP_ACCEPTEX != olp->Operation )
            ReleaseSocketLock(  );
    }

	//do after io complete operations 
	OnOverlappedIoComplete(dwError,olp);

    if ( NULL != olp->lpCallerCompletionRoutine )
    {
        //
        // If the app supplied a completion routine, queue it up for completion
        //
        olp->lpCallerOverlapped->Internal = (ULONG_PTR)olp->lpCallerCompletionRoutine;

        ret = gMainUpCallTable.lpWPUQueueApc(
               &olp->CallerThreadId,
               CallUserApcProc,
               (DWORD_PTR) olp->lpCallerOverlapped,
              &Error
               );
        if ( SOCKET_ERROR == ret )
        {
            dbgprint(_T("IntermediateCompletionRoutine: WPUQueueApc() failed: %d"), Error);
        }
    }
    else
    {
        //
        // Otherwise we signal that the op has completed
        //
        ret = WPUCompleteOverlappedRequest(
                olp->CallerSocket,
                olp->lpCallerOverlapped,
                dwError,
                cbTransferred,
               &Error
                );
        if ( SOCKET_ERROR == ret )
        {
            dbgprint(_T("WPUCompleteOverlappedRequest failed: %d (provider socket 0x%x)"), 
                    Error, olp->CallerSocket );
        }
    }

    if ( ( NULL != olp ) && ( TRUE == olp->CloseThread ) )
    {
        ret = gMainUpCallTable.lpWPUCloseThread( &olp->CallerThreadId, &Error );
        if ( SOCKET_ERROR == ret )
        {
            dbgprint(_T("WPUCloseThread failed: %d"), Error );
        }
        olp->CloseThread = FALSE;
    }

    //
    // Cleanup the accounting on the socket
    //

    //CheckForContextCleanup( olp );
    AcquireSocketLock( );
    ReleaseAsync();
    ReleaseSocketLock();

	Release();//add ref by overlapped manager

    if ( NULL != olp )
        gOverlapManager.FreeOverlappedStructure( olp );

    return;
}



//
// Function: CallerUserApcProc
//
// Description:
//    This function completes an overlapped request that supplied an APC.
//
VOID CALLBACK CLSPOverlappedSocket::CallUserApcProc(ULONG_PTR   Context)
{
    LPOVERLAPPED                        lpOverlapped;
    LPWSAOVERLAPPED_COMPLETION_ROUTINE  UserCompletionRoutine;

    lpOverlapped = (LPOVERLAPPED) Context;
    UserCompletionRoutine  = (LPWSAOVERLAPPED_COMPLETION_ROUTINE)lpOverlapped->Internal;
    lpOverlapped->Internal = 0; // Remove the WSS_OPERATION_IN_PROGRESS value 

    UserCompletionRoutine(
            (DWORD)lpOverlapped->OffsetHigh,
            (DWORD)lpOverlapped->InternalHigh,
            lpOverlapped,
            (DWORD)lpOverlapped->Offset
            );
    return;
}

//
// Function: CheckForContextCleanup
//
// Description:
//    After an overlapped operation completes (or from WSPCloseSocket) we
//    need to see if the socket has been marked for closure and make sure
//    no one else is referencing it. If so then the associated structures
//    can be freed and the socket closed.
//
void CLSPOverlappedSocket::CheckForContextCleanup(WSAOVERLAPPEDPLUS  *ol)
{
    CLSPSocket				*pThis = NULL;
	int						Error,
							ret;

	pThis=theInstance.GetSockFactoryInstance()->LookupSocketObj(ol->CallerSocket,&Error);
    if ( NULL == pThis )
    {
        dbgprint( _T("CheckForContextCleanup: FindAndRefSocketContext failed!" ));
        return;
    }
    EnterCriticalSection( &gCriticalSection );

    AcquireSocketLock( );

	ReleaseAsync();



    if ( ( TRUE == m_bClosing ) && 
         ( 0 == m_dwOutstandingAsync ) &&
         ( 1 == m_dwRefCount )
       )
    {

        //
        // If the calling app closed the socket while there were still outstanding
        //  async operations then all the outstanding operations have completed so
        //  we can close the apps socket handle.
        //
          ret = gMainUpCallTable.lpWPUCloseSocketHandle(
                ol->CallerSocket, 
               &ol->Error
                );
        if ( SOCKET_ERROR == ret )
        {
            dbgprint(_T("CheckForContextClenaup: WPUCloseSocketHandle() failed: %d"), ol->Error);
        }

        m_CallerSocket = INVALID_SOCKET;

		theInstance.GetSockFactoryInstance()->RemoveSocketObject(m_NextProvider,this );

        dbgprint(_T("CheckForContxtCleanup: Closing socket %d Bytes Sent [%I64d] Bytes Recv [%I64d]"), 
                ol->CallerSocket, BytesSent, BytesRecv);

        ReleaseSocketLock( );

        ol->SockInfo = NULL;

        theInstance.GetSockFactoryInstance()->FreeSockObject( this );
        goto cleanup;
    }
    ReleaseSocketLock();

	Release();

cleanup:
    LeaveCriticalSection( &gCriticalSection );
    return;
}


int WSPAPI CLSPOverlappedSocket::WSPSendTo(
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
	LPWSAOVERLAPPEDPLUS ProviderOverlapped = NULL;
	//
    // Check for overlapped
    //
    if ( NULL != lpOverlapped )
    {
        ProviderOverlapped = PrepareOverlappedOperation(
														LSP_OP_SENDTO,
														lpBuffers,
														dwBufferCount,
														lpOverlapped,
														lpCompletionRoutine,
														lpThreadId,
														lpErrno
														);
        if ( NULL == ProviderOverlapped )
            goto cleanup;

        __try 
        {
            ProviderOverlapped->SendToArgs.dwFlags             = dwFlags;
            ProviderOverlapped->SendToArgs.iToLen              = iToLen;
            ProviderOverlapped->SendToArgs.dwNumberOfBytesSent = (lpNumberOfBytesSent ? *lpNumberOfBytesSent : 0);
            if ( iToLen <= sizeof( ProviderOverlapped->SendToArgs.To ) )
                CopyMemory(&ProviderOverlapped->SendToArgs.To, lpTo, iToLen);
        }
        __except( EXCEPTION_EXECUTE_HANDLER )
        {
            // Return to original state and indicate error
            *lpErrno = WSAEFAULT;
            goto cleanup;
        }

        //
        // Make the overlapped call which will always fail (either with WSA_IO_PENDING
        // or actual error code if something is wrong).
        //
		InitSockOverlap();
	    ret = gOverlapManager.QueueOverlappedOperation(ProviderOverlapped->lpCallerOverlapped ,lpErrno);
        if ( NO_ERROR == ret )
        {
			ret=ExecuteOverlappedOperation(ProviderOverlapped,lpErrno);
        }
		
    }
	else
	{
		ret= CLSPSocket::WSPSendTo(lpBuffers,dwBufferCount,lpNumberOfBytesSent,dwFlags,
									lpTo,iToLen,
									lpOverlapped,
									lpCompletionRoutine, lpThreadId,
									lpErrno
									);
	}
cleanup:
	return ret;
}


 int WSPAPI CLSPOverlappedSocket::WSPSend(
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
	LPWSAOVERLAPPEDPLUS ProviderOverlapped = NULL;

    //
    // Check for overlapped I/O
    // 
    if ( NULL != lpOverlapped )
    {
        ProviderOverlapped = PrepareOverlappedOperation(
														LSP_OP_SEND,
														lpBuffers,
														dwBufferCount,
														lpOverlapped,
														lpCompletionRoutine,
														lpThreadId,
														lpErrno
														);
        if ( NULL == ProviderOverlapped )
        {
            goto cleanup;
        }

        __try 
        {
            ProviderOverlapped->SendArgs.dwFlags       = dwFlags;
        }
        __except( EXCEPTION_EXECUTE_HANDLER )
        {
            // Return to original state and indicate error
            *lpErrno = WSAEFAULT;
            goto cleanup;
        }

        //
        // Make the overlapped call which will always fail (either with WSA_IO_PENDING
        // or actual error code if something is wrong).
        //
		InitSockOverlap();
	    ret = gOverlapManager.QueueOverlappedOperation(ProviderOverlapped->lpCallerOverlapped ,lpErrno);
        if ( NO_ERROR == ret )
        {
			ret=ExecuteOverlappedOperation(ProviderOverlapped,lpErrno);
        }
    }
    else
    {
		ret = CLSPSocket::WSPSend(
									lpBuffers, 
									dwBufferCount,
									lpNumberOfBytesSent, 
									dwFlags, 
									lpOverlapped, 
									lpCompletionRoutine, 
									lpThreadId, 
									lpErrno
									);

    }
cleanup:
	return ret;
 }

int WSPAPI 	CLSPOverlappedSocket::WSPRecvFrom(
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
	LPWSAOVERLAPPEDPLUS ProviderOverlapped = NULL;
	int                 ret = SOCKET_ERROR;
	//
    // Check for overlapped I/O
    //
    if ( NULL != lpOverlapped )
    {
        ProviderOverlapped = PrepareOverlappedOperation(
														LSP_OP_RECVFROM,
														lpBuffers,
														dwBufferCount,
														lpOverlapped,
														lpCompletionRoutine,
														lpThreadId,
														lpErrno
														);
        if ( NULL == ProviderOverlapped )
        {
            goto cleanup;
        }

        __try 
        {
			ProviderOverlapped->RecvFromArgs.lpActualFrom  =lpFrom;
			ProviderOverlapped->RecvFromArgs.lpActualFromLen= lpFromLen;
			ProviderOverlapped->RecvFromArgs.blpFromParamChanged=ChangeRecvFromParams(
														ProviderOverlapped->RecvFromArgs.lpFrom ,
														ProviderOverlapped->RecvFromArgs.lpFromLen );
			if(!ProviderOverlapped->RecvFromArgs.blpFromParamChanged)
			{
				ProviderOverlapped->RecvFromArgs.lpFrom        = lpFrom;
				ProviderOverlapped->RecvFromArgs.lpFromLen     = lpFromLen;
			}
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            // Return to original state and indicate error
            *lpErrno = WSAEFAULT;
            goto cleanup;
        }

        //
        // Make the overlapped call which will always fail (either with WSA_IO_PENDING
        // or actual error code if something is wrong).
        //
		InitSockOverlap();
	    ret = gOverlapManager.QueueOverlappedOperation(ProviderOverlapped->lpCallerOverlapped ,lpErrno);
        if ( NO_ERROR == ret )
        {
			ret=ExecuteOverlappedOperation(ProviderOverlapped,lpErrno);
        }
    }
    else
    {
		 LPSOCKADDR     lpActualFrom=lpFrom;
		 LPINT          lpActualFromLen=lpFromLen;
		 BOOL  bRecvFromParamschanged= ChangeRecvFromParams(lpFrom,lpFromLen);
         ret = CLSPSocket::WSPRecvFrom(
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
        if ( SOCKET_ERROR != ret )
        {
            BytesRecv += *lpNumberOfBytesRecvd;
        }
		OnRecvFromComplete(ret,bRecvFromParamschanged,lpActualFrom,lpActualFromLen,lpFrom,lpFromLen);   
    }
cleanup:
	return ret;
}
BOOL CLSPOverlappedSocket::ChangeRecvFromParams(LPSOCKADDR &lpFrom,LPINT &lpFromLen)
{
	return FALSE;
}
void CLSPOverlappedSocket::OnRecvFromComplete(DWORD dwError,BOOL blpFromParamChanged,LPSOCKADDR lpActualFrom,LPINT lpActualFromLen,LPSOCKADDR lpFrom,LPINT lpFromLen)
{
}
int WSPAPI CLSPOverlappedSocket::WSPRecv(
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
    LPWSAOVERLAPPEDPLUS ProviderOverlapped = NULL;
    int                 ret = SOCKET_ERROR;
   //
    // Check for overlapped I/O
    //
    if ( NULL != lpOverlapped )
    {
        ProviderOverlapped = PrepareOverlappedOperation(
														LSP_OP_RECV,
														lpBuffers,
														dwBufferCount,
														lpOverlapped,
														lpCompletionRoutine,
														lpThreadId,
														lpErrno
														);
        if ( NULL == ProviderOverlapped )
            goto cleanup;

        __try
        {
            ProviderOverlapped->RecvArgs.dwNumberOfBytesRecvd = (lpNumberOfBytesRecvd ? *lpNumberOfBytesRecvd : 0);
            ProviderOverlapped->RecvArgs.dwFlags              = (lpFlags ? *lpFlags : 0);
        }
        __except( EXCEPTION_EXECUTE_HANDLER )
        {
            // Return to original state and indicate error
            *lpErrno = WSAEFAULT;
            goto cleanup;
        }

        //
        // Make the overlapped call which will always fail (either with WSA_IO_PENDING
        // or actual error code if something is wrong).
        //
		InitSockOverlap();
	    ret = gOverlapManager.QueueOverlappedOperation(ProviderOverlapped->lpCallerOverlapped ,lpErrno);
        if ( NO_ERROR == ret )
        {
			ret=ExecuteOverlappedOperation(ProviderOverlapped,lpErrno);
        }
    }
    else
    {
        ret = CLSPSocket::WSPRecv(
									lpBuffers, 
									dwBufferCount,
									lpNumberOfBytesRecvd, 
									lpFlags, 
									lpOverlapped, 
									lpCompletionRoutine, 
									lpThreadId,
									lpErrno);
    }
cleanup:
	return ret;
}

int WSPAPI CLSPOverlappedSocket::WSPIoctl(
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

    GUID                AcceptExGuid = WSAID_ACCEPTEX,
                        TransmitFileGuid = WSAID_TRANSMITFILE,
                        GetAcceptExSockAddrsGuid = WSAID_GETACCEPTEXSOCKADDRS,
                        ConnectExGuid = WSAID_CONNECTEX,
                        DisconnectExGuid = WSAID_DISCONNECTEX,
                        TransmitPacketsGuid = WSAID_TRANSMITPACKETS,
                        WSARecvMsgGuid = WSAID_WSARECVMSG,
						WSASendMsgGuid=WSAID_WSASENDMSG;
    DWORD               dwBytesReturned = 0;
	int                 ret = NO_ERROR;
	LPWSAOVERLAPPEDPLUS ProviderOverlapped = NULL;

	dbgprint(_T("Enter WSPIoctl dwIoControlCode::%X ProviderSocket %X"),dwIoControlCode,m_NextProviderSocket );
   if ( SIO_GET_EXTENSION_FUNCTION_POINTER == dwIoControlCode )
    {
        LPVOID      lpFunction = NULL;


        // Sanity check the buffers
        
        if( cbInBuffer < sizeof(GUID) || lpvInBuffer == NULL ||
            cbOutBuffer < sizeof(LPVOID) || lpvOutBuffer == NULL )        
        {
            ret = SOCKET_ERROR;
            *lpErrno = WSAEFAULT;
            goto cleanup;            
        }
        
        __try
        {
            //
            // Check to see which extension function is being requested.
            //
            if ( 0 == memcmp( lpvInBuffer, &TransmitFileGuid, sizeof( GUID ) ) )
            {
                // Return a pointer to our intermediate extesion function
                dwBytesReturned = sizeof(LPFN_TRANSMITFILE);
				lpFunction = ::ExtTransmitFile;
				
				dbgprint(_T("WSPIoctl ::ExtTransmitFile"));
                // Attempt to load the lower provider's extension function
                if ( NULL == m_NextProvider->NextProcTableExt.lpfnTransmitFile )
                {
                    SetBlockingProvider(m_NextProvider);
                    LoadExtensionFunction(
                            (FARPROC **) &m_NextProvider->NextProcTableExt.lpfnTransmitFile,
                            TransmitFileGuid,
                            m_NextProvider->NextProcTable.lpWSPIoctl,
                            m_NextProviderSocket
                            );
                    SetBlockingProvider(NULL);
                }
            }
            else if ( 0 == memcmp( lpvInBuffer, &AcceptExGuid, sizeof( GUID ) ) )
            {
                // Return a pointer to our intermediate extension function
                dwBytesReturned = sizeof( LPFN_ACCEPTEX );
				lpFunction = ::ExtAcceptEx;

				dbgprint(_T("WSPIoctl ::ExtAcceptEx"));
                // Attempt to load the lower provider's extension function
                if ( NULL == m_NextProvider->NextProcTableExt.lpfnAcceptEx )
                {
                    SetBlockingProvider(m_NextProvider);
                    LoadExtensionFunction(
                            (FARPROC **) &m_NextProvider->NextProcTableExt.lpfnAcceptEx,
                            AcceptExGuid,
                            m_NextProvider->NextProcTable.lpWSPIoctl,
                            m_NextProviderSocket
                            );
                    SetBlockingProvider(NULL);
                }
            }
            else if ( 0 == memcmp( lpvInBuffer, &ConnectExGuid, sizeof( GUID ) ) )
            {
                // Return a pointer to our intermediate extension function
                dwBytesReturned = sizeof( LPFN_CONNECTEX );
				lpFunction = ::ExtConnectEx;

				dbgprint(_T("WSPIoctl ::ExtConnectEx"));
                // Attempt to load the lower provider's extension function
                if ( NULL == m_NextProvider->NextProcTableExt.lpfnConnectEx )
                {
                    SetBlockingProvider(m_NextProvider);
                    LoadExtensionFunction(
                            (FARPROC **) &m_NextProvider->NextProcTableExt.lpfnConnectEx,
                            ConnectExGuid,
                            m_NextProvider->NextProcTable.lpWSPIoctl,
                            m_NextProviderSocket
                            );
                    SetBlockingProvider(NULL);
                }
            }
            else if ( 0 == memcmp( lpvInBuffer, &DisconnectExGuid, sizeof( GUID ) ) )
            {
                // Return a pointer to our intermediate extension function
                dwBytesReturned = sizeof( LPFN_DISCONNECTEX );
				lpFunction = ::ExtDisconnectEx;

				dbgprint(_T("WSPIoctl ::ExtDisconnectEx"));
                // Attempt to load the lower provider's extension function
                if ( NULL == m_NextProvider->NextProcTableExt.lpfnDisconnectEx )
                {
                    SetBlockingProvider(m_NextProvider);
                    LoadExtensionFunction(
                            (FARPROC **) &m_NextProvider->NextProcTableExt.lpfnDisconnectEx,
                            DisconnectExGuid,
                            m_NextProvider->NextProcTable.lpWSPIoctl,
                            m_NextProviderSocket
                            );
                    SetBlockingProvider(NULL);
                }
            }
            else if ( 0 == memcmp( lpvInBuffer, &TransmitPacketsGuid, sizeof( GUID ) ) )
            {
                // Return a pointer to our intermediate extension function
                dwBytesReturned = sizeof( LPFN_TRANSMITPACKETS );
				lpFunction = ::ExtTransmitPackets;

				dbgprint(_T("WSPIoctl ::ExtTransmitPackets"));
                // Attempt to load the lower provider's extension function
                if ( NULL == m_NextProvider->NextProcTableExt.lpfnTransmitPackets )
                {
                    SetBlockingProvider(m_NextProvider);
                    LoadExtensionFunction(
                            (FARPROC **) &m_NextProvider->NextProcTableExt.lpfnTransmitPackets,
                            TransmitPacketsGuid,
                            m_NextProvider->NextProcTable.lpWSPIoctl,
                            m_NextProviderSocket
                            );
                    SetBlockingProvider(NULL);
                }
            }
            else if ( 0 == memcmp( lpvInBuffer, &WSARecvMsgGuid, sizeof( GUID ) ) )
            {
                // Return a pointer to our intermediate extension function
                dwBytesReturned = sizeof( LPFN_WSARECVMSG );
				lpFunction = ::ExtWSARecvMsg;

				dbgprint(_T("WSPIoctl ::WSARecvMsgGuid"));
                // Attempt to load the lower provider's extension function
                if ( NULL == m_NextProvider->NextProcTableExt.lpfnWSARecvMsg )
                {
                    SetBlockingProvider(m_NextProvider);
                    LoadExtensionFunction(
                            (FARPROC **) &m_NextProvider->NextProcTableExt.lpfnWSARecvMsg,
                            WSARecvMsgGuid,
                            m_NextProvider->NextProcTable.lpWSPIoctl,
                            m_NextProviderSocket
                            );
                    SetBlockingProvider(NULL);
                }
            }
			else if ( 0 == memcmp( lpvInBuffer,&WSASendMsgGuid,sizeof( GUID ) ) )
			{
				// Return a pointer to our intermediate extension function
                dwBytesReturned = sizeof( LPFN_WSASENDMSG );
				lpFunction = ::ExtWSASendMsg;

				dbgprint(_T("WSPIoctl ::WSASendMsgGuid"));
                // Attempt to load the lower provider's extension function
                if ( NULL == m_NextProvider->NextProcTableExt.lpfnWSASendMsg )
                {
                    SetBlockingProvider(m_NextProvider);
                    LoadExtensionFunction(
                            (FARPROC **) &m_NextProvider->NextProcTableExt.lpfnWSASendMsg,
                            WSASendMsgGuid,
                            m_NextProvider->NextProcTable.lpWSPIoctl,
                            m_NextProviderSocket
                            );
                    SetBlockingProvider(NULL);
                }
			}
            else if ( 0 == memcmp( lpvInBuffer, &GetAcceptExSockAddrsGuid, sizeof( GUID ) ) )
            {
                // No socket handle translation needed, let the call pass through below
                // (i.e. we really don't have any need to intercept this call)
				dbgprint(_T("WSPIoctl ::GetAcceptExSockAddrsGuid"));
            }
            else 
            {
				//pass lower provider extension directly	
				dbgprint(_T("WSPIoctl ::Notunderstood"));

            /*    ret = SOCKET_ERROR;
                *lpErrno = WSAEINVAL;
                goto cleanup;*/
            }
        }
        __except( EXCEPTION_EXECUTE_HANDLER )
        {
            ret = SOCKET_ERROR;
            *lpErrno = WSAEFAULT;
            goto cleanup;
                    
        }
        //
        // Update the output parameters if successful
        //
        if ( NULL != lpFunction )
        {
            __try 
            {
                *((DWORD_PTR *)lpvOutBuffer) = (DWORD_PTR) lpFunction;
                *lpcbBytesReturned = dwBytesReturned;
            }
            __except( EXCEPTION_EXECUTE_HANDLER )
            {
                ret = SOCKET_ERROR;
                *lpErrno = WSAEFAULT;
            }
            goto cleanup;
        }

        // Only if GetAcceptExSockAddresGuid was passed in will we get here
        // We fall through and make the call to the lower layer

    }
    else if ( SIO_QUERY_TARGET_PNP_HANDLE == dwIoControlCode )
    {
        //
        // If the next layer is another LSP, keep passing. Otherwise return the 
        //    lower provider's handle so it may be used in PNP event notifications.
        //
        if ( m_NextProvider->NextProvider.ProtocolChain.ChainLen != BASE_PROTOCOL )
        {
            __try
            {
                *((SOCKET *)lpvOutBuffer) = m_NextProviderSocket;
                dwBytesReturned = *lpcbBytesReturned = sizeof(m_NextProviderSocket);
            }
            __except( EXCEPTION_EXECUTE_HANDLER )
            {
                ret = SOCKET_ERROR;
                *lpErrno = WSAEFAULT;
                goto cleanup;
            }

            if ( NULL != lpOverlapped )
            {
                ProviderOverlapped = PrepareOverlappedOperation(
																LSP_OP_IOCTL,
																NULL,
																0,
																lpOverlapped,
																lpCompletionRoutine,
																lpThreadId,
																lpErrno
																);
                if ( NULL == ProviderOverlapped )
                    goto cleanup;

                __try
                {
                    ProviderOverlapped->IoctlArgs.cbBytesReturned   = (lpcbBytesReturned ? *lpcbBytesReturned : 0);
                }
                __except( EXCEPTION_EXECUTE_HANDLER )
                {
                    ret = SOCKET_ERROR;
                    *lpErrno = WSAEFAULT;
                    goto cleanup;
                }

                //
                // Call the completion routine immediately since there is nothing
                //  else to do. For this ioctl all we do is return the provider
                //  socket. If it was called overlapped just complete the operation.
                //

                dbgprint(_T("SIO_QUERY_TARGET_PNP_HANDLE overlapped"));

                IntermediateCompletionRoutine(
                        0,
                        dwBytesReturned,
                        ProviderOverlapped,
                        0);
            }

            goto cleanup;
        }
    }


	if(SIO_ROUTING_INTERFACE_QUERY == dwIoControlCode)
	{
		dbgprint(_T("SIO_ROUTING_INTERFACE_QUERY Query %X:%X"),((sockaddr_in*)lpvInBuffer)->sin_addr,((sockaddr_in*)lpvInBuffer)->sin_port);
	}
    //
    // Check for overlapped I/O
    // 
    if ( NULL != lpOverlapped )
    {
        ProviderOverlapped = PrepareOverlappedOperation(
														LSP_OP_IOCTL,
														NULL,
														0,
														lpOverlapped,
														lpCompletionRoutine,
														lpThreadId,
														lpErrno
														);
        if ( NULL == ProviderOverlapped )
        {
            ret = SOCKET_ERROR;
            // lpErrno is set by PrepareOverlappedOperation
            goto cleanup;
        }

        __try
        {
            ProviderOverlapped->IoctlArgs.cbBytesReturned = (lpcbBytesReturned ? *lpcbBytesReturned : 0);
            ProviderOverlapped->IoctlArgs.dwIoControlCode = dwIoControlCode;
            ProviderOverlapped->IoctlArgs.lpvInBuffer     = lpvInBuffer;
            ProviderOverlapped->IoctlArgs.cbInBuffer      = cbInBuffer;
            ProviderOverlapped->IoctlArgs.lpvOutBuffer    = lpvOutBuffer;
            ProviderOverlapped->IoctlArgs.cbOutBuffer     = cbOutBuffer;
        }
        __except( EXCEPTION_EXECUTE_HANDLER )
        {
            ret = SOCKET_ERROR;
            *lpErrno = WSAEFAULT;
            goto cleanup;
        }

        //
        // Make the overlapped call which will always fail (either with WSA_IO_PENDING
        // or actual error code if something is wrong).
        //
		InitSockOverlap();
	    ret = gOverlapManager.QueueOverlappedOperation(ProviderOverlapped->lpCallerOverlapped ,lpErrno);
        if ( NO_ERROR == ret )
        {
			ret=ExecuteOverlappedOperation(ProviderOverlapped,lpErrno);
        }
    }
    else
    {
        
      ret=  CLSPSocket::WSPIoctl(
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
	  dbgprint(_T(" CLSPSocket::WSPIoctl IOcode %X ret %X"),dwIoControlCode,ret);

    }

cleanup:
	return ret;
}

BOOL PASCAL FAR CLSPOverlappedSocket::ExtTransmitFile(
											IN HANDLE hFile,
											IN DWORD nNumberOfBytesToWrite,
											IN DWORD nNumberOfBytesPerSend,
											IN LPOVERLAPPED lpOverlapped,
											IN LPTRANSMIT_FILE_BUFFERS lpTransmitBuffers,
											IN DWORD dwFlags,
											INT   &Errno
											)
{
    LPWSAOVERLAPPEDPLUS		ProviderOverlapped = NULL;
    int                     ret = FALSE;

    // Make sure we already have the extension function
    if ( NULL == m_NextProvider->NextProcTableExt.lpfnTransmitFile )
    {
        GUID    guidTransmitFile = WSAID_TRANSMITFILE;

        ret = LoadExtensionFunction(
                (FARPROC **)&m_NextProvider->NextProcTableExt.lpfnTransmitFile,
                guidTransmitFile,
                m_NextProvider->NextProcTable.lpWSPIoctl,
                m_NextProviderSocket
                );
        if ( FALSE == ret )
        {
            dbgprint( _T("Next provider's TransmitFile pointer is NULL!" ));
            Errno = WSAEFAULT;
            goto cleanup;
        }   
    }

    //
    // Check for overlapped I/O
    //
    if ( NULL != lpOverlapped )
    {
        ProviderOverlapped = PrepareOverlappedOperation(
                LSP_OP_TRANSMITFILE,
                NULL,
                0,
                lpOverlapped,
                NULL,
                NULL,
               &Errno
                );
        if ( NULL == ProviderOverlapped )
        {
            goto cleanup;
        }

        // Save the arguments
        ProviderOverlapped->TransmitFileArgs.hFile                 = hFile;
        ProviderOverlapped->TransmitFileArgs.nNumberOfBytesToWrite = nNumberOfBytesToWrite;
        ProviderOverlapped->TransmitFileArgs.nNumberOfBytesPerSend = nNumberOfBytesPerSend;
        ProviderOverlapped->TransmitFileArgs.lpTransmitBuffers     = lpTransmitBuffers;
        ProviderOverlapped->TransmitFileArgs.dwFlags               = dwFlags;

		InitSockOverlap();
	    ret = gOverlapManager.QueueOverlappedOperation(ProviderOverlapped->lpCallerOverlapped ,&Errno);
        if ( NO_ERROR == ret )
        {
			ret=ExecuteOverlappedOperation(ProviderOverlapped,&Errno);
        }
        if ( NO_ERROR != ret )
        {
            ret = FALSE;
        }
        else
        {
            ret = TRUE;
        }
    }
    else
    {
		ret = CLSPSocket::ExtTransmitFile(
                hFile,
                nNumberOfBytesToWrite,
                nNumberOfBytesPerSend,
                lpOverlapped,
                lpTransmitBuffers,
                dwFlags,
				Errno
                );
    }

cleanup:
	return ret;
}
BOOL PASCAL FAR CLSPOverlappedSocket::ExtAcceptEx(
										CLSPSocket *AcceptSocketContext,
										IN SOCKET sCallerAcceptSocket,
										IN PVOID lpOutputBuffer,
										IN DWORD dwReceiveDataLength,
										IN DWORD dwLocalAddressLength,
										IN DWORD dwRemoteAddressLength,
										OUT LPDWORD lpdwBytesReceived,
										IN LPOVERLAPPED lpOverlapped,
										INT   &Errno
										)
{
    LPWSAOVERLAPPEDPLUS ProviderOverlapped = NULL;
    int                 ret = FALSE;
    // Make sure we already have the extension function
    if ( NULL == m_NextProvider->NextProcTableExt.lpfnAcceptEx )
    {
        GUID    guidAcceptEx = WSAID_ACCEPTEX;

        ret = LoadExtensionFunction(
                 (FARPROC **)&m_NextProvider->NextProcTableExt.lpfnAcceptEx,
                 guidAcceptEx,
                 m_NextProvider->NextProcTable.lpWSPIoctl,
                 m_NextProviderSocket
                 );
        if ( FALSE == ret )
        {
            dbgprint(_T("Lower provider AcceptEx == NULL!"));
            Errno = WSAEFAULT;
            goto cleanup;
        }
    }

    // Check for overlapped I/O
    if ( NULL != lpOverlapped )
    {
        ProviderOverlapped = PrepareOverlappedOperation(
                LSP_OP_ACCEPTEX,
                NULL,
                0,
                lpOverlapped,
                NULL,
                NULL,
               &Errno
                );
        if ( NULL == ProviderOverlapped )
        {
            goto cleanup;
        }

        __try
        {
            // Save the arguments
            ProviderOverlapped->AcceptExArgs.dwBytesReceived       = (lpdwBytesReceived ? *lpdwBytesReceived : 0);
            ProviderOverlapped->AcceptExArgs.sAcceptSocket         = sCallerAcceptSocket;
            ProviderOverlapped->AcceptExArgs.sProviderAcceptSocket = AcceptSocketContext->GetProviderSocket();
            ProviderOverlapped->AcceptExArgs.lpOutputBuffer        = lpOutputBuffer;
            ProviderOverlapped->AcceptExArgs.dwReceiveDataLength   = dwReceiveDataLength;
            ProviderOverlapped->AcceptExArgs.dwLocalAddressLength  = dwLocalAddressLength;
            ProviderOverlapped->AcceptExArgs.dwRemoteAddressLength = dwRemoteAddressLength;

			//allow to change this param
			ProviderOverlapped->AcceptExArgs.lpActualOutputBuffer = lpOutputBuffer;
			ProviderOverlapped->AcceptExArgs.dwActualReceiveDataLength = dwReceiveDataLength;
			ProviderOverlapped->AcceptExArgs.dwActualLocalAddresslength = dwLocalAddressLength;
			ProviderOverlapped->AcceptExArgs.dwActualRemoteAddressLength = dwRemoteAddressLength;

			ProviderOverlapped->AcceptExArgs.bParamChanged=ChangeAcceptExParam(
								AcceptSocketContext,
								ProviderOverlapped->AcceptExArgs.sProviderAcceptSocket,
								ProviderOverlapped->AcceptExArgs.lpOutputBuffer,
								ProviderOverlapped->AcceptExArgs.dwReceiveDataLength,
								ProviderOverlapped->AcceptExArgs.dwLocalAddressLength,
								ProviderOverlapped->AcceptExArgs.dwRemoteAddressLength
								);
        }
        __except( EXCEPTION_EXECUTE_HANDLER )
        {
            Errno = WSAEFAULT;
            goto cleanup;
        }

        // Add extra reference on accept socket - there is a race condition in the
        // overlapped completion routine under the following conditions:
        // 1. AcceptEx is pending
        // 2. App closes the accept socket
        // 3. IntermeidateCompletionRoutine processes the failed AcceptEx call and
        //    finds the accept socket context before #2 removes it
        // 4. Step #2 completes and now the context the IntermediateCompletionRoutine
        //    points to freed memory

        AcceptSocketContext->ReleaseAtomicAsync();

		InitSockOverlap();
	    ret = gOverlapManager.QueueOverlappedOperation(ProviderOverlapped->lpCallerOverlapped ,&Errno);
        if ( NO_ERROR == ret )
        {
			ret=ExecuteOverlappedOperation(ProviderOverlapped,&Errno);
        }
        if ( NO_ERROR != ret )
        {
            ret = FALSE;
        }
        else
        {
            ret = TRUE;
        }
    }
    else
    {
		PVOID	lpActualBuffer=	lpOutputBuffer;
		DWORD   dwActualReceiveDataLength=dwReceiveDataLength,
				dwActualLocalAddrlen=dwLocalAddressLength,
				dwActualRemoteAddrLen=dwRemoteAddressLength;
		SOCKET  NewAcceptProviderSocket=AcceptSocketContext->GetProviderSocket() ;
		BOOL	bParamChanged=ChangeAcceptExParam(AcceptSocketContext,NewAcceptProviderSocket ,lpOutputBuffer,dwReceiveDataLength,dwLocalAddressLength,dwRemoteAddressLength);   

		ret = CLSPSocket::ExtAcceptEx(
                AcceptSocketContext,
				NewAcceptProviderSocket,
                lpOutputBuffer,
                dwReceiveDataLength,
                dwLocalAddressLength,
                dwRemoteAddressLength,
                lpdwBytesReceived,
                lpOverlapped,
				Errno
                );
		if(bParamChanged)
		{
			OnChangeAcceptExParamComplete(
										  NewAcceptProviderSocket,
										  ret,
										  lpdwBytesReceived?*lpdwBytesReceived:0,
										  lpActualBuffer,
										  lpOutputBuffer,
										  dwActualReceiveDataLength ,
										  dwActualLocalAddrlen,
										  dwActualRemoteAddrLen 
										  );
		}
    }
cleanup:
	return ret;
}
BOOL CLSPOverlappedSocket::ChangeAcceptExParam(
											  CLSPSocket *AcceptSocketContext,	
											  SOCKET &AcceptProviderSocket,
											   PVOID &lpOutputBuffer,
											   DWORD &dwReceiveDataLength,
											   DWORD &dwLocalAddressLength,
											   DWORD &dwRemoteAddressLength)
{
	return FALSE;
}
void CLSPOverlappedSocket::OnChangeAcceptExParamComplete(
											  SOCKET  AcceptProviderSocket,
											  DWORD dwError,
											  DWORD dwBytesTransferred,
											  PVOID lpActualOutputBuffer,
											  PVOID lpAppReturnedBuffer,
										  	  DWORD dwActualRecvLen,
											  DWORD dwActualLocalAddrLen,
											  DWORD dwActualRemoteAddrLen
											  )
{
}
BOOL PASCAL FAR CLSPOverlappedSocket::ExtConnectEx(
										IN const struct sockaddr FAR *name,
										IN int namelen,
										IN PVOID lpSendBuffer OPTIONAL,
										IN DWORD dwSendDataLength,
										OUT LPDWORD lpdwBytesSent,
										IN LPOVERLAPPED lpOverlapped,
										INT   &Errno
										)
{
	LPWSAOVERLAPPEDPLUS  ProviderOverlapped = NULL;
    int                  ret = FALSE;

  // Make sure we already have the extension function
    if ( NULL == m_NextProvider->NextProcTableExt.lpfnConnectEx )
    {
        GUID    guidConnectEx = WSAID_CONNECTEX;

        ret = LoadExtensionFunction(
                (FARPROC **)&m_NextProvider->NextProcTableExt.lpfnConnectEx,
                guidConnectEx,
                m_NextProvider->NextProcTable.lpWSPIoctl,
                m_NextProviderSocket
                );
        if ( FALSE == ret )
        {
            dbgprint(_T("Next proc table ConnectEx == NULL!"));
            Errno = WSAEFAULT;
            goto cleanup;
        }
    }

    // Check for overlapped I/O
    if ( NULL != lpOverlapped )
    {
        ProviderOverlapped = PrepareOverlappedOperation(
                LSP_OP_CONNECTEX,
                NULL,
                0,
                lpOverlapped,
                NULL,
                NULL,
               &Errno
                );
        if ( NULL == ProviderOverlapped )
        {
            dbgprint(_T("ExtConnectEx: PrepareOverlappedOperation returned NULL"));
            goto cleanup;
        }

        __try
        {
            ProviderOverlapped->ConnectExArgs.s                = m_CallerSocket;
            ProviderOverlapped->ConnectExArgs.namelen          = namelen;
            ProviderOverlapped->ConnectExArgs.lpSendBuffer     = lpSendBuffer;
            ProviderOverlapped->ConnectExArgs.dwSendDataLength = dwSendDataLength;
            ProviderOverlapped->ConnectExArgs.dwBytesSent      = (lpdwBytesSent ? *lpdwBytesSent : 0);
            if ( namelen <= sizeof( ProviderOverlapped->ConnectExArgs.name ) )
                CopyMemory( &ProviderOverlapped->ConnectExArgs.name, name, namelen );
        }
        __except( EXCEPTION_EXECUTE_HANDLER )
        {
            Errno = WSAEFAULT;
            goto cleanup;
        }

		InitSockOverlap();
	    ret = gOverlapManager.QueueOverlappedOperation(ProviderOverlapped->lpCallerOverlapped ,&Errno);
        if ( NO_ERROR == ret )
        {
			ret=ExecuteOverlappedOperation(ProviderOverlapped,&Errno);
        }

        if ( NO_ERROR != ret )
        {
            ret = FALSE;
        }
        else
        {
            ret = TRUE;
        }
    }
    else
    {
		ret = CLSPSocket::ExtConnectEx(
                name,
                namelen,
                lpSendBuffer,
                dwSendDataLength,
                lpdwBytesSent,
                lpOverlapped,
				Errno
                );
    }
cleanup:
	return ret;
}
BOOL PASCAL FAR CLSPOverlappedSocket::ExtTransmitPackets(
										LPTRANSMIT_PACKETS_ELEMENT lpPacketArray,
										DWORD nElementCount,
										DWORD nSendSize,
										LPOVERLAPPED lpOverlapped,
										DWORD dwFlags,
										INT   &Errno)
{
   LPWSAOVERLAPPEDPLUS  ProviderOverlapped = NULL;
    int                  ret = FALSE;

   // Make sure we already have the extension function
    if ( NULL == m_NextProvider->NextProcTableExt.lpfnTransmitPackets )
    {
        GUID    guidTransmitPackets = WSAID_TRANSMITPACKETS;

        ret = LoadExtensionFunction(
                (FARPROC **)&m_NextProvider->NextProcTableExt.lpfnTransmitPackets,
                guidTransmitPackets,
                m_NextProvider->NextProcTable.lpWSPIoctl,
                m_NextProviderSocket
                );
        if ( FALSE == ret )
        {
            dbgprint( _T("Next provider's TransmitPackets function is NULL!") );
            Errno = WSAEFAULT;
            goto cleanup;
        }
    }

    //
    // Check for overlapped I/O
    //
    if ( NULL != lpOverlapped )
    {
        ProviderOverlapped = PrepareOverlappedOperation(
                LSP_OP_TRANSMITPACKETS,
                NULL,
                0,
                lpOverlapped,
                NULL,
                NULL,
               &Errno
                );
        if ( NULL == ProviderOverlapped )
        {
            dbgprint(_T("ExtTransmitPackets: PrepareOverlappedOperation returned NULL"));
            goto cleanup;
        }
        
        ProviderOverlapped->lpCallerCompletionRoutine         = NULL;
        ProviderOverlapped->TransmitPacketsArgs.s             = m_CallerSocket;
        ProviderOverlapped->TransmitPacketsArgs.lpPacketArray = lpPacketArray;
        ProviderOverlapped->TransmitPacketsArgs.nElementCount = nElementCount;
        ProviderOverlapped->TransmitPacketsArgs.nSendSize     = nSendSize;
        ProviderOverlapped->TransmitPacketsArgs.dwFlags       = dwFlags;
	
		InitSockOverlap();
	    ret = gOverlapManager.QueueOverlappedOperation(ProviderOverlapped->lpCallerOverlapped ,&Errno);
        if ( NO_ERROR == ret )
        {
			ret=ExecuteOverlappedOperation(ProviderOverlapped,&Errno);
        }

        if ( NO_ERROR != ret )
        {
            ret = FALSE;
        }
        else
        {
            ret = TRUE;
        }
    }
    else
    {
        ret = CLSPSocket::ExtTransmitPackets(
                lpPacketArray,
                nElementCount,
                nSendSize,
                lpOverlapped,
                dwFlags,
				Errno
                );
    }
cleanup:
	return ret;
}

BOOL PASCAL FAR CLSPOverlappedSocket::ExtDisconnectEx(
										IN LPOVERLAPPED lpOverlapped,
										IN DWORD  dwFlags,
										IN DWORD  dwReserved,
										INT   &Errno
										)
{
    LPWSAOVERLAPPEDPLUS  ProviderOverlapped = NULL;
    int                  ret = FALSE;

  // Make sure we already have the extension function
    if ( NULL == m_NextProvider->NextProcTableExt.lpfnDisconnectEx )
    {
        GUID    guidDisconnectEx = WSAID_DISCONNECTEX;

        ret = LoadExtensionFunction(
                 (FARPROC **)&m_NextProvider->NextProcTableExt.lpfnDisconnectEx,
                 guidDisconnectEx,
                 m_NextProvider->NextProcTable.lpWSPIoctl,
                 m_NextProviderSocket
                 );
        if ( FALSE == ret )
        {
            dbgprint( _T("Next provider's DisconnectEx function is NULL!" ));
            Errno = WSAEFAULT;
            goto cleanup;
        }
    }

    // Check for overlapped I/O

    if ( NULL != lpOverlapped )
    {
        ProviderOverlapped = PrepareOverlappedOperation(
                LSP_OP_DISCONNECTEX,
                NULL,
                0,
                lpOverlapped,
                NULL,
                NULL,
               &Errno
                );
        if ( NULL == ProviderOverlapped )
        {
            dbgprint(_T("ExtDisconnectEx: PrepareOverlappedOperation returned NULL"));
            goto cleanup;
        }

        ProviderOverlapped->DisconnectExArgs.s          = m_CallerSocket;
        ProviderOverlapped->DisconnectExArgs.dwFlags    = dwFlags;
        ProviderOverlapped->DisconnectExArgs.dwReserved = dwReserved;

 		InitSockOverlap();
	    ret = gOverlapManager.QueueOverlappedOperation(ProviderOverlapped->lpCallerOverlapped ,&Errno);
        if ( NO_ERROR == ret )
        {
			ret=ExecuteOverlappedOperation(ProviderOverlapped,&Errno);
        }

        if ( NO_ERROR != ret )
        {
            ret = FALSE;
        }
        else
        {
            ret = TRUE;
        }
    }
    else
    {
		ret = CLSPSocket::ExtDisconnectEx(
                lpOverlapped,
                dwFlags,
                dwReserved,
				Errno
                );
    }
cleanup:
	return ret;
}
INT PASCAL FAR CLSPOverlappedSocket::ExtWSARecvMsg(
									IN OUT LPWSAMSG lpMsg,
									OUT LPDWORD lpdwNumberOfBytesRecvd,
									IN LPWSAOVERLAPPED lpOverlapped,
									IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
									INT   &Errno
									)
{
    int                 ret = SOCKET_ERROR;
    LPWSAOVERLAPPEDPLUS  ProviderOverlapped = NULL;

   // Make sure we already have the extension function
    if ( NULL == m_NextProvider->NextProcTableExt.lpfnWSARecvMsg )
    {
        GUID    guidWSARecvMsg = WSAID_WSARECVMSG;

        ret = LoadExtensionFunction(
                (FARPROC **)&m_NextProvider->NextProcTableExt.lpfnWSARecvMsg,
                guidWSARecvMsg,
                m_NextProvider->NextProcTable.lpWSPIoctl,
                m_NextProviderSocket
                );
        if ( FALSE == ret )
        {
            dbgprint(_T("Next proc table WSARecvMsg == NULL!"));
            Errno = WSAEFAULT;
            goto cleanup;
        }
    }

    //
    // Check for overlapped I/O
    //
    if ( NULL != lpOverlapped )
    {
        ProviderOverlapped = PrepareOverlappedOperation(
                LSP_OP_WSARECVMSG,
                NULL,
                0,
                lpOverlapped,
                lpCompletionRoutine,
                NULL,
               &Errno
                );
        if ( NULL == ProviderOverlapped )
        {
            dbgprint(_T("ExtWSARecvMsg: PrepareOverlappedOperation returned NULL"));
            goto cleanup;
        }

        __try 
        {
            ProviderOverlapped->WSARecvMsgArgs.dwNumberOfBytesRecvd = (lpdwNumberOfBytesRecvd ? *lpdwNumberOfBytesRecvd : 0);
            ProviderOverlapped->WSARecvMsgArgs.s                    = m_CallerSocket;
            ProviderOverlapped->WSARecvMsgArgs.lpMsg                = lpMsg;
        }
        __except( EXCEPTION_EXECUTE_HANDLER )
        {
            Errno = WSAEFAULT;
            goto cleanup;
        }
 		InitSockOverlap();
	    ret = gOverlapManager.QueueOverlappedOperation(ProviderOverlapped->lpCallerOverlapped ,&Errno);
        if ( NO_ERROR == ret )
        {
			ret=ExecuteOverlappedOperation(ProviderOverlapped,&Errno);
        }
        if ( NO_ERROR != ret )
        {
            ret = SOCKET_ERROR;
        }
    }
    else
    {
        ret = CLSPSocket::ExtWSARecvMsg(
                lpMsg,
                lpdwNumberOfBytesRecvd,
                NULL,
                NULL,
				Errno
                );
    }
cleanup:
	return ret;
}
INT PASCAL FAR CLSPOverlappedSocket::ExtWSASendMsg(
									  IN   LPWSAMSG lpMsg,
									  IN   DWORD dwFlags,
									  OUT  LPDWORD lpNumberOfBytesSent,
									  IN   LPWSAOVERLAPPED lpOverlapped,
									  IN   LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
									  INT   &Errno
									  )
{
    int                 ret = SOCKET_ERROR;
    LPWSAOVERLAPPEDPLUS  ProviderOverlapped = NULL;

   // Make sure we already have the extension function
    if ( NULL == m_NextProvider->NextProcTableExt.lpfnWSASendMsg )
    {
        GUID    guidWSASendMsg = WSAID_WSASENDMSG;

        ret = LoadExtensionFunction(
                (FARPROC **)&m_NextProvider->NextProcTableExt.lpfnWSASendMsg,
                guidWSASendMsg,
                m_NextProvider->NextProcTable.lpWSPIoctl,
                m_NextProviderSocket
                );
        if ( FALSE == ret )
        {
            dbgprint(_T("Next proc table WSASendMsg == NULL!"));
            Errno = WSAEFAULT;
            goto cleanup;
        }
    }

    //
    // Check for overlapped I/O
    //
    if ( NULL != lpOverlapped )
    {
        ProviderOverlapped = PrepareOverlappedOperation(
                LSP_OP_WSASENDMSG,
                NULL,
                0,
                lpOverlapped,
                lpCompletionRoutine,
                NULL,
               &Errno
                );
        if ( NULL == ProviderOverlapped )
        {
            dbgprint(_T("ExtWSASendMsg: PrepareOverlappedOperation returned NULL"));
            goto cleanup;
        }

        __try 
        {
            ProviderOverlapped->WSASendMsgArgs.dwFlags				= dwFlags;
			ProviderOverlapped->WSASendMsgArgs.dwNumberOfBytesSent  = (lpNumberOfBytesSent ? *lpNumberOfBytesSent : 0);
            ProviderOverlapped->WSASendMsgArgs.s                    = m_CallerSocket;
            ProviderOverlapped->WSASendMsgArgs.lpMsg                = lpMsg;
        }
        __except( EXCEPTION_EXECUTE_HANDLER )
        {
            Errno = WSAEFAULT;
            goto cleanup;
        }
 		InitSockOverlap();
	    ret = gOverlapManager.QueueOverlappedOperation(ProviderOverlapped->lpCallerOverlapped ,&Errno);
        if ( NO_ERROR == ret )
        {
			ret=ExecuteOverlappedOperation(ProviderOverlapped,&Errno);
        }
        if ( NO_ERROR != ret )
        {
            ret = SOCKET_ERROR;
        }
    }
    else
    {
        ret = CLSPSocket::ExtWSASendMsg(
										 lpMsg,
									     dwFlags,
									     lpNumberOfBytesSent,
										 NULL,
									     NULL,
									     Errno
										);
    }
cleanup:
	return ret;
}