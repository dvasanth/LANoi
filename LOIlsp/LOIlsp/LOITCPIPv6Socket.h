#pragma once
#include "lspoverlappedsocket.h"
class CLOITCPIPv6Socket:public CLSPOverlappedSocket
{
public:
	CLOITCPIPv6Socket(PROVIDER *Provider ) ;
	CLOITCPIPv6Socket (PROVIDER *Provider ,SOCKET ProviderSocket) ;
	~CLOITCPIPv6Socket(void);
	virtual BOOL IsKindOf(LPCTSTR lpszClassName)
	{
		return _tcsicmp(lpszClassName,_T("CLOITCPIPv6Socket"))==0;
	}
	virtual void Inherit(CLSPSocket *pAssignee);
protected:
	int WSPAPI				WSPBind(
								const struct sockaddr FAR * name,
								int                   namelen,
								LPINT                 lpErrno
								);
private:
	void						Init();
};
