// LANoiprouiDlg.cpp : implementation file
//

#include "stdafx.h"
#include "LANoiproui.h"
#include "LANoiprouiDlg.h"
#include "inviterwizard/InviterWizard.h"
#include "joinerwizard/JoinerWizard.h"
#include "existingwizard/ExistingWizard.h"
#include "support/ContactSupportDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CLANoiprouiDlg dialog


const UINT CLANoiprouiDlg::SERVICE_IPC_NOTIFY=RegisterWindowMessage(_T("IpcNotify"));
const UINT CLANoiprouiDlg::ADD_FIREWALL_RULE=RegisterWindowMessage(_T("LSPNotify")); 
const UINT CLANoiprouiDlg::TRAY_MESSAGE=RegisterWindowMessage(_T("TrayNotify")); 
const TCHAR	CLANoiprouiDlg::WINDOW_CAPTION[]=_T("LANoiproui");
const UINT	CLANoiprouiDlg::DDE_MESSAGE_FILE_OPEN=RegisterWindowMessage(_T("PrevInstNotify"));
const TCHAR	CLANoiprouiDlg::STARTUP_COMMAND[]=_T("-startup");

CLANoiprouiDlg::CLANoiprouiDlg(CWnd* pParent /*=NULL*/)
	: CSkinDialog(CLANoiprouiDlg::IDD, pParent)
{
	m_hIcon=NULL;
	m_bShowWindow=TRUE;
	m_pMenu =NULL;
	m_bP2PEnabled=FALSE;
}
/******************
standard destructor
******************/
CLANoiprouiDlg::~CLANoiprouiDlg()
{
	if(m_pMenu)
	{
		delete m_pMenu;
		m_pMenu=NULL;
	}
}
void CLANoiprouiDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ONLINE_PEERS, m_TreeCtrlOnlinePeers);
	DDX_Control(pDX, IDC_ADD, m_BtnAdd);
	DDX_Control(pDX, IDC_JOIN, m_BtnJoin);
	DDX_Control(pDX, IDC_EXISTING, m_BtnExisting);
}

BEGIN_MESSAGE_MAP(CLANoiprouiDlg, CSkinDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_COMMAND(IDM_INVITE_PARTNER, &CLANoiprouiDlg::OnInvitePartner)
	ON_COMMAND(IDM_JOIN_PARTNER, &CLANoiprouiDlg::OnJoinPartner)
	ON_COMMAND(IDM_EXISTING_INVITATIONS, &CLANoiprouiDlg::OnExistingInvitations)
	ON_BN_CLICKED(IDC_ADD, &CLANoiprouiDlg::OnInvitePartner)
	ON_BN_CLICKED(IDC_JOIN, &CLANoiprouiDlg::OnJoinPartner)
	ON_BN_CLICKED(IDC_EXISTING, &CLANoiprouiDlg::OnExistingInvitations)
	ON_COMMAND(IDM_EXIT, &CLANoiprouiDlg::OnExit)
	ON_REGISTERED_MESSAGE(SERVICE_IPC_NOTIFY, OnServiceCmdReceive)
	ON_REGISTERED_MESSAGE(ADD_FIREWALL_RULE, OnAddFirewallRule)
	ON_REGISTERED_MESSAGE(DDE_MESSAGE_FILE_OPEN, OnAnotherInstanceFileOpen)
	ON_REGISTERED_MESSAGE(TRAY_MESSAGE, OnTrayNotification)
	ON_WM_DESTROY()
	ON_COMMAND(ID_SHOWLANOIPRO, &CLANoiprouiDlg::OnShowlanoipro)
	ON_COMMAND(ID_EXIT, &CLANoiprouiDlg::OnExit)
	ON_WM_WINDOWPOSCHANGING()
	ON_STN_CLICKED(IDC_CLOSE, &CLANoiprouiDlg::OnStnClickedClose)
	ON_COMMAND(ID_HELP_CONTACTSUPPORT, &CLANoiprouiDlg::OnHelpContactsupport)
	ON_COMMAND(ID_HELP_ABOUT, &CLANoiprouiDlg::OnHelpAbout)
	ON_WM_TIMER()
	ON_COMMAND(IDM_REFRESH, &CLANoiprouiDlg::OnRefresh)
END_MESSAGE_MAP()


// CLANoiprouiDlg message handlers

BOOL CLANoiprouiDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	OnLoad();

	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

/*********************************
Write dialog intitialization code 
***********************************/
void CLANoiprouiDlg::OnLoad()
{
	CString  sAppName;

	if(!m_ServiceIPC.NotifyServiceRequest (this->m_hWnd, SERVICE_IPC_NOTIFY))
	{
		AfxMessageBox((LPCTSTR)m_ServiceIPC.GetLastErrorMessage()  , MB_OK  |MB_ICONSTOP|MB_TOPMOST  );
		OnOK();
		return;
	}

	//font for the ui components
	m_Font.CreateFont(16,0,0,0,200,0,0,0,0,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,
						DEFAULT_PITCH,L"Verdana");

	m_TreeCtrlOnlinePeers.Init(m_Font,&m_ServiceIPC);

	//do tray initailization
	sAppName.LoadString(IDS_APP_TITLE); 
	m_hIcon = (HICON)LoadImage (AfxGetInstanceHandle(),_T("icons\\trayicon.ico"),IMAGE_ICON,0,0,LR_DEFAULTSIZE|LR_SHARED|LR_LOADFROMFILE);
	if (!m_TrayIcon.Create(this, IDR_TRAY_POPUP , sAppName, _T(""), _T(""), 10, CTrayNotifyIcon::None, m_hIcon, TRAY_MESSAGE, 0, TRUE))
	{
		OnOK();
		return ;
	}
	//update tray icon depending on the status of p2p
	m_ServiceIPC.GetP2PStatus();

	//no window in taskbar
	CenterDialog(this);
	ModifyStyleEx(WS_EX_APPWINDOW,WS_EX_TOOLWINDOW );


	//caption for IPC
	SetWindowText( WINDOW_CAPTION);

	//timer to notify 
	//SetTimer(0,500,NULL);

	//execute any command line if exists
	ExecuteCmdLine(FALSE);

	//set background
	SetBkImage(_T("images\\wndbackground.bmp"));

	//initialise the button images
	m_BtnAdd.SetSkin(_T("images\\add_btn_default.bmp"),
					 _T("images\\add_btn_default.bmp"),
					 _T("images\\add_btn_active.bmp"),
					 NULL,
					 NULL,
					_T("images\\Add_btn_Mask.bmp"),
					1,2,0);

	m_BtnJoin.SetSkin(_T("images\\join_btn_default.bmp"),
					 _T("images\\join_btn_default.bmp"),
					 _T("images\\join_btn_active.bmp"),
					 NULL,
					 NULL,_T("images\\join_btn_mask.bmp"),1,2,0);

	m_BtnExisting.SetSkin(_T("images\\existing_btn_default.bmp"),
					 _T("images\\existing_btn_default.bmp"),
					 _T("images\\existing_btn_active.bmp"),
					 NULL,
					 NULL,_T("images\\existing_btn_mask.bmp"),1,2,0);
	

	//set the menu

	m_pMenu = new CFreeMenu(IDR_UI_MENU,CPoint(28,57),this,
		                RGB(105,207,8),
						RGB(255,255,255),
						RGB(255,255,255),
						RGB(255,255,255),&m_Font,1);
	if(m_pMenu)
	{
		m_pMenu->DrawMenu();
	}


	//load data to control
	UpdateData(0);
}
/************************************
save the data into persistance storage for loading it next time
*************************************/
void CLANoiprouiDlg::OnSave()
{

	
}
/***********************
sys menu handling
************************/
void CLANoiprouiDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}
/**************************
In vista, messages can only be received from process with same access 
*************************/
void CLANoiprouiDlg::DisableVistaUIPI()
{
	typedef BOOL (WINAPI *LPFN_CHANGEWINDOWMESSAGEFILTER) (UINT, DWORD);
	LPFN_CHANGEWINDOWMESSAGEFILTER fnChangeWindowMessageFilter;
	fnChangeWindowMessageFilter =      (LPFN_CHANGEWINDOWMESSAGEFILTER)GetProcAddress(GetModuleHandle(TEXT("user32")),"ChangeWindowMessageFilter"); 

	if (NULL != fnChangeWindowMessageFilter)
	{
		fnChangeWindowMessageFilter(ADD_FIREWALL_RULE,MSGFLT_ADD);
		fnChangeWindowMessageFilter(DDE_MESSAGE_FILE_OPEN,MSGFLT_ADD);
	}
}
// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CLANoiprouiDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CPaintDC dc(this);
		SendMessage(WM_ERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);
		CDialog::OnPaint();

	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CLANoiprouiDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


/***************************************
show the invitation creation wizard screen
**************************************/
void CLANoiprouiDlg::OnInvitePartner()
{
	// TODO: Add your command handler code here

	//invoke create invitation thro the service
	CInviterWizard      Wizard(IDS_ABOUTBOX);

	if(Wizard.ShowWizard()!=IDCANCEL)
	{
		CString  sAppSessionXml;
		PBYTE	 pbyAppData=NULL;
		DWORD	 dwAppDataLen=0;
		BOOL     bSessionXMLChanged;

		bSessionXMLChanged=m_AppIntegrate.GetAppDataBlob(Wizard.GetSessionName(),
									  Wizard.GetNickName(),
									  Wizard.GetPassword(),
									  sAppSessionXml,
									  &pbyAppData,
									  dwAppDataLen
									  );


		//create the invitation
		m_ServiceIPC.CreateInvitation(Wizard.GetNickName(),
										Wizard.GetPassword(),
										Wizard.GetSessionName(),
										bSessionXMLChanged? sAppSessionXml:Wizard.GetSessionXML(),
										Wizard.GetExpiryTime(),
										Wizard.GetFilePath(),
										pbyAppData,
										dwAppDataLen
										);
		m_AppIntegrate.FreeAppDataBlob(pbyAppData); 
	}
	
}
/******************************
Invitation creation status
*******************************/
BOOL	CLANoiprouiDlg::PostCreateInvitationStatus(CString sSessionName,BOOL bSuccess,CString sErrorMessage)
{

	//session information no longer required
	m_AppIntegrate.ReleaseSessionData();

	if(bSuccess==FALSE)
	{
		//show error message & exit
		AfxMessageBox(sErrorMessage, MB_OK  |MB_ICONSTOP|MB_TOPMOST  );
		return 0;
	}	

	AfxMessageBox(IDS_CREATE_INVITATION_SUCCESS,MB_OK|MB_ICONINFORMATION|MB_TOPMOST);

	return TRUE;
}
/***********************************
Show the joining invitation wizard 
***********************************/
void CLANoiprouiDlg::OnJoinPartner()
{
	// TODO: Add your command handler code here

	//join the created invitation 

	OpenDocument(NULL);
}
/*************************************
Execute command line for open
**************************************/
void CLANoiprouiDlg::OpenDocument(CString sFilePath)
{
	CJoinerWizard    Wizard(IDS_ABOUTBOX);

	SetForegroundWindow(); 
	SetActiveWindow(); 
	if(Wizard.ShowWizard(m_ServiceIPC,sFilePath)!=IDCANCEL)
	{
		OnJoinStart();
		
		//join the network with the invitation file
		m_ServiceIPC.JoinInvitation(Wizard.GetNickname(),
										Wizard.GetFilepath(),
										Wizard.GetPassword()
										);
		m_JoinProgressDlg.SetStatusText(IDS_JOINER_CONNECTING_INVITER); 
	}
}
/********************
app data associated with the invitation file
**********************/
BOOL	CLANoiprouiDlg::PostInvitationAppData(BYTE *pbyAppData,DWORD dwAppDataLen)
{
	m_AppIntegrate.SetAppDataBlobData(pbyAppData,dwAppDataLen);
	return TRUE;
}
/*********************************
status about the client update
**********************************/
BOOL	CLANoiprouiDlg::PostJoinInvitationStatus(BOOL bConnectSuccess,CString sErrorMessage,CString sSessionName,CString sInviterName)
{
	CString   sMsgFormat;
	CString	  sMsg;



	OnJoinComplete();
	if(bConnectSuccess)
	{	
		CAppIntegrate App;

		if(!m_AppIntegrate.PreInviteeSessionStart(sSessionName,sInviterName))
		{
			sMsgFormat.LoadString(IDS_JOIN_INVITATION_SUCCESS);
			sMsg.Format((LPCTSTR)sMsgFormat,(LPCTSTR)sSessionName);   
			AfxMessageBox (sMsg,MB_OK  |MB_ICONINFORMATION|MB_TOPMOST);

		}	
		//session information no longer required
		m_AppIntegrate.ReleaseSessionData();
		return TRUE;
	}
	//session information no longer required
	m_AppIntegrate.ReleaseSessionData();
	if(sErrorMessage.IsEmpty()==FALSE)
	{
		AfxMessageBox (sErrorMessage,MB_OK  |MB_ICONSTOP|MB_TOPMOST);
	}
	return FALSE;
}
/*********************************
exit the ui 
*************************************/
void CLANoiprouiDlg::OnExit()
{
	// TODO: Add your command handler code here
	CDialog::OnOK();  
}

void CLANoiprouiDlg::OnExistingInvitations()
{
	// TODO: Add your command handler code here
	CExistingWizard  Wizard(IDS_ABOUTBOX);
	Wizard.ShowWizard();
}

/*******************************
dialog getting exit
********************************/
void CLANoiprouiDlg::OnDestroy()
{
	CSkinDialog::OnDestroy();

	// TODO: Add your message handler code here
	OnSave(); 
}
/**********************************
Command received from service need to dispatch
**********************************/
LRESULT CLANoiprouiDlg::OnServiceCmdReceive(WPARAM wParam, LPARAM lParam)
{
	m_ServiceIPC.HandleCommand(this);
	return 0;
}

/*******************
status changed update in try
*******************/
BOOL	CLANoiprouiDlg::PostP2PStatusChange(BOOL bEnabled)
{	
	m_bP2PEnabled=bEnabled;
	ToggleP2PStatus(bEnabled,TRUE);
	m_TreeCtrlOnlinePeers.UpdatePeerList();
	return TRUE;
}
/****************************
Toggle p2p status
****************************/
void CLANoiprouiDlg::ToggleP2PStatus(BOOL bEnabled,BOOL bShowBallon)
{
	CString		sBallonTip,sToolTip,sCaption;

	if(bEnabled)
	{
		sBallonTip.LoadString(IDS_BALLON_P2P_STARTED);
		sCaption.LoadString(IDS_APP_TITLE);
	/*	if(bShowBallon)
			m_TrayIcon.SetBalloonDetails((LPCTSTR)sBallonTip,
										(LPCTSTR) sCaption,
										CTrayNotifyIcon::Info,
										3000);  */
		sToolTip.LoadString(IDS_TOOLTIP_P2P_STARTED);
		m_TrayIcon.SetTooltipText((LPCTSTR)sToolTip); 
		m_TrayIcon.SetIcon (m_hIcon);
	}
	else
	{
		sBallonTip.LoadString(IDS_BALLONTIP_P2P_FAILED);
		sCaption.LoadString(IDS_APP_TITLE );
		m_TrayIcon.SetBalloonDetails((LPCTSTR)sBallonTip,(LPCTSTR) sCaption,CTrayNotifyIcon::Error,2000);  
		sToolTip.LoadString(IDS_TOOLTIP_P2P_FAILED);
		m_TrayIcon.SetTooltipText((LPCTSTR)sToolTip); 
		m_TrayIcon.SetIcon (m_hIcon);
		//grayed tray
		m_TrayIcon.SetGrayIcon(m_hIcon);
	}

}
/************************************************
LSP asks the service for adding firewall rule for the application
*************************************************/
LRESULT	CLANoiprouiDlg::OnAddFirewallRule(WPARAM wParam, LPARAM lParam)
{
	m_ServiceIPC.FirewallAdd(wParam); 
	return 0;
}
/***********************************
Joining peer start
************************************/
void CLANoiprouiDlg::OnJoinStart()
{
	m_JoinProgressDlg.Create(MAKEINTRESOURCE(IDD_PROGRESS_DIALOG),this); 
	m_JoinProgressDlg.ShowWindow(SW_SHOW); 
	m_JoinProgressDlg.SetStatusText(IDS_JOINER_RESOLVING_INVITER); 
	//hide main window
	//ShowWindow(SW_HIDE); 
}
/********************************
joining the peer completed
********************************/
void CLANoiprouiDlg::OnJoinComplete()
{

	m_JoinProgressDlg.Close();
	//show main window
	//ShowWindow(SW_SHOW); 
}
/***********************************
show the main window
************************************/
void CLANoiprouiDlg::OnShowlanoipro()
{

	m_bShowWindow=TRUE;
	ShowWindow(SW_SHOW); 
	SetForegroundWindow(); 

	// TODO: Add your command handler code here
}
/**********************
Hidw window ion start
************************/
void CLANoiprouiDlg::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
	if ( !m_bShowWindow )
	lpwndpos->flags &= ~SWP_SHOWWINDOW ;

	CDialog::OnWindowPosChanging(lpwndpos);

	// TODO: Add your message handler code here
}
/***************************
hide the dialog on close click
****************************/
void CLANoiprouiDlg::OnStnClickedClose()
{
	// TODO: Add your control notification handler code here
	m_bShowWindow=FALSE;
	ShowWindow(SW_HIDE); 

}
/*****************************
tray icon notification
*****************************/
LRESULT CLANoiprouiDlg::OnTrayNotification(WPARAM wParam, LPARAM lParam)
{
  //Delegate all the work back to the default implementation in
  //CTrayNotifyIcon.
  static    BOOL bAllowOnlyOneCommand=TRUE;
  if(bAllowOnlyOneCommand )
  {
	  bAllowOnlyOneCommand=FALSE;
	
	  m_TrayIcon.OnTrayNotification(wParam, lParam);
	  bAllowOnlyOneCommand=TRUE;
  }
  return 0L;
}
/***********************
Launch contact support url from webp[age
**************************/
void CLANoiprouiDlg::OnHelpContactsupport()
{
	// TODO: Add your command handler code here
	CContactSupportDlg	SupportDlg;

	SupportDlg.DoModal(); 
	
}

/**************************************
gets already running window handle
***************************************/
HWND CLANoiprouiDlg::GetPrevWindow()
{

	HWND  hMainWnd = ::FindWindow(_T("#32770"),WINDOW_CAPTION);
	return hMainWnd;
}
/*******************************
Parse the commandline & execute it
return TRRUE if windows needs to be shown
*******************************/
void CLANoiprouiDlg::ExecuteCmdLine(BOOL bPostExistingInstance)
{
	CCommandLineInfo		cmd;

	
	m_bShowWindow=TRUE;

	
			
	//check for minimize option
	if(CString(AfxGetApp()->m_lpCmdLine)==STARTUP_COMMAND)
	{
		m_bShowWindow=FALSE;
		return ;
	}

	AfxGetApp()->ParseCommandLine( cmd );
	if(bPostExistingInstance)
	{
		WPARAM wpCmdLine = NULL;
		if(cmd.m_strFileName.IsEmpty())
		{
			//command line
			::PostMessage(GetPrevWindow(), DDE_MESSAGE_FILE_OPEN,0,0);
		}
		wpCmdLine = (ATOM)::GlobalAddAtom( cmd.m_strFileName );
		//find window 
		::PostMessage(GetPrevWindow(), DDE_MESSAGE_FILE_OPEN,wpCmdLine,0);
		return;
	}
	else
	{
		if(!cmd.m_strFileName.IsEmpty())
		{
			OpenDocument( cmd.m_strFileName );
			return;
		}
	}

	//without any commnad line just show the existing window

}
/*******************************
message posted from another instance for commanline parse
********************************/
LRESULT	CLANoiprouiDlg::OnAnotherInstanceFileOpen(WPARAM wParam, LPARAM lParam)
{
	TCHAR		szCmdLine[MAX_PATH]={0};

	//get the path from atom
	if( wParam != NULL ) 
	{
		::GlobalGetAtomName( (ATOM)wParam, szCmdLine, MAX_PATH );			
		::GlobalDeleteAtom(  (ATOM)wParam );		
		OpenDocument(szCmdLine);
		return 0;
	}

	//just posted then show window
	OnShowlanoipro();
	return 0;
}
/************
show about dialog with version
**************/
void CLANoiprouiDlg::OnHelpAbout()
{
	// TODO: Add your command handler code here
	CAboutDlg Dlg;

	Dlg.DoModal(); 
}
/**********************
upate p2p status from service
**********************/
void CLANoiprouiDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	//update tray icon depending on the status of p2p
	m_ServiceIPC.GetP2PStatus();

	__super::OnTimer(nIDEvent);
}
/***********************
refresh the peer list
************************/

void CLANoiprouiDlg::OnRefresh()
{
	// TODO: Add your command handler code here
	m_TreeCtrlOnlinePeers.UpdatePeerList();
}
