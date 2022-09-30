// InvitationSavePage.cpp : implementation file
//

#include "stdafx.h"
#include "InvitationSavePage.h"

const DWORD CInvitationSavePage::MIN_PASSWORD_LENGTH=6;

// CInvitationSavePage dialog

IMPLEMENT_DYNAMIC(CInvitationSavePage, CPropertyPage)

CInvitationSavePage::CInvitationSavePage()
	: CPropertyPage(CInvitationSavePage::IDD)
	, m_sFilePath(_T(""))
	, m_sPassword(_T(""))
	, m_sConfirmPassword(_T(""))
{
	m_sHeaderTitle.LoadString(IDS_INVITESAVEPAGE_HEADER);
	m_sSubtitle.LoadString(IDS_INVITESAVEPAGE_SUBTITLE);

	m_psp.dwFlags |= PSP_DEFAULT|PSP_USEHEADERTITLE|PSP_USEHEADERSUBTITLE;
	m_psp.pszHeaderTitle =(LPCTSTR)m_sHeaderTitle;
	m_psp.pszHeaderSubTitle = (LPCTSTR)m_sSubtitle;
	m_psp.dwFlags &= ~PSP_HASHELP;
}

CInvitationSavePage::~CInvitationSavePage()
{
	OnSave();
}

void CInvitationSavePage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_FILEPATH    , m_sFilePath);
	DDX_Text(pDX, IDC_PASSWORD    , m_sPassword);
	DDX_Text(pDX, IDC_CONFIRM_PASSWORD    , m_sConfirmPassword);
}


BEGIN_MESSAGE_MAP(CInvitationSavePage, CPropertyPage)
	ON_BN_CLICKED(IDC_BROWSE, &CInvitationSavePage::OnBnClickedBrowse)
	ON_EN_CHANGE(IDC_PASSWORD, &CInvitationSavePage::OnEnChangePassword)
END_MESSAGE_MAP()


// CInvitationSavePage message handlers
/******************************
dialog init code here
******************************/
BOOL CInvitationSavePage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	OnLoad();
	return TRUE;
}

/*********************************
Write dialog control intitialization code 
***********************************/
void CInvitationSavePage::OnLoad()
{

}
/************
do button initialisation
**************/
BOOL CInvitationSavePage::OnSetActive( )
{
     return TRUE;
}
/************************************
save the data into persistance storage for loading it next time
*************************************/
void CInvitationSavePage::OnSave()
{

}
/*********************
enable finish button only after user types at least 6 characters for password
***********************/
void CInvitationSavePage::OnEnChangePassword()
{
	CString sPassword;
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CPropertyPage::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
/*	GetDlgItem(IDC_PASSWORD)->GetWindowText(sPassword);

	if(sPassword.GetLength()>=MIN_PASSWORD_LENGTH)
	{
		CPropertySheet* psheet = (CPropertySheet*) GetParent();   
		psheet->SetWizardButtons(PSWIZB_BACK | PSWIZB_FINISH);
	}
	else
	{
		CPropertySheet* psheet = (CPropertySheet*) GetParent();   
		psheet->SetWizardButtons(PSWIZB_BACK | PSWIZB_DISABLEDFINISH);
	}*/
}

/********************************
Validate the user input
*********************************/
BOOL CInvitationSavePage::IsValidInput()
{
	UpdateData(TRUE); 

	HANDLE hInviteFile = CreateFile((LPCTSTR)m_sFilePath,GENERIC_WRITE, FILE_SHARE_READ,NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if(hInviteFile==INVALID_HANDLE_VALUE )
	{
		AfxMessageBox(IDS_ERR_INVALID_INVITATION_FILEPATH, MB_OK  |MB_ICONSTOP |MB_TOPMOST  );
		return FALSE;
	}

	CloseHandle(hInviteFile);
	DeleteFile((LPCTSTR)m_sFilePath); 

	if(m_sPassword.IsEmpty())
	{
		AfxMessageBox(IDS_ERR_EMPTY_PASSWORD, MB_OK  |MB_ICONSTOP|MB_TOPMOST   );
		return FALSE;

	}

	if(m_sPassword.GetLength()<MIN_PASSWORD_LENGTH)
	{
		AfxMessageBox(IDS_PASSWORD_LEN_LESS, MB_OK  |MB_ICONSTOP|MB_TOPMOST   );
		return FALSE;

	}
	if(m_sConfirmPassword != m_sPassword)
	{
		AfxMessageBox(IDS_ERR_PASSWORD_NO_MATCH, MB_OK  |MB_ICONSTOP|MB_TOPMOST   );
		return FALSE;
	}

	return TRUE;
}
/****************************************
show up the file dialog to select the save location for the file
*****************************************/
void CInvitationSavePage::OnBnClickedBrowse()
{
	// TODO: Add your control notification handler code here
	CString		sFileExtFilter;
	CString		sSaveDlgCaption;
	CString		sFileExt;

	sFileExt.LoadString(IDS_INVITE_FILE_EXTENSION);
	sSaveDlgCaption.LoadString(IDS_SAVE_INVITE_DLG_CAPTION);
	sFileExtFilter.LoadString(IDS_INVITE_FILE_EXTENSION_FILTER);

	CFileDialog fSaveDlg(FALSE, 
						(LPCTSTR)sFileExt,
						_T(""), 
						OFN_HIDEREADONLY|OFN_FILEMUSTEXIST,
						(LPCTSTR)sFileExtFilter,
						this);

	// Initializes m_pOFN structure
	fSaveDlg.m_pOFN->lpstrTitle=sSaveDlgCaption;


	if(fSaveDlg.DoModal()==IDOK)
	{
		m_sFilePath=fSaveDlg.GetPathName();
		UpdateData(FALSE); 
	}
}
/*******************************************************
get the file path
********************************************************/
CString CInvitationSavePage::GetFilePath(void)
{
	return m_sFilePath;
}
/*******************************************
get the session password
*********************************************/
CString CInvitationSavePage::GetSessionPassword(void)
{
	return m_sPassword;
}
