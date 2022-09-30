// InvitationInfoPage.cpp : implementation file
//

#include "stdafx.h"

#include "InvitationInfoPage.h"


// CInvitationInfoPage dialog

IMPLEMENT_DYNAMIC(CInvitationInfoPage, CPropertyPage)

CInvitationInfoPage::CInvitationInfoPage()
	: CPropertyPage(CInvitationInfoPage::IDD)
{

}

CInvitationInfoPage::~CInvitationInfoPage()
{
}

void CInvitationInfoPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CInvitationInfoPage, CPropertyPage)
END_MESSAGE_MAP()


// CInvitationInfoPage message handlers
