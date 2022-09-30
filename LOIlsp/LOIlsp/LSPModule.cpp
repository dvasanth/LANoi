#include "StdAfx.h"
#include "lspdef.h"
#include "LSPModule.h"

#include <strsafe.h>

CRITICAL_SECTION    gDebugCritSec;



CLSPModule::CLSPModule()
{
	m_bProcessDetached= FALSE;

}

CLSPModule::~CLSPModule(void)
{
	
}


BOOL CLSPModule::OnInitInstance(HINSTANCE hDllInstance)
{
	m_DllInstance = hDllInstance;
	
     //
     // Initialize some critical section objects 
     //
    __try
    {
        InitializeCriticalSection( &gCriticalSection );
        InitializeCriticalSection( &gOverlappedCS );
		InitializeCriticalSection( &gDebugCritSec );
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
        return FALSE;
    }

    gTlsIndex = TlsAlloc();

	ZeroMemory(m_szAppName,sizeof(m_szAppName));
	TCHAR   szAppPath[MAX_PATH]={0};
	GetModuleFileName (NULL,szAppPath,sizeof(szAppPath)/sizeof(szAppPath[0]));
	if(_tcsrchr(szAppPath,'\\'))
	StringCbCopy(m_szAppName,sizeof(m_szAppName),_tcsrchr(szAppPath,'\\')+1);
	_tcslwr_s(m_szAppName,sizeof(m_szAppName)/sizeof(m_szAppName[0]));

	return TRUE;
}

void CLSPModule::OnExitInstance(LPVOID lpvReserved)
{   
	int Error;
   
	m_bProcessDetached = TRUE;

    EnterCriticalSection( &gCriticalSection );
    m_LspProvider.FreeAllInformation( TRUE, &Error );
    LeaveCriticalSection( &gCriticalSection );

    DeleteCriticalSection( &gCriticalSection );
    DeleteCriticalSection( &gOverlappedCS );
	DeleteCriticalSection( &gDebugCritSec );



    if ( NULL == lpvReserved )
    {
        if ( 0xFFFFFFFF != gTlsIndex )
        {
            TlsFree( gTlsIndex );
             gTlsIndex = 0xFFFFFFFF;
        }
    }

    LspDestroyHeap();
	
}

BOOL CLSPModule::IsProcessDetached()
{
	return m_bProcessDetached;
}

DWORD CLSPModule::GetThreadIndex()
{
	return m_dwThreadIndex;
}

CLSPProviderOperations* CLSPModule::GetProviderInstance()
{
	
	return &m_LspProvider;
}

CSocketFactory* CLSPModule::GetSockFactoryInstance()
{
	
	return &m_SocketFactory;
}
void CLSPModule::LoadRegistryValues()
{

	
}
//
// Function: dbgprint
//
// Description:
//    Format and print a message to the debugger.
//

#ifdef DBG
void 
dbgprint(
    char *format,
    ...
    )
{
    static  DWORD pid=0;
    va_list vl;
    char    dbgbuf1[2048],
            dbgbuf2[2048];

    // Prepend the process ID to the message
    if ( 0 == pid )
    {
        pid = GetCurrentProcessId();
    }

    EnterCriticalSection(&gDebugCritSec);
    va_start(vl, format);
    vswprintf_s(dbgbuf1, sizeof dbgbuf1,format, vl);
    swprintf_s(dbgbuf2, sizeof dbgbuf2,"%lu: %s\r\n", pid, dbgbuf1);
    va_end(vl);

    OutputDebugString(dbgbuf2);
    LeaveCriticalSection(&gDebugCritSec);
}
#endif
