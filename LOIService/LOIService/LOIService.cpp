#include "StdAfx.h"
#include "LOIService.h"
#include "ipc/IPCCommandHandler.h"
#include "teredo/TeredoHlpr.h"
#include "p2p/P2PManager.h"


const TCHAR CLOIService::SERVICE_NAME[]=_T("LOIService");
const TCHAR CLOIService::SERVICE_DISPLAY_NAME[]=_T("LAN On Internet");
template <typename T>
 T* IWinService<T>::s_pInstance=NULL;

CLOIService::CLOIService(void)
{
}

CLOIService::~CLOIService(void)
{
}

BOOL CLOIService::InstallService()
{
	return IWinService::InstallService(SERVICE_NAME,SERVICE_DISPLAY_NAME);
}

BOOL CLOIService::UnInstallService()
{
	return IWinService::UnInstallService(SERVICE_NAME);
}
/***********************************************
Do initialisation & starting thread for the service
***********************************************/
BOOL CLOIService::StartService()
{
	return CreateThread();
}
/****************************
Execute the service
*****************************/
BOOL CLOIService::Execute()
{
	return IWinService::Execute(SERVICE_NAME);
}
/********************************
Invoked when service manager wish to stop the service
*******************************/
BOOL CLOIService::StopService()
{
  DEBUGLOG(LOI_ERROR,_T("Received Service Stop request"));
  WaitForThreadExit();
  return TRUE;
}
/******************************************
Actual processing of the service
******************************************/
BOOL CLOIService::Run() 
{
	HANDLE						hEvents[2];
	CIPCCommandHandler			UIRPC;
	DWORD_PTR					dwRetVal;
	CTeredoHlpr					Teredo;
	CP2PManager					*pP2PManager=NULL;
	const	DWORD				TEREDO_ACTIVE_MONITOR_TIMEOUT=30*1000;//so sec
	const	DWORD				TEREDO_PASSIVE_MONITOR_TIMEOUT=5*60*1000;//5 min
	const	DWORD				TEREDO_SYMMETRIC_NAT_MONITOR_TIMEOUT=30*60*1000;//30 min

	DWORD						dwEventCount=(sizeof(hEvents)/sizeof(hEvents[0]));
	DWORD						dwTeredoMonitorTimeout=TEREDO_ACTIVE_MONITOR_TIMEOUT;

	hEvents[0]=WSACreateEvent();//notify abt IPC command from UI 
	UIRPC.NotifyUIRequest(hEvents[0]);

	hEvents[1]=WSACreateEvent();//notify abt teredo adapter notify	
//	SetEvent(hEvents[1]);//intiate the process
	

	do{
		//if count is greter than max then time slice and wait
		dwRetVal= MsgWaitForEvents(dwEventCount,hEvents,dwTeredoMonitorTimeout);
		if(dwRetVal == WAIT_OBJECT_0 )
		{
			BOOL bExit;
			//we have received a message
			//dispatch message
			DispatchMessage( bExit);
			if(bExit)break;//if exit then return
			continue;
		}	
		if(dwRetVal==WAIT_TIMEOUT)
		{
			//check the teredo status if changed notify teredo event
			SetEvent(hEvents[1]);
			continue;
		}
		if(dwRetVal>=WAIT_OBJECT_0+1 && dwRetVal<(WAIT_OBJECT_0+dwEventCount+1))
		{
			switch(dwRetVal-WAIT_OBJECT_0-1)
			{
				case 0://handle the command
					{
						CP2PManagerDefaultRPC DefaultExecute(&UIRPC);
						UIRPC.HandleCommand(pP2PManager?static_cast<IRPCExecute*>(pP2PManager):static_cast<IRPCExecute*>(&DefaultExecute));
						SetEvent(hEvents[1]);
					}
					break;
				case 1:
					//if adapter running & teredo is down then close p2p manager
					if(	pP2PManager && Teredo.IsAddressChanged())
					{
						DEBUGLOG(LOI_ERROR,_T("Teredo down"));
						//teredo adpter not enabled
						delete pP2PManager;
						pP2PManager=NULL;
						//monitor the adpter for changes
						dwTeredoMonitorTimeout=TEREDO_ACTIVE_MONITOR_TIMEOUT;
					}
					//P2Pmanager is stopped & then try starting the adpter 
					if(pP2PManager == NULL && 
						Teredo.StartTeredoAdapter()
						)
					{
						DEBUGLOG(LOI_ERROR,_T("Teredo now enabled again"));
						in_addr6 TeredoIPv6Addr;
						Teredo.GetTeredoIpv6Address(TeredoIPv6Addr);

						pP2PManager =  new CP2PManager; 

						if(!pP2PManager->Init(TeredoIPv6Addr,&UIRPC))
						{
							//no use in running this if init failed
							delete pP2PManager;
							pP2PManager=NULL;
						}
						dwTeredoMonitorTimeout=TEREDO_PASSIVE_MONITOR_TIMEOUT;
					}
					if(Teredo.IsSymmetricNAT())
					{
						//symmetric nat try after very long time
						dwTeredoMonitorTimeout=TEREDO_SYMMETRIC_NAT_MONITOR_TIMEOUT;
					}
					//in any case wait for adapter status change
					Teredo.NotifyAdapterChange(hEvents[1]);
					break;
			}
			continue;
		}
		break;//some error case
	}while(1);
	if(	pP2PManager )
	{
		//teredo adpter not enabled
		delete pP2PManager;
		pP2PManager=NULL;
	}
	//close the events
	WSACloseEvent( hEvents[0]);
	WSACloseEvent( hEvents[1]);
	return TRUE;	
}
