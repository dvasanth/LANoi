#pragma once
#include "afxwin.h"
#include"../skinning/suptedit.h"

// CContactSupportDlg dialog

class CContactSupportDlg : public CDialog
{
	DECLARE_DYNAMIC(CContactSupportDlg)

public:
	CContactSupportDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CContactSupportDlg();

// Dialog Data
	enum { IDD = IDD_CONTACT_SUPPORT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	CString     m_sFromAddress;
	CString     m_sSubject;
	CString		m_sMessage;
	CString     m_sUserName;

	BOOL		OnInitDialog();
	void		OnLoad();
	void		OnSave();
	BOOL		IsValidInput();
	afx_msg void OnBnClickedSendEmail();
	afx_msg void OnEnSetfocusMessageEmail();
	//CSuptEdit		m_EdtMessage;
	CEdit			m_EdtMessage;
};
