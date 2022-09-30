#include "StdAfx.h"
#include "WinService.h"


	
CWinService::CWinService(void)
{

}

CWinService::~CWinService(void)
{
}

CWinService* CWinService::GetInstance()
{

}

void CWinService::DestroyInstance()
{

}

BOOL CWinService::InstallService(LPCTSTR lpszServiceName,LPCTSTR lpszDisplayName)
{
}	
	
BOOL CWinService::UnInstallService(LPCTSTR lpszServiceName)
{
}

BOOL CWinService::Execute(LPCTSTR lpszServiceName)
{

}

VOID WINAPI	CWinService::ServiceMainProc(DWORD dwArgc, LPTSTR *lpszArgv)
{


}

VOID WINAPI	CWinService::ServiceMain(DWORD dwArgc, LPTSTR *lpszArgv)
{
	
}

void WINAPI CServiceBase::ServiceCtrlHandlerProc(DWORD dwControl)
{
	
}

void CServiceBase::ServiceCtrlHandler(DWORD dwControl)
{

	
}

void CServiceBase::SetStatus(DWORD dwStatus)
{ 


}