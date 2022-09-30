// ContactSupportDlg.cpp : implementation file
//

#include "stdafx.h"
#include "../LANoiproui.h"
#include "ContactSupportDlg.h"
#include "URLEncode.h"

// CContactSupportDlg dialog

IMPLEMENT_DYNAMIC(CContactSupportDlg, CDialog)

CContactSupportDlg::CContactSupportDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CContactSupportDlg::IDD, pParent)
{

}

CContactSupportDlg::~CContactSupportDlg()
{
}

void CContactSupportDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_FROM_E_MAIL  , m_sFromAddress);
	DDX_Text(pDX, IDC_SUBJECT_EMAIL  , m_sSubject);
	DDX_Text(pDX, IDC_MESSAGE_EMAIL  , m_sMessage);
	DDX_Text(pDX,IDC_USERNAME,m_sUserName); 
	DDX_Control(pDX, IDC_MESSAGE_EMAIL, m_EdtMessage);
}


BEGIN_MESSAGE_MAP(CContactSupportDlg, CDialog)
	ON_BN_CLICKED(IDC_SEND , &CContactSupportDlg::OnBnClickedSendEmail)
	ON_EN_SETFOCUS(IDC_MESSAGE_EMAIL, &CContactSupportDlg::OnEnSetfocusMessageEmail)
END_MESSAGE_MAP()


// CContactSupportDlg message handlers

// CSessionSelectionPage message handlers
BOOL CContactSupportDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	OnLoad();
	return TRUE;
}

/*********************************
Write dialog intitialization code 
***********************************/
void CContactSupportDlg::OnLoad()
{


	m_sMessage.LoadString(IDS_MAIL_DEFAULT_MESSAGE);
	//m_EdtMessage.InitString(m_sMessage);
	//load data to control
	UpdateData(0);
	SendDlgItemMessage(IDC_MESSAGE_EMAIL,EM_SETSEL,-1,0);
}
/************************************
save the data into persistance storage for loading it next time
*************************************/
void CContactSupportDlg::OnSave()
{


}
 /********************************
Validate the input
*********************************/
BOOL CContactSupportDlg::IsValidInput()
{
	UpdateData(TRUE);
	
	if(m_sFromAddress.IsEmpty() ||
		m_sFromAddress.Find('@')==-1
		)
	{
		AfxMessageBox(IDS_VALID_EMAIL_ADDRESS,MB_OK  |MB_ICONSTOP|MB_TOPMOST);
		return FALSE;
	}

	
	return true;
}
/*********************************
On click send e-mail
**********************************/
void CContactSupportDlg::OnBnClickedSendEmail()
{
	CInternetSession				session;
	CString							strHeaders ;
    CString							strFormData;
	CHttpConnection*				pConnection=NULL;
	CHttpFile*						pFile=NULL;
	BOOL							bResult=FALSE;
	CFile							LogFile;
	CURLEncode						urlEncode;
	const	LONG					READ_BYTES=25*1024;
	DWORD							dwCharsRead;
	CHAR							*pchReadChars=new CHAR[READ_BYTES];
	CStringA						sEncodedBytes;
	CString							sLogFileData;
	TCHAR							szServiceLogFilePath[MAX_PATH]={0};
	CString							sPostDataFormat;

	if(!IsValidInput())
	{
		delete pchReadChars;
		return;
	}
	IFileLogging::GetLogFileDirectory(szServiceLogFilePath,sizeof(szServiceLogFilePath));
	StringCbCat(szServiceLogFilePath,sizeof(szServiceLogFilePath),_T("\\servicelog.txt")); 
	LogFile.Open(szServiceLogFilePath,CFile::modeRead|CFile::shareDenyNone );
	ZeroMemory(pchReadChars,READ_BYTES);
	if(LogFile.GetLength()>  READ_BYTES)
	{
		SetFilePointer(LogFile.m_hFile,- READ_BYTES,0,FILE_END);  
	}
	else
		LogFile.Seek(4,CFile::begin);
	dwCharsRead=LogFile.Read(pchReadChars, READ_BYTES-2);//-2 to make it null terminated
	if(dwCharsRead<4)
	{
		delete pchReadChars;
		return;
	}

	sLogFileData=CString((LPCTSTR)pchReadChars);

	//form the post data string 
	strHeaders = _T("Content-Type: application/x-www-form-urlencoded");
	sPostDataFormat.LoadString(IDS_MAIL_SEND_POST_DATA);
	//_T("departmentid=1&name=t&email=vas_kum_2000%40yahoo.com&subject=test&message=test&email_send=Submit");
	//departmentid=1&name=%s&email=%s&subject=%s&message=%s&email_send=Submit
	
	m_sMessage+=sLogFileData;
	CStringA	sUserName=m_sUserName.IsEmpty()?"NotEntered":urlEncode.URLEncode(m_sUserName);
	CStringA	sFromAddr=urlEncode.URLEncode(m_sFromAddress);
	CStringA	sSubject= m_sSubject.IsEmpty()?"NotEntered":urlEncode.URLEncode(m_sSubject);
	CStringA	sMessage=m_sMessage.IsEmpty()?"NotEntered":urlEncode.URLEncode(m_sMessage);
	
	strFormData.Format((LPCTSTR)sPostDataFormat,
												(LPCSTR)sUserName,
												(LPCSTR)sFromAddr,
												(LPCSTR)sSubject,
												(LPCSTR)sMessage
												);
						
	CStringA sStrA;
	sStrA.Format("%S",strFormData);
	
    pConnection = session.GetHttpConnection(_T("lanoninternet.com"));
	if(pConnection)
	{
		pFile =  pConnection->OpenRequest(CHttpConnection::HTTP_VERB_POST,
                              _T("/chat/module.php?module=HelpCenter"));
		bResult = pFile->SendRequest(strHeaders, 
									(LPVOID)(LPCSTR)sStrA,//(LPVOID)(LPCTSTR)strFormData,
									sStrA.GetLength()//strFormData.GetLength()
									);
	}


	if(pchReadChars)
	{
		delete pchReadChars;
		pchReadChars=NULL;
	}

	if(pFile)
	{
		delete pFile;
		pFile=NULL;
	}
	if(pConnection)
	{
		delete pConnection;
		pConnection=NULL;
	}
	if(bResult==FALSE)
	{
		AfxMessageBox(IDS_MAIL_SEND_FAILED,MB_OK  |MB_ICONSTOP|MB_TOPMOST);
	}
	else
	{
		AfxMessageBox(IDS_MAIL_SEND_SUCCESS,MB_OK  |MB_ICONINFORMATION|MB_TOPMOST);
		OnOK();
	}
}
/*******************************
On MessageFocus
*********************************/
void CContactSupportDlg::OnEnSetfocusMessageEmail()
{
	// TODO: Add your control notification handler code here
	GetDlgItem(IDC_MESSAGE_EMAIL)->SetWindowText(_T(""));  
}
