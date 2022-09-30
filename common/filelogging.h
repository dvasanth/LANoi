#pragma once
#include <atlbase.h>
#include <atlstr.h>
#include < Shlobj.h>
#include <strsafe.h>

#define TRACE_LEVEL_REG_VALUE_NAME  _T("LogTraceLevel")
#define	LOG_FILE_DIRECTORY			_T("crowsoft\\log")
#define MAX_LOG_FILE_SIZE			1*1024*1024
	enum LogLevel{
		LOI_ERROR=0x1, 
		LOI_WARNING=0x2,
		LOI_INFO=0x4
	};
class IFileLogging
{
private:
	DWORD						m_dwReportingLevel;	
	HANDLE						m_hLogFile;
public:

	//members
	static DWORD				m_dwTraceLevel;

public:

	IFileLogging()
	{
		m_dwReportingLevel=0;
		m_hLogFile = INVALID_HANDLE_VALUE;

	}
	~IFileLogging(void)
	{
		CloseFile();
	}
	void Init(LPCTSTR lpszTraceLevelKey,LPCTSTR lpszFileName)
	{
		//store the reporting level
		CRegKey         TraceKey;
		TraceKey.Create(HKEY_LOCAL_MACHINE ,lpszTraceLevelKey);
		TraceKey.QueryDWORDValue(TRACE_LEVEL_REG_VALUE_NAME,m_dwTraceLevel);

		InitFile(lpszFileName);
	}

	void cdecl			WriteLog(TCHAR* szFormat, ...)
	{
		if(!m_dwTraceLevel)
		{
			return ;
		}
		try{
			TCHAR		 Buffer1[1024];
			TCHAR		 Buffer2[1024];


			StringCbVPrintf( Buffer1, sizeof(Buffer1),szFormat, (va_list)(&szFormat+1));
			StringCbPrintf(Buffer2,sizeof(Buffer2 ), _T("%lu : %s\r\n"), GetCurrentThreadId(),  Buffer1);

			#if _DEBUG
				OutputDebugString(Buffer2);
			#else
				if(m_hLogFile != INVALID_HANDLE_VALUE)
				{
					DWORD dwWritten=0;
					WriteFile(m_hLogFile,Buffer2,(_tcslen(Buffer2))*sizeof(TCHAR),&dwWritten,NULL);  
				}
			#endif
		}
		catch(...)
		{
		}

	}
	static void GetLogFileDirectory(LPTSTR lpszDirPath,DWORD dwSizeInBytes)
	{
		TCHAR			szAppDirectory[MAX_PATH]={0};

		//expand the appdata directory
		SHGetFolderPath(NULL,CSIDL_COMMON_APPDATA ,NULL,SHGFP_TYPE_CURRENT,szAppDirectory);
		StringCbPrintf(lpszDirPath,dwSizeInBytes,
										_T("%s\\%s\\"),
										szAppDirectory,
										LOG_FILE_DIRECTORY);
	}
private:
	//consts


	void	InitFile(LPCTSTR lpszFileName)
	{
		TCHAR			szLogFilePath[MAX_PATH]={0};

		GetLogFileDirectory(szLogFilePath,sizeof(szLogFilePath));
		SHCreateDirectory(NULL,szLogFilePath);
		StringCbCat(szLogFilePath,sizeof(szLogFilePath),lpszFileName);
		//open the file
		m_hLogFile=CreateFile(szLogFilePath,GENERIC_WRITE,FILE_SHARE_READ,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
		if(m_hLogFile == INVALID_HANDLE_VALUE)
		{
			//failed to open the file
			m_dwTraceLevel=0;
			return;
		}
		BYTE	byUnicodeHeader[]={0xff,0xfe};
		DWORD	dwWrite;

		WriteFile(m_hLogFile,byUnicodeHeader,sizeof(byUnicodeHeader),&dwWrite,NULL);
		//truncate the file if size large
		if( GetFileSize( m_hLogFile,NULL) > MAX_LOG_FILE_SIZE)
		{
			SetEndOfFile( m_hLogFile);
		}
		SetFilePointer(m_hLogFile,0,0,FILE_END);
	}
	void	CloseFile()
	{
		if(m_hLogFile != INVALID_HANDLE_VALUE)
		{
			CloseHandle(m_hLogFile);
			m_hLogFile=INVALID_HANDLE_VALUE;
		}
	}

	
};
