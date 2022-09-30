// InvitationAuthPage.cpp : implementation file
//

#include "stdafx.h"
#include "InvitationAuthPage.h"
#include "RegOperations.h"

// CInvitationAuthPage dialog

IMPLEMENT_DYNAMIC(CInvitationAuthPage, CPropertyPage)

CInvitationAuthPage::CInvitationAuthPage()
	: CPropertyPage(CInvitationAuthPage::IDD)
	, m_sExpiryDate(_T(""))
	, m_sNickname(_T(""))
	, m_sPassword(_T(""))
{
	m_sHeaderTitle.LoadString(IDS_AUTHPAGE_HEADER);
	m_sSubtitle.LoadString(IDS_AUTHPAGE_SUBTITLE);

	m_psp.dwFlags |= PSP_DEFAULT|PSP_USEHEADERTITLE|PSP_USEHEADERSUBTITLE;
	m_psp.pszHeaderTitle =(LPCTSTR)m_sHeaderTitle;
	m_psp.pszHeaderSubTitle = (LPCTSTR)m_sSubtitle;
	m_psp.dwFlags &= ~PSP_HASHELP;

}

CInvitationAuthPage::~CInvitationAuthPage()
{
	OnSave();
}

void CInvitationAuthPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_JOIN_NICKNAME  , m_sNickname);
	DDX_Text(pDX, IDC_JOIN_PASSWORD  , m_sPassword);
}


BEGIN_MESSAGE_MAP(CInvitationAuthPage, CPropertyPage)
END_MESSAGE_MAP()


// CInvitationAuthPage message handlers

BOOL CInvitationAuthPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	OnLoad();
	return TRUE;
}
/*********************
on dialog initialisation
*********************/
BOOL CInvitationAuthPage::OnSetActive()
{
	// TODO: Add your specialized code here and/or call the base class
	CPropertySheet *JoinSheet = reinterpret_cast<CPropertySheet *>(GetParent());

	JoinSheet->	SetWizardButtons(PSWIZB_FINISH);

	return CPropertyPage::OnSetActive();
}
/*********************************
Write dialog intitialization code 
***********************************/
void CInvitationAuthPage::OnLoad()
{
	CRegOperations Load;

	//load the nickname
	m_sNickname = Load.GetNickname();
	//load data to control
	UpdateData(0);
}
/************************************
save the data into persistance storage for loading it next time
*************************************/
void CInvitationAuthPage::OnSave()
{
	CRegOperations Save;

	//save nickname to registry
	if(!m_sNickname.IsEmpty()) 
		Save.SetNickname(m_sNickname); 

}
 /********************************
Validate the input
*********************************/
BOOL CInvitationAuthPage::IsValidInput()
{
	UpdateData(TRUE);

	if(m_sNickname.IsEmpty() )
	{
		AfxMessageBox(IDS_ERR_EMPTY_NICKNAME, MB_OK  |MB_ICONSTOP|MB_TOPMOST  );
		return false;
	}

	if(m_sPassword.IsEmpty() )
	{
		AfxMessageBox(IDS_ERR_EMPTY_PASSWORD, MB_OK  |MB_ICONSTOP|MB_TOPMOST  );
		return false;
	}
	return true;
}
/******************
Set the expiry date for the invitation file
*******************/
void CInvitationAuthPage::SetExpiryDate(CString sExpiryDate)
{
	m_sExpiryDate=sExpiryDate;
}
/*********************
Get the nickname
**********************/
CString CInvitationAuthPage::GetNickname()
{
	return m_sNickname;
}
/***********************
get the password
************************/
CString CInvitationAuthPage::GetPassword()
{
	return m_sPassword;
}