// ExistingWizard.cpp : implementation file
//

#include "stdafx.h"


#include "ExistingWizard.h"



// CExistingWizard

IMPLEMENT_DYNAMIC(CExistingWizard, CPropertySheet)

CExistingWizard::CExistingWizard(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{

}

CExistingWizard::CExistingWizard(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{

}

CExistingWizard::~CExistingWizard()
{
}


BEGIN_MESSAGE_MAP(CExistingWizard, CPropertySheet)
	ON_BN_CLICKED(ID_WIZNEXT,OnWizNext)
	ON_BN_CLICKED(ID_WIZFINISH,OnWizFinish)
END_MESSAGE_MAP()


// CExistingWizard message handlers

/*********************************
Funtion added to handle page addition & displaying
*********************************/

int CExistingWizard ::ShowWizard(void)
{

	AddPage(&m_PgExist);
	AddPage(&m_PgInvitationSave);
	m_psh.dwFlags |= PSH_WIZARD97|PSH_WATERMARK|PSH_HEADER|PSH_USEHBMHEADER;
//	m_psh.pszbmWatermark = MAKEINTRESOURCE(IDB_WIZ_WATERMARK);
//	m_psh.pszbmHeader = MAKEINTRESOURCE(IDB_WIZ_BANNER_ICON);
	m_psh.hbmHeader =(HBITMAP) LoadImage( AfxGetInstanceHandle(),_T("images\\banner_icon_existing.bmp"), IMAGE_BITMAP, 0,0, LR_LOADFROMFILE|LR_SHARED );
	m_psh.hInstance = AfxGetInstanceHandle();
	if(DoModal()!=IDCANCEL)
	{

		m_PgExist.SaveInvitation(m_PgInvitationSave.GetFilePath(),m_PgInvitationSave.GetSessionPassword()  ) ;
	}
	DeleteObject(m_psh.hbmHeader);
	return 0;
}
/*********************
on dialog initialisation
*********************/
BOOL CExistingWizard::OnInitDialog()
{
	// TODO: Add your specialized code here and/or call the base class
	CPropertySheet::OnInitDialog(); 
	CString		sCaption;

	sCaption.LoadString(IDS_EXISTING_NETWORK_WIZARD_CAPTION);
	SetWizardButtons(PSWIZB_NEXT);
	SetWindowText(sCaption);

	if(m_PgExist.GetCount()==0)
	{
		AfxMessageBox(IDS_NO_VALID_INVITATION_EXIST,MB_OK|MB_ICONERROR);  
		EndDialog(IDCANCEL); 
	}
	return TRUE;
}
/*********************************
Wizard shows next handlet whether to put finish button
*********************************/
void CExistingWizard ::OnWizNext()
{
	//validate the input
	if(!m_PgExist.IsValidInput())
	{
		return;
	}
	SetWizardButtons(PSWIZB_FINISH|PSWIZB_BACK);
	Default();
}
/*************************************
Create the invitation file in specified location or throw error message
*************************************/
void CExistingWizard::OnWizFinish()
{
	//validate the input 
	if(!m_PgInvitationSave.IsValidInput())
	{
		return;
	}
	Default();
}