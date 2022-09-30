#pragma once

#include "SocketFactory.h"
#include "LSPProviderOperations.h"
class CLSPModule
{
public:
	CLSPModule();
	~CLSPModule(void);
	HINSTANCE        m_DllInstance;  

	DWORD			 GetThreadIndex();
	BOOL			 OnInitInstance(HINSTANCE hDllInstance);
	void			 OnExitInstance(LPVOID lpvReserved);
	BOOL			 IsProcessDetached();
	CLSPProviderOperations* GetProviderInstance();
	void			 LoadRegistryValues();
	CSocketFactory*  GetSockFactoryInstance();
	TCHAR		     m_szAppName[MAX_PATH];
	TCHAR			 m_szUIAppPath[MAX_PATH];
private:

	DWORD			 m_dwThreadIndex;
	BOOL			 m_bProcessDetached;
	CLSPProviderOperations m_LspProvider;
	CSocketFactory    m_SocketFactory;
};
