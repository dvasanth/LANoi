// ProgressDlg.cpp : implementation file
//

#include "stdafx.h"

#include "ProgressDlg.h"

// CProgressDlg dialog

IMPLEMENT_DYNAMIC(CProgressDlg, CDialog)

CProgressDlg::CProgressDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CProgressDlg::IDD, pParent)
	, m_sStatusText(_T(""))
{

}

CProgressDlg::~CProgressDlg()
{
}

void CProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATUS_UPDATE_EDIT   , m_sStatusText);
	DDX_Control(pDX, IDC_PROGRESS, m_prgProgressUpdate);
}


BEGIN_MESSAGE_MAP(CProgressDlg, CDialog)
	ON_BN_CLICKED(IDCANCEL, &CProgressDlg::OnBnClickedCancel)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProgressDlg message handlers
BOOL CProgressDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_prgProgressUpdate.SetRange(0, 10);
	m_prgProgressUpdate.SetStep(2);  
	m_prgProgressUpdate.SetBarColor( RGB(105,207,8)); 
	m_prgProgressUpdate.SetBkColor(0); 
	UpdateData(FALSE); 
	CenterDialog(this);
	//ModifyStyleEx(WS_EX_APPWINDOW,WS_EX_TOOLWINDOW );
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
// CProgressDlg message handlers

void CProgressDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	CDialog::OnCancel();
}
void CProgressDlg::OnCancel()
{
	OnBnClickedCancel();  
}
void CProgressDlg::SetStatusText(UINT uiStringID)
{
	if(IsWindow(m_hWnd))
	{
		CString  sProgressTxt;

		sProgressTxt.LoadString(uiStringID);
		m_sStatusText=  sProgressTxt;
		UpdateData(FALSE); 
		m_prgProgressUpdate.StepIt(); 
		m_prgProgressUpdate.StepIt(); 
	}
}
void CProgressDlg::Close()
{
	if(IsWindow(m_hWnd))
	{
		DestroyWindow() ;
		this->Detach(); 
	}
}
