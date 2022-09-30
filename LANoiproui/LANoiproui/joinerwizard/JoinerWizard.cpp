// JoinerWizard.cpp : implementation file
//

#include "stdafx.h"
#include "JoinerWizard.h"




// CJoinerWizard

IMPLEMENT_DYNAMIC(CJoinerWizard, CPropertySheet)

CJoinerWizard::CJoinerWizard(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{

}

CJoinerWizard::CJoinerWizard(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{

}

CJoinerWizard::~CJoinerWizard()
{
}


BEGIN_MESSAGE_MAP(CJoinerWizard, CPropertySheet)
	ON_BN_CLICKED(ID_WIZNEXT,OnWizNext)
	ON_BN_CLICKED(ID_WIZFINISH,OnWizFinish)
END_MESSAGE_MAP()


// CJoinerWizard message handlers

/*********************************
Funtion added to handle page addition & displaying
get the filepath from dde or from user
*********************************/
INT_PTR CJoinerWizard::ShowWizard(CIPCCommandHandler &IpcExecute,CString sFilepath)
{
	INT_PTR					  iptrRetVal;

	m_sFilepath=sFilepath;
	//get the filepath of the file
	if(m_sFilepath.IsEmpty())
	{
		//get the filepath from user
		CString		sFileExtFilter;
		CString		sOpenDlgCaption;
		CString		sFileExt;

		sFileExt.LoadString(IDS_INVITE_FILE_EXTENSION);
		sOpenDlgCaption.LoadString(IDS_OPEN_INVITE_DLG_CAPTION);
		sFileExtFilter.LoadString(IDS_INVITE_FILE_EXTENSION_FILTER);
		sFileExt.LoadString(IDS_INVITE_FILE_EXTENSION);

		CFileDialog fOpenDlg(TRUE,(LPCTSTR)sFileExt, 
							_T(""), OFN_HIDEREADONLY|OFN_FILEMUSTEXIST,
							sFileExtFilter, 
							this);

		// Initializes m_pOFN structure
		fOpenDlg.m_pOFN->lpstrTitle=sOpenDlgCaption;

		if(fOpenDlg.DoModal()==IDCANCEL) 
		{
			return IDCANCEL;
		}
		m_sFilepath=fOpenDlg.GetPathName();
	}

	AddPage(&m_PgAuth);

	m_psh.dwFlags |= PSH_WIZARD97|PSH_WATERMARK|PSH_HEADER|PSH_USEHBMHEADER;
//	m_psh.pszbmWatermark = MAKEINTRESOURCE(IDB_WIZ_WATERMARK);
//	m_psh.pszbmHeader = MAKEINTRESOURCE(IDB_WIZ_BANNER_ICON);
	m_psh.hbmHeader =(HBITMAP) LoadImage( AfxGetInstanceHandle(),_T("images\\banner_icon_add.bmp"), IMAGE_BITMAP, 0,0, LR_LOADFROMFILE|LR_SHARED );
	m_psh.hInstance = AfxGetInstanceHandle();



	iptrRetVal=DoModal();

	DeleteObject(m_psh.hbmHeader);
	return iptrRetVal;
}
/*********************
on dialog initialisation
*********************/
BOOL CJoinerWizard::OnInitDialog()
{
	// TODO: Add your specialized code here and/or call the base class
	CPropertySheet::OnInitDialog(); 
	CString		sCaption;

	sCaption.LoadString(IDS_JOIN_NETWORK_WIZARD_CAPTION);
	SetWizardButtons(PSWIZB_FINISH);
	SetWindowText(sCaption);
	return TRUE;
}
/*********************************
Wizard shows next handlet whether to put finish button
*********************************/
void CJoinerWizard ::OnWizNext()
{

	Default();
}
/*************************************
validate the input
*************************************/
void CJoinerWizard::OnWizFinish()
{
	//validate the input 
	if(!m_PgAuth.IsValidInput())
	{
		return;
	}
	Default();
}
