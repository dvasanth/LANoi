// LANoiprouiDlg.h : header file
//

#pragma once
#include "afxcmn.h"
#include "./skinning/skintreectrl.h"
#include "./skinning/SkinDialog.h"
#include "loisession/ui/UILOISession.h"
#include "IPc/IPCCommandHandler.h"
#include "peerlist/OnlineUsersTree.h"
#include "progress/progressdlg.h"
#include "tray/ntray.h"
#include "afxwin.h"
#include "./skinning/xSkinButton.h"
#include "skinning/freemenu.h"
#include "appintegrate/appintegrate.h"

// CLANoiprouiDlg dialog
class CLANoiprouiDlg : public CSkinDialog,
						public IRPCExecute
{
// Construction
public:
	CLANoiprouiDlg(CWnd* pParent = NULL);	// standard constructor
	virtual ~CLANoiprouiDlg();

// Dialog Data
	enum { IDD = IDD_LANOIPROUI_DIALOG };
	void				ExecuteCmdLine(BOOL bPostExistingInstance);
	protected:
	virtual void		DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	void				OpenDocument(CString sFilePath);

// Implementation
protected:

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
private:

	static const UINT			SERVICE_IPC_NOTIFY;
	static const UINT			ADD_FIREWALL_RULE;
	static const UINT			TRAY_MESSAGE;
	static const TCHAR			WINDOW_CAPTION[];
	static const TCHAR			STARTUP_COMMAND[];
	static const UINT			DDE_MESSAGE_FILE_OPEN;
	// tree containing all the list of online peers with their sessions
	CIPCCommandHandler			m_ServiceIPC;
	COnlineUsersTree			m_TreeCtrlOnlinePeers;
	CProgressDlg				m_JoinProgressDlg;
	CTrayNotifyIcon				m_TrayIcon;
	CFreeMenu					*m_pMenu;
	CFont						m_Font;
	HICON						m_hIcon;//tray icon
	BOOL						m_bP2PEnabled;
	CAppIntegrate				m_AppIntegrate;
	BOOL						m_bShowWindow;
	void						OnLoad();
	void						OnSave();
	afx_msg LRESULT				OnClientInfoChanged(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT				OnServiceCmdReceive(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT				OnAddFirewallRule(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT				OnTrayNotification(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT				OnAnotherInstanceFileOpen(WPARAM wParam, LPARAM lParam);
	/*********************
	RPC related
	********************/
	virtual BOOL				PostJoinInvitationStatus(BOOL bConnectSuccess,CString sErrorMessage,CString sSessionName,CString sInviterName);
	virtual BOOL				PostP2PStatusChange(BOOL bEnabled);
	virtual BOOL				PostCreateInvitationStatus(CString sSessionName,BOOL bSuccess,CString sErrorMessage);
	virtual BOOL				PostInvitationAppData(BYTE *pbyAppData,DWORD dwAppDataLen);
	afx_msg void				OnInvitePartner();
	afx_msg void				OnJoinPartner();
	afx_msg void				OnExit();
	afx_msg void				OnExistingInvitations();
	void						OnJoinStart();
	void						OnJoinComplete();
	void						DisableVistaUIPI();
	void						ToggleP2PStatus(BOOL bEnabled,BOOL bShowBallon);
	afx_msg void				OnDestroy();
	afx_msg void				OnShowlanoipro();
	afx_msg void				OnWindowPosChanging(WINDOWPOS* lpwndpos);
	afx_msg void				OnStnClickedClose();
	afx_msg void				OnHelpContactsupport();
	HWND						GetPrevWindow();
	afx_msg void				OnHelpAbout();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
private:
	CxSkinButton				m_BtnAdd;
	CxSkinButton				m_BtnJoin;
	CxSkinButton				m_BtnExisting;
	afx_msg void OnRefresh();

};
