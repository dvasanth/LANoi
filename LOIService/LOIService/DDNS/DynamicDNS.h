#pragma once
#include <p2p.h>
#include <initguid.h>
#include "error.h"

class CDynamicDNS:public CError 
{
public:
	CDynamicDNS(void);
	~CDynamicDNS(void);
	void		InitPNRP();
	void		ClosePNRP();
	BOOL		RegisterDNSName(in_addr6 DNSMappedIPv6Addr,USHORT usDNSServicePort,BYTE *pDNSBlobData,DWORD dwDNSBlobDataLen);
	BOOL		ResolveDNSName(CString sDNSName,in_addr6 &DNSMappedIPv6Addr,USHORT &usDNSServicePort);
	void		UnRegisterDNSName();
	CString		GetDNSName();
private:
	static const TCHAR					FRIENDLY_NAME[];
	static const TCHAR					DNS_COMMENT[];
	static const TCHAR					CLOUD_NAME[];
	static const TCHAR					PNRP_SEED_SERVER[];
	static const DWORD					DNS_UPDATE_INTERVAL;
	HANDLE								m_hRegistration;
	PEER_PNRP_REGISTRATION_INFO			m_RegInfo;
	SOCKADDR_IN6						m_DNSIpv6SockAddr;
	CString								m_sOldPNRPServer;
	//updating registration in periodic interval
	HANDLE								m_hDNSRegistrationThread;
	HANDLE								m_hDNSRegistrationStopThread;
	CString								m_sDNSName;
	static DWORD WINAPI					DNSRegistrationUpdateProc(LPVOID lpParam);
	void								StartUpdateRegistrationThread();
	void								StopUpdateRegistrationThread();
	void								UpdateRegistration();
	BOOL								_ResolveDNSName(CString sDNSName,in_addr6 &DNSMappedIPv6Addr,USHORT &usDNSServicePort);
	
	///
	HRESULT								GetDefaultIdentity(CString &sIdentity);
};
