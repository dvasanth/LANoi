#pragma once
#include <windows.h>
#include <list>
using namespace std;


class IThread{
private:
	HANDLE						m_hThread;
	DWORD						m_dwThreadID;
	list<MSG>					m_MsgList;
	HANDLE						m_hMsgEvent;
	CRITICAL_SECTION			m_MsgLock;
	HANDLE						m_hMaxWait[MAXIMUM_WAIT_OBJECTS];
public:
	IThread()
	{
		m_hThread=NULL;
		m_dwThreadID=0;
		m_hMsgEvent=CreateEvent(NULL,FALSE,FALSE,NULL);
		InitializeCriticalSection(&m_MsgLock);  
		ZeroMemory(m_hMaxWait,sizeof(m_hMaxWait));
	}
	virtual ~IThread()
	{
		WaitForThreadExit();
		if(m_hMsgEvent)
		{
			CloseHandle(m_hMsgEvent);
			m_hMsgEvent=NULL;
		}
		DeleteCriticalSection(&m_MsgLock);  
	}
	BOOL CreateThread()
	{
		WaitForThreadExit();//close thread if already running
		m_hThread= ::CreateThread(0,0,ThreadProc,this,0,&m_dwThreadID);
		return BOOL(m_hThread);
	}
	void WaitForThreadExit(DWORD dwMilliseconds=INFINITE)
	{
		if( m_hThread)
		{
			PostQuitMessage();
			if(!IsSameThreadContext())//wait on thread only if running from another thread
						WaitForSingleObject(m_hThread,dwMilliseconds);
			CloseHandle(m_hThread);
			m_hThread=NULL;
		}
	}
	BOOL IsThreadRunning()
	{
		if(m_hThread)
		{
			return (WaitForSingleObject(m_hThread,0)!=WAIT_OBJECT_0); 
		}
		return FALSE;
	}
	void PostQuitMessage()
	{
		PostThreadMessage(WM_QUIT,0,0);
	}

	void PostThreadMessage(UINT uiMsg,WPARAM wParam,LPARAM lParam)
	{
		if(IsSameThreadContext())//we are running in the same thread context just invoke onmsg handler
		{
			OnThreadMsg(uiMsg,wParam,lParam);
			return ;
		}
	
		MSG  msg={0};

		EnterCriticalSection(&m_MsgLock);
		msg.message = uiMsg;
		msg.wParam = wParam;
		msg.lParam = lParam;
		m_MsgList.push_front( msg);
		SetEvent(m_hMsgEvent); //notify thread
		LeaveCriticalSection(&m_MsgLock);  

	}
	void DispatchMessage(BOOL &bExit)
	{
		MSG				msg={0};

		EnterCriticalSection(&m_MsgLock);
		while(!m_MsgList.empty())		
		{
			msg=m_MsgList.front ();
			OnThreadMsg(msg.message,msg.wParam,msg.lParam); 
			m_MsgList.pop_front();//processed the msg remove it
			bExit=(msg.message==WM_QUIT);
			if(bExit) 
			{
				LeaveCriticalSection(&m_MsgLock);  
				return ;
			}
		}
		LeaveCriticalSection(&m_MsgLock);  
		return ;
	}
	/************************
	return WAIT_OBJECT_0 for msg notify
	WAIT_OBJECT_0 + 1 to WAIT_OBJECT_0 + dwEventCount
	*************************/
	DWORD MsgWaitForEvents(DWORD dwEventCount,PHANDLE pEventArray,DWORD dwMilliSec=INFINITE)
	{
		const DWORD		MAX_WAIT_SUPPORTED=MAXIMUM_WAIT_OBJECTS-1;//1 for msg event notify
		DWORD			dwRetVal=0;	

		//always first event is for message notification
		m_hMaxWait[0]=m_hMsgEvent;
		if(dwEventCount)
		{
			//copy the handles for wait from user passed
			memcpy( &m_hMaxWait[1],pEventArray,(dwEventCount>MAX_WAIT_SUPPORTED?MAX_WAIT_SUPPORTED:dwEventCount)*sizeof(HANDLE));
		}
		dwRetVal=WaitForMultipleObjects(dwEventCount+1,m_hMaxWait,FALSE,dwMilliSec);
		return dwRetVal;
	}
	BOOL WaitForExitMessage(DWORD dwMilliSeconds=INFINITE)
	{
		BOOL		bExit=FALSE;
		DWORD		dwRetVal;

		dwRetVal=MsgWaitForEvents(0,NULL,dwMilliSeconds);
		if(dwRetVal == WAIT_OBJECT_0 )
		{
			//dispatch message
			DispatchMessage( bExit);
		}	
		return bExit;
	}	
	virtual DWORD OnThreadMsg(UINT uiMsg,WPARAM wParam,LPARAM lParam)
	{
		return 0;
	}
	virtual DWORD OnThreadRun()
	{
		return 0;
	}

	BOOL IsSameThreadContext()
	{
		return m_dwThreadID==GetCurrentThreadId();  
	}

private:
	static DWORD WINAPI			ThreadProc(LPVOID lParam)
	{
		return ((IThread*)lParam)->OnThreadRun(); 
	}	
};