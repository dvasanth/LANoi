#include "StdAfx.h"
#include "NetMeeting.h"

CNetMeeting::CNetMeeting(void)
{
	m_pClassFactory=NULL;
	m_pConfMgr=NULL;
    ::CoInitialize(NULL);
}

CNetMeeting::~CNetMeeting(void)
{
	if(m_pClassFactory)
	{
		m_pClassFactory->Release();
		m_pClassFactory=NULL;
	}
    //
    // Release conference manager
    //
    if (NULL != m_pConfMgr)
    {
        m_pConfMgr->Release();
        m_pConfMgr = NULL;
    }
}

HRESULT CNetMeeting::InitConfMgr()
{
    HRESULT					hResult= S_OK;
    LPCLASSFACTORY			pcf	= NULL;
	ULONG					ll=0;


    // Notify the system we want to use the conferencing services
    // by creating a conference manager object
    //
    hResult = CoGetClassObject(CLSID_NmManager,
                          CLSCTX_INPROC,
                          NULL,
                          IID_IClassFactory,
                          (void**)&m_pClassFactory);
    if (SUCCEEDED(hResult))
    {
        //
        // Get the conference manager object
        //
			
        hResult = m_pClassFactory->CreateInstance(NULL, IID_INmManager, (void**)&m_pConfMgr);
        if (SUCCEEDED(hResult))
        {
			 ULONG		uOptions = 0;
			 ULONG		uchCaps = NMCH_ALL;
			 hResult = m_pConfMgr->Initialize(&uOptions, &uchCaps);
		}
	}

	return hResult;
}

HRESULT CNetMeeting::HostMeeting(CString sMeetingName)
{
	INmConference*				pConference=NULL;
	ULONG						uchChannelType= NMCH_ALL;
    HRESULT						hResult= S_OK;

	TerminateConf();
	hResult=InitConfMgr();
	if(SUCCEEDED(hResult))
	{	
		hResult=m_pConfMgr->CreateConference(&pConference,(BSTR)(LPCTSTR)sMeetingName,NULL,uchChannelType);
		if (SUCCEEDED(hResult))
		{
			hResult=pConference->Host();//host a meeting
			pConference->Release();  //anycase free the object
			if (hResult == S_OK)
			{
				return S_OK;//only success part
			}
		}
	}

	SetLastErrorMessage(IDS_INITIALISE_NETMEET_FAILED,hResult);
	return hResult;
}

HRESULT CNetMeeting::JoinMeeting(CString sUserName)
{
	// TODO: Add your control notification handler code here
	SHELLEXECUTEINFO 	 shInfo={0};
	CString				sCmdLine;

	sCmdLine.Format(_T("msconf.dll,CallToProtocolHandler %s"),(LPCTSTR)sUserName);

	shInfo.cbSize =sizeof(shInfo);
	shInfo.lpVerb = _T("open");
	shInfo.lpFile = _T("rundll32.exe");
	shInfo.lpParameters= (LPCTSTR)sCmdLine;
	shInfo.nShow = SW_SHOW;
	if(!ShellExecuteEx(&shInfo))
	{
		SetLastErrorMessage(IDS_INITIALISE_NETMEET_FAILED,GetLastError() );
		return E_FAIL;
	}

	return S_OK;
}

void CNetMeeting::TerminateConf()
{
	DWORD			dwProcesses[1024], cbNeeded, cProcesses;


    if ( !EnumProcesses( dwProcesses, sizeof(dwProcesses), &cbNeeded ) )
        return;

    // Calculate how many process identifiers were returned.
    cProcesses = cbNeeded / sizeof(DWORD);

	for(DWORD dwProcessCount=0;dwProcessCount<cProcesses;dwProcessCount++)
	{
		TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");

		// Get a handle to the process.
	    HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION |
                                   PROCESS_VM_READ|PROCESS_TERMINATE,
                                   FALSE, dwProcesses[dwProcessCount] );

		 // Get the process name.
	    if (NULL != hProcess )
		{
			HMODULE				hMod;
			DWORD				cbNeeded;

			if ( EnumProcessModules( hProcess, &hMod, sizeof(hMod),&cbNeeded) )
			{
				GetModuleBaseName( hProcess, hMod, szProcessName, 
								   sizeof(szProcessName)/sizeof(TCHAR) );
			}
			_tcslwr_s(szProcessName,sizeof(szProcessName)/sizeof(szProcessName[0]));  
			if(_tcsstr(szProcessName,_T("conf.exe")))
			{
				DWORD  dwExitCode;
				GetExitCodeProcess(hProcess,&dwExitCode);
				TerminateProcess(hProcess,dwExitCode);
				CloseHandle( hProcess );
				return ;
			}
					
			CloseHandle( hProcess );
		}//NULL != hProcess
	}//for

}