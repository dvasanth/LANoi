// RegOperations.cpp: implementation of the CRegOperations class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RegOperations.h"
#include <strsafe.h>
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


#define  NICKNAME					_T("NickName")
#define  VIRTUAL_ADDRESS_START		_T("VirtualAddressStart")
#define  INSTALL_PATH              _T("InstallPath")
#define  TEREDO_FORWARD_PORT       _T("TeredoPortForward")
#define  DEFAULT_FORWARD_PORT		5757

#define	ENABLE_RA_KEY			_T("System\\CurrentControlSet\\Control\\Terminal Server")
#define ENABLE_RA_VALUE			_T("fAllowToGetHelp")

#define RA_UNSOLICIT_USER_ALLOW_DOMAIN_USER_KEY	_T("SOFTWARE\\Policies\\Microsoft\\Windows NT\\Terminal Services\\RAUnsolicit")

#define ENABLE_RA_UNSOLICIT_ALLOW_KEY			_T("SOFTWARE\\Policies\\Microsoft\\Windows NT\\Terminal Services")
#define ENABLE_RA_UNSOLICIT_VALUE_ALLOW			_T("fAllowUnsolicited")
#define ENABLE_RA_UNSOLICIT_VALUE_FULL_CONTROL  _T("fAllowUnsolicitedFullControl")
#define ENABLE_RA_UNSOLICIT_VALUE_ALLOWHELP     _T("fAllowToGetHelp")
#define ENABLE_RA_UNSOLICIT_VALUE_SALEM		     _T("fEnableSalem")
#define ENABLE_RA_UNSOLICIT_VALUE_SALEM		     _T("fEnableSalem")

//app integrate media server lsitening port
#define MEDIA_SERVER_PORT_VALUE_NAME			_T("MediaStreamingPort")
#define MEDIA_SERVER_DEFAULT_PORT				5758
//idle timeout related
#define IDLE_SESSION_TIMEOUT					_T("SessionTimeOut")
#define DEFAULT_IDLE_SESSIONTIMEOUT				60


const TCHAR	CRegOperations::PNRP_REG_PATH[]=_T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\PeerNet\\PNRP\\IPV6-Global\\Global_");
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRegOperations::CRegOperations()
{
	m_RegCurrentUser.Create(HKEY_CURRENT_USER,
					_T("SOFTWARE\\CrowSoft\\Lan On Internet Pro")
					);
	m_RegLocalMachine.Create(HKEY_LOCAL_MACHINE,
					_T("SOFTWARE\\CrowSoft\\Lan On Internet Pro")
					);
}

CRegOperations::~CRegOperations()
{
	m_RegCurrentUser.Close();  
	m_RegLocalMachine.Close();
}

CString CRegOperations::GetInstallPath()
{

	TCHAR      szPath[MAX_PATH]={0};
	DWORD      dwSize=sizeof(szPath)/sizeof(TCHAR);


	m_RegCurrentUser.QueryStringValue(INSTALL_PATH,szPath,&dwSize);

	return CString(szPath);
}

void CRegOperations::SetNickname(CString sNickName)
{
	m_RegCurrentUser.SetStringValue(NICKNAME,sNickName);
}

CString CRegOperations::GetNickname()
{
	TCHAR      szNickName[MAX_PATH]={0};
	DWORD      dwSize=sizeof(szNickName)/sizeof(TCHAR);


	m_RegCurrentUser.QueryStringValue(NICKNAME,szNickName,&dwSize);

	return CString(szNickName);
	
}



DWORD CRegOperations::GetStartVirtualAddress(CString sSessionName)
{
	DWORD      dwValue=0;
	DWORD      dwSize=sizeof(DWORD);

	m_RegLocalMachine.QueryDWORDValue(VIRTUAL_ADDRESS_START,dwValue);
	//load session specfic virtual address
	if(m_RegLocalMachine.QueryDWORDValue(sSessionName+VIRTUAL_ADDRESS_START,dwValue)!=ERROR_SUCCESS )
	{
		m_RegLocalMachine.QueryDWORDValue(VIRTUAL_ADDRESS_START,dwValue);
	}
	return dwValue;

}

DWORD CRegOperations::GetMediaServerListeningPort()
{
	DWORD      dwValue=MEDIA_SERVER_DEFAULT_PORT;
	DWORD      dwSize=sizeof(DWORD);

	m_RegCurrentUser.QueryDWORDValue(MEDIA_SERVER_PORT_VALUE_NAME,dwValue);
	return dwValue;
}
/************************************************
Get/Set for idle timeout
************************************************/
DWORD CRegOperations::GetIdleTimeout()
{
	DWORD      dwValue=DEFAULT_IDLE_SESSIONTIMEOUT;
	DWORD      dwSize=sizeof(DWORD);

	m_RegLocalMachine.QueryDWORDValue(IDLE_SESSION_TIMEOUT,dwValue);
	if(dwValue==0)
	{
		dwValue=DEFAULT_IDLE_SESSIONTIMEOUT;
	}
	return dwValue;
}
void CRegOperations::SetIdleTimeOut(DWORD dwTimeOut)
{
	m_RegCurrentUser.SetDWORDValue(IDLE_SESSION_TIMEOUT,dwTimeOut);
}

/*******************************
teredo forward port 
********************************/
USHORT CRegOperations::GetTeredoForwardPort()
{
	DWORD      dwValue=DEFAULT_FORWARD_PORT;
	DWORD      dwSize=sizeof(DWORD);

	m_RegLocalMachine.QueryDWORDValue(TEREDO_FORWARD_PORT,dwValue);
	if(dwValue==0)
	{
		dwValue=DEFAULT_FORWARD_PORT;
	}
	return USHORT(dwValue);
}

/*******************************************
Remote assitance related
********************************************/
DWORD CRegOperations::EnableRA(BOOL bValue)
{
	CRegKey	   RARegKey;
	DWORD	   dwCurrentValue=0;

	RARegKey.Create(HKEY_LOCAL_MACHINE, ENABLE_RA_KEY);
	//store the current value
	RARegKey.QueryDWORDValue(ENABLE_RA_VALUE,dwCurrentValue);
	//change to  value specified
	RARegKey.SetDWORDValue(ENABLE_RA_VALUE,bValue);

	return dwCurrentValue;	
}

DWORD CRegOperations::EnableRAUnSolicitPolicy(BOOL bValue)
{
	CRegKey					RAUnsolicitDomainRegKey,RAUnsolicitAllowKey;
	DWORD					dwCurrentValue=0;
	TCHAR					szUserName[1024]={0};
	TCHAR					szHostName[1024]={0};
	TCHAR					szDomainUserName[1024]={0};
	DWORD					dwLen=0;

	//write the domain/user key
	RAUnsolicitDomainRegKey.Create(HKEY_LOCAL_MACHINE, RA_UNSOLICIT_USER_ALLOW_DOMAIN_USER_KEY);
	// Get user name
	dwLen= sizeof(szUserName)/sizeof(TCHAR);
	GetUserName(szUserName,&dwLen);
	// Get PC name
	dwLen=sizeof(szHostName);
	GetComputerNameEx(ComputerNameDnsHostname,szHostName,&dwLen);
	StringCbPrintf(szDomainUserName,sizeof(szDomainUserName),_T("%s\\%s"),szHostName,szUserName);
	RAUnsolicitDomainRegKey.SetStringValue( szDomainUserName,szDomainUserName);

	//Enable the policy values
	RAUnsolicitAllowKey.Create(HKEY_LOCAL_MACHINE,ENABLE_RA_UNSOLICIT_ALLOW_KEY);
	RAUnsolicitAllowKey.QueryDWORDValue(ENABLE_RA_UNSOLICIT_VALUE_ALLOW,dwCurrentValue);

	//change to  value specified
	RAUnsolicitAllowKey.SetDWORDValue(ENABLE_RA_UNSOLICIT_VALUE_ALLOW,bValue);
	RAUnsolicitAllowKey.SetDWORDValue(ENABLE_RA_UNSOLICIT_VALUE_FULL_CONTROL,bValue);
	RAUnsolicitAllowKey.SetDWORDValue(ENABLE_RA_UNSOLICIT_VALUE_ALLOWHELP,bValue);
	RAUnsolicitAllowKey.SetDWORDValue(ENABLE_RA_UNSOLICIT_VALUE_SALEM,bValue);
	return dwCurrentValue;
}

/*****************************************
Gets the configured pnrp servers
*******************************************/
CString CRegOperations::GetPNRPSeedServers()
{
	CRegKey				RegPNRPSeedServer;
	TCHAR				szSeedServer[1024]={0};
	DWORD				dwSize=sizeof(szSeedServer)/sizeof(TCHAR);

	RegPNRPSeedServer.Create(HKEY_LOCAL_MACHINE,PNRP_REG_PATH);

	RegPNRPSeedServer.QueryStringValue(_T("SeedServer"),szSeedServer,&dwSize);

	return CString(szSeedServer);
}
/**************************
Set  the PNRP seed server
**************************/
void CRegOperations::SetPNRPSeedServer(CString sSeedServer)
{
	CRegKey				RegPNRPSeedServer;
	TCHAR				szSeedServer[1024]={0};
	DWORD				dwSize=sizeof(szSeedServer)/sizeof(TCHAR);

	RegPNRPSeedServer.Create(HKEY_LOCAL_MACHINE,PNRP_REG_PATH);

	RegPNRPSeedServer.SetStringValue(_T("SeedServer"),sSeedServer);
}