#pragma once
#include <winsock2.h>
#include <windows.h>
#include<Ws2spi.h>

int WSPAPI NSPLookupServiceBegin(
    LPGUID lpProviderId,
    LPWSAQUERYSETW lpqsRestrictions,
    LPWSASERVICECLASSINFOW lpServiceClassInfo,
    DWORD dwControlFlags,
    LPHANDLE lphLookup);

int WSPAPI NSPLookupServiceNext(
    HANDLE hLookup,
    DWORD dwControlFlags,
    LPDWORD lpdwBufferLength,
    LPWSAQUERYSETW lpqsResults);

int WSPAPI NSPLookupServiceEnd(HANDLE hLookup);


 int WSPAPI	NSPGetServiceClassInfo(	LPGUID inProviderID, LPDWORD ioSize, LPWSASERVICECLASSINFOW ioServiceClassInfo );
 int WSPAPI	NSPRemoveServiceClass( LPGUID inProviderID, LPGUID inServiceClassID );
  int WSPAPI	NSPInstallServiceClass( LPGUID inProviderID, LPWSASERVICECLASSINFOW inServiceClassInfo );
  int WSPAPI
	NSPSetService(
		LPGUID					inProviderID,						
		LPWSASERVICECLASSINFOW	inServiceClassInfo,   
		LPWSAQUERYSETW			inRegInfo,				  
		WSAESETSERVICEOP		inOperation,			   
		DWORD					inFlags );