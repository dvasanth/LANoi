#pragma once
#include "LSPOverlappedSocket.h"
class COverlappedManager
{
public:
	COverlappedManager(void);
	~COverlappedManager(void);
	int					InitOverlappedManager();
	BOOL				QueueOverlappedOperation(LPWSAOVERLAPPED  lpCallerOverlapped,LPINT lpErrno);
	int					StopOverlappedManager();
	// Either allocates a new structure or returns a free one on a lookaside list
	LPWSAOVERLAPPEDPLUS AllocOverlappedStructure(CLSPOverlappedSocket  *SocketContext);
	// Returns the overlapped structure to a list of free structures
	void				FreeOverlappedStructure(WSAOVERLAPPEDPLUS  *olp);
	HANDLE				AssociateHandle(SOCKET ProviderSocket,SOCKET CallerSocket,BOOL& bSynchronous);
	void				FreeOverlappedLookasideList();
private:	
	// Overlapped thread which either handles overlapped completion via completion port
	// or executes overlapped operations with APCs.
	static DWORD WINAPI   OverlappedManagerThread(LPVOID  lpParam);


	// Sets the overlapped in progress flag in the given OVERLAPPED structure
	void				SetOverlappedInProgress(OVERLAPPED         *ol);


		//
	// For each overlapped operation given to us from the upper layer, we'll
	// assign a WSAOVERLAPPEDPLUS structure so that we can make the overlapped
	// call to the lower layer. The following variables are pointers to these
	// structures.
	//

	LIST_ENTRY   m_FreeOverlappedPlus;    // look aside list of free structures

	//
	// NOTE: On Win9x a semaphore is used to trigger when an overlapped operation
	//       is queued for execution. On NT based OSes (including XP), a completion
	//       port is used. That is, the use of 'gIocp' and 'gWakeupSemaphore' is
	//       mutually exclusive.
	//

	HANDLE             *m_WorkerThread,       // Array of handles to worker threads
						m_Iocp;               // Handle to the IOCP
	DWORD               m_ThreadCount;   // Number of worker threads
};
