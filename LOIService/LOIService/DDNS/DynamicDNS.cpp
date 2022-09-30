#include "StdAfx.h"
#include "DynamicDNS.h"
#include <pnrpns.h> 
#include "../resource.h"
#include "IPv6DNSResolver.h"
#include "RegOperations.h"
//constants
const TCHAR CDynamicDNS::FRIENDLY_NAME[]=_T("LANoiPro 3.0");
const TCHAR	CDynamicDNS::DNS_COMMENT[]=_T("LAN On Internet 3.0");
const DWORD	CDynamicDNS::DNS_UPDATE_INTERVAL=500*1;//seconds
const TCHAR	CDynamicDNS::CLOUD_NAME[]=_T("Global_");
const TCHAR	CDynamicDNS::PNRP_SEED_SERVER[]=_T("pnrpv2.ipv6.microsoft.com");

CDynamicDNS::CDynamicDNS(void)
{
    WSADATA     wsaData = {0};
    WSAStartup(0x0002, &wsaData);
    
    // Setup PNRP infrastructure
    PeerPnrpStartup(PNRP_VERSION);

	//init
	ZeroMemory(&m_RegInfo,sizeof(m_RegInfo));
	ZeroMemory(&m_DNSIpv6SockAddr,sizeof(m_DNSIpv6SockAddr));
	m_hRegistration=NULL;
	m_hDNSRegistrationStopThread=CreateEvent(NULL,FALSE,FALSE,NULL);
	m_hDNSRegistrationThread=NULL;
}

CDynamicDNS::~CDynamicDNS(void)
{
	UnRegisterDNSName();
	if(m_hDNSRegistrationStopThread)
	{
		CloseHandle(m_hDNSRegistrationStopThread);
		m_hDNSRegistrationStopThread=NULL;
	}
	PeerPnrpShutdown ();
	WSACleanup(); 
	ClosePNRP();

	
}
/****************************************
Initialise PNRP 
******************************************/
void CDynamicDNS::InitPNRP()
{
	CIPv6DNSResolver		PNRPServerResolve;

	if(!PNRPServerResolve.IsResolvable(PNRP_SEED_SERVER) )
	{
		in_addr6   ipv6Addr;
		//address not resolveable do manual resolve
		if(PNRPServerResolve.ManualResolve (PNRP_SEED_SERVER,ipv6Addr))
		{
			CRegOperations Reg;
			m_sOldPNRPServer=Reg.GetPNRPSeedServers();
			Reg.SetPNRPSeedServer(GetFormattedIPv6(ipv6Addr)+_T(";")+m_sOldPNRPServer);
			DEBUGLOG(LOI_ERROR,_T("Seed server changed to %s"),(LPCTSTR)(GetFormattedIPv6(ipv6Addr)+_T(";")+m_sOldPNRPServer));
		}
	}

}
/*******************************
Close PNRP
*******************************/
void CDynamicDNS::ClosePNRP()
{
	CRegOperations Reg;
	if(!m_sOldPNRPServer.IsEmpty())
	{
		Reg.SetPNRPSeedServer(m_sOldPNRPServer);
		DEBUGLOG(LOI_ERROR,_T("Seed server changed to %s"),(LPCTSTR)(m_sOldPNRPServer));
	}

}
/**************************************************************
Registers via PNRP name
*****************************************************************/
BOOL CDynamicDNS::RegisterDNSName(in_addr6 DNSMappedIPv6Addr,USHORT usDNSServicePort,BYTE *pDNSBlobData,DWORD dwDNSBlobDataLen)
{
	CString					sIdentity;
	HRESULT					hResult=S_OK;
	PWSTR					pwzPeerName=NULL;
	SOCKADDR**              ppRegAddr = NULL;
	

	UnRegisterDNSName();
	hResult = GetDefaultIdentity(sIdentity);
	if(FAILED(hResult)) 
	{
		//log the error
		SetLastErrorMessage(IDS_ERR_P2P_IDENTITY_CREATE, hResult);
		DEBUGLOG(LOI_ERROR,_T("GetDefaultIdentity failed %d"),hResult);
		return FALSE;
	}

	hResult=PeerCreatePeerName((LPCTSTR)sIdentity,NULL,&pwzPeerName);
	if(FAILED(hResult))
	{
		//log the error
		DEBUGLOG(LOI_ERROR,_T("PeerCreatePeerName failed %d"),hResult);
		SetLastErrorMessage(IDS_ERR_P2P_NAME_REGISTER,hResult); 
		return FALSE;
	}

	///fill the registration information
	ZeroMemory(&m_DNSIpv6SockAddr,sizeof(m_DNSIpv6SockAddr));
	m_DNSIpv6SockAddr.sin6_addr = DNSMappedIPv6Addr;
	m_DNSIpv6SockAddr.sin6_port = usDNSServicePort;
	m_DNSIpv6SockAddr.sin6_family = AF_INET6;
	m_RegInfo.pwzCloudName=(LPWSTR)CLOUD_NAME;

	m_RegInfo.pwzComment = (PWSTR)DNS_COMMENT;
	//address to be register
	ppRegAddr = (SOCKADDR**)new BYTE[sizeof(SOCKADDR*) * 1];
	ppRegAddr[0]= (SOCKADDR*)&m_DNSIpv6SockAddr;
	m_RegInfo.ppAddresses=ppRegAddr;
	m_RegInfo.cAddresses =1;
	m_RegInfo.wPort=  usDNSServicePort;
	
//	m_RegInfo.pwzPublishingIdentity = new WCHAR[MAX_PATH];
//	StringCbCopy(m_RegInfo.pwzPublishingIdentity,sizeof(WCHAR)*MAX_PATH,(LPCTSTR)pwzPeerName);

	if(dwDNSBlobDataLen)
	{
		m_RegInfo.payload.cbData=dwDNSBlobDataLen;
		m_RegInfo.payload.pbData= new BYTE[dwDNSBlobDataLen];
		memcpy(m_RegInfo.payload.pbData,pDNSBlobData,m_RegInfo.payload.cbData);
	}

	hResult=PeerPnrpRegister(pwzPeerName,&m_RegInfo,&m_hRegistration);
	if (FAILED(hResult))
    {
		//log the error
		if (pwzPeerName) PeerFreeData(pwzPeerName);
		UnRegisterDNSName();
		DEBUGLOG(LOI_ERROR,_T("PeerPnrpRegister failed %d"),hResult);
		SetLastErrorMessage(IDS_ERR_P2P_NAME_REGISTER,hResult); 
		return FALSE;
	}

	///registration is success
	m_sDNSName=CString(pwzPeerName);
	if(m_sDNSName.IsEmpty())
	{
		DEBUGLOG(LOI_ERROR,_T("PeerPnrpRegister return null"));
		SetLastErrorMessage(IDS_ERR_P2P_NAME_REGISTER,hResult); 
		return FALSE;
	}

	if (pwzPeerName ) PeerFreeData(pwzPeerName);

	DEBUGLOG(LOI_INFO,_T("PeerPnrpRegister return %s"),(LPCTSTR)m_sDNSName);
	//start the upadting thread
	StartUpdateRegistrationThread();
	return TRUE;
}
/*****************************
windows has PNRP resolve problem try to resolve it as much as possible
******************************/
BOOL CDynamicDNS::ResolveDNSName(CString sDNSName,in_addr6 &DNSMappedIPv6Addr,USHORT &usDNSServicePort)
{
	const UINT			DNS_RESOLVE_RETRY=5;
	INT					iRetryCount=0;
	BOOL				bRet;

	do{
		bRet=_ResolveDNSName(sDNSName,DNSMappedIPv6Addr,usDNSServicePort);
		iRetryCount++;
	}while(iRetryCount<=DNS_RESOLVE_RETRY && bRet==FALSE);

	return bRet;
}
BOOL CDynamicDNS::_ResolveDNSName(CString sDNSName,in_addr6 &DNSMappedIPv6Addr,USHORT &usDNSServicePort)
{
	DWORD				querySize   = 1;
    int					retVal      = 0;
    HANDLE				hLookUp     = 0;
    WSAQUERYSET			querySet    = {0};
	BLOB				Blob		= {0};
    PNRPINFO			pnrpInfo   = {0};
    HRESULT				hResult		= S_FALSE;
	GUID				SVCID_PNRPNAME_V1={0xc2239ce5, 0x00c0, 0x4fbf, 0xba, 0xd6, 0x18, 0x13, 0x93, 0x85, 0xa4, 0x9a};
	WSAQUERYSET			*pResults = NULL;


    // Begin Cloud Enumeration for all clouds in this machine.
    pnrpInfo.dwSize = sizeof(PNRPINFO);
	pnrpInfo.nMaxResolve = 1;
	pnrpInfo.dwTimeout = 200;
	pnrpInfo.enResolveCriteria =PNRP_RESOLVE_CRITERIA_ANY_PEER_NAME;

    Blob.cbSize = sizeof(pnrpInfo);
    Blob.pBlobData = (LPBYTE)&pnrpInfo;
    querySet.lpBlob = &Blob;

    querySet.dwSize = sizeof(WSAQUERYSET);
    querySet.dwNameSpace = NS_PNRPNAME;
    querySet.lpServiceClassId = (LPGUID)(&SVCID_PNRPNAME);
	querySet.lpszServiceInstanceName =  (LPWSTR)(LPCTSTR)sDNSName;

	querySet.lpszContext = (LPWSTR)CLOUD_NAME; 

    retVal = WSALookupServiceBegin(&querySet, LUP_RETURN_ADDR|LUP_RETURN_NAME|LUP_RETURN_COMMENT, &hLookUp);
    if (retVal != 0)
    {
		retVal = WSAGetLastError();
		//log error
		DEBUGLOG(LOI_ERROR,_T("WSALookupServiceBegin failed %d"),retVal);
        SetLastErrorMessage(IDS_ERR_P2P_NAME_RESOLVE,retVal); 
        return FALSE;
    }


    querySize = 0;
    
    // We purposefully give it no space the first time so
    // it will fail and tell us how much space we need
    retVal = WSALookupServiceNext(hLookUp, 0, &querySize,&querySet);
    retVal = WSAGetLastError();

    if (retVal == WSAEFAULT)
    {
        pResults = (WSAQUERYSET*) LocalAlloc(LPTR, querySize);
        retVal = WSALookupServiceNext(hLookUp, 0, &querySize,pResults);
	}

    // retrieve the ipv6 address & port
    if (retVal == 0)
    {
		for (UINT iAddresses = 0; iAddresses < pResults->dwNumberOfCsAddrs;iAddresses++)
		{
			if(pResults->lpcsaBuffer->RemoteAddr.iSockaddrLength==sizeof(SOCKADDR_IN6))
			{
				DNSMappedIPv6Addr=((SOCKADDR_IN6*)pResults->lpcsaBuffer->RemoteAddr.lpSockaddr)->sin6_addr;
				usDNSServicePort=((SOCKADDR_IN6*)pResults->lpcsaBuffer->RemoteAddr.lpSockaddr)->sin6_port;
				DEBUGLOG(LOI_ERROR,_T("found ipv6 addr %d:%d:%d port: %d"),DNSMappedIPv6Addr.u.Word[5],DNSMappedIPv6Addr.u.Word[6],DNSMappedIPv6Addr.u.Word[7], usDNSServicePort  ); 
				WSALookupServiceEnd(hLookUp);
				if(pResults)LocalFree(pResults);
				return TRUE;
			}
		}
		
	}
	DEBUGLOG(LOI_ERROR,_T("failed to resolve name %s Error:%d"),(LPCTSTR)sDNSName,retVal); 
	if(pResults)LocalFree(pResults);
    // End Cloud enumeration and shutdown WSA.
    WSALookupServiceEnd(hLookUp);
	SetLastErrorMessage(IDS_ERR_P2P_NAME_RESOLVE,retVal);
	//log the error
    return FALSE;
}
void CDynamicDNS::UnRegisterDNSName()
{
	HRESULT					hResult=S_OK;

	StopUpdateRegistrationThread();
	if(m_hRegistration)
	{
		hResult=PeerPnrpUnregister(m_hRegistration);
		m_hRegistration=NULL;
	}
	if(m_RegInfo.payload.pbData)
	{
		delete m_RegInfo.payload.pbData;
		m_RegInfo.payload.pbData=NULL;
	}

	if(m_RegInfo.ppAddresses)
	{
		delete m_RegInfo.ppAddresses;
		m_RegInfo.ppAddresses=NULL;
	}

	if(m_RegInfo.pwzPublishingIdentity )
	{
		delete m_RegInfo.pwzPublishingIdentity ;
		m_RegInfo.pwzPublishingIdentity =NULL;
	}
}


void CDynamicDNS::StartUpdateRegistrationThread()
{
	m_hDNSRegistrationThread=CreateThread(NULL,0,DNSRegistrationUpdateProc,this,0,NULL);
}

void CDynamicDNS::StopUpdateRegistrationThread()
{
	if(m_hDNSRegistrationThread)
	{
		SetEvent(m_hDNSRegistrationStopThread); 
		WaitForSingleObject( m_hDNSRegistrationThread,15000);
		CloseHandle(m_hDNSRegistrationThread);
		m_hDNSRegistrationThread=NULL;
	}
}

DWORD WINAPI CDynamicDNS::DNSRegistrationUpdateProc(LPVOID lpParam)
{
	CDynamicDNS			*pThis=(CDynamicDNS*)lpParam;

	while(1)
	{
		switch(WaitForSingleObject( pThis->m_hDNSRegistrationStopThread,DNS_UPDATE_INTERVAL*1000))
		{
			case WAIT_TIMEOUT:
				pThis->UpdateRegistration();
				break;
			default:
				return 0;
				break;
		}
	}

	return 0;

}
void CDynamicDNS::UpdateRegistration()
{		
	HRESULT hResult;
	if(m_hRegistration)
	{

		hResult=PeerPnrpUpdateRegistration(m_hRegistration,&m_RegInfo);
		if(FAILED(hResult))
		{
			DEBUGLOG(LOI_ERROR,_T("Failed PeerPnrpUpdateRegistration %d"),hResult);
		}
	}
}

HRESULT CDynamicDNS::GetDefaultIdentity(CString &sIdentity)
{
	HRESULT						hResult=E_FAIL;
    HPEERENUM                   hIdentityEnum = NULL;
    ULONG                       ulIdentities = 0;
	PWSTR						pwzIdentity=NULL;
	PEER_NAME_PAIR**            ppNamePairs = NULL;

    // Retreieve list of available identities
    hResult = PeerEnumIdentities(&hIdentityEnum);
    if (SUCCEEDED(hResult))
    {
        hResult = PeerGetItemCount(hIdentityEnum, &ulIdentities);
        if (SUCCEEDED(hResult) && ulIdentities )
        {
            hResult = PeerGetNextItem(hIdentityEnum, &ulIdentities, (PVOID **) &ppNamePairs);
	        for (UINT iIdentities = 0; iIdentities < ulIdentities; iIdentities++)
	        {
				if(_tcsicmp(ppNamePairs[iIdentities]->pwzFriendlyName,FRIENDLY_NAME)==0)
				{
					sIdentity=ppNamePairs[iIdentities]->pwzPeerName;
					PeerEndEnumeration(hIdentityEnum);
					PeerFreeData(ppNamePairs);
					DEBUGLOG(LOI_ERROR,_T("Found existing PeerIdentity"));
					return S_OK;
				}
			}
            PeerEndEnumeration(hIdentityEnum);
        }
	}

	DEBUGLOG(LOI_ERROR,_T("Enter PeerIdentityCreate"));
	//identity not found create it
	hResult=PeerIdentityCreate(NULL,FRIENDLY_NAME,0,&pwzIdentity);
	if (SUCCEEDED(hResult))
	{
		sIdentity=pwzIdentity;
		PeerFreeData(pwzIdentity);
		return S_OK;
	}

	DEBUGLOG(LOI_ERROR,_T("Enter PeerIdentityGetDefault"));
	//get the default identity
	hResult=PeerIdentityGetDefault(&pwzIdentity);
	if (SUCCEEDED(hResult))
	{
		sIdentity=pwzIdentity;
		PeerFreeData(pwzIdentity);
		return S_OK;
	}
	DEBUGLOG(LOI_ERROR,_T("GetDefaultIdentity error %X"),hResult);
	//something went wrong
	return hResult;

}

CString CDynamicDNS::GetDNSName()
{
	return m_sDNSName;
}