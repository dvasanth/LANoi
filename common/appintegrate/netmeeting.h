#pragma once
#include "imsconf2.h"
#include "iapplet.h"
#include "error.h"
class CNetMeeting:public CError
{
public:
	CNetMeeting(void);
	virtual		~CNetMeeting(void);
	HRESULT		 HostMeeting(CString sMeetingName);
	HRESULT		 JoinMeeting(CString sUserName);
private:
	 LPCLASSFACTORY			 m_pClassFactory;
	 INmManager			    *m_pConfMgr;
	 HRESULT				 InitConfMgr();
	 void					 TerminateConf();

};
