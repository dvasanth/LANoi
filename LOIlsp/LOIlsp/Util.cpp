#include "StdAfx.h"
#include "lspdef.h"
#include "Util.h"
#include <strsafe.h>
HANDLE				g_hLspHeap;


//
// Function: LspCreateHeap
//
// Description:
//    Creates the private heap used by all LSP routines.
//
int LspCreateHeap(int *lpErrno)
{
    g_hLspHeap = HeapCreate( 0, 128000, 0 );
    if ( NULL == g_hLspHeap )
    {
        *lpErrno = WSAEPROVIDERFAILEDINIT;
        return SOCKET_ERROR;
    }
    return NO_ERROR;
}




//
// Function: LspDestroyHeap
//
// Description:
//    Frees the private heap used by all LSP routines.
//
void LspDestroyHeap()
{
    if ( NULL != g_hLspHeap )
    {
        HeapDestroy( g_hLspHeap );
        g_hLspHeap = NULL;
    }
}


//
// Function: LspAlloc
//
// Description:
//    Allocates a buffer of the requested size from the LSP created heap. If the
//    allocation fails, set the error value to WSAENOBUFS.
//
void *LspAlloc(SIZE_T  size,int    *lpErrno)
{
    void *mem = NULL;
    mem = HeapAlloc( 
            g_hLspHeap, 
            HEAP_ZERO_MEMORY, 
            size
            );
    if ( NULL == mem )
    {
        *lpErrno = WSAENOBUFS;
    }

    return mem;
}

//
// Function: LspFree
//
// Description:
//    Frees a buffer that was previously allocated by LspAlloc.
//
void LspFree(LPVOID  buf)
{
	if(g_hLspHeap)
    HeapFree( g_hLspHeap, 0, buf );
}

#ifdef LOG
void cdecl dbgprint(TCHAR* szFormat, ...)
{
	static	TCHAR Buffer1[1024];
	static	TCHAR Buffer2[1024];
	TCHAR	szLogPath[MAX_PATH]={0};
	TCHAR	szInstallDir[MAX_PATH]={0};


	StringCbVPrintf( Buffer1, sizeof(Buffer1),szFormat, (va_list)(&szFormat+1));
	StringCbPrintf(Buffer2,sizeof(Buffer2 ), _T("%lu : %s\r\n"), GetCurrentThreadId(),  Buffer1);

/*

	GetCurrentDirectory(MAX_PATH, szLogPath);
	StringCbCat(szLogPath,sizeof(szLogPath),_T("\\tracelog.txt"));


	HANDLE hFile = CreateFile(szLogPath, GENERIC_WRITE, FILE_SHARE_WRITE,
                   NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if(hFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwWritten=0;
		SetFilePointer(hFile,0,0,FILE_END);
		WriteFile(hFile,Buffer2,_tcslen(Buffer2)+1,&dwWritten,NULL);  
		CloseHandle(hFile);
	}
*/
	OutputDebugString(Buffer2);
}
#endif