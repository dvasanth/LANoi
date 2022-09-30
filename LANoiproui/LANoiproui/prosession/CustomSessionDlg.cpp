// CustomSessionDlg.cpp : implementation file
//

#include "stdafx.h"
#include "../resource.h"
#include "CustomSessionDlg.h"
#include "SessionTypes.h"


// CCustomSessionDlg dialog

IMPLEMENT_DYNAMIC(CCustomSessionDlg, CDialog)

CCustomSessionDlg::CCustomSessionDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCustomSessionDlg::IDD, pParent)
	, m_sSessionName(_T(""))
	, m_sPortValues(_T(""))
{

}

CCustomSessionDlg::~CCustomSessionDlg()
{
	
}

void CCustomSessionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ADVANCED_BUTTON, m_BtnAdvanced);
	DDX_Control(pDX, IDC_PROCESS_LIST, m_CbnProcessPathList);
	DDX_Text(pDX, IDC_PORT_INFORMATION  , m_sPortValues);
	DDX_Text(pDX, IDC_SESSION_NAME  , m_sSessionName);
}


BEGIN_MESSAGE_MAP(CCustomSessionDlg, CDialog)
	ON_BN_CLICKED(IDC_ADD_PROCESS, &CCustomSessionDlg::OnBnClickedAddProcess)
	ON_BN_CLICKED(IDC_ADVANCED_BUTTON, &CCustomSessionDlg::OnBnClickedAdvancedButton)
	ON_BN_CLICKED(ID_ADD, &CCustomSessionDlg::OnBnClickedAdd)
END_MESSAGE_MAP()

/*********************
on dialog initialisation
*********************/
BOOL CCustomSessionDlg::OnInitDialog()
{
	// TODO: Add your specialized code here and/or call the base class
	CDialog::OnInitDialog(); 
	OnLoad();
	return TRUE;
}
/*********************************
Write dialog intitialization code 
***********************************/
void CCustomSessionDlg::OnLoad()
{
	CenterDialog(this); 

	ShowAdvancedOption(FALSE);
	//load data to control
	UpdateData(0);
}
/***********************************************
Add the new session to the file
***********************************************/
void CCustomSessionDlg::OnBnClickedAdd()
{
	// TODO: Add your control notification handler code here
	if(!IsValidInput())
		return;
	if(!OnSave())
		return;
	OnOK();
}
/************************************
save the data into persistance storage for loading it next time
*************************************/
BOOL CCustomSessionDlg::OnSave()
{

	//save the new session to the file
	//add the new session 
	CSessionTypes  Add;
	if(!Add.AddSessionType(m_sSessionName,m_SessionXMLString) )
	{
		AfxMessageBox(IDS_SESSION_NAME_EXISTS,MB_OK  |MB_ICONSTOP|MB_TOPMOST  );
		return FALSE;
	}
	return TRUE;

}
 /********************************
Validate the input
*********************************/
BOOL CCustomSessionDlg::IsValidInput()
{
	UpdateData(TRUE);

	if(m_sSessionName.IsEmpty() &&m_sPortValues.IsEmpty())
	{
		if(m_sSessionName.IsEmpty() )
		{
			AfxMessageBox(IDS_ERR_EMPTY_SESSION_NAME, MB_OK  |MB_ICONSTOP|MB_TOPMOST  );
			return false;
		}

		if(m_sPortValues.IsEmpty() )
		{
			AfxMessageBox(IDS_ERR_EMPTY_PORT_VALUES, MB_OK  |MB_ICONSTOP|MB_TOPMOST  );
			return false;
		}
	}
	CLOISessionStringFormat LOIFormat;
	//form the xml string from the input
	if(!PrasePortValues(m_sPortValues,LOIFormat) &&
		!AddApplicationNames(m_CbnProcessPathList,LOIFormat) 
		)
	{
		AfxMessageBox(IDS_ERR_EMPTY_PORT_VALUES, MB_OK  |MB_ICONSTOP|MB_TOPMOST  );
		return false;
	}
	m_SessionXMLString=LOIFormat.GetXMLString(); 
	return true;
}
/**************************************
get the port values from typed port string
**************************************/
BOOL CCustomSessionDlg::PrasePortValues(CString sPort,CLOISessionStringFormat &sLOIFormat)
{
	LPTSTR			  token,Context=NULL;
	TCHAR			  delimiter[]   =_T(" ,;:");
	LPTSTR			  pszLocalAllocPortlist= new TCHAR[_tcslen((LPCTSTR)sPort)+4];
	LPTSTR			  pszPortList=pszLocalAllocPortlist;
	BOOL			  bAtleastOneEntyValid=FALSE;

	StringCbCopy(pszPortList,(_tcslen((LPCTSTR)sPort)+4)*sizeof(TCHAR),(LPCTSTR)sPort);

	token = _tcstok_s(pszPortList, delimiter,&Context );
    while( token != NULL )
	{
	  BOOL        bServicePort=FALSE;
	  USHORT	  usPort;

	  //special case for service port handling
	  if(token[0] == 'S')
	  {
		bServicePort=TRUE;
		token++;
	   }
	   if(_tcsstr(token,_T("-")))//user asked for port ranges
	   {
		  TCHAR			szStartPort[10]={0},szEndPort[10]={0};
		  _tcsncpy_s(szStartPort,10,token, _tcsstr(token,_T("-"))-token);
		  _tcscpy_s(szEndPort,10,_tcsstr(token,_T("-"))+1);
		  sLOIFormat.AddPortRange(_ttoi(szStartPort),_ttoi(szEndPort),bServicePort);
		  bAtleastOneEntyValid=TRUE;
		}
		else //only one port
		{
			usPort=_ttoi(token);
			sLOIFormat.AddPortRange(usPort,usPort,bServicePort);
			bAtleastOneEntyValid=TRUE;
		}
	     /* Get next token: */
		 token = _tcstok_s( NULL, delimiter,&Context );
	}

	if(pszLocalAllocPortlist)
	{
		delete pszLocalAllocPortlist;
		pszLocalAllocPortlist=NULL;
	}
	return bAtleastOneEntyValid;
}
/**************************************************
Add the applications for the session
***************************************************/
BOOL CCustomSessionDlg::AddApplicationNames(CComboBox &CbnItems,CLOISessionStringFormat &sLOIFormat)
{
	BOOL	bAtLeastOneEntryValid=FALSE;

	for (int iIndex = 0; iIndex < CbnItems.GetCount(); iIndex++)
	{
		CString		sAppName;
		CbnItems.GetLBText(iIndex, sAppName);

		sLOIFormat.AddApplication(sAppName); 
		bAtLeastOneEntryValid=TRUE;
	}	
	return bAtLeastOneEntryValid;
}
// CCustomSessionDlg message handlers
/***********************************
Show the exe browse files...
************************************/
void CCustomSessionDlg::OnBnClickedAddProcess()
{
	// TODO: Add your control notification handler code here
	//get the filepath from user
	CString		sFileExtFilter;
	CString		sOpenDlgCaption;
	CString		sFileExt;
	CString		sFilepath;

	sOpenDlgCaption.LoadString(IDS_OPEN_EXE_DLG_CAPTION);
	sFileExtFilter.LoadString(IDS_EXE_FILE_EXTENSION_FILTER);
	sFileExt.LoadString(IDS_EXE_FILE_EXTENSION);

	CFileDialog fOpenDlg(TRUE,(LPCTSTR)sFileExt, 
						_T(""), OFN_HIDEREADONLY|OFN_FILEMUSTEXIST,
						sFileExtFilter, 
						this);

	// Initializes m_pOFN structure
	fOpenDlg.m_pOFN->lpstrTitle=sOpenDlgCaption;

	if(fOpenDlg.DoModal()==IDCANCEL) 
	{
		return ;
	}
	sFilepath=fOpenDlg.GetPathName();

	//add the filename to the combo
	int iIndex;
	iIndex=m_CbnProcessPathList.AddString( fOpenDlg.GetFileName ());
	m_CbnProcessPathList.SetCurSel(iIndex);
}

/*********************************
toggle between expand & contract
***********************************/
void CCustomSessionDlg::OnBnClickedAdvancedButton()
{
	CString  sAdvancedTxt;
	CString  sExpandCaption;


	sExpandCaption.LoadString(IDS_ADVANCED_BTN_EXPAND);
	m_BtnAdvanced.GetWindowText(sAdvancedTxt);
	if(sAdvancedTxt==sExpandCaption)
	{
		ShowAdvancedOption(TRUE);
		return;
	}
	ShowAdvancedOption(FALSE);
}

/**************************************
helper for sizing the window
**************************************/
void CCustomSessionDlg::ShowAdvancedOption(BOOL bShow)
{
	RECT		rtBorderButton={0};
	RECT		rtDlgClient={0};
	CString     sAdvanced;

	if(bShow)
	{
		sAdvanced.LoadString(IDS_ADVANCED_BTN_CONTRACT);
		m_BtnAdvanced.SetWindowText(sAdvanced);
		GetDlgItem(IDC_ADD_PROCESS)->GetWindowRect(&rtBorderButton);
	}
	else
	{
		sAdvanced.LoadString(IDS_ADVANCED_BTN_EXPAND);
		m_BtnAdvanced.SetWindowText(sAdvanced);
		GetDlgItem(IDC_ADVANCED_BUTTON)->GetWindowRect(&rtBorderButton);
	}

	GetWindowRect(&rtDlgClient);
	MoveWindow(rtDlgClient.left ,rtDlgClient.top ,(rtDlgClient.right-rtDlgClient.left) ,(rtBorderButton.bottom-rtDlgClient.top) +14,TRUE);
}