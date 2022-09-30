#pragma once

enum COMMAND_TYPE {INSTALL,UNINSTALL,RUNASEXE,EXECUTE};

typedef struct __COMMANDLINE_INFO{
	TCHAR            szCommandLine[MAX_PATH];
	COMMAND_TYPE	 CommandType;
}COMMANDLINE_INFO,*LPCOMMANDLINE_INFO;


class CLOIApp
{
public:
	CLOIApp(void);
	~CLOIApp(void);
	BOOL				InitInstance(HINSTANCE hInstance, LPTSTR  lpCmdLine);
	void				ExitInstance();
	inline HINSTANCE	GetHInstance()
	{
		return m_hInstance;
	}
	CString				GetApplicationDataPath(void);
private:
	static const COMMANDLINE_INFO       CmdInfo[];
	HINSTANCE							m_hInstance;
	CString								m_sApplicationDataFolder;
/// member functions
	COMMAND_TYPE			ParseCommandLine(LPTSTR  lpCmdLine);
	BOOL					ExecuteCmdLine(COMMAND_TYPE CmdType);


};
extern CLOIApp     theApp;