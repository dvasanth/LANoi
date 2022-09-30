#pragma once
#include "RemoteAssitance.h"

class CAppIntegrate
{
public:
	CAppIntegrate(void);
	~CAppIntegrate(void);
	void			ReleaseSessionData();
	BOOL			PreInviterSessionStart(CString sSessionName,CString sNickName);
	BOOL			PreInviteeSessionStart(CString sSessionName,CString sNickName);
	static	DWORD	GetVirtualAddressForSession(CString sSessionName);
	BOOL			GetAppDataBlob(CString sSessionName,CString sPeername,CString sPassword,CString &sNewSessionXML,PBYTE *ppByData,DWORD &dwInOutLen);
	void			FreeAppDataBlob(BYTE *byData);
	BOOL			SetAppDataBlobData(BYTE *pByData,DWORD dwInLen);

private:
	static const UINT	MEETINGNAMES[];
	static const USHORT	RDP_PORT;
	BYTE				*m_pbyAppBlobData;
	DWORD				 m_dwAppBlobDataLen;
	CRemoteAssitance	 *m_pRemoteAsst;
//	CRemoteMediaStreaming *m_pRemoteStream;
};
