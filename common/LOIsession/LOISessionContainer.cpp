#include "stdafx.h"
#include "LOISessionContainer.h"


//////////////
///ISessionContainer consts
/////////////
const TCHAR	ISessionContainer::UI_TO_LSP_NOTIFY_NAME[]={_T("Global\\{65AFFDA5-0BE5-4d6a-85C9-997A691FDF1F}.LSP.Notify")};
const TCHAR	ISessionContainer::LSP_TO_UI_NOTIFY_NAME[]={_T("Global\\{65AFFDA5-0BE5-4d6a-85C9-997A691FDF1F}.UI.Notify")};

/////////////////
//const
/////////////////
const   TCHAR	ISessionContainer::ENABLE_LOI_IPC[]=_T("Global\\{65AFFDA5-0BE5-4d6a-85C9-997A691FDF1F}.EnableLOI");//Global\\//
//event name used for notification
//map max size -- limitation needs changes
UI_TO_LSP_IPC_INFO ISessionContainer::m_IpcInfo[TOTAL_ITEMS]={
	{_T("SessionList"),500*sizeof(PORT_INFORMATION)+1000*sizeof(PEER_INFORMATION) },
	{_T("DynamicUpdate"),15*sizeof(DYNAMIC_SESSION_UPDATE_INFO)}
};

CLOISessionContainer::CLOISessionContainer(void)
{
}

CLOISessionContainer::~CLOISessionContainer()
{
}

/*************************************
Initilise the IPC objects & the change notifier
*************************************/
void CLOISessionContainer::Init()
{
	//lsp writes in this map abt binded address & notifies
	RegisterLSPToUINotifier();
	//just keep the events open to be set by LSP instnaces
	RegisterUIToLSPNotifier();

	
}
/******************************
Free all resources & sessions used
******************************/
void CLOISessionContainer::Close()
{
	UnRegisterEventNotifiers();
	ClearSessionData();
}
