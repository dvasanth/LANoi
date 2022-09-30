// InviterWizard.cpp : implementation file
//

#include "stdafx.h"
#include "InviterWizard.h"

// CInviterWizard

IMPLEMENT_DYNAMIC(CInviterWizard, CPropertySheet)

CInviterWizard::CInviterWizard(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{

}

CInviterWizard::CInviterWizard(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{

}

CInviterWizard::~CInviterWizard()
{
}


BEGIN_MESSAGE_MAP(CInviterWizard, CPropertySheet)
	ON_BN_CLICKED(ID_WIZNEXT, OnWizNext)
	ON_BN_CLICKED(ID_WIZBACK, OnWizBack)
	
	ON_BN_CLICKED(ID_WIZFINISH,OnWizFinish)
END_MESSAGE_MAP()


// CInviterWizard message handlers
/*********************************
Function added to handle page addition & dispalying
*********************************/
int CInviterWizard::ShowWizard()
{

	INT_PTR					  iptrRetVal;

	AddPage(&m_PgSession);
	AddPage(&m_PgInvitationSave);
	m_psh.dwFlags |= PSH_WIZARD97|PSH_WATERMARK|PSH_HEADER|PSH_USEHBMHEADER;
//	m_psh.pszbmWatermark = MAKEINTRESOURCE(IDB_WIZ_WATERMARK);
//	m_psh.pszbmHeader = MAKEINTRESOURCE(IDB_WIZ_BANNER_ICON);
	m_psh.hbmHeader = (HBITMAP)LoadImage( AfxGetInstanceHandle(),_T("images\\banner_icon_add.bmp"), IMAGE_BITMAP, 0,0, LR_LOADFROMFILE|LR_SHARED );
	m_psh.hInstance = AfxGetInstanceHandle();
	m_psh.dwFlags &= ~PSH_HASHELP;

	iptrRetVal=DoModal();

	DeleteObject(m_psh.hbmHeader);
	return iptrRetVal;
}
/*********************
on dialog initialisation
*********************/
BOOL CInviterWizard::OnInitDialog()
{
	// TODO: Add your specialized code here and/or call the base class
	CPropertySheet::OnInitDialog(); 
	CString		sCaption;

	sCaption.LoadString(IDS_ADD_NETWORK_WIZARD_CAPTION);
	SetWizardButtons(PSWIZB_NEXT);
	SetWindowText(sCaption);
	return TRUE;
}
/*******************************
Wizard shows next handler decide whether to put finish button
*******************************/
void CInviterWizard::OnWizNext()
{
	//validate the input
	if(!m_PgSession.IsValidInput())
	{
		return;
	}

	SetWizardButtons(PSWIZB_FINISH|PSWIZB_BACK);
	Default();
}

/*************************************
Create the invitation file in specified location or throw error message
*************************************/
void CInviterWizard::OnWizFinish()
{
	//validate the input 
	if(!m_PgInvitationSave.IsValidInput())
	{
		return;
	}
	Default();
}
/****************************************
wizard back 
*****************************************/
void CInviterWizard::OnWizBack()
{
	SetWizardButtons(PSWIZB_NEXT);
	Default();
}