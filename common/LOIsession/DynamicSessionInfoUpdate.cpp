#include "StdAfx.h"
#include "DynamicSessionInfoUpdate.h"
#include <strsafe.h>
CDynamicSessionInfoUpdate::CDynamicSessionInfoUpdate(void)
{
}

CDynamicSessionInfoUpdate::~CDynamicSessionInfoUpdate(void)
{
}
void CDynamicSessionInfoUpdate::Initialise(CIPCFileMap& FileMap)
{
	FileMap.SingleUseLock();
	//clear any exiting opened items
	FileMap.ClearMap(); 
	FileMap.SingleUseUnLock();
}	

void CDynamicSessionInfoUpdate::UpdateDynamicSessionInfo(CIPCFileMap& FileMap,LPCTSTR lpszSessionID,DWORD dwNetOrderIPAddress,USHORT usNetOrderPort,DWORD dwVirtualAddress)
{
	FileMap.SingleUseLock();

	FileMap.SetFilePointer(FILE_END);
	DYNAMIC_SESSION_UPDATE_INFO  Info={0};
	Info.dwBindedLocalAddress = dwNetOrderIPAddress;
	Info.dwVirtualPeerAddress = dwVirtualAddress;
	Info.usBoundPort = usNetOrderPort;
	StringCbCopy(Info.szSessionID,sizeof(Info.szSessionID),lpszSessionID);   
	FileMap.Write((LPBYTE)&Info,sizeof(Info));
	FileMap.SingleUseUnLock();
}

DYNAMIC_SESSION_UPDATE_INFO* CDynamicSessionInfoUpdate::ClearDynamicUpdatedItems(CIPCFileMap& FileMap,DWORD &dwItemCount)
{
	DYNAMIC_SESSION_UPDATE_INFO  *pItems= NULL;
 	FileMap.SingleUseLock();

	if(FileMap.GetMapSize())
	{
		FileMap.SetFilePointer(FILE_BEGIN);
		dwItemCount=FileMap.GetMapSize()/sizeof(DYNAMIC_SESSION_UPDATE_INFO);
		pItems=(DYNAMIC_SESSION_UPDATE_INFO*)new BYTE[FileMap.GetMapSize()];
		FileMap.Read((LPBYTE)pItems,FileMap.GetMapSize());
	}
	//clear the items
	FileMap.ClearMap(); 
	FileMap.SingleUseUnLock();
	return pItems;
 }

void CDynamicSessionInfoUpdate::FreeDynamicUpdatedItems(DYNAMIC_SESSION_UPDATE_INFO* pItems)
{
	if(pItems)
	{
		delete pItems;
	}
}