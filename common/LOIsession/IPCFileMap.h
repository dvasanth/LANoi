#pragma once
#include <sddl.h>
#include <AccCtrl.h>
#include <Aclapi.h>
typedef struct __IPC_MAP_HEADER {
		DWORD   dwSizeWritten;
} IPC_MAP_HEADER ,*LPIPC_MAP_HEADER;

class CIPCFileMap
{
public:
	enum Operation{
		store,
		load
	};	
	CIPCFileMap(LPCTSTR lpszMapName,Operation Op);
	~CIPCFileMap(void);
	
	BOOL			Open(DWORD dwMaxMapSize);
	void			Close();
	int				Write(LPBYTE pBuffer,int iBuffLen);
	int				Read(LPBYTE pBuffer,int iBuffLen);
	DWORD			GetMapSize();
	void			SetFilePointer(DWORD dwMoveMethod);
	BOOL			IsStoring(void);
	void			SingleUseLock();
	void			SingleUseUnLock();
	BOOL			IsDirty();
	void			ClearMap();
private:
	static const  TCHAR NAMED_OBJECT_PREFIX[];
	TCHAR				m_szMapName[MAX_PATH];
	Operation			m_Operation;
	HANDLE				m_hInfoSync;
	HANDLE				m_hMappedFile;
	LPIPC_MAP_HEADER	m_pMapHeader;
	PVOID				m_pMappedInfoData;
	DWORD				m_dwCurrentOffset;
	DWORD				m_dwMaxOffset;
	DWORD				m_dwWrittenSize;
	BOOL				m_bDirtyFlag;


};

class ISerialize
{
public:
	ISerialize(void)
	{}
	~ISerialize(void)
	{
	}
	 virtual void Serialize(CIPCFileMap& FileMap)=0;
};

class CSecurityObject{
private:
	SECURITY_ATTRIBUTES m_sa;
public:
	CSecurityObject()
	{
		ZeroMemory(&m_sa,sizeof(m_sa));
	}
	~CSecurityObject()
	{
		FreeSA(m_sa);
	}
	static BOOL SetObjectToLowIntegrity(HANDLE hObject, SE_OBJECT_TYPE type=SE_KERNEL_OBJECT)
	{
		BOOL						 bRet = FALSE;
		DWORD						 dwErr = ERROR_SUCCESS;
		PSECURITY_DESCRIPTOR		 pSD = NULL;
		PACL						 pSacl = NULL;
		BOOL						 fSaclPresent = FALSE;
		BOOL						 fSaclDefaulted = FALSE;
		// The LABEL_SECURITY_INFORMATION SDDL SACL to be set for low integrity
		LPCWSTR						 LOW_INTEGRITY_SDDL_SACL_W = L"S:(ML;;NW;;;LW)";

	 
		if ( ConvertStringSecurityDescriptorToSecurityDescriptorW (LOW_INTEGRITY_SDDL_SACL_W, SDDL_REVISION_1, &pSD, NULL ))
		{
			if (GetSecurityDescriptorSacl(pSD,&fSaclPresent,&pSacl,&fSaclDefaulted))
			{
				 dwErr = SetSecurityInfo(hObject,type,LABEL_SECURITY_INFORMATION,NULL,NULL,NULL,pSacl);
				 bRet = (ERROR_SUCCESS == dwErr);
			}
	 		LocalFree ( pSD );
		 }
		 
		return bRet;

	}
	/******************************************
	creates a security attribute with everyone access
	*******************************************/
	LPSECURITY_ATTRIBUTES GetAllAccessSA()
	{
		TCHAR * szSD = TEXT("D:")       // Discretionary ACL
		TEXT("(A;OICI;GA;;;S-1-1-0)");    // Allow full control to everyone

		m_sa.nLength = sizeof(SECURITY_ATTRIBUTES);
		m_sa.bInheritHandle = FALSE;

		ConvertStringSecurityDescriptorToSecurityDescriptor(
														szSD,
														SDDL_REVISION_1,
														&(m_sa.lpSecurityDescriptor),
														NULL);

		return &m_sa;
	}
	/*******************************************
	Close & free SA
	*********************************************/
	void FreeSA(SECURITY_ATTRIBUTES &Sa)
	{
		if(Sa.lpSecurityDescriptor)
		{
			LocalFree(Sa.lpSecurityDescriptor); 
			ZeroMemory(&Sa,sizeof(Sa));
		}
	}
};