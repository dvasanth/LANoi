#pragma once
#include "../resource.h"

// CInvitationSavePage dialog

class CInvitationSavePage : public CPropertyPage
{
	DECLARE_DYNAMIC(CInvitationSavePage)

public:
	CInvitationSavePage();
	virtual ~CInvitationSavePage();

// Dialog Data
	enum { IDD = IDD_INVITATION_SAVE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	static const DWORD   MIN_PASSWORD_LENGTH;
	CString m_sFilePath;
	CString m_sPassword;
	CString m_sConfirmPassword;
	CString			m_sHeaderTitle;
	CString			m_sSubtitle;
	BOOL	OnInitDialog();
	void	OnLoad();
	void	OnSave();

	afx_msg void OnBnClickedBrowse();
	virtual BOOL OnSetActive( );
	afx_msg void OnEnChangePassword();
public:
	CString GetFilePath(void);
	CString GetSessionPassword(void);
	BOOL	IsValidInput();

};
