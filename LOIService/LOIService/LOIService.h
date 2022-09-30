#pragma once
#include "winservice.h"
#include "thread.h"
class CLOIService :	public IWinService<CLOIService>,
					public IThread
{
public:
	CLOIService(void);
	~CLOIService(void);
	BOOL			InstallService();
	BOOL			UnInstallService();
	BOOL			Execute();
private:
	static const TCHAR		       SERVICE_NAME[];
	static const TCHAR			   SERVICE_DISPLAY_NAME[];

	virtual BOOL				StartService();
	virtual BOOL				StopService();
	virtual  BOOL				Run();

};
