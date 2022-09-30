#pragma once

#include "filelogging.h"
class CServiceLogging:public IFileLogging
{
public:
	CServiceLogging(void);
	~CServiceLogging(void);
	void			StartLogging();
};

extern CServiceLogging     g_Logging;

#define DEBUGLOG(LogLevel, ...) \
		if (!g_Logging.m_dwTraceLevel); \
		else if(LogLevel &  g_Logging.m_dwTraceLevel) \
						g_Logging.WriteLog(__VA_ARGS__); 
