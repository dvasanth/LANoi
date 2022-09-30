#pragma once
#include "../resource.h"
#include "afxwin.h"



// CExistingInvitationPage dialog

class CExistingInvitationPage : public CPropertyPage
{
	DECLARE_DYNAMIC(CExistingInvitationPage)

public:
	CExistingInvitationPage();
	virtual ~CExistingInvitationPage();

// Dialog Data
	enum { IDD = IDD_EXISTING_INVITATIONS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	CString			m_sInvitationID;
	BOOL			OnInitDialog();
	CComboBox		m_CbnExistingInvitations;
	CString			m_sSelectedSessionName;
	CStringArray 	m_sInvitationIDs;
	CStringArray 	m_sExpiryTimes;
	CString			m_sExpiryTime;
	CString			m_sHeaderTitle;
	CString			m_sSubtitle;

	void			OnLoad();
	void			OnSave();
	void			LoadInvitations();
	afx_msg void	OnBnClickedDeleteInvitation();

	afx_msg void OnCbnSelchangeExistingInvitationList();

public:
	CString			GetSessionName();
	CString			GetInvitationID();
	BOOL			IsValidInput();
	BOOL			SaveInvitation(CString sFilePath,CString sPassword);
	DWORD			GetCount();


};
