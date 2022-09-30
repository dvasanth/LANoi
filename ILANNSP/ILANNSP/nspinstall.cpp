#include "stdafx.h"
#include <winsock2.h>
#include <ws2spi.h>
#include "globals.h"
#include <stdlib.h>
#include <stdio.h>
#include "resource.h"

//  {7E8D5486-ABAE-4aed-A195-CFABD10F4ECC}
 static const  GUID NamespaceProviderGuid = 
{ 0x7e8d5486, 0xabae, 0x4aed, { 0xa1, 0x95, 0xcf, 0xab, 0xd1, 0xf, 0x4e, 0xcc } };

#define ILAN_NSP  25
HRESULT DllUnregisterServer();


HRESULT DllRegisterServer()
{
	int            iRet;
	TCHAR		   szNSPPath[MAX_PATH]={0}; 
	TCHAR		   szNameSpace[MAX_PATH]={0};

	GetModuleFileName(gDllInstance,szNSPPath,MAX_PATH);
	LoadString (gDllInstance,IDS_NAMESPACE_CAPTION,szNameSpace,sizeof(szNameSpace)/sizeof(TCHAR));
	
	iRet = WSCInstallNameSpace(szNameSpace,
                szNSPPath, 0x0000000C, 1,(LPGUID) &NamespaceProviderGuid);
	if(NO_ERROR != iRet)
	{
		return E_UNEXPECTED;
	}

	iRet = WSCEnableNSProvider((LPGUID) &NamespaceProviderGuid,true);
	if(NO_ERROR != iRet)
	{
		DllUnregisterServer();
		return E_UNEXPECTED;
	}

    return S_OK;
}


HRESULT DllUnregisterServer()
{
	int            iRet;

	WSCEnableNSProvider((LPGUID) &NamespaceProviderGuid,false);
	iRet = WSCUnInstallNameSpace((LPGUID)&NamespaceProviderGuid);
	return (iRet == SOCKET_ERROR)?E_UNEXPECTED:S_OK;
}