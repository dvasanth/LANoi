

#include "stdafx.h"
#include "instlsp.h"
#include "sporder.h"
#include "resource.h"
#include <shlobj.h >
#include <shlwapi.h>
#include <strsafe.h>


// {195C5AAC-289F-4ca0-8481-4F8447E787E7}
 GUID LayeredProviderGuid = 
{ 0x195c5aac, 0x289f, 0x4ca0, { 0x84, 0x81, 0x4f, 0x84, 0x47, 0xe7, 0x87, 0xe7 } };

/////////////////////////////////////////////////////////////////////////////////////

void UninstallMyProvider()
{
    INT Errno;

    WSCDeinstallProvider(
        &LayeredProviderGuid,
        &Errno);
	
}

INT
InstallMyProvider(
    PDWORD CatalogId
    )
/*++
--*/
{
    WSAPROTOCOL_INFOW  proto_info;
    int               install_result;
    int               install_error;
	TCHAR			  szLSPPath[MAX_PATH]={0};

    // Create a PROTOCOL_INFO to install for our provider DLL.
    proto_info.dwServiceFlags1 = 0x0;
    proto_info.dwServiceFlags2 = 0;
    proto_info.dwServiceFlags3 = 0;
    proto_info.dwServiceFlags4 = 0;
    proto_info.dwProviderFlags = PFL_HIDDEN;
    proto_info.ProviderId      = LayeredProviderGuid;
    proto_info.dwCatalogEntryId = 0;   // filled in by system
    proto_info.ProtocolChain.ChainLen = LAYERED_PROTOCOL;
        // Do  not  need  to  fill  in  chain  for LAYERED_PROTOCOL or
        // BASE_PROTOCOL
    proto_info.iVersion = 0;
    proto_info.iAddressFamily = AF_INET;
    proto_info.iMaxSockAddr = 16;
    proto_info.iMinSockAddr = 16;
    proto_info.iSocketType = SOCK_STREAM;
    proto_info.iProtocol = IPPROTO_TCP;   // mimic TCP/IP
    proto_info.iProtocolMaxOffset = 0;
    proto_info.iNetworkByteOrder = BIGENDIAN;
    proto_info.iSecurityScheme = SECURITY_PROTOCOL_NONE;
    proto_info.dwMessageSize = 0;  // stream-oriented
    proto_info.dwProviderReserved = 0; 

	LoadString (theInstance.m_DllInstance,IDS_LSP_NAME_INFO,proto_info.szProtocol,MAX_PATH);
	GetModuleFileName(theInstance.m_DllInstance,szLSPPath,MAX_PATH);

    install_result = WSCInstallProvider(
									&LayeredProviderGuid,
									szLSPPath,                   // lpszProviderDllPath
									& proto_info,                 // lpProtocolInfoList
									1,                            // dwNumberOfEntries
									& install_error);             // lpErrno

    *CatalogId = proto_info.dwCatalogEntryId;

	return(install_result);

} // Install_My_Layer

INT
InstallNewChain(
    LPWSAPROTOCOL_INFOW BaseProtocolInfoBuff,
    DWORD               LayeredProviderCatalogId,
    HKEY                ConfigRegisteryKey
    )
{
    WSAPROTOCOL_INFOW ProtocolChainProtoInfo;
	WCHAR             DebugPrefix[] = L"LAN On Internet Client LAYERED ";

    INT               ReturnCode;
    INT               Errno;
    UUID              NewChainId;
    RPC_STATUS        Status;
    PUCHAR            GuidString;
    HKEY              NewKey;
    DWORD             KeyDisposition;
    BOOL              Continue;
	TCHAR			  szLSPPath[MAX_PATH]={0};

    ReturnCode = NO_ERROR;

    // We are only layering on top of base providers
	GetModuleFileName(theInstance.m_DllInstance,szLSPPath,MAX_PATH);



    if (BaseProtocolInfoBuff->ProtocolChain.ChainLen == BASE_PROTOCOL && 
		IsProtocolOfInterest(BaseProtocolInfoBuff->szProtocol)
		)
	{
        Continue = FALSE;


        // Get a new GUID for the protocol chain we are about to install
        Status = UuidCreate(
            &NewChainId);
        if (RPC_S_OK == Status){

            //Get the string representaion of the GUID
            Status = UuidToStringA(
                &NewChainId,
                &GuidString);
            if (RPC_S_OK == Status){
                // Write down the GUID  in the registry so we know who to
                // uninstall
                RegCreateKeyExA(
                    ConfigRegisteryKey,                 // hkey
                    (LPCSTR)GuidString,                 // lpszSubKey
                    0,                                  // dwReserved
                    NULL,                               // lpszClass
                    REG_OPTION_NON_VOLATILE,            // fdwOptions
                    KEY_ALL_ACCESS,                     // samDesired
                    NULL,                               // lpSecurityAttributes
                    & NewKey,                           // phkResult
                    & KeyDisposition                    // lpdwDisposition
                    );
                RpcStringFreeA(&GuidString);

                Continue =TRUE;
            } //if
        } //if
       
        if (Continue){

            ProtocolChainProtoInfo = *BaseProtocolInfoBuff;
     //       ProtocolChainProtoInfo.dwServiceFlags1 &= ~XP1_IFS_HANDLES;

            ProtocolChainProtoInfo.ProviderId = NewChainId;

            StringCbCopy(
                ProtocolChainProtoInfo.szProtocol,sizeof(ProtocolChainProtoInfo.szProtocol),
                DebugPrefix);
            StringCbCat(
                ProtocolChainProtoInfo.szProtocol,sizeof(ProtocolChainProtoInfo.szProtocol),
                BaseProtocolInfoBuff->szProtocol);

            ProtocolChainProtoInfo.ProtocolChain.ChainLen = 2;
            ProtocolChainProtoInfo.ProtocolChain.ChainEntries[0] =
                LayeredProviderCatalogId;
            ProtocolChainProtoInfo.ProtocolChain.ChainEntries[1] =
                BaseProtocolInfoBuff->dwCatalogEntryId;
			 ReturnCode = WSCInstallProvider(
											&NewChainId,
											szLSPPath,
											&ProtocolChainProtoInfo,
											1,
											&Errno);
            if (ReturnCode==0)
				dbgprint (L"crclsp:Installed over %ls.\n", BaseProtocolInfoBuff->szProtocol);
            else
                dbgprint (L"crclsp:Installation over %ls failed with error %ld.\n",
                     BaseProtocolInfoBuff->szProtocol, Errno);
        } //if
    } //if
    return(ReturnCode);
}

BOOL IsProtocolOfInterest(LPWSTR lpszProtocolName)
{
	if(
		wcscmp(lpszProtocolName,L"MSAFD Tcpip [TCP/IP]")==0 ||
	    wcscmp(lpszProtocolName,L"MSAFD Tcpip [UDP/IP]")==0 ||
		wcscmp(lpszProtocolName,L"MSAFD Tcpip [TCP/IPv6]")==0 ||
		wcscmp(lpszProtocolName,L"MSAFD Tcpip [UDP/IPv6]")==0
		)
		return TRUE;

	return FALSE;

}
HRESULT DllRegisterServer()
{
	TCHAR					szRegConfigPath[MAX_PATH]={0};
    LPWSAPROTOCOL_INFOW		ProtocolInfoBuff = NULL;
    DWORD					ProtocolInfoBuffSize = 0;
    INT						ErrorCode;
    INT						EnumResult;
    LONG					lresult;
    HKEY					NewKey;
    DWORD					KeyDisposition;
    INT						Index;
    DWORD					CatalogEntryId;
    BOOL					EntryIdFound;
	DWORD					*CatIdBuff;
	DWORD					nCatIds;


	LoadString(theInstance.m_DllInstance,IDS_REG_CONFIG_PATH,szRegConfigPath,MAX_PATH );

	 RegCreateKeyEx(
            HKEY_LOCAL_MACHINE,                 // hkey
            szRegConfigPath,                  // lpszSubKey
            0,                                  // dwReserved
            NULL,                               // lpszClass
            REG_OPTION_NON_VOLATILE,            // fdwOptions
            KEY_ALL_ACCESS,                     // samDesired
            NULL,                               // lpSecurityAttributes
            & NewKey,                           // phkResult
            & KeyDisposition                    // lpdwDisposition
            );
        // Install a dummy PROTOCOL_INFO for the layered provider.
        lresult = InstallMyProvider(
            &CatalogEntryId);
        if (NO_ERROR == lresult){
            //
            // Enumerate the installed providers and chains
            //
            // Call WSCEnumProtocols with a zero length buffer so we know what
            // size to  send in to get all the installed PROTOCOL_INFO
            // structs.
            WSCEnumProtocols(
                NULL,                     // lpiProtocols
                ProtocolInfoBuff,         // lpProtocolBuffer
                & ProtocolInfoBuffSize,   // lpdwBufferLength
                & ErrorCode);             // lpErrno

            ProtocolInfoBuff = (LPWSAPROTOCOL_INFOW)
                malloc(ProtocolInfoBuffSize);
            if (ProtocolInfoBuff){

                EnumResult = WSCEnumProtocols(
                    NULL,                     // lpiProtocols
                    ProtocolInfoBuff,         // lpProtocolBuffer
                    & ProtocolInfoBuffSize,   // lpdwBufferLength
                    & ErrorCode);

                if (SOCKET_ERROR != EnumResult){

                    // Find our provider entry to get our catalog entry ID
                    EntryIdFound = FALSE;
                    for (Index =0; Index < EnumResult; Index++){
                        if (memcmp (&ProtocolInfoBuff[Index].ProviderId,
                                &LayeredProviderGuid,
                                sizeof (LayeredProviderGuid))==0){

                            CatalogEntryId =
                                ProtocolInfoBuff[Index].dwCatalogEntryId;
                            EntryIdFound = TRUE;
                        } //if
                    } //for
                    if (EntryIdFound){
                        for (Index =0; Index < EnumResult; Index++){
                            InstallNewChain(
                                &ProtocolInfoBuff[Index],
                                CatalogEntryId,
                                NewKey);
                        } //for
						free (ProtocolInfoBuff);

						//
						// Enumerate the installed providers and chains
						//
						// Call WSCEnumProtocols with a zero length buffer so we know what
						// size to  send in to get all the installed PROTOCOL_INFO
						// structs.
						ProtocolInfoBuffSize = 0;
						WSCEnumProtocols(
							NULL,                     // lpiProtocols
							NULL,					  // lpProtocolBuffer
							& ProtocolInfoBuffSize,   // lpdwBufferLength
							& ErrorCode);             // lpErrno

						ProtocolInfoBuff = (LPWSAPROTOCOL_INFOW)
							malloc(ProtocolInfoBuffSize);
						if (ProtocolInfoBuff){
			
							EnumResult = WSCEnumProtocols(
								NULL,                     // lpiProtocols
								ProtocolInfoBuff,         // lpProtocolBuffer
								& ProtocolInfoBuffSize,   // lpdwBufferLength
								& ErrorCode);

							if (SOCKET_ERROR != EnumResult){
								// Allocate buffer to hold catalog ID array
								CatIdBuff = (DWORD *)
									malloc (sizeof (DWORD)*EnumResult);
								if (CatIdBuff!=NULL) {
									// Put our catalog chains first
									nCatIds = 0;
									for (Index =0; Index < EnumResult; Index++){
										if ((ProtocolInfoBuff[Index].ProtocolChain.ChainLen>1)
												&& (ProtocolInfoBuff[Index].ProtocolChain.ChainEntries[0]==CatalogEntryId))
											CatIdBuff[nCatIds++] = ProtocolInfoBuff[Index].dwCatalogEntryId;
									}

									// Put the rest next
									for (Index =0; Index < EnumResult; Index++){
										if ((ProtocolInfoBuff[Index].ProtocolChain.ChainLen<=1)
												|| (ProtocolInfoBuff[Index].ProtocolChain.ChainEntries[0]!=CatalogEntryId))
											CatIdBuff[nCatIds++] = ProtocolInfoBuff[Index].dwCatalogEntryId;
									}
									ErrorCode = WSCWriteProviderOrder (CatIdBuff, nCatIds);
									if (ErrorCode!=NO_ERROR)
										return E_UNEXPECTED;
								}
							}
						}
                    } //if
                } //if
            } //if
        } //if

		return S_OK;
}



HRESULT DllUnregisterServer()
{
	TCHAR					szRegConfigPath[MAX_PATH]={0};
    LPWSAPROTOCOL_INFOW		ProtocolInfoBuff = NULL;
    DWORD					ProtocolInfoBuffSize = 0;
    INT						ErrorCode;
    LONG					lresult;
    HKEY					NewKey;
    GUID					ProviderID;
    INT						Index;
    TCHAR					GuidStringBuffer[40];
    DWORD					GuidStringBufferLen;
    FILETIME				FileTime;




	LoadString(theInstance.m_DllInstance,IDS_REG_CONFIG_PATH,szRegConfigPath,MAX_PATH );

    // See if we are installing or deinstalling
    lresult = RegOpenKeyEx(
							HKEY_LOCAL_MACHINE,                     // hkey
							szRegConfigPath,                      // lpszSubKey
							0,                                      // dwReserved
							KEY_ALL_ACCESS,                        // samDesired
							& NewKey                               // phkResult
							);

    if (ERROR_SUCCESS == lresult)
	{
        // The layered provider is installed so we are going uninstall.

        //
        // Enumerate all the provider IDs we stored on install and deinstall
        // the providers
        //
        Index = 0;
        GuidStringBufferLen = sizeof(GuidStringBuffer);
        lresult = RegEnumKeyEx(
            NewKey,               //hKey
            Index,                // Index of subkey
            &GuidStringBuffer[0],    // Buffer to hold key name
            &GuidStringBufferLen,  // Length of buffer
            NULL,                 // Reserved
            NULL,                 // Class buffer
            NULL,                 // Class buffer length
            &FileTime              // Last write time
            );

        while (lresult != ERROR_NO_MORE_ITEMS)
		{
            UuidFromString(
                (RPC_WSTR) GuidStringBuffer,
                &ProviderID);
            // Deinstall the provider chain we installed
            WSCDeinstallProvider(
                &ProviderID,
                &ErrorCode);
            // Delete our registry key
            if(RegDeleteKey(
                NewKey,
                &GuidStringBuffer[0])!=ERROR_SUCCESS)
			{
				return E_ACCESSDENIED;
			}

            GuidStringBufferLen = sizeof(GuidStringBuffer);
			lresult = RegEnumKeyEx(
									NewKey,               //hKey
									Index,                // Index of subkey
									&GuidStringBuffer[0],    // Buffer to hold key name
									&GuidStringBufferLen,  // Length of buffer
									NULL,                 // Reserved
									NULL,                 // Class buffer
									NULL,                 // Class buffer length
									&FileTime              // Last write time
									);

        } //while

        // Clen up the registry
        RegCloseKey(
            NewKey);
        RegDeleteKey(
            HKEY_LOCAL_MACHINE,
            szRegConfigPath);

        // Uninstall the real provider
        UninstallMyProvider();
    } //if

	return S_OK;
}

