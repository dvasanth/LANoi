#pragma once

#include "../resource.h"

// CInvitationInfoPage dialog

class CInvitationInfoPage : public CPropertyPage
{
	DECLARE_DYNAMIC(CInvitationInfoPage)

public:
	CInvitationInfoPage();
	virtual ~CInvitationInfoPage();

// Dialog Data
	enum { IDD = IDD_INVITATION_INFO };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
