#include "StdAfx.h"
#include "LOIApp.h"
#include "ServiceLogging.h"
#include "LOIService.h"

const COMMANDLINE_INFO CLOIApp::CmdInfo[]={
									{_T("-install"),INSTALL},
									{_T("-uninstall"),UNINSTALL},
									{_T("-runasexe"),RUNASEXE},
									{_T(""),EXECUTE}
								};

CLOIApp::CLOIApp(void)
{
	m_hInstance=NULL;
}

CLOIApp::~CLOIApp(void)
{
}


BOOL CLOIApp::InitInstance(HINSTANCE hInstance, LPTSTR  lpCmdLine)
{
	TCHAR				szTraceLevelKey[MAX_PATH]={0};
	TCHAR				szTraceFileName[MAX_PATH]={0};
	COMMAND_TYPE		Cmdtype;

	m_hInstance=hInstance;

	//initialize the logging modules
	LoadString( IDS_LOG_TRACE_REG_KEY,szTraceLevelKey,sizeof(szTraceLevelKey)/sizeof(TCHAR));
	LoadString( IDS_TRACE_FILENAME,szTraceFileName,sizeof(szTraceFileName)/sizeof(TCHAR));
	g_Logging.Init(szTraceLevelKey,szTraceFileName);

	TCHAR		szPath[MAX_PATH]={0};
	SHGetFolderPath(NULL,CSIDL_COMMON_APPDATA ,NULL,SHGFP_TYPE_CURRENT,szPath);
	m_sApplicationDataFolder+=szPath;
	m_sApplicationDataFolder+=	_T("\\crowsoft\\LANoiPro\\");;
	SHCreateDirectoryEx(NULL,m_sApplicationDataFolder,NULL);

	DEBUGLOG(LOI_INFO,_T("ParseCommandLine"));
	//parse the command line
	Cmdtype=ParseCommandLine(lpCmdLine);
	return ExecuteCmdLine(Cmdtype);
}

CString CLOIApp::GetApplicationDataPath(void)
{
	return m_sApplicationDataFolder;
}
//return true if command is processed 
COMMAND_TYPE CLOIApp::ParseCommandLine(LPTSTR  lpCmdLine)
{
	if(!lpCmdLine  || *lpCmdLine == 0)
	{
		return EXECUTE;
	}

	for(int iCmdList=0; iCmdList<sizeof(CmdInfo)/sizeof(CmdInfo[0]);iCmdList++)
	{
		if(_tcsicmp(lpCmdLine,CmdInfo[iCmdList].szCommandLine)==0)
		{
			return CmdInfo[iCmdList].CommandType;
		}
	}

	return EXECUTE;
}

BOOL CLOIApp::ExecuteCmdLine(COMMAND_TYPE CmdType)
{
	CLOIService			*pService=CLOIService::GetInstance();
	

	switch(CmdType)
	{
		case INSTALL:
			{
				return pService->InstallService();
			}
			break;
		case UNINSTALL:
			{
				return pService->UnInstallService();
			}
			break;
		case EXECUTE:
			{
				return pService->Execute();
			}
			break;
		case RUNASEXE:
			{
				pService->RunAsExe();//only for testing
			}
	}

	return FALSE;
}

void CLOIApp::ExitInstance()
{
}