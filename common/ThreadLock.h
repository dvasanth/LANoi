#pragma once

class ThreadLock{
public:
	ThreadLock()
	{
		InitializeCriticalSection(&m_Lock); 
	}
	~ThreadLock()
	{
		DeleteCriticalSection(&m_Lock);
	}
	void Lock()
	{
		EnterCriticalSection(&m_Lock);
	}

	void UnLock()
	{
		LeaveCriticalSection(&m_Lock);
	}
private:
	CRITICAL_SECTION   m_Lock;
};