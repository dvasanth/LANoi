#pragma once
#include <strsafe.h>
#include "loisession\IPCFileMap.h"
class CProcessLock{
public:
	CProcessLock(LPTSTR lpszName)
	{
		CSecurityObject Sa;
		TCHAR	szName[MAX_PATH]={0};
		StringCbPrintf(szName,sizeof(szName),_T("%s.{12EBD5A2-5105-4f48-B5B8-B133B3E348E2}"),lpszName);
		m_hMutex=CreateMutex( Sa.GetAllAccessSA(),FALSE,szName);
		CSecurityObject::SetObjectToLowIntegrity(m_hMutex);
	}
	~CProcessLock()
	{
		if(m_hMutex!=NULL)
		{
			CloseHandle(m_hMutex);
			m_hMutex=NULL;
		}
	}
	void Lock()
	{
		WaitForSingleObject(m_hMutex,INFINITE); 
	}

	void UnLock()
	{
		ReleaseMutex(m_hMutex);  
	}
private:
	HANDLE	m_hMutex;
};