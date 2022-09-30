#include "stdafx.h"
#include "Error.h"

CError::CError(void)
{
}

CError::~CError(void)
{
}
void CError::SetLastErrorMessage(UINT uiTextID)
{
	m_sLastErrorMsg.LoadString(uiTextID);
}

void CError::SetLastErrorMessage(UINT uiAppendTextID,HRESULT hResult )
{
    CString				sText;
	LPTSTR				szErrorString;


	sText.LoadString(uiAppendTextID); 

    if(HRESULT_FACILITY(hResult) == FACILITY_WINDOWS)
        hResult = HRESULT_CODE(hResult);



    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |FORMAT_MESSAGE_FROM_SYSTEM, NULL, hResult, 
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
        (LPTSTR)&szErrorString, 0, NULL);

	m_sLastErrorMsg = sText + _T(". Reason: ");
	if(szErrorString)
		m_sLastErrorMsg += szErrorString; 
	else
		m_sLastErrorMsg.Format(_T("%s. Reason: n0x%X"),(LPCTSTR)sText,hResult); 

	if(szErrorString)
		LocalFree(szErrorString);

}
void CError::SetLastErrorMessage(CString sErrMsg )
{
	m_sLastErrorMsg=sErrMsg;
}
CString CError::GetLastErrorMessage()
{
	return m_sLastErrorMsg;
}
#include <strsafe.h>

BOOL  g_bEnableTracing;
#ifdef LOG

void cdecl DBGPRINT(TCHAR* szFormat, ...)
{
	//if(!g_bEnableTracing) return;

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
#else
	#define DBGPRINT //
#endif

void PrintErrorMessage(HWND hWnd, DWORD dwErrorCode, CString sCaption)
{
	try{

		 TCHAR szBuf[500]; 
		 LPVOID lpMsgBuf;
		 DWORD dw = GetLastError(); 

		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			dwErrorCode,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR) &lpMsgBuf,
			0, NULL );

		StringCbPrintf(szBuf,sizeof(szBuf), 
			_T("%s failed with error %d: %s"), 
			sCaption, dw, lpMsgBuf); 
 
		MessageBox((HWND)NULL, szBuf, _T("Proxy Server Error"), MB_OK); 
		//AfxMessageBox(szBuf, MB_OK|MB_ICONERROR,0);

		LocalFree(lpMsgBuf);
		lpMsgBuf=0;

	}
	catch(...)
	{
	}
}