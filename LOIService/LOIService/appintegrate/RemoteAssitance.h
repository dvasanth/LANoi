#pragma once
#include "error.h"
class CRemoteAssitance
{
public:
	CRemoteAssitance(void);
	~CRemoteAssitance(void);
	BOOL				JoinInvitation(BYTE *pbyBlobData,DWORD dwDataLen);
	BOOL				CreateInvitation(CString sPeerName,CString &sArgInvitationString,CString &sRDPListeningPort);

private:
	BOOL				m_bRAEnabled;
	BOOL				m_bRAUnSolicitedEnabled;
	BOOL				m_bHAUserAccountEnabled;
	BOOL				m_bRAPolicyChanged;

	BOOL				IsWindowsVista();
	void				EnableRAPolicy();
	BOOL				EnableRAUserAccount(BOOL bValue);
	void				RestoreRAPolicy();
	CString				GetInvitationString(CString sPeerName,CString sConnString);
	CString				GetRDPListeningPort(CString sConnString);
	BOOL				CreateXPInvitation(CString sPeerName,CString &sArgInvitationString,CString &sRDPListeningPort);
	BOOL				CreateVistaInvitation(CString sPeerName,CString &sArgInvitationString,CString &sRDPListeningPort);

};
