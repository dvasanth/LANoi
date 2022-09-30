#include "StdAfx.h"
#include "RemoteAssitance.h"
#include "RegOperations.h"
#include "msra_h.h"
#include <lm.h>
#include "../resource.h"
#include <strsafe.h>

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
	Restore.EnableRA(m_bRAEnabled);
	EnableRAUserAccount(m_bHAUserAccountEnabled);
	if(IsWindowsVista()) return;
	Restore.EnableRAUnSolicitPolicy(m_bRAUnSolicitedEnabled);
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
BOOL CRemoteAssitance::CreateInvitation(CString sPeerName,CString &sArgInvitationString,CString &sRDPListeningPort)
{
	EnableRAPolicy();
	if(IsWindowsVista())
	{
		if(!CreateVistaInvitation(sPeerName,sArgInvitationString,sRDPListeningPort))
		{
			//guide the user for vista invitation creation
			//AfxMessageBox(IDS_VISTA_RA_GUIDE_HLP, MB_OK  |MB_ICONSTOP|MB_TOPMOST   ); 
			return CreateVistaInvitation(sPeerName,sArgInvitationString,sRDPListeningPort);
		}
		return TRUE;
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
//			AfxMessageBox(Err.GetLastErrorMessage(), MB_OK  |MB_ICONSTOP|MB_TOPMOST ); 
		}
		pPchService->Release(); 
	}
	SysFreeString(bstrUserName);
	SysFreeString(bstrHostname);
	SysFreeString(bstrUserHelpBlob);


	return bRet;
}


BOOL CRemoteAssitance::CreateVistaInvitation(CString sPeerName,CString &sArgInvitationString,CString &sRDPListeningPort)
{
	CString				sFileExt;
	CFileDialog			fOpenDlg(TRUE, 
								_T("msrcincident"), _T(""),
								OFN_HIDEREADONLY|OFN_FILEMUSTEXIST,
								(LPCTSTR)_T("Vista Remote Assistance Files(*.msrcincident)|*.msrcincident||"),
								NULL);

	
	// Initializes m_pOFN structure
	fOpenDlg.m_pOFN->lpstrTitle=_T("Select the Vista Remote Assistance File");

	//Select the file 
	if(fOpenDlg.DoModal()==IDOK)
	{
		HANDLE			hInvitationFile;

		
		hInvitationFile = CreateFile((LPCTSTR)fOpenDlg.GetPathName(), GENERIC_READ, FILE_SHARE_WRITE,
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
BOOL CRemoteAssitance::JoinInvitation(BYTE *pbyBlobData,DWORD dwDataLen)
{
	CString				sInvitationFilePath=theApp.GetApplicationDataPath()+_T("RemoteAssist");
	HANDLE				hFile;


	CreateDirectory(sInvitationFilePath,NULL);
	sInvitationFilePath+=_T("\\RAInvitation.msrcincident");
	hFile = CreateFile((LPCTSTR)sInvitationFilePath, GENERIC_WRITE, FILE_SHARE_WRITE,
                   NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if(hFile != INVALID_HANDLE_VALUE)
	{
		DWORD			dwWritten=0;
		BYTE			byHeader[2]={0xff,0xfe};	
		WriteFile(hFile,byHeader,sizeof(byHeader),&dwWritten,NULL);
		WriteFile(hFile,pbyBlobData,dwDataLen,&dwWritten,NULL);
		CloseHandle(hFile);

		//Launch Invitation File
		SHELLEXECUTEINFO 		shInfo={0};

		shInfo.cbSize =sizeof(shInfo);
		shInfo.lpVerb = _T("open");
		shInfo.lpFile = (LPCTSTR)sInvitationFilePath;
		shInfo.nShow = SW_SHOW;
		ShellExecuteEx(&shInfo);
		return TRUE;
	}

	return FALSE;
}
