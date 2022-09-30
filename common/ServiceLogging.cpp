#include "StdAfx.h"
#include "ServiceLogging.h"


CServiceLogging     g_Logging;
DWORD IFileLogging::m_dwTraceLevel=0;

CServiceLogging::CServiceLogging(void)
{
}

CServiceLogging::~CServiceLogging(void)
{
}
/*******************************
initalise the logging paths
********************************/
void CServiceLogging::StartLogging()
{
	CString				sTraceLevelKey;
	CString				sTraceFileName;

	//initialize the logging modules
	sTraceLevelKey.LoadString( IDS_LOG_TRACE_REG_KEY);
	sTraceFileName.LoadString( IDS_TRACE_FILENAME);
	Init(sTraceLevelKey,sTraceFileName);
}


