#include "StdAfx.h"
#include "RemoteAssitance.h"
#include "RegOperations.h"
#include "msra_h.h"
#include <lm.h>
#include "error.h"
#include "../cmnresource.h"
#include <strsafe.h>
#include <psapi.h>
CRemoteAssitance::CRemoteAssitance(void)
{
	m_bRAEnabled=FALSE;
	m_bRAUnSolicitedEnabled=FALSE;
	m_bHAUserAccountEnabled=FALSE;
	m_bRAPolicyChanged=FALSE;
	
}

CRemoteAssitance::~CRemoteAssitance(void)
{
	if(m_bRAPolicyChanged)
	{
		RestoreRAPolicy();
		m_bRAPolicyChanged=FALSE;
	}
}

void CRemoteAssitance::EnableRAPolicy()
{
	CRegOperations   Enable;
	
	if(IsWindowsVista()) return;

	m_bRAPolicyChanged=TRUE;
	//enable remote assitance
	m_bRAEnabled=Enable.EnableRA(TRUE);
	m_bHAUserAccountEnabled=EnableRAUserAccount(TRUE);

	//enable unsolicited ticket creation
	m_bRAUnSolicitedEnabled=Enable.EnableRAUnSolicitPolicy(TRUE);

}
BOOL CRemoteAssitance::EnableRAUserAccount(BOOL bValue)
{
	LPUSER_INFO_1				pBuf = NULL;
	LPUSER_INFO_1				pTmpBuf;
	DWORD						dwLevel = 1;
	DWORD						dwPrefMaxLen = MAX_PREFERRED_LENGTH;
	DWORD						dwEntriesRead = 0;
	DWORD						dwTotalEntries = 0;
	DWORD						dwResumeHandle = 0;
	DWORD						i;
	DWORD						dwTotalCount = 0;
	NET_API_STATUS				nStatus;
	LPTSTR						pszServerName = NULL;
	BOOL						bCurrentValue=FALSE;

	do 
	{
		nStatus = NetUserEnum(pszServerName,
			dwLevel,
			0, // global users
			(LPBYTE*)&pBuf,
			dwPrefMaxLen,
			&dwEntriesRead,
			&dwTotalEntries,
			&dwResumeHandle);
		if ((nStatus == NERR_Success) || (nStatus == ERROR_MORE_DATA))
			{
			if ((pTmpBuf = pBuf) != NULL)
				{
				//
				// Loop through the entries.
				//
				for (i = 0; (i < dwEntriesRead); i++)
					{
					if (pTmpBuf == NULL)
						{
						break;
						}

					if (wcscmp(pTmpBuf->usri1_name,L"HelpAssistant") == 0)
						{
							if ((pTmpBuf->usri1_flags & UF_ACCOUNTDISABLE) == 0)
								bCurrentValue=TRUE;
							if (bValue)
								pTmpBuf->usri1_flags &= ~UF_ACCOUNTDISABLE;
							else
								pTmpBuf->usri1_flags |= UF_ACCOUNTDISABLE;
							pTmpBuf->usri1_password = 0;
							NetUserSetInfo(0,L"HelpAssistant",1,(LPBYTE)pTmpBuf,0);
						}

					pTmpBuf++;
					dwTotalCount++;
					}
				}
			}
		if (pBuf != NULL)
			{
			NetApiBufferFree(pBuf);
			pBuf = NULL;
			}
		}
	while (nStatus == ERROR_MORE_DATA);

	if (pBuf != NULL)
		NetApiBufferFree(pBuf);
	return bCurrentValue;
}

void CRemoteAssitance::RestoreRAPolicy()
{
	CRegOperations		Restore;

	if(IsWindowsVista()) return;
//	Restore.EnableRA(m_bRAEnabled);
//	EnableRAUserAccount(m_bHAUserAccountEnabled);
	if(IsWindowsVista()) return;
//	Restore.EnableRAUnSolicitPolicy(m_bRAUnSolicitedEnabled);
}

BOOL CRemoteAssitance::IsWindowsVista()
{
	OSVERSIONINFO osvi;
    
    ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    GetVersionEx(&osvi);
	//WindowsVista or Later
    if(osvi.dwMajorVersion >= 6)
		return TRUE;
	return FALSE;
}
BOOL CRemoteAssitance::IsWindows7()
{
	OSVERSIONINFO osvi;
    
    ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    GetVersionEx(&osvi);
	//WindowsVista or Later
    if(osvi.dwMajorVersion >= 7)
		return TRUE;
	return FALSE;
}
BOOL CRemoteAssitance::CreateInvitation(CString sPeerName,CString sPassword,CString &sArgInvitationString,CString &sRDPListeningPort)
{
	BOOL		bRet=FALSE;  

	EnableRAPolicy();
	if(IsWindowsVista())
	{
	//	AfxMessageBox(IDS_VISTA_RA_GUIDE_HLP, MB_OK  |MB_ICONSTOP|MB_TOPMOST   );
		if(CreateVistaInvitation(sPeerName,sPassword,sArgInvitationString,sRDPListeningPort))
		{
			//guide the user for vista invitation creation
			//AfxMessageBox(IDS_VISTA_RA_GUIDE_HLP, MB_OK  |MB_ICONSTOP|MB_TOPMOST   ); 
			//return CreateVistaInvitation(sPeerName,sArgInvitationString,sRDPListeningPort);
			bRet=TRUE;
		}
		return bRet;
	}
	return CreateXPInvitation(sPeerName,sArgInvitationString,sRDPListeningPort);
}
BOOL CRemoteAssitance::CreateXPInvitation(CString sPeerName,CString &sArgInvitationString,CString &sRDPListeningPort)
{
	HRESULT					hResult = S_OK;
	TCHAR					szUserName[1024]={0};
	TCHAR					szHostName[1024]={0};
	TCHAR					szTemp[1024]={0};
	TCHAR					szUserHelpBlob[2046]={0};
	DWORD					dwLen=0;
    IPCHService				*pPchService = NULL;
	COSERVERINFO            ServInfo = {0};
	MULTI_QI                MultiQi = {0};
	BOOL					bRet=FALSE;
	//833E4010-AFF7-4AC3-AAC2-9F24C1457BCE
	CLSID					CLSID_PCHService ={0x833E4010,0xAFF7,0x4AC3,{0xaa,0xc2,0x9f,0x24,0xc1,0x45,0x7b,0xce}};
	//833E4200-AFF7-4AC3-AAC2-9F24C1457BCE
	IID						IID_IPCHService ={0x833E4200,0xaff7,0x4ac3,{0xaa,0xc2,0x9f,0x24,0xc1,0x45,0x7b,0xce}};
	// Get user name
	dwLen= sizeof(szUserName)/sizeof(TCHAR);
	GetUserName(szUserName,&dwLen);
	// Get PC name
	dwLen=sizeof(szHostName);
	GetComputerNameEx(ComputerNameDnsHostname,szHostName,&dwLen);

	// Configure blob
	// L"13;UNSOLICITED=122;ID=GATOR\\Administrator";
	StringCbPrintf(szTemp,sizeof(szTemp),_T("ID=%s\\%s"),szHostName,szUserName);
	StringCbPrintf(szUserHelpBlob,sizeof(szUserHelpBlob),_T("13;UNSOLICITED=1%u;%s"),_tcslen(szTemp),szTemp);

	BSTR		bstrUserName = SysAllocString(szUserName);
	BSTR		bstrHostname = SysAllocString(szHostName);
	BSTR		bstrUserHelpBlob = SysAllocString(szUserHelpBlob);
	BSTR		bstrConnectionString = NULL;


    ServInfo.pwszName = (LPWSTR)szHostName;
    MultiQi.pIID     = &(IID_IPCHService);

	//RARunning(true);
    hResult = ::CoCreateInstanceEx(CLSID_PCHService, NULL, CLSCTX_LOCAL_SERVER, &ServInfo, 1, &MultiQi );
	pPchService = (IPCHService*)MultiQi.pItf;
	if (SUCCEEDED (hResult) && pPchService)
	{
		DWORD  dwSid = WTSGetActiveConsoleSessionId();
		UINT	uiCallRetry=5;
		do{
			hResult = pPchService->RemoteConnectionParms(bstrUserName,bstrHostname,dwSid,bstrUserHelpBlob,&bstrConnectionString);
			uiCallRetry--;
			Sleep(1000);
		}while(!SUCCEEDED (hResult) && uiCallRetry>0);


		if(SUCCEEDED (hResult))
		{
			sArgInvitationString=GetInvitationString(sPeerName,CString(bstrConnectionString));
			sRDPListeningPort=GetRDPListeningPort(sArgInvitationString);
			SysFreeString(bstrConnectionString);
			bRet=TRUE;
		}
		else
		{
			CError  Err;
			Err.SetLastErrorMessage(IDS_RA_CREATE_FAILED,hResult); 
			AfxMessageBox(Err.GetLastErrorMessage(), MB_OK  |MB_ICONSTOP|MB_TOPMOST ); 
		}
		pPchService->Release(); 
	}
	SysFreeString(bstrUserName);
	SysFreeString(bstrHostname);
	SysFreeString(bstrUserHelpBlob);


	return bRet;
}
/****************************
check whether vista rdp enabled
*****************************/
BOOL CRemoteAssitance::IsVistaRAEnabled()
{
	CRegKey				RegRAKey;
	DWORD				dwRAEnabled=0;

	if(RegRAKey.Open(HKEY_LOCAL_MACHINE,
		_T("SYSTEM\\CurrentControlSet\\Control\\Remote Assistance"),KEY_READ)==ERROR_SUCCESS)
	{
		RegRAKey.QueryDWORDValue(_T("fAllowToGetHelp"),dwRAEnabled);
	}
	return dwRAEnabled;
}
/*******************************
just keep waiting for dwmilliseconds for RA getting enabled
*******************************/
void CRemoteAssitance::WaitForVistaGettingEnabled()
{
	DWORD		dwRetryTime=5;

	do{
		WaitAndDispatch(0,NULL,5000);
		dwRetryTime--;
	}while(!IsVistaRAEnabled() && dwRetryTime>0);
}
/************************
wait as well as dispatch message
**************************/
DWORD CRemoteAssitance::WaitAndDispatch(DWORD dwHandleCount,PHANDLE phHandle,DWORD dwMilliseconds)
{
	DWORD		dwRetVal;
	DWORD		dwStartTick=GetTickCount(); 

	while(1)
	{
		MSG		msg={0};

		dwRetVal=MsgWaitForMultipleObjects(dwHandleCount,phHandle,FALSE,2000,QS_ALLINPUT); 
		while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                        DispatchMessage(&msg);
		if(dwHandleCount && WAIT_OBJECT_0==dwRetVal)
		{
			break;
		}
		if((GetTickCount()-dwStartTick) >= dwMilliseconds)
		{
			break;
		}
	}

	return dwRetVal;
}
/*******************************
Execute RA
*********************************/
BOOL CRemoteAssitance::ExecuteVistaRA(CString sFilePath,CString sPassword,BOOL bAdmin,HANDLE &hProcess)
{
	SHELLEXECUTEINFO		sinfo={0};
	BOOL					bRet=FALSE;
	TCHAR					szCommandLine[1024];

	StringCbPrintf(szCommandLine,sizeof(szCommandLine),
					_T("/saveasfile \"%s\" %s"),
					(LPCTSTR)sFilePath,
					sPassword
					);
	memset(&sinfo, 0, sizeof(SHELLEXECUTEINFO));
	sinfo.cbSize       = sizeof(SHELLEXECUTEINFO);
	sinfo.fMask        = SEE_MASK_FLAG_DDEWAIT |SEE_MASK_NOCLOSEPROCESS;
	sinfo.hwnd         = NULL;
	sinfo.lpFile       = _T("msra.exe");
	sinfo.lpParameters = szCommandLine;
	sinfo.lpVerb       = bAdmin?L"runas":NULL; // <<-- this is what makes a UAC prompt show up
	sinfo.nShow        = SW_SHOW;

	TerminateMSRA();
	DeleteFile((LPCTSTR)sFilePath); 
	// The only way to get a UAC prompt to show up
	// is by calling ShellExecuteEx() with the correct
	// SHELLEXECUTEINFO struct.  Non privlidged applications
	// cannot open/start a UAC prompt by simply spawning
	// a process that has the correct XML manifest.
	if(ShellExecuteEx(&sinfo))
	{
		hProcess=sinfo.hProcess;
		bRet=TRUE;
	}
	return bRet;
}
/******************
vista invitation create
**********************/
BOOL CRemoteAssitance::CreateVistaInvitation(CString sPeerName,CString sPassword,CString &sArgInvitationString,CString &sRDPListeningPort)
{
	CString				sFileExt;
	CFileDialog			fOpenDlg(TRUE, 
								_T("msrcincident"), _T(""),
								OFN_HIDEREADONLY|OFN_FILEMUSTEXIST,
								(LPCTSTR)_T("Vista Remote Assistance Files(*.msrcincident)|*.msrcincident||"),
								NULL);
					
	HANDLE				hProcess=NULL;	
	CString				sTempFilePath=GetInvitationFilePath();
						
	
	//check if RA enabled
	if(!IsVistaRAEnabled())
	{
		//prompt user to enable RA
		if(ExecuteVistaRA(sTempFilePath,sPassword,FALSE,hProcess))
		{
			if(IsWindows7())
			{
				WaitAndDispatch(1,&hProcess,30000);
			}
			else
			{
				WaitForVistaGettingEnabled();
			}
			CloseHandle(hProcess);
		}
	}

	//hope ra is enabled by the user
	

	// Initializes m_pOFN structure
//	fOpenDlg.m_pOFN->lpstrTitle=_T("Select the Vista Remote Assistance File");
//	if(fOpenDlg.DoModal()==IDOK)


	//create the invitation
	if(ExecuteVistaRA(sTempFilePath,sPassword,FALSE,hProcess))	
	{
		HANDLE			hInvitationFile;
		DWORD			dwRetryCount=8;

		//wait for the file to be created
		do{
			if(WaitAndDispatch (1,&hProcess,500)==WAIT_OBJECT_0)
			{
				break;
			}
			dwRetryCount--;
		}while(dwRetryCount>0 && !PathFileExists( (LPCTSTR)sTempFilePath));

		

		hInvitationFile = CreateFile((LPCTSTR)sTempFilePath, GENERIC_READ, FILE_SHARE_WRITE,
                   NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if(hInvitationFile != INVALID_HANDLE_VALUE)
		{
			DWORD			dwRead=0;
			BYTE			byData[4096]={0};
			CString			sConnString;

			ReadFile(hInvitationFile,byData,sizeof(byData),&dwRead,NULL);
			CloseHandle(hInvitationFile);
			//convert the string
			if(byData[0]==0xff)
			{
				sConnString=CString((LPCTSTR)byData);
			}
			else
				sConnString.Format(_T("%hs"),(LPCSTR)byData);
			sArgInvitationString=GetInvitationString(sPeerName,sConnString);
			sRDPListeningPort=GetRDPListeningPort(sArgInvitationString);
			return TRUE;
		}

	}

	return FALSE;
}
CString CRemoteAssitance::GetInvitationString(CString sPeerName,CString sConnString)
{
	CString           sXmlFormatString;
	CString			  sTime;
	CString			  sNewIpinfo;
	SYSTEMTIME		  SystemTime={0}; 
	FILETIME		  FileTime={0};
	ULARGE_INTEGER	  ulTime={0};
	CString			  sXmlInvitation;
	int				  iAddrPos;
	CString			  sTemp;
	CString			  sSignature(_T("65538,1,"));
    time_t			  ltime;
	TCHAR			  szTime[MAX_PATH]={0};	

	sXmlFormatString.LoadString( IDS_RA_INVITATION_FILE_FORMAT);
	//get the new address which is reachable from invitees
	sNewIpinfo=sPeerName+_T(":")+GetRDPListeningPort(sConnString);

	//append sNewIpinfo to string
	sTemp=sConnString;
	iAddrPos=sTemp.Find(sSignature)+sSignature.GetLength();
	sConnString= sSignature+sNewIpinfo+_T(";")+sConnString.Mid(iAddrPos);


	//get UTC in string
	time( &ltime );
	StringCbPrintf(szTime,sizeof(szTime),_T("%I64d"),ltime);

	//form the invitaiton file
	if(IsWindowsVista())
	{
		sXmlFormatString.LoadString(IDS_RA_XML_VISTA_FORMAT);
		sXmlInvitation.Format(sXmlFormatString,(LPCTSTR)sConnString);
		return sXmlInvitation;
	}

	sXmlInvitation.Format(sXmlFormatString,(LPCTSTR)sConnString,szTime);

	return sXmlInvitation;
}

CString CRemoteAssitance::GetRDPListeningPort(CString sConnString)
{
	CString					sIPString;
	TCHAR					szHostName[1024]={0};
	CString					sHostString;
	DWORD					dwLen;

	if(IsWindowsVista())
	{
		CString			  sSignature(_T("65538,1,"));
		int				  iPos;

		iPos=sConnString.Find(sSignature)+sSignature.GetLength();
		sIPString= sConnString.Mid(iPos);
		iPos=sConnString.Find(_T(":"));
		sIPString=sConnString.Mid(iPos+1);
		sIPString=sIPString.Left( sIPString.Find(_T(",")));
		return sIPString;
	}
	// Get PC name
	dwLen=sizeof(szHostName);
	GetComputerNameEx(ComputerNameDnsHostname,szHostName,&dwLen);

	//RCTICKET="65538,1,122.170.19.15:38860;192.168.1.2:3389;192.168.107.1:3389;5.119.10.93:3389;aspire:3389,
	sHostString.Format(_T("%s:"),szHostName);
	sIPString= sConnString.Mid(sConnString.Find(sHostString));
	sIPString=sIPString.Mid(sHostString.GetLength());
	sIPString=sIPString.Left( sIPString.Find(_T(",")));

	return sIPString;
}
/**************************************
forms the invitation filepath
**************************************/
CString CRemoteAssitance::GetInvitationFilePath()
{
	TCHAR				szAppDirectory[MAX_PATH]={0};
	TCHAR				szInvitationDir[MAX_PATH]={0};
	CString             sInvitationFilePath;

	//expand the appdata directory
	SHGetFolderPath(NULL,CSIDL_COMMON_APPDATA ,NULL,SHGFP_TYPE_CURRENT,szAppDirectory);
	StringCbPrintf(szInvitationDir,sizeof(szInvitationDir),
										_T("%s\\crowsoft\\LANoipro\\RemoteAssist\\"),
										szAppDirectory);
	SHCreateDirectory(NULL,szInvitationDir);
	sInvitationFilePath=CString(szInvitationDir)+_T("RAInvitation.msrcincident");

	return sInvitationFilePath;
}
/************************************
launch the rA invitation
*************************************/
BOOL CRemoteAssitance::JoinInvitation(BYTE *pbyBlobData,DWORD dwDataLen)
{
	HANDLE				hFile;
	CString				sTempFilePath=GetInvitationFilePath();


	hFile = CreateFile((LPCTSTR)sTempFilePath, GENERIC_WRITE, FILE_SHARE_WRITE,
                   NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if(hFile != INVALID_HANDLE_VALUE)
	{
		DWORD			dwWritten=0;
		BYTE			byHeader[2]={0xff,0xfe};	
		WriteFile(hFile,byHeader,sizeof(byHeader),&dwWritten,NULL);
		WriteFile(hFile,pbyBlobData,dwDataLen,&dwWritten,NULL);
		CloseHandle(hFile);

		TerminateMSRA();
		//Launch Invitation File
		SHELLEXECUTEINFO 		shInfo={0};

		shInfo.cbSize =sizeof(shInfo);
		shInfo.lpVerb = _T("open");
		shInfo.lpFile = (LPCTSTR)sTempFilePath;
		shInfo.nShow = SW_SHOW;
		ShellExecuteEx(&shInfo);
		return TRUE;
	}

	return FALSE;
}


/*******************************
Terminates MSra if runnning
*******************************/
void CRemoteAssitance::TerminateMSRA()
{
	DWORD							dwProcesses[1024], cbNeeded, cProcesses;
	typedef  BOOL (_stdcall *LPQueryFullProcessImageName)(HANDLE hProcess,DWORD dwFlags,LPTSTR lpExeName,PDWORD lpdwSize);
	LPQueryFullProcessImageName		fpQueryFullProcessImageName;
	HMODULE							hKernelDLL = LoadLibrary(_T("kernel32.dll"));

	if (!hKernelDLL) 
	{
		return;
	}

   //Now use pointer to get access to functions defined in DLL
   fpQueryFullProcessImageName = (LPQueryFullProcessImageName)GetProcAddress(hKernelDLL, "QueryFullProcessImageNameW"); //ANSI version
    if (!fpQueryFullProcessImageName) 
	{
		FreeLibrary (hKernelDLL);
		return ;
	}

    if ( !EnumProcesses( dwProcesses, sizeof(dwProcesses), &cbNeeded ) )
	{
		FreeLibrary (hKernelDLL);
        return;
	}

    // Calculate how many process identifiers were returned.
    cProcesses = cbNeeded / sizeof(DWORD);

	for(DWORD dwProcessCount=0;dwProcessCount<cProcesses;dwProcessCount++)
	{
		TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");

		
		// Get a handle to the process.
	    HANDLE hProcess = OpenProcess( PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_TERMINATE,
                                   FALSE, dwProcesses[dwProcessCount] );

		 // Get the process name.
	    if (NULL != hProcess )
		{
			DWORD				cbNeeded=sizeof(szProcessName)/sizeof(TCHAR);

			if(fpQueryFullProcessImageName(hProcess,0,szProcessName,&cbNeeded))
			{
				if(_tcsstr(szProcessName,_T("msra.exe")))
				{
					DWORD  dwExitCode;
					GetExitCodeProcess(hProcess,&dwExitCode);
					TerminateProcess(hProcess,dwExitCode);
					CloseHandle( hProcess );
					FreeLibrary (hKernelDLL);
					return ;
				}
			}
			CloseHandle( hProcess );
		}//NULL != hProcess
	}//for

	FreeLibrary (hKernelDLL);
}