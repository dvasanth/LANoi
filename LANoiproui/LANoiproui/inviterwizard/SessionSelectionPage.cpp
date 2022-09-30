// SessionSelectionPage.cpp : implementation file
//

#include "stdafx.h"
#include "SessionSelectionPage.h"
#include "RegOperations.h"
#include "../prosession/CustomSessionDlg.h"
#include "cmnresource.h"

const    EXPIRY_DATE_FORMAT CSessionSelectionPage::ExpiryDaysLists[]={
	{_T("1 hour"),1},
	{_T("2 hour"),2},
	{_T("1 day"),24},
	{_T("2 day"),48},
	{_T("15 day"),360},
	{_T("30 day"),720},
};


// CSessionSelectionPage dialog

IMPLEMENT_DYNAMIC(CSessionSelectionPage, CPropertyPage)

CSessionSelectionPage::CSessionSelectionPage()
	: CPropertyPage(CSessionSelectionPage::IDD)
	, m_sNickname(_T(""))
{
	m_sHeaderTitle.LoadString(IDS_SESSIONPAGE_HEADER);
	m_sSubtitle.LoadString(IDS_SESSIONPAGE_SUBTITLE);

	m_psp.dwFlags |= PSP_DEFAULT|PSP_USEHEADERTITLE|PSP_USEHEADERSUBTITLE;
	m_psp.pszHeaderTitle =(LPCTSTR)m_sHeaderTitle;
	m_psp.pszHeaderSubTitle = (LPCTSTR)m_sSubtitle;
	m_psp.dwFlags &= ~PSP_HASHELP;
}

CSessionSelectionPage::~CSessionSelectionPage()
{
	OnSave();
}

void CSessionSelectionPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SESSION_LIST, m_CbnSessionList);
	DDX_Text(pDX, IDC_CREATE_NICKNAME  , m_sNickname);

	DDX_Control(pDX, IDC_EXPIRY_DATE_LISTS, m_cbnExpiryDateList);
}


BEGIN_MESSAGE_MAP(CSessionSelectionPage, CPropertyPage)
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_ADD_NEW_SESSION, &CSessionSelectionPage::OnBnClickedAddNewSession)
END_MESSAGE_MAP()


// CSessionSelectionPage message handlers
BOOL CSessionSelectionPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	OnLoad();
	return TRUE;
}

/*********************************
Write dialog intitialization code 
***********************************/
void CSessionSelectionPage::OnLoad()
{
	CRegOperations Load;

	LoadSessionTypes();

	//load the nickname
	m_sNickname = Load.GetNickname();

	m_cbnExpiryDateList.ResetContent();
	//add the expiry time to the combo list
	for(int iIndex=0;iIndex<sizeof(ExpiryDaysLists)/sizeof(ExpiryDaysLists[0]);iIndex++)
	{
		m_cbnExpiryDateList.AddString(ExpiryDaysLists[iIndex].szTimeFormat);  
	}
	m_cbnExpiryDateList.SetCurSel(2); 

	//load data to control
	UpdateData(0);
}
/************************************
save the data into persistance storage for loading it next time
*************************************/
void CSessionSelectionPage::OnSave()
{
	CRegOperations Save;

	//load nickname to registry
	if(!m_sNickname.IsEmpty()) 
		Save.SetNickname(m_sNickname); 

}
 /********************************
Validate the input
*********************************/
BOOL CSessionSelectionPage::IsValidInput()
{
	UpdateData(TRUE);
	//get the combo box selected session port string
	m_CbnSessionList.GetLBText(m_CbnSessionList.GetCurSel(), m_sSelectedSessionName);
	m_sSessionResourceString=m_SessionNames.GetSessionValue(  m_sSelectedSessionName);

	//store the expiry time for the invitation
	CString sExpiryTimeSelected;
	m_cbnExpiryDateList.GetLBText(m_cbnExpiryDateList.GetCurSel(), sExpiryTimeSelected);
	for(int iIndex=0;iIndex<sizeof(ExpiryDaysLists)/sizeof(ExpiryDaysLists[0]);iIndex++)
	{
		if(CString(ExpiryDaysLists[iIndex].szTimeFormat)==sExpiryTimeSelected)
		{
			m_ftNotValidAfterTime= GetIncrementedFiletime(ExpiryDaysLists[iIndex].uiHours); 
			break;
		}
	}

	if(m_sNickname.IsEmpty() )
	{
		AfxMessageBox(IDS_ERR_EMPTY_NICKNAME, MB_OK  |MB_ICONSTOP|MB_TOPMOST  );
		return false;
	}


	return true;
}
/************************************
increment hours to the current filetime
************************************/
FILETIME CSessionSelectionPage::GetIncrementedFiletime(UINT uiHours)
{
	SYSTEMTIME			stTime;
	FILETIME			ftFileTime;
	const ULONGLONG     SECOND= ((ULONGLONG) 10000000);
	const ULONGLONG		MINUTE = (60 *SECOND);
	const ULONGLONG		HOUR  = (60 *MINUTE);
	ULONGLONG			qwResult;

	GetSystemTime(&stTime);
	SystemTimeToFileTime(&stTime,&ftFileTime); 
			
    // Copy the time into a quadword.
    qwResult = (((ULONGLONG) ftFileTime.dwHighDateTime) << 32) + ftFileTime.dwLowDateTime;

   // Add 30 days.
   qwResult += uiHours * HOUR;

   // Copy the result back into the FILETIME structure.
   ftFileTime.dwLowDateTime  = (DWORD) (qwResult & 0xFFFFFFFF );
   ftFileTime.dwHighDateTime = (DWORD) (qwResult >> 32 );

   return ftFileTime;
}

/***********************************************
loads all sessiontypes from the dat file to the combo box
*************************************************/
void CSessionSelectionPage::LoadSessionTypes()
{	
	//load the session types
	m_SessionNames.Reload(); 
	m_CbnSessionList.ResetContent(); 
	for(int i=0;i<m_SessionNames.GetLength();i++)
		m_CbnSessionList.AddString(m_SessionNames[i]);

	CString sRemoteAssitance;
	sRemoteAssitance.LoadString(IDS_RA_SESSION_NAME);  

	m_CbnSessionList.SetCurSel(0); 

	int iRASessionIndex=m_CbnSessionList.FindString(0,(CString)sRemoteAssitance);

	if(iRASessionIndex!=CB_ERR)
	{
		m_CbnSessionList.SetCurSel(iRASessionIndex); 
	}
}


/**************************************
Get the session display name
***************************************/
CString CSessionSelectionPage::GetSessionDisplayName()
{
	return m_sSelectedSessionName;
}
/*********************************
Get the session resource string
*********************************/
CString CSessionSelectionPage::GetSessionResourceString()
{
	return m_sSessionResourceString;
}
/*******************************************
get the nickname
******************************************/
CString CSessionSelectionPage::GetNickName()
{
	return m_sNickname;
}
/***********************
expiry time for the invitation
***********************/
FILETIME CSessionSelectionPage::GetExpiryTime()
{
	return m_ftNotValidAfterTime;
}
/***************************************
Add new custom session
****************************************/
void CSessionSelectionPage::OnBnClickedAddNewSession()
{
	// TODO: Add your control notification handler code here
	CCustomSessionDlg  CustomDlg;

	CustomDlg.DoModal();

	
	LoadSessionTypes();

	int iNewSessionIndex=m_CbnSessionList.FindString(0,(CString)CustomDlg.GetSessionName());

	if(iNewSessionIndex!=CB_ERR)
	{
		m_CbnSessionList.SetCurSel(iNewSessionIndex); 
	}
}
