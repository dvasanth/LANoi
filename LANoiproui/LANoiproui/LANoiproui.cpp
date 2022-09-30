// LANoiproui.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "LANoiproui.h"
#include "LANoiprouiDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CLANoiprouiApp

BEGIN_MESSAGE_MAP(CLANoiprouiApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

const TCHAR	CLANoiprouiApp::APP_INSTANCE_NAME[]=_T("{7AA95431-3E28-40fd-B8E2-EC4DDA64F1AA}.LANoipro.instance");
// CLANoiprouiApp construction

CLANoiprouiApp::CLANoiprouiApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CLANoiprouiApp object

CLANoiprouiApp theApp;


// CLANoiprouiApp initialization

BOOL CLANoiprouiApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX	InitCtrls;
	HANDLE					hAppMutex;
	CLANoiprouiDlg			dlg;

	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	//init the logging routines
	g_Logging.StartLogging();

	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));


	//create the app mutex
	hAppMutex=::CreateMutex( NULL, FALSE, APP_INSTANCE_NAME );
	if(::GetLastError()==ERROR_ALREADY_EXISTS ||
		::GetLastError() == ERROR_ACCESS_DENIED 
		)
	{
		//already application exists
		//pass the commandline if exists
		dlg.ExecuteCmdLine(TRUE);
		CloseHandle(hAppMutex);
		return FALSE;
	}
	
	
	m_pMainWnd = &dlg;
	//execute if any command line
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	CloseHandle(hAppMutex);
	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
