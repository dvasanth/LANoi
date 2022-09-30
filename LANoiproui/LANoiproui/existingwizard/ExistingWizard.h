#pragma once
#include "../resource.h"
#include "../inviterwizard/InvitationSavePage.h"
#include "ExistingInvitationPage.h"


// CExistingWizard

class CExistingWizard : public CPropertySheet
{
	DECLARE_DYNAMIC(CExistingWizard)

public:
	CExistingWizard(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CExistingWizard(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	virtual ~CExistingWizard();

protected:
	DECLARE_MESSAGE_MAP()
public:
	int ShowWizard(void);
private:
	CExistingInvitationPage  m_PgExist;
	CInvitationSavePage      m_PgInvitationSave;


	BOOL					OnInitDialog();
	void OnWizNext();
	void OnWizFinish();
};


