#pragma once

#include "../resource.h"
#include "IPc/IPCCommandHandler.h"
#include "InvitationAuthPage.h"
// CJoinerWizard

class CJoinerWizard : public CPropertySheet
{
	DECLARE_DYNAMIC(CJoinerWizard)

public:
	CJoinerWizard(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CJoinerWizard(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	virtual ~CJoinerWizard();

protected:
	DECLARE_MESSAGE_MAP()
public:
	INT_PTR ShowWizard(CIPCCommandHandler &IpcExecute,CString sFilepath);
	inline CString GetNickname()
	{
		return m_PgAuth.GetNickname();
	}
	inline CString GetPassword()
	{
		return m_PgAuth.GetPassword();
	}
	inline CString GetFilepath()
	{
		return m_sFilepath;
	}

private:
	CInvitationAuthPage  m_PgAuth;
	CString				m_sFilepath;
	BOOL				OnInitDialog();
	void OnWizNext();
	void OnWizFinish();
};


