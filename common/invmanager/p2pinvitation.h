#pragma once
#include <wincrypt.h>
#include "LOICert.h"
#include "error.h"

class CP2PInvitationLocalEnum;

class CP2PInvitation:public CError
{

public:
	CP2PInvitation(void);
	~CP2PInvitation(void);
	PCCERT_CONTEXT GetCACert();
	friend class CP2PInvitationLocalEnum;
	/******************************
	Invitation load & store functions
	*****************************/
	CString			SaveInvite(CString sFilePath,CString sPassword);
	void			SaveInvitationInStore(PCCERT_CONTEXT pClientCert);
	BOOL			CreateSecureInvitation(CString sInvitationFilePath,CString sPassword,FILETIME ftExpiryTime);
	PCCERT_CONTEXT  LoadSecureInvitation(CString sInvitationFilePath,CString sPassword);
	BOOL			LoadCachedInvitation(PCCERT_CONTEXT pClientCert);
	BOOL			LoadCachedInvitation(CString sFilePath);
	BOOL			IsValidClientCert(PCCERT_CONTEXT pClientCert,PCCERT_CONTEXT pServerCert);
	CString			GetInvitationObjectID();
	CString			GetInvitationObjectID(PCCERT_CONTEXT pCertContext);
	CString			GetInvitationExpiryTime();
	void			DeleteCachedInvitation(CString sInvitationID);
	BOOL			CreateSecureInvitationByID(CString	sObjectID,CString sFilePath,CString sPassword);
	/****************************
	set get handlers of the invitation parameter
	*****************************/
	void			SetNickname(CString sNickname);
	CString			GetNickname();
	void			SetSessionName(CString sSessionString);
	CString			GetSessionName();
	void			SetP2PDDNSName(CString sP2PDDNSName);
	CString			GetP2PDDNSName();
	void			SetAddrInfo(in_addr6 IPv6Addr,USHORT usListeningPort);
	void			GetAddrInfo(in_addr6 &IPv6Addr,USHORT &usListeningPort);
	void			SetSessionXMLString(CString sXMLString);
	CString			GetSessionXMLString();
	inline BOOL				IsInvitationExpired()
	{
		return m_bInvitationExpired;
	}
	/*********
	appdata
	*************/
	void			SetAppData(BYTE *pbyAppdata,DWORD dwAppDataLen);
	PBYTE			GetAppData(DWORD &dwAppdataLen);

private:
	static const TCHAR			CA_CERT_FILENAME[];
	static const TCHAR			STORE_PATH[];
	static const TCHAR			LOCAL_INVITATION_STORE_FOLDERNAME[];
	static const TCHAR			CA_CERT_SUBJECTNAME[];
	static const TCHAR			CA_PASSWORD[];
	static const TCHAR			INVITATE_FILE_HEADER[];
	static const TCHAR			INVITATION_NODE_NAME[];
	static const TCHAR			NICKNAME_ATTRIBUTE_NAME[];
	static const TCHAR			DDNSNAME_ATTRIBUTE_NAME[];
	static const TCHAR			IPV6ADDR_ATTRIBUTE_NAME[];
	static const TCHAR			IPV6LISTENINGPORT_ATTRIBUTE_NAME[];
	static const TCHAR			SESSION_INFO_XML_STRING[];
	static const TCHAR			LOCAL_CACHE_PASS[];
	static const TCHAR			SESSION_DISPLAY_NAME[];
	static const TCHAR			APPDATA_ATTRIBUTE_NAME[];
	/*************************
	Invitation related parameters
	need to be set by the caller
	*************************/
	CString					m_sNickName;
	CString					m_sPassword;
	CString					m_sSessionName;
	CString					m_sSessionXMLString;
	CString					m_sP2PDDNSName;
	IN6_ADDR				m_IPv6Addr;
	USHORT					m_usIpv6ListeningPort; 
	CLOICert				m_ClientCert;
	CString					m_sCACertFilePath;
	CString					m_sLocalCachedInvitationsFolder;
	CString					m_sCertStorePath;
	CString					m_sObjectID;
	FILETIME				m_ftValidityTime;
	BYTE*					m_pbyAppData;
	DWORD					m_dwAppDataLen;
	BOOL					m_bInvitationExpired;
	BOOL					SaveCachedInvitation(PCCERT_CONTEXT pCertContext,CString sObjectID);
	PCCERT_CONTEXT			LoadCachedInvitaionByObjectID(CString sObjectID);
	void					FreeAppdata();
};


/*************************************************
Invitation enum class
*************************************************/
class CP2PInvitationLocalEnum{
private:
	HANDLE		m_hFileStore;
public: 
    CP2PInvitationLocalEnum()
	{
		m_hFileStore=INVALID_HANDLE_VALUE;
	}
	~CP2PInvitationLocalEnum()
	{
		Close();
	}
	BOOL FindFirst(CP2PInvitation &P2PFirst)
	{
		CString				sFolder;
		WIN32_FIND_DATA		win32={0};
		CString				sFilePath;

		sFolder=P2PFirst.m_sLocalCachedInvitationsFolder+_T("*.cert"); 
		m_hFileStore=FindFirstFile((LPCTSTR)sFolder,  &win32);
		if(m_hFileStore==INVALID_HANDLE_VALUE)
		{
			return FALSE;
		}
		sFilePath = P2PFirst.m_sLocalCachedInvitationsFolder+win32.cFileName;

		return P2PFirst.LoadCachedInvitation(sFilePath);
	}
	BOOL FindNext(CP2PInvitation &P2PNext)
	{
		WIN32_FIND_DATA		win32={0};
		CString				sFilePath;
		if(!FindNextFile( m_hFileStore,&win32))
		{
			return FALSE;
		}
		sFilePath = P2PNext.m_sLocalCachedInvitationsFolder+win32.cFileName;

		return P2PNext.LoadCachedInvitation(sFilePath);
	}
	void Close()
	{
		if(m_hFileStore!=INVALID_HANDLE_VALUE)
		{
			::FindClose(m_hFileStore);
			m_hFileStore=INVALID_HANDLE_VALUE;
		}
	}
};