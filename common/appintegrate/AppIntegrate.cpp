#include "StdAfx.h"
#include "AppIntegrate.h"
#include "netmeeting.h"
#include "RegOperations.h"
#include "loisession\LOISessionStringFormat.h"
#include "cmnresource.h"
const UINT CAppIntegrate::MEETINGNAMES[]={
					IDS_NETMEETING_SESSION_NAME,
					IDS_RDP_SESSION_NAME,
					IDS_RA_SESSION_NAME,
					IDS_REMOTE_STREAM_SESSION_NAME
				};
const USHORT CAppIntegrate::RDP_PORT=3389;

CAppIntegrate::CAppIntegrate(void)
{
	m_pbyAppBlobData=NULL;
	m_dwAppBlobDataLen=0;
	m_pRemoteAsst=NULL;
	//m_pRemoteStream=NULL;
}

CAppIntegrate::~CAppIntegrate(void)
{
	ReleaseSessionData();
}

void CAppIntegrate::ReleaseSessionData()
{
	if(m_pbyAppBlobData)
	{
		FreeAppDataBlob(m_pbyAppBlobData);
		m_pbyAppBlobData=NULL;
		m_dwAppBlobDataLen=0;
	}
	if(m_pRemoteAsst)
	{
		delete m_pRemoteAsst;
		m_pRemoteAsst=NULL;
	}
/*	if(m_pRemoteStream)
	{
		delete m_pRemoteStream;
		m_pRemoteStream=NULL;
	}*/
}
BOOL CAppIntegrate::GetAppDataBlob(CString sSessionName,CString sPeername,CString sPassword,CString &sNewSessionXML,PBYTE *ppByData,DWORD &dwInOutLen)
{
	for(int iSessionIndex=0;iSessionIndex<sizeof(MEETINGNAMES)/sizeof(MEETINGNAMES[0]);iSessionIndex++)
	{
		CString				sPreDefinedName;
		DWORD				dwInvitationBlobLen;

		sPreDefinedName.LoadString(MEETINGNAMES[iSessionIndex]);
		sPreDefinedName.MakeLower();
		sSessionName.MakeLower();
		if(sPreDefinedName==sSessionName)
		{
			switch(MEETINGNAMES[iSessionIndex])
			{
				case IDS_RA_SESSION_NAME:
					{
						m_pRemoteAsst =new CRemoteAssitance;
						CString			  	sInvitationFileData,sPort;
						BOOL				bRet;
			
						bRet= m_pRemoteAsst->CreateInvitation(sPeername,sPassword, sInvitationFileData,sPort);
						if(bRet)
						{
							CLOISessionStringFormat SessionXML;
							USHORT					usRAPort=(USHORT)_ttoi((LPCTSTR)sPort);

							SessionXML.AddPortRange(usRAPort, usRAPort,TRUE);
							SessionXML.AddPortRange(RDP_PORT,RDP_PORT,TRUE);
							sNewSessionXML=SessionXML.GetXMLString() ;
							//copy in custom blob data
							dwInvitationBlobLen=(_tcslen( (LPCTSTR)sInvitationFileData))*sizeof(TCHAR);
							*ppByData = new BYTE[dwInvitationBlobLen];
							dwInOutLen=dwInvitationBlobLen;
							memcpy(*ppByData,(LPCTSTR)sInvitationFileData,dwInvitationBlobLen);
							return TRUE;
						}
					}
					break;
				case IDS_REMOTE_STREAM_SESSION_NAME:
					{
					/*	CString sStreamPort;
						m_pRemoteStream = new CRemoteMediaStreaming;
						sStreamPort=m_pRemoteStream->GetServerPort();
						sNewPortString+=_T(",")+sStreamPort; 
						//add this appdata
						dwInvitationBlobLen=(_tcslen((LPCTSTR)sStreamPort))*sizeof(TCHAR);
						*ppByData=new BYTE[dwInvitationBlobLen+sizeof(TCHAR)];
						ZeroMemory(*ppByData,dwInvitationBlobLen+sizeof(TCHAR));
						memcpy(*ppByData,(LPCTSTR)sStreamPort,dwInvitationBlobLen);
						dwInOutLen=dwInvitationBlobLen+sizeof(TCHAR);
						if(!m_pRemoteStream->GetBroadCastParams())
						{
							delete m_pRemoteStream;
							m_pRemoteStream=NULL;
						}
						return TRUE;*/

					}
					break;
			}
		}
	}
	
	return FALSE;
}

void CAppIntegrate::FreeAppDataBlob(BYTE *byData)
{
	if(byData)
		delete byData;

}

BOOL CAppIntegrate::SetAppDataBlobData(BYTE *pByData,DWORD dwInLen)
{

	m_pbyAppBlobData= new BYTE[dwInLen];
	memcpy(m_pbyAppBlobData,pByData,dwInLen);
	m_dwAppBlobDataLen=dwInLen;
	return TRUE;
}

BOOL CAppIntegrate::PreInviterSessionStart(CString sSessionName,CString sNickName)
{
	BOOL		bRet=FALSE;

	sSessionName.MakeLower();
	for(int iSessionIndex=0;iSessionIndex<sizeof(MEETINGNAMES)/sizeof(MEETINGNAMES[0]);iSessionIndex++)
	{
		CString  sPreDefinedName;
		sPreDefinedName.LoadString(MEETINGNAMES[iSessionIndex]);
		sPreDefinedName.MakeLower();
		if(sPreDefinedName==sSessionName)
		{
			switch(MEETINGNAMES[iSessionIndex])
			{
				case IDS_NETMEETING_SESSION_NAME:
					{
				/*		CNetMeeting  HostMeet;
						HRESULT		 hResult;
						hResult=HostMeet.HostMeeting( sNickName);
						if(!SUCCEEDED(hResult))
							AfxMessageBox(HostMeet.GetLastErrorMessage(),MB_OK  |MB_ICONSTOP|MB_TOPMOST); 
						//bRet=TRUE;*/
					}
					break;
				case IDS_RDP_SESSION_NAME:
				{
					//bRet=TRUE;
				}
				break;
				case IDS_REMOTE_STREAM_SESSION_NAME:
					{
				/*		bRet=TRUE;
						if(m_pRemoteStream)
						{
							bRet=m_pRemoteStream->StartTheBroadcast(m_pRemoteStream->GetServerPort()); 
					
							bRet=!bRet;//if false then don't the success message box
						}*/
					}
					break;
			}//switch
		}
	}
	return bRet;
}

DWORD CAppIntegrate::GetVirtualAddressForSession(CString sSessionName)
{
	CRegOperations Reg;

	return Reg.GetStartVirtualAddress(sSessionName);
}

BOOL CAppIntegrate::PreInviteeSessionStart(CString sSessionName,CString sNickName)
{
	BOOL  bRet=FALSE;

	sSessionName.MakeLower();
	for(int iSessionIndex=0;iSessionIndex<sizeof(MEETINGNAMES)/sizeof(MEETINGNAMES[0]);iSessionIndex++)
	{
		CString  sPreDefinedName;
		sPreDefinedName.LoadString(MEETINGNAMES[iSessionIndex]);
		sPreDefinedName.MakeLower();
		if(sPreDefinedName==sSessionName)
		{
			switch(MEETINGNAMES[iSessionIndex])
			{
				case IDS_NETMEETING_SESSION_NAME:
				{
			/*		CNetMeeting  Join;
					HRESULT		 hResult;
					hResult=Join.JoinMeeting( sNickName);
					if(!SUCCEEDED(hResult))
						AfxMessageBox(Join.GetLastErrorMessage(),MB_OK  |MB_ICONSTOP|MB_TOPMOST); 
					bRet=TRUE;*/
				}
				break;
				case IDS_RDP_SESSION_NAME:
				{
					SHELLEXECUTEINFO 	 shInfo={0};
					CString				sCmdLine;
					sCmdLine.Format(_T("-v:%s"),(LPCTSTR)sNickName);
					shInfo.cbSize =sizeof(shInfo);
					shInfo.lpVerb = _T("open");
					shInfo.lpFile = _T("mstsc.exe");
					shInfo.lpParameters= (LPCTSTR)sCmdLine;
					shInfo.nShow = SW_SHOW;
					ShellExecuteEx(&shInfo);
					bRet=TRUE;
				}
				break;
				case IDS_RA_SESSION_NAME:
					{
						CRemoteAssitance    RA;
						if(m_pbyAppBlobData)
						{
							bRet= RA.JoinInvitation(m_pbyAppBlobData,m_dwAppBlobDataLen );
						}
						
					}
					break;
				case IDS_REMOTE_STREAM_SESSION_NAME:
					{
					/*	CRemoteMediaStreaming	LaunchPlayer;
						TCHAR					szPort[MAX_PATH]={0};
						CString					sPort;
						
						bRet=TRUE;
						memcpy(szPort,m_pbyAppBlobData,m_dwAppBlobDataLen>sizeof(szPort)?sizeof(szPort):m_dwAppBlobDataLen);
						sPort = szPort;
						LaunchPlayer.PlayStream(sNickName,sPort); */
					}
					break;
			}//switch
		}//sPreDefinedName==sSessionName
	}//for
	
	return bRet;
}