#pragma once

#include "../resource.h"

// CInvitationAuthPage dialog

class CInvitationAuthPage : public CPropertyPage
{
	DECLARE_DYNAMIC(CInvitationAuthPage)

public:
	CInvitationAuthPage();
	virtual ~CInvitationAuthPage();

// Dialog Data
	enum { IDD = IDD_INVITATION_AUTH };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	CString m_sExpiryDate;
	CString m_sNickname;
	CString m_sPassword;
	CString m_sHeaderTitle;
	CString m_sSubtitle;
	void	SetExpiryDate(CString sExpiryDate);
	BOOL	OnSetActive();
	BOOL	OnInitDialog();
	void	OnLoad();
	void	OnSave();
public:
	BOOL	IsValidInput();
	CString	GetNickname();
	CString	GetPassword();
};
