#pragma once

#include "../resource.h"

#include "SessionSelectionPage.h"
#include "InvitationSavePage.h"
#include "IPc/IPCCommandHandler.h"
// CInviterWizard

class CInviterWizard : public CPropertySheet
{
	DECLARE_DYNAMIC(CInviterWizard)

public:
	CInviterWizard(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CInviterWizard(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	virtual ~CInviterWizard();

protected:
	DECLARE_MESSAGE_MAP()
public:

	int ShowWizard();
	inline CString GetNickName()
	{
		return m_PgSession.GetNickName();
	}
	inline CString GetPassword()
	{
		return m_PgInvitationSave.GetSessionPassword();
	}
	inline CString GetSessionName()
	{
		return m_PgSession.GetSessionDisplayName();
	}
	inline CString GetSessionXML()
	{
		return m_PgSession.GetSessionResourceString();
	}
	inline FILETIME GetExpiryTime()
	{
		return m_PgSession.GetExpiryTime();
	}
	inline CString GetFilePath()
	{
		return m_PgInvitationSave.GetFilePath();
	}
private:
	CSessionSelectionPage     m_PgSession;
	CInvitationSavePage		  m_PgInvitationSave;

	BOOL					  OnInitDialog();
	void					  OnWizNext();
	void					  OnWizFinish();
	void					  OnWizBack();
};


