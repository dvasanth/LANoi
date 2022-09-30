#pragma once
#include "LSPSocket.h"
class CSocketFactory
{
public:
	CSocketFactory(void);
	~CSocketFactory(void);
	CLSPSocket*		CreateSocketObject( PROVIDER *Provider ,int iAfFamily,int iSockType,int iProtocol,LPINT lpErrno);
	CLSPSocket*		CreateSocketObject(SOCKET ProviderSocket,PROVIDER *Provider,CLSPSocket *Inherit,LPINT lpErrno);
	void			FreeSockObject(CLSPSocket* pSockObj);
	SOCKET			MapAndStoreSocketObject(CLSPSocket* pSockObj,INT* lpErrno);
	BOOL			CloseProviderAndFreeLSPObject(CLSPSocket* pSockObj,INT* lpErrno);
	CLSPSocket*		LookupSocketObj(SOCKET LayeredSocket,INT *lpErrno) const;
	void			DerefSocketObject(CLSPSocket* pSockObj, int        *lpErrno);
	void			InsertSocketObject(PROVIDER  *provider, CLSPSocket* pSockObj);
	void			RemoveSocketObject(PROVIDER  *provider, CLSPSocket* pSockObj);
	void			CloseAndFreeSocketInfo(PROVIDER *provider,BOOL      processDetach );
	CLSPSocket*		FindSockObjectFromProviderSocket(PROVIDER   *provider,SOCKET      socket);
	CLSPSocket*		GetCallerSocketObject(PROVIDER *provider,SOCKET    ProviderSock);
	DWORD			NotifyNewSockInsert(DWORD dwWaitMilliSecond);
	void			Init();
private:
	HANDLE          m_AddContextEvent;
};
