#include "StdAfx.h"
#include "P2PInvitation.h"
#include "XmlSerializer.h"
/**********************************
const definitions
**********************************/
const TCHAR	CP2PInvitation::INVITATE_FILE_HEADER[]=_T("LANOiPro.1");
const TCHAR	CP2PInvitation::INVITATION_NODE_NAME[]=_T("Invitation");
const TCHAR	CP2PInvitation::NICKNAME_ATTRIBUTE_NAME[]=_T("Nickname");
const TCHAR	CP2PInvitation::DDNSNAME_ATTRIBUTE_NAME[]=_T("DDNSNAME");
const TCHAR	CP2PInvitation::IPV6ADDR_ATTRIBUTE_NAME[]=_T("IPV6Addr");
const TCHAR	CP2PInvitation::IPV6LISTENINGPORT_ATTRIBUTE_NAME[]=_T("IPV6Port");
const TCHAR	CP2PInvitation::APPDATA_ATTRIBUTE_NAME[]=_T("Appdata");
const TCHAR	CP2PInvitation::CA_CERT_FILENAME[]=_T("LANoiCA.cert");
const TCHAR CP2PInvitation::CA_PASSWORD[]=_T("pass");
const TCHAR CP2PInvitation::CA_CERT_SUBJECTNAME[]=_T("CN=LANoipro CA");
const TCHAR CP2PInvitation::SESSION_DISPLAY_NAME[]=_T("SessionDisplayName");
const TCHAR CP2PInvitation::SESSION_INFO_XML_STRING[]=_T("SessionInfo");
const TCHAR CP2PInvitation::LOCAL_CACHE_PASS[]=_T("hi");
const TCHAR CP2PInvitation::STORE_PATH[]=_T("\\crowsoft\\LANoipro\\store\\");
const TCHAR CP2PInvitation::LOCAL_INVITATION_STORE_FOLDERNAME[]=_T("local\\");


CP2PInvitation::CP2PInvitation()
{
	TCHAR		szPath[MAX_PATH]={0};

	SHGetFolderPath(NULL,CSIDL_COMMON_APPDATA ,NULL,SHGFP_TYPE_CURRENT,szPath);
	m_sCertStorePath+=szPath;
	m_sCertStorePath+=STORE_PATH;
	SHCreateDirectoryEx(NULL,(LPCTSTR)m_sCertStorePath,NULL);

	m_sCACertFilePath=m_sCertStorePath+CA_CERT_FILENAME;
	m_sLocalCachedInvitationsFolder=m_sCertStorePath+LOCAL_INVITATION_STORE_FOLDERNAME;
	SHCreateDirectoryEx(NULL,(LPCTSTR)m_sLocalCachedInvitationsFolder,NULL);
	ZeroMemory(&m_ftValidityTime,sizeof(m_ftValidityTime));
	m_bInvitationExpired=FALSE;
	m_pbyAppData=NULL;
	m_dwAppDataLen=0;
}

CP2PInvitation::~CP2PInvitation(void)
{
	FreeAppdata();
}

/***********************************************
create a ca cert if file not found 
***********************************************/
PCCERT_CONTEXT CP2PInvitation::GetCACert()
{
	CLOICert		CACert;


	if(!CACert.Deserialise(m_sCACertFilePath, CA_PASSWORD))
	{
		SYSTEMTIME			stTime={0};
		FILETIME			ftTime={0};	

		GetSystemTime(&stTime);
		//increment years by 2 for ca cert
		stTime.wYear+=2;
		SystemTimeToFileTime(&stTime,&ftTime);

		//create new cert
		CACert.CreateCertificate(CA_CERT_SUBJECTNAME,ftTime,NULL);

		//serilse the file
		CACert.Serialise(m_sCACertFilePath,CA_PASSWORD);

		return CACert.DuplicateCertContext();
	}

	return CACert.DuplicateCertContext();
	
}
/**********************
returns the subject name of the cert
***********************/
CString CP2PInvitation::GetInvitationObjectID()
{
	//return uuid string of the client cert subject name
	return m_sObjectID; 
}
/*********************************
Gets the unique invitation ID from cert context
*********************************/
CString CP2PInvitation::GetInvitationObjectID(PCCERT_CONTEXT pCertContext)
{
	CLOICert		Cert(pCertContext);

	//get the subject name from the cert
	//return the UUID of the subject
	return Cert.GetCertSubjectName(); 
}
/*******************************
Save the invitation data to a file for third party
*******************************/
BOOL CP2PInvitation::CreateSecureInvitation(CString sInvitationFilePath,CString sPassword,FILETIME ftExpiryTime)
{
	CLOICert			ClientCert;
	UUID				Uuid;
	LPTSTR				lpszSubjectName = NULL;
	CString				sObjectID;
	PCCERT_CONTEXT		pCACertContext=NULL;
	BOOL				bRet=FALSE;
	CString				sSubjectName;

	// convert random uuid to a string, we will use it as a container
	UuidCreate(&Uuid);
	UuidToString(&Uuid, (RPC_WSTR *)&lpszSubjectName);
	sSubjectName.Format(_T("CN=%s"),lpszSubjectName);
	sObjectID=lpszSubjectName;
	RpcStringFree((RPC_WSTR *)&lpszSubjectName);

	pCACertContext=GetCACert();
	if(!pCACertContext)
	{

		goto cleanup;
	}
	

	//create the client cert first
	if(!ClientCert.CreateCertificate(sSubjectName,ftExpiryTime,pCACertContext ))
	{
		SetLastErrorMessage(ClientCert.GetLastErrorMessage() ); 
		DEBUGLOG(LOI_ERROR,(LPTSTR)(LPCTSTR)GetLastErrorMessage());
		goto cleanup;
	}

	//write all invitation params to the cert
	ClientCert.AddCertParam(NICKNAME_ATTRIBUTE_NAME,m_sNickName);
	ClientCert.AddCertParam(SESSION_DISPLAY_NAME,m_sSessionName);
	ClientCert.AddCertParam(DDNSNAME_ATTRIBUTE_NAME,m_sP2PDDNSName);
	ClientCert.AddCertParam(IPV6ADDR_ATTRIBUTE_NAME,(PBYTE)&m_IPv6Addr,sizeof(m_IPv6Addr));
	ClientCert.AddCertParam(IPV6LISTENINGPORT_ATTRIBUTE_NAME,(PBYTE)&m_usIpv6ListeningPort,sizeof(m_usIpv6ListeningPort));
	if(m_pbyAppData)
	{
		ClientCert.AddCertParam(APPDATA_ATTRIBUTE_NAME,m_pbyAppData,m_dwAppDataLen);
	}

	//serialise the cert to a file with password
	if(!ClientCert.Serialise(sInvitationFilePath,sPassword))
	{
		SetLastErrorMessage(ClientCert.GetLastErrorMessage());  
		DEBUGLOG(LOI_ERROR,(LPTSTR)(LPCTSTR)GetLastErrorMessage());
		goto cleanup;
	}

	//save to local 
	SaveCachedInvitation(ClientCert.DuplicateCertContext(),sObjectID);
	m_sObjectID=sObjectID;
	bRet=TRUE;


cleanup:
	if(pCACertContext)
	{
		CertFreeCertificateContext(pCACertContext); 
	}
	return bRet;
}
/***********************************************
create the secure invitation by id
***********************************************/
BOOL CP2PInvitation::CreateSecureInvitationByID(CString	sObjectID,CString sFilePath,CString sPassword)
{
	
	BOOL				bRet=FALSE;
	CString				sSubjectName;
	PCCERT_CONTEXT		pCert=NULL;

	pCert=LoadCachedInvitaionByObjectID(sObjectID);
	if(pCert==NULL)
	{
		return FALSE;
	}
	CLOICert			ClientCert(pCert);
	//write all invitation params to the cert
	ClientCert.AddCertParam(NICKNAME_ATTRIBUTE_NAME,m_sNickName);
	ClientCert.AddCertParam(SESSION_DISPLAY_NAME,m_sSessionName);
	ClientCert.AddCertParam(DDNSNAME_ATTRIBUTE_NAME,m_sP2PDDNSName);
	ClientCert.AddCertParam(IPV6ADDR_ATTRIBUTE_NAME,(PBYTE)&m_IPv6Addr,sizeof(m_IPv6Addr));
	ClientCert.AddCertParam(IPV6LISTENINGPORT_ATTRIBUTE_NAME,(PBYTE)&m_usIpv6ListeningPort,sizeof(m_usIpv6ListeningPort));
	if(m_pbyAppData)
	{
		ClientCert.AddCertParam(APPDATA_ATTRIBUTE_NAME,m_pbyAppData,m_dwAppDataLen);
	}

	//serialise the cert to a file with password
	if(!ClientCert.Serialise(sFilePath,sPassword))
	{
		SetLastErrorMessage(ClientCert.GetLastErrorMessage());  
		DEBUGLOG(LOI_ERROR,(LPTSTR)(LPCTSTR)GetLastErrorMessage());
		goto cleanup;
	}

	//save to local 
	bRet=TRUE;


cleanup:
	if(pCert)
	{
		CertFreeCertificateContext(pCert);
		pCert=NULL;
	}
	return bRet;
}

/********************************************
writes the invitation file to the local cache
***********************************************/
BOOL CP2PInvitation::SaveCachedInvitation(PCCERT_CONTEXT pCertContext,CString sObjectID)
{
	CLOICert			ClientCert(pCertContext);
	CString				sCacheLocalPath;

	//write all invitation params to the cert
	ClientCert.AddCertParam(NICKNAME_ATTRIBUTE_NAME,m_sNickName);
	ClientCert.AddCertParam(SESSION_DISPLAY_NAME,m_sSessionName);
	ClientCert.AddCertParam(DDNSNAME_ATTRIBUTE_NAME,m_sP2PDDNSName);
	ClientCert.AddCertParam(IPV6ADDR_ATTRIBUTE_NAME,(PBYTE)&m_IPv6Addr,sizeof(m_IPv6Addr));
	ClientCert.AddCertParam(IPV6LISTENINGPORT_ATTRIBUTE_NAME,(PBYTE)&m_usIpv6ListeningPort,sizeof(m_usIpv6ListeningPort));
	if(m_pbyAppData)
	{
		ClientCert.AddCertParam(APPDATA_ATTRIBUTE_NAME,m_pbyAppData,m_dwAppDataLen);
	}

	//write to the local with session information
	ClientCert.AddCertParam(SESSION_INFO_XML_STRING,m_sSessionXMLString);
	//form the local path to cache
	sCacheLocalPath = m_sLocalCachedInvitationsFolder+sObjectID+_T(".cert");
	//serialise the cert to a file with password
	if(!ClientCert.Serialise(sCacheLocalPath,(LPCTSTR)LOCAL_CACHE_PASS))
	{
		SetLastErrorMessage(ClientCert.GetLastErrorMessage());  
		DEBUGLOG(LOI_ERROR,(LPTSTR)(LPCTSTR)GetLastErrorMessage());
		CertFreeCertificateContext(pCertContext);
		return FALSE;
	}
	CertFreeCertificateContext(pCertContext);

	return TRUE;
	
}
/**********************************
Reads the invitation with all its param & returns the duplicate cert context should be freed by the caller
***********************************/
PCCERT_CONTEXT CP2PInvitation::LoadSecureInvitation(CString sInvitationFilePath,CString sPassword)
{
	CLOICert			ClientCert;
	BOOL				bRet=FALSE;
	PCCERT_CONTEXT		pCert=NULL;
	DWORD				dwSize=0;

	//create the LOIcert from file
	if(!ClientCert.Deserialise(sInvitationFilePath,sPassword))
	{
		SetLastErrorMessage(ClientCert.GetLastErrorMessage()); 
		DEBUGLOG(LOI_ERROR,(LPTSTR)(LPCTSTR)GetLastErrorMessage());
		goto cleanup;
	}

	//read all params from the cert
	m_sNickName=ClientCert.GetCertparam(NICKNAME_ATTRIBUTE_NAME);
	m_sSessionName=ClientCert.GetCertparam(SESSION_DISPLAY_NAME);
	m_sP2PDDNSName=ClientCert.GetCertparam(DDNSNAME_ATTRIBUTE_NAME);
	dwSize=sizeof(m_IPv6Addr);
	ClientCert.GetCertparam(IPV6ADDR_ATTRIBUTE_NAME,(PBYTE)&m_IPv6Addr,dwSize);
	dwSize=sizeof(m_usIpv6ListeningPort);
	ClientCert.GetCertparam(IPV6LISTENINGPORT_ATTRIBUTE_NAME,(PBYTE)&m_usIpv6ListeningPort,dwSize);
	
	//load appdata if exists
	FreeAppdata(); //free existing appdata
	m_pbyAppData=new BYTE[20*1024];
	m_dwAppDataLen=20*1024;
	if(!ClientCert.GetCertparam(APPDATA_ATTRIBUTE_NAME,m_pbyAppData,m_dwAppDataLen))
	{
		//free memory no appdata
		FreeAppdata(); 
	}

	pCert=ClientCert.DuplicateCertContext();  

	m_sObjectID=ClientCert.GetCertSubjectName(); 
	ClientCert.GetCertExpiryTime( m_ftValidityTime);
	m_bInvitationExpired=ClientCert.IsCertExpired(); 

	

cleanup:
	return pCert;
}
/****************************************
loads by invitation id
*****************************************/
PCCERT_CONTEXT CP2PInvitation::LoadCachedInvitaionByObjectID(CString sObjectID)
{
	CLOICert		LOICert;
	CString			sInvitationFile = m_sLocalCachedInvitationsFolder+sObjectID+_T(".cert");

	//form the local path to retrieve invitation parameters
	if(LoadCachedInvitation(sInvitationFile))
	{
		if(LOICert.Deserialise(sInvitationFile,(LPCTSTR)LOCAL_CACHE_PASS))
		{
			return LOICert.DuplicateCertContext();
		}
	}
	return NULL;
}
/****************************************
Load the cached invitation by filepath
*****************************************/
BOOL CP2PInvitation::LoadCachedInvitation(CString sFilePath)
{
	CLOICert			CertFile;
	DWORD				dwSize=0;

	//create the LOIcert from file
	if(!CertFile.Deserialise(sFilePath,(LPCTSTR)LOCAL_CACHE_PASS))
	{
		SetLastErrorMessage(CertFile.GetLastErrorMessage());  
		DEBUGLOG(LOI_ERROR,(LPTSTR)(LPCTSTR)GetLastErrorMessage());
		return FALSE;		
	}

	//read all params from the cert
	m_sNickName=CertFile.GetCertparam(NICKNAME_ATTRIBUTE_NAME);
	m_sSessionName=CertFile.GetCertparam(SESSION_DISPLAY_NAME);
	m_sP2PDDNSName=CertFile.GetCertparam(DDNSNAME_ATTRIBUTE_NAME);
	dwSize=sizeof(m_IPv6Addr);
	CertFile.GetCertparam(IPV6ADDR_ATTRIBUTE_NAME,(PBYTE)&m_IPv6Addr,dwSize);
	dwSize=sizeof(m_usIpv6ListeningPort);
	CertFile.GetCertparam(IPV6LISTENINGPORT_ATTRIBUTE_NAME,(PBYTE)&m_usIpv6ListeningPort,dwSize);
	//load appdata if exists
	FreeAppdata(); //free existing appdata
	m_pbyAppData=new BYTE[20*1024];
	m_dwAppDataLen=20*1024;
	if(!CertFile.GetCertparam(APPDATA_ATTRIBUTE_NAME,m_pbyAppData,m_dwAppDataLen))
	{
		//free memory
		FreeAppdata(); 
	}
	

	//read the session information
	m_sSessionXMLString=CertFile.GetCertparam(SESSION_INFO_XML_STRING);
	m_sObjectID=CertFile.GetCertSubjectName();
	CertFile.GetCertExpiryTime( m_ftValidityTime);
	m_bInvitationExpired=CertFile.IsCertExpired();
	return TRUE;
}
/******************************
Load cached invitation & read all params
********************************/
BOOL CP2PInvitation::LoadCachedInvitation(PCCERT_CONTEXT pClientCert)
{
	CLOICert			CertSubject(pClientCert);
	CString				sObjectID;
	CString				sCachedLocalPath;

	sObjectID= CertSubject.GetCertSubjectName();

	//form the local path to retrieve invitation parameters
	sCachedLocalPath = m_sLocalCachedInvitationsFolder+sObjectID+_T(".cert");

	
	return LoadCachedInvitation(sCachedLocalPath);
}
/*****************************
clear the local cache file
******************************/
void CP2PInvitation::DeleteCachedInvitation(CString sInvitationID)
{
	CString				sCachedLocalPath;


	if(!sInvitationID.IsEmpty())
	{
		//form the local path to retrieve invitation parameters
		sCachedLocalPath = m_sLocalCachedInvitationsFolder+sInvitationID+_T(".cert");
	}

	DeleteFile((LPCTSTR)sCachedLocalPath); 

}
/*******************************************
Verify the invitation certificate
********************************************/
BOOL CP2PInvitation::IsValidClientCert(PCCERT_CONTEXT pClientCert,PCCERT_CONTEXT pServerCert)
{
	CLOICert	VerifyCert;
	return VerifyCert.VerifySSLClientCert(pClientCert,pServerCert); 
}

/*****************************
formats the time in string
*******************************/
CString CP2PInvitation::GetInvitationExpiryTime()
{
    SYSTEMTIME			stUTC, stLocal;

    // Convert the last-write time to local time.
    FileTimeToSystemTime(&m_ftValidityTime, &stUTC);
    SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

	COleDateTime        oletime(stUTC);

	return oletime.Format(_T("%A, %B %d, %Y %I:%M:%S %p"));

   
}


/*********************************
set/get the nickname
**********************************/
void CP2PInvitation::SetNickname(CString sNickname)
{
	m_sNickName = sNickname;
}
CString CP2PInvitation::GetNickname()
{
	return m_sNickName;
}
/*******************************
set & get fucntion for session string
*******************************/
void CP2PInvitation::SetSessionName(CString sSessionString)
{
	m_sSessionName = sSessionString;
}
CString CP2PInvitation::GetSessionName()
{
	return m_sSessionName;
}
/*********************
Get & Set session xml string
********************/
void CP2PInvitation::SetSessionXMLString(CString sXMLString)
{
	m_sSessionXMLString	=sXMLString;
}
CString CP2PInvitation::GetSessionXMLString()
{
	return m_sSessionXMLString;
}

/**********************************
Get/Set for DDNS NAME
***********************************/
void CP2PInvitation::SetP2PDDNSName(CString sP2PDDNSName)
{
	m_sP2PDDNSName = sP2PDDNSName;
}
CString CP2PInvitation::GetP2PDDNSName()
{
	return m_sP2PDDNSName;
}
/*******************************
Set/get the ipv6addr/port
********************************/
void CP2PInvitation::SetAddrInfo(in_addr6 IPv6Addr,USHORT usListeningPort)
{
	m_IPv6Addr=IPv6Addr;
	m_usIpv6ListeningPort=usListeningPort;
}
void CP2PInvitation::GetAddrInfo(in_addr6 &IPv6Addr,USHORT &usListeningPort)
{
	IPv6Addr=m_IPv6Addr;
	usListeningPort=m_usIpv6ListeningPort;
}
/*********************************
set/get appdata
**********************************/
void CP2PInvitation::SetAppData(BYTE *pbyAppdata,DWORD dwAppDataLen)
{
	FreeAppdata();
	if(dwAppDataLen)
	{
		m_pbyAppData=new BYTE[dwAppDataLen];
		m_dwAppDataLen=dwAppDataLen;
		memcpy(m_pbyAppData,pbyAppdata,dwAppDataLen);
	}
}
PBYTE CP2PInvitation::GetAppData(DWORD &dwAppdataLen)
{
	dwAppdataLen=m_dwAppDataLen;
	return m_pbyAppData;
}
void CP2PInvitation::FreeAppdata()
{
	if(m_pbyAppData)
	{
		delete m_pbyAppData;
		m_pbyAppData=NULL;
		m_dwAppDataLen=0;
	}
}




