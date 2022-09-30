#pragma once
#include "thread.h"
class CEventNotify:public IThread{
private:
	HWND					m_hNotifyWnd;
	UINT					m_uiNotifyMsg;
	HANDLE					m_hEvent;

public:
	CEventNotify()
	{
		m_uiNotifyMsg=0;
		m_hNotifyWnd=NULL;
		m_hEvent=NULL;
	}
	~CEventNotify()
	{
		UnRegisterEvent();
	}
	void RegisterEvent(HANDLE hEvent,HWND hNotifyWnd,UINT uiNotifyMsg) 
	{

		m_uiNotifyMsg=uiNotifyMsg;
		m_hNotifyWnd=hNotifyWnd;
		m_hEvent=hEvent;

		CreateThread();

	}
	/******************************************************
	Main single thread which posts the message
	******************************************************/
	DWORD OnThreadRun()
	{
		DWORD_PTR			dwRetVal;

		do{
			dwRetVal=MsgWaitForEvents(1,&m_hEvent);
			if(dwRetVal == WAIT_OBJECT_0)
			{
				BOOL bExit;
				//we have received a message
				//dispatch message
				DispatchMessage( bExit);
				if(bExit)break;//if exit then return
				continue;
			}	
			if(dwRetVal==WAIT_OBJECT_0+1 )
			{
				::SendMessage(m_hNotifyWnd,m_uiNotifyMsg,0,0);
			}
			else
			{
				break;
			}
		}while(1);

		return 0;
	}	
	void UnRegisterEvent()
	{	
		WaitForThreadExit();

		m_uiNotifyMsg=0;
		m_hNotifyWnd=NULL;
		m_hEvent=NULL;
	}
};