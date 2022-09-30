#include "StdAfx.h"
#include "IPCFileMap.h"
#include <strsafe.h>

const  TCHAR CIPCFileMap::NAMED_OBJECT_PREFIX[]=_T("Global\\{65AFFDA5-0BE5-4d6a-85C9-997A691FDF1F}");//Global\\


CIPCFileMap::CIPCFileMap(LPCTSTR lpszMapName,Operation Op)
{
	TCHAR		szMutexName[MAX_PATH];

	StringCbPrintf(m_szMapName,sizeof(m_szMapName),_T("%s.%s"),NAMED_OBJECT_PREFIX,lpszMapName);
	StringCbPrintf(szMutexName,sizeof(szMutexName),_T("%s.Lock"),m_szMapName);

	m_Operation=Op;
	m_hInfoSync=m_hMappedFile=m_pMappedInfoData=m_pMapHeader=NULL;
	m_dwCurrentOffset=m_dwMaxOffset=0;

	if(!(m_hInfoSync=OpenMutex(SYNCHRONIZE ,FALSE, szMutexName)))
	{
		CSecurityObject Sa;
		m_hInfoSync=CreateMutex( Sa.GetAllAccessSA() ,FALSE,szMutexName);
		CSecurityObject::SetObjectToLowIntegrity(m_hInfoSync);
	}

}

CIPCFileMap::~CIPCFileMap(void)
{
	Close();
	if(	m_hInfoSync)
	{
		CloseHandle(m_hInfoSync);
		m_hInfoSync=NULL;
	}

}

void CIPCFileMap::SingleUseLock()
{
	WaitForSingleObject(m_hInfoSync,INFINITE);
}

void CIPCFileMap::SingleUseUnLock()
{
	ReleaseMutex(m_hInfoSync);
}

BOOL CIPCFileMap::Open(DWORD dwMaxMapSize)
{
	BOOL			bMapInitalise=FALSE;
	CSecurityObject Sa;
	Close();

	if(m_Operation==store)
	{
		m_hMappedFile = CreateFileMapping(
					 INVALID_HANDLE_VALUE,    // use paging file
					 Sa.GetAllAccessSA() ,                    // default security 
					 PAGE_READWRITE,          // read/write access
					 0,                       // max. object size 
					 dwMaxMapSize,                // buffer size  
					 m_szMapName);                 // name of mapping object
		if(m_hMappedFile && GetLastError () != ERROR_ALREADY_EXISTS)
									bMapInitalise=TRUE;
		CSecurityObject::SetObjectToLowIntegrity(m_hMappedFile);
	}
	else
	{
	  m_hMappedFile = OpenFileMapping(
                   FILE_MAP_READ,   // read only access
                   FALSE,                 // do not inherit the name
                   m_szMapName);  
	}

	if (m_hMappedFile == NULL || m_hMappedFile == INVALID_HANDLE_VALUE) 
	{ 
		Close();
		return FALSE;
	}

	LPVOID pMappedInfoData = (LPVOID) MapViewOfFile(m_hMappedFile,   // handle to map object
                         m_Operation==store?FILE_MAP_ALL_ACCESS:FILE_MAP_READ, // read/write permission
                        0,                   
                        0,                   
                        dwMaxMapSize);           
 
	if (pMappedInfoData == NULL) 
	{ 
		Close();
        return FALSE;
	}


	MEMORY_BASIC_INFORMATION mInfo={0};
	VirtualQuery(pMappedInfoData,&mInfo,sizeof(mInfo));
	m_dwMaxOffset=mInfo.RegionSize-sizeof(IPC_MAP_HEADER);
	m_dwCurrentOffset=0;
	
	m_pMapHeader=(LPIPC_MAP_HEADER)pMappedInfoData;
	m_pMappedInfoData = (LPBYTE)pMappedInfoData+sizeof(IPC_MAP_HEADER);


	if(bMapInitalise)
	{
		//first time open
		m_pMapHeader->dwSizeWritten =0;	
		//reset values
		memset(m_pMappedInfoData,0,m_dwMaxOffset);
	}

	return TRUE;
}





BOOL CIPCFileMap::IsDirty()
{
	return m_bDirtyFlag;
}

void CIPCFileMap::Close()
{
	if(m_pMapHeader)
	{
		UnmapViewOfFile(m_pMapHeader);
		m_pMappedInfoData=NULL;
	}
	if(m_hMappedFile)
	{
		CloseHandle(m_hMappedFile);
		m_hMappedFile=NULL;
	}

}



int CIPCFileMap::Write(LPBYTE pBuffer,int iBuffLen)
{

	if(m_dwCurrentOffset+iBuffLen > m_dwMaxOffset ) return 0;

	CopyMemory(	((BYTE*)m_pMappedInfoData)+m_dwCurrentOffset,pBuffer,iBuffLen);
	m_dwCurrentOffset+=iBuffLen;
	m_pMapHeader->dwSizeWritten=m_dwCurrentOffset;//update total size written
	return iBuffLen;
}

int CIPCFileMap::Read(LPBYTE pBuffer,int iBuffLen)
{
	if(m_dwCurrentOffset+iBuffLen > m_dwMaxOffset  ) return 0;

	CopyMemory(	pBuffer,((BYTE*)m_pMappedInfoData)+m_dwCurrentOffset ,iBuffLen);
	m_dwCurrentOffset+=iBuffLen;
	if(m_dwCurrentOffset>m_pMapHeader->dwSizeWritten)return 0;//we are reading more than written
	return iBuffLen;
}
void CIPCFileMap::SetFilePointer(DWORD dwMoveMethod)
{
	if(dwMoveMethod==FILE_BEGIN)
			m_dwCurrentOffset=0;
	if(dwMoveMethod==FILE_END)
			m_dwCurrentOffset=m_pMapHeader->dwSizeWritten;//seek upto to the written size
}
DWORD CIPCFileMap::GetMapSize()
{
	return m_pMapHeader->dwSizeWritten;
}
void CIPCFileMap::ClearMap()
{
	m_pMapHeader->dwSizeWritten=0;
	m_dwCurrentOffset=0;
	memset(m_pMappedInfoData,0,m_dwMaxOffset);
}
BOOL CIPCFileMap::IsStoring(void)
{
	return m_Operation==store;
}

