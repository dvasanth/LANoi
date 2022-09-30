#ifndef _INSTALLLSP_H_
#define _INSTALLLSP_H_



void UninstallMyProvider();



INT
InstallMyProvider(
    PDWORD CatalogId
    );


INT
InstallNewChain(
    LPWSAPROTOCOL_INFOW BaseProtocolInfoBuff,
    DWORD               LayeredProviderCatalogId,
    HKEY                ConfigRegisteryKey
    );


HRESULT DllRegisterServer();

HRESULT DllUnregisterServer();
BOOL IsProtocolOfInterest(LPWSTR lpszProtocolName);

extern GUID LayeredProviderGuid;



#endif