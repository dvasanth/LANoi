#pragma once


class CError
{
public:
	CError(void);
	~CError(void);
	CString		GetLastErrorMessage();
	CString     m_sLastErrorMsg;
	void		SetLastErrorMessage(UINT uiTextID);
	void		SetLastErrorMessage(UINT uiAppendTextID,HRESULT hResult );
	void		SetLastErrorMessage(CString sErrMsg );
	DWORD       m_dwLastError;

};	
extern BOOL  g_bEnableTracing;
#ifdef LOG
	void cdecl DBGPRINT(TCHAR* szFormat, ...);
#else
	#define DBGPRINT //
#endif