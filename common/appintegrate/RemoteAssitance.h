#pragma once

class CRemoteAssitance
{
public:
	CRemoteAssitance(void);
	~CRemoteAssitance(void);
	BOOL				JoinInvitation(BYTE *pbyBlobData,DWORD dwDataLen);
	BOOL				CreateInvitation(CString sPeerName,CString sPassword,CString &sArgInvitationString,CString &sRDPListeningPort);

private:
	BOOL				m_bRAEnabled;
	BOOL				m_bRAUnSolicitedEnabled;
	BOOL				m_bHAUserAccountEnabled;
	BOOL				m_bRAPolicyChanged;

	BOOL				IsWindowsVista();
	BOOL				IsWindows7();
	void				WaitForVistaGettingEnabled();
	BOOL				IsVistaRAEnabled();
	CString				GetInvitationFilePath();
	BOOL				ExecuteVistaRA(CString sFilePath,CString sPassword,BOOL bAdmin,HANDLE &hProcess);
	void				EnableRAPolicy();
	void				TerminateMSRA();
	BOOL				EnableRAUserAccount(BOOL bValue);
	void				RestoreRAPolicy();
	DWORD				WaitAndDispatch(DWORD dwHandleCount,PHANDLE phHandle,DWORD dwMilliseconds);
	CString				GetInvitationString(CString sPeerName,CString sConnString);
	CString				GetRDPListeningPort(CString sConnString);
	BOOL				CreateXPInvitation(CString sPeerName,CString &sArgInvitationString,CString &sRDPListeningPort);
	BOOL				CreateVistaInvitation(CString sPeerName,CString sPassword,CString &sArgInvitationString,CString &sRDPListeningPort);

};
