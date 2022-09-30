#pragma once
#include "error.h"
#include "ServiceLogging.h"
template <typename T> 
class IWinService:public CError 
{
public:
	friend class CLOIService;


	static T*		GetInstance()
	{
		if(s_pInstance==NULL)
		{
			s_pInstance=new T;
		}
		return s_pInstance;
	}
	

	static void	DestroyInstance()
	{
		if(s_pInstance)
		{
			delete s_pInstance;
			s_pInstance=NULL;
		}
	}

	BOOL   InstallService(LPCTSTR lpszServiceName,LPCTSTR lpszDisplayName)
	{
		SC_HANDLE				hSCManager	= NULL;
		SC_HANDLE				hAddService = NULL;
		TCHAR					szModuleName[MAX_PATH]={0};

		hSCManager=OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS); 
		if (hSCManager==NULL) 
		{
			DEBUGLOG(ERROR,(LPTSTR)(LPCTSTR)GetLastErrorMessage());
			SetLastErrorMessage(IDS_OPEN_SERVICE_FAILED,GetLastError());
			return FALSE;
		}
		GetModuleFileName(NULL, szModuleName, sizeof(szModuleName)/sizeof(szModuleName[0]));
		hAddService = CreateService( 
									hSCManager,	/* SCManager database      */ 
									lpszServiceName,			/* name of service         */ 
									lpszDisplayName,			/* service name to display */ 
									SERVICE_ALL_ACCESS,        /* desired access          */ 
									SERVICE_WIN32_OWN_PROCESS|SERVICE_INTERACTIVE_PROCESS , /* service type            */ 
									SERVICE_AUTO_START,      /* start type              */ 
									SERVICE_ERROR_NORMAL,      /* error control type      */ 
									szModuleName,			/* service's binary        */ 
									NULL,                      /* no load ordering group  */ 
									NULL,                      /* no tag identifier       */ 
									NULL,                      /* no dependencies         */ 
									NULL,                      /* LocalSystem account     */ 
									NULL
								);                     /* no password             */ 
		if (hAddService==NULL) 
		{
			SetLastErrorMessage(IDS_CREATE_SERVICE_FAILED,GetLastError());
			CloseServiceHandle(hSCManager);
			DEBUGLOG(LOI_ERROR,(LPTSTR)(LPCTSTR)GetLastErrorMessage());
			return FALSE;
		}
		::StartService(hAddService,0,NULL);

		///add firewall rule exceptionfor teredo access
		CFirewall  Firewall;
		Firewall.AddFirewallRule(szModuleName);

		CloseServiceHandle(hAddService); 
		CloseServiceHandle(hSCManager);
		return TRUE;

	}
	BOOL UnInstallService(LPCTSTR lpszServiceName)
	{
		SC_HANDLE				hSCManager	= NULL;
		SC_HANDLE				hDeleteService = NULL;
		TCHAR					szModuleName[MAX_PATH]={0};
		SERVICE_STATUS			Status={0};


		hSCManager=OpenSCManager( NULL, NULL, SC_MANAGER_CREATE_SERVICE); 
		if (hSCManager==NULL) 
		{
			SetLastErrorMessage(IDS_OPEN_SERVICE_FAILED,GetLastError());
			DEBUGLOG(LOI_ERROR,(LPTSTR)(LPCTSTR)GetLastErrorMessage());
			return FALSE;
		}

		hDeleteService = OpenService( hSCManager, lpszServiceName, SERVICE_ALL_ACCESS);
		if (hDeleteService==NULL) 
		{
			SetLastErrorMessage(IDS_DELETE_SERVICE_FAILED,GetLastError());
			CloseServiceHandle(hSCManager);
			DEBUGLOG(LOI_ERROR,(LPTSTR)(LPCTSTR)GetLastErrorMessage());
			return FALSE;
		}
		//stop the service
		ControlService(hDeleteService,SERVICE_CONTROL_STOP,&Status);

		if(!DeleteService(hDeleteService)) 
		{
			SetLastErrorMessage(IDS_DELETE_SERVICE_FAILED,GetLastError());
			CloseServiceHandle(hDeleteService);
			CloseServiceHandle(hSCManager);
			DEBUGLOG(LOI_ERROR,(LPTSTR)(LPCTSTR)GetLastErrorMessage());
			return FALSE;
		}
		CloseServiceHandle(hDeleteService);
		CloseServiceHandle(hSCManager);
		return FALSE;

	}
	BOOL	Execute(LPCTSTR lpszServiceName)
	{
	    SERVICE_TABLE_ENTRY Entries[] = {
										{_T(""), T::ServiceMainProc},
										{NULL, NULL}
										};
		StringCbCopy(m_szServiceName,sizeof(m_szServiceName),lpszServiceName);
		return StartServiceCtrlDispatcher(Entries);
	}


	void RunAsExe()
	{
		if(StartService())
		{
			SetStatus(SERVICE_RUNNING);
			Run();
			SetStatus(SERVICE_STOP_PENDING);
			SetStatus(SERVICE_STOPPED);
		}
	}
protected:
	virtual BOOL				StartService()=0;
	virtual BOOL				Run()=0;
	virtual BOOL                StopService()=0;
private:
	static const SERVICE_TABLE_ENTRY				SERVICE_MAIN_ENTRY[];
	static	T*										s_pInstance;
	DWORD											m_dwServiceStatus;
	SERVICE_STATUS_HANDLE							m_hServiceStatus;

	TCHAR											m_szServiceName[MAX_PATH];
//functions	
	IWinService(void)
	{
		ZeroMemory(m_szServiceName,sizeof(m_szServiceName));
		m_dwServiceStatus=SERVICE_STOPPED;
		ZeroMemory(&m_hServiceStatus,sizeof(m_hServiceStatus));
	}
	virtual ~IWinService(void)
	{
	}
	static VOID WINAPI	ServiceMainProc(DWORD dwArgc, LPTSTR *lpszArgv)
	{
		return GetInstance()->ServiceMain(dwArgc,lpszArgv);
	}
	VOID WINAPI	ServiceMain(DWORD dwArgc, LPTSTR *lpszArgv)
	{

		m_hServiceStatus=RegisterServiceCtrlHandler(m_szServiceName,(LPHANDLER_FUNCTION)ServiceCtrlHandlerProc);
		if(m_hServiceStatus==NULL)
		{
			SetLastErrorMessage(IDS_SERVICE_EXECUTE_FAILED,GetLastError());
			DEBUGLOG(LOI_ERROR,(LPTSTR)(LPCTSTR)GetLastErrorMessage());
			return;
		}
		SetStatus(SERVICE_START_PENDING);
		if(StartService())
		{
			SetStatus(SERVICE_RUNNING);
			Run();
			SetStatus(SERVICE_STOPPED);
		}
	}

	static void WINAPI	ServiceCtrlHandlerProc(DWORD dwControl)
	{
		GetInstance()->ServiceCtrlHandler(dwControl);
	}
	void WINAPI	ServiceCtrlHandler(DWORD dwControl)
	{
	   switch(dwControl)
	   {
			case SERVICE_CONTROL_INTERROGATE:
				SetStatus(m_dwServiceStatus);
				break;
			case SERVICE_CONTROL_STOP:
			case SERVICE_CONTROL_SHUTDOWN:
				DEBUGLOG(LOI_ERROR,_T("Received SERVICE_CONTROL_STOP"));
				SetStatus(SERVICE_STOP_PENDING);
				StopService();
				SetStatus(SERVICE_STOPPED);
				break;
		}
	}

	void	SetStatus(DWORD dwStatus)
	{
		SERVICE_STATUS				Status={0};


		if(!m_hServiceStatus)return;

		m_dwServiceStatus=dwStatus;
		
		Status.dwServiceType              = SERVICE_WIN32_OWN_PROCESS;
		Status.dwCurrentState             = m_dwServiceStatus;
		Status.dwControlsAccepted         = SERVICE_ACCEPT_STOP|SERVICE_ACCEPT_SHUTDOWN;
		Status.dwWin32ExitCode            = NOERROR;
		Status.dwServiceSpecificExitCode  = NOERROR;
		Status.dwCheckPoint               = 0;
		Status.dwWaitHint                 = 3000;

	
		SetServiceStatus(m_hServiceStatus, &Status);
	}
};
