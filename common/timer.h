#include "thread.h"
#include <windows.h>

class ITimerNotify{
public:
	ITimerNotify(){}
	~ITimerNotify(){}
	virtual void OnTimer(){}
};


class CTimer:public IThread{
private:
	HANDLE			m_hWaitObject;
	HANDLE			m_hTimer;
	ITimerNotify    *m_pTimerNotify;
	HANDLE			 m_hWait;
	LARGE_INTEGER    m_liDueTime;
	DWORD			 m_dwSeconds;
public:
	CTimer()
	{
		m_pTimerNotify=NULL;
		
		m_dwSeconds=0;
	}
	~CTimer()
	{
		StopTimer();
		
	}
	void StartTimer(ITimerNotify *pTimerNotify,DWORD dwSeconds)
	{	
		m_dwSeconds=dwSeconds;
		m_pTimerNotify=pTimerNotify;
		CreateThread(); 

	}

	void StopTimer()
	{
		WaitForThreadExit();
	}
	/******************************************************
	Main single thread which handles invokes the timer proc
	******************************************************/
	DWORD OnThreadRun()
	{
		DWORD_PTR			dwRetVal;
		HANDLE				hDummyEvent=CreateEvent(NULL,FALSE,FALSE,NULL);

		do{
			dwRetVal=MsgWaitForEvents(1,&hDummyEvent,m_dwSeconds*1000);
			if(dwRetVal == WAIT_OBJECT_0 )
			{
				BOOL bExit;
				//we have received a message
				//dispatch message
				DispatchMessage( bExit);
				if(bExit)break;//if exit then return
				continue;
			}	
			if(dwRetVal>=WAIT_OBJECT_0 +1)
			{
				m_pTimerNotify->OnTimer();
			}
		}while(1);

		CloseHandle(hDummyEvent);
		return 0;
	}

};