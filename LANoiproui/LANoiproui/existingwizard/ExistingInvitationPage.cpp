// ExistingInvitationPage.cpp : implementation file
//

#include "stdafx.h"

#include "ExistingInvitationPage.h"
#include "invmanager/p2pinvitation.h"

// CExistingInvitationPage dialog

IMPLEMENT_DYNAMIC(CExistingInvitationPage, CPropertyPage)

CExistingInvitationPage::CExistingInvitationPage()
	: CPropertyPage(CExistingInvitationPage::IDD)
	, m_sExpiryTime(_T(""))
{
	m_sHeaderTitle.LoadString(IDS_EXISTINGPAGE_HEADER);
	m_sSubtitle.LoadString(IDS_EXISTINGPAGE_SUBTITLE);

	m_psp.dwFlags |= PSP_DEFAULT|PSP_USEHEADERTITLE|PSP_USEHEADERSUBTITLE;
	m_psp.pszHeaderTitle =(LPCTSTR)m_sHeaderTitle;
	m_psp.pszHeaderSubTitle = (LPCTSTR)m_sSubtitle;
	m_psp.dwFlags &= ~PSP_HASHELP;
}

CExistingInvitationPage::~CExistingInvitationPage()
{
}

void CExistingInvitationPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EXISTING_INVITATION_LIST, m_CbnExistingInvitations);
	DDX_Text(pDX, IDC_EXPIRY_TIME    , m_sExpiryTime);
}


BEGIN_MESSAGE_MAP(CExistingInvitationPage, CPropertyPage)

	ON_BN_CLICKED(IDC_DELETE_INVITATION, &CExistingInvitationPage::OnBnClickedDeleteInvitation)
	ON_CBN_SELCHANGE(IDC_EXISTING_INVITATION_LIST, &CExistingInvitationPage::OnCbnSelchangeExistingInvitationList)
END_MESSAGE_MAP()


// CExistingInvitationPage message handlers
BOOL CExistingInvitationPage::OnInitDialog()
{
	CDialog::OnInitDialog();

	OnLoad();
	return TRUE;

}
/*********************************
Write dialog control intitialization code 
***********************************/
void CExistingInvitationPage::OnLoad()
{
	LoadInvitations();
}
/************************************
save the data into persistance storage for loading it next time
*************************************/
void CExistingInvitationPage::OnSave()
{

}
/*****************************************
enumerate all invitaions & load it
****************************************/
void CExistingInvitationPage::LoadInvitations()
{
	CP2PInvitationLocalEnum   InvitationEnum;
	CP2PInvitation			  P2PInvitation;

	m_CbnExistingInvitations.ResetContent(); 
	m_sInvitationIDs.RemoveAll(); 
	m_sExpiryTimes.RemoveAll();
	if(InvitationEnum.FindFirst(P2PInvitation))
	{
		do{
			CString sInvitationID= P2PInvitation.GetInvitationObjectID();
			//add the invitations to the list
			if(P2PInvitation.IsInvitationExpired()) 
				P2PInvitation.DeleteCachedInvitation( sInvitationID);
			else
			{
				m_sInvitationIDs.Add(sInvitationID);
				m_CbnExistingInvitations.AddString(P2PInvitation.GetSessionName() );
				m_sExpiryTimes.Add(P2PInvitation.GetInvitationExpiryTime());
			}
		}while(InvitationEnum.FindNext(P2PInvitation));
	}

	m_CbnExistingInvitations.SetCurSel(0);
	OnCbnSelchangeExistingInvitationList();

}
/**********************************
gets the count of unexpired invitations
************************************/
DWORD CExistingInvitationPage::GetCount()
{
	CP2PInvitationLocalEnum   InvitationEnum;
	CP2PInvitation			  P2PInvitation;
	DWORD					  dwValidCount=0;

	if(InvitationEnum.FindFirst(P2PInvitation))
	{
		do{
			CString sInvitationID= P2PInvitation.GetInvitationObjectID();
			//add the invitations to the list
			if(P2PInvitation.IsInvitationExpired()) 
				P2PInvitation.DeleteCachedInvitation( sInvitationID);
			else
			{
				dwValidCount++;
			}
		}while(InvitationEnum.FindNext(P2PInvitation));
	}

	return dwValidCount;
}
/********************************
Validate the user input
*********************************/
BOOL CExistingInvitationPage::IsValidInput()
{
	UpdateData(TRUE); 

	if(m_CbnExistingInvitations.GetCurSel()==CB_ERR)
	{
		AfxMessageBox(IDS_ERR_INVALID_SESSIONNAME, MB_OK  |MB_ICONSTOP|MB_TOPMOST  );
		return false;
	}
	//get the combo box selected session port string
	m_CbnExistingInvitations.GetLBText(m_CbnExistingInvitations.GetCurSel(), m_sSelectedSessionName);
	m_sInvitationID=m_sInvitationIDs[m_CbnExistingInvitations.GetCurSel()];
	return TRUE;
}
/*********************************************
delete the selected invitation file
********************************************/
void CExistingInvitationPage::OnBnClickedDeleteInvitation()
{
	// TODO: Add your control notification handler code here
	if(m_CbnExistingInvitations.GetCurSel()!=CB_ERR)
	{
		CP2PInvitation			  P2PInvitation;

		P2PInvitation.DeleteCachedInvitation(m_sInvitationIDs[ m_CbnExistingInvitations.GetCurSel()]); 
		LoadInvitations();
		
	
	}
}
/*****************************
get the selected sesssionname
******************************/
CString CExistingInvitationPage::GetSessionName()
{
	return m_sSelectedSessionName;
}
/******************************
get the unique session ID
*******************************/
CString CExistingInvitationPage::GetInvitationID()
{
	return m_sInvitationID;
}
/******************************
Save the selected invitation in selected path
*******************************/
BOOL CExistingInvitationPage::SaveInvitation(CString sFilePath,CString sPassword)
{
	CP2PInvitation  InvitationSave;

	if(!InvitationSave.CreateSecureInvitationByID(m_sInvitationID,sFilePath,sPassword))
	{
		AfxMessageBox((LPCTSTR)InvitationSave.GetLastErrorMessage()  , MB_OK  |MB_ICONSTOP|MB_TOPMOST  );
		return FALSE;
	}
	AfxMessageBox(IDS_CREATE_INVITATION_SUCCESS,MB_OK|MB_ICONINFORMATION|MB_TOPMOST);
	return TRUE;
}
/**************************
on selection change update expiry date
***************************/
void CExistingInvitationPage::OnCbnSelchangeExistingInvitationList()
{
	// TODO: Add your control notification handler code here
	CString sInvitationID;
	if(m_CbnExistingInvitations.GetCurSel()!=CB_ERR)
	{
		m_sExpiryTime= m_sExpiryTimes[m_CbnExistingInvitations.GetCurSel()];
		UpdateData(0);
	}
}
