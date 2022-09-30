#pragma once

#include "../resource.h"
#include "afxwin.h"
#include "../prosession/SessionTypes.h"

typedef struct _EXPIRY_DATE_FORMAT{
	TCHAR     szTimeFormat[MAX_PATH];
	UINT	  uiHours;
  } EXPIRY_DATE_FORMAT , *LPEXPIRY_DATE_FORMAT;
// CSessionSelectionPage dialog

class CSessionSelectionPage : public CPropertyPage
{
	DECLARE_DYNAMIC(CSessionSelectionPage)

public:
	CSessionSelectionPage();
	virtual ~CSessionSelectionPage();

// Dialog Data
	enum { IDD = IDD_SESSION_SELECT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
private:
	static const    EXPIRY_DATE_FORMAT   ExpiryDaysLists[];
	CSessionTypes	m_SessionNames;
	CString			m_sHeaderTitle;
	CString			m_sSubtitle;
	CComboBox		m_CbnSessionList;
	CString			m_sSessionResourceString;
	CString			m_sSelectedSessionName;
	CString			m_sNickname;
	CComboBox		m_cbnExpiryDateList;
	FILETIME		m_ftNotValidAfterTime;
	void			OnLoad();
	void			OnSave();
	void			LoadSessionTypes();
	FILETIME		GetIncrementedFiletime(UINT uiHours);
public:
	CString			GetSessionDisplayName();
	CString			GetSessionResourceString();
	CString			GetNickName();
	FILETIME		GetExpiryTime();
	BOOL			IsValidInput();


	afx_msg void OnBnClickedAddNewSession();
};
