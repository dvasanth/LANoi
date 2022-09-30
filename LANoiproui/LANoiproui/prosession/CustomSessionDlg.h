#pragma once
#include "afxwin.h"
#include "loisession\LOISessionStringFormat.h"

// CCustomSessionDlg dialog

class CCustomSessionDlg : public CDialog
{
	DECLARE_DYNAMIC(CCustomSessionDlg)

public:
	CCustomSessionDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCustomSessionDlg();

// Dialog Data
	enum { IDD = IDD_NEW_SESSION };
	inline CString GetSessionName()
	{
		return m_sSessionName;
	}
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	CString			m_sSessionName;
	CString			m_sPortValues;
	CButton			m_BtnAdvanced;
	CString			m_SessionXMLString;
	CComboBox		m_CbnProcessPathList;
	BOOL			OnInitDialog();
	void			OnLoad();
	BOOL			OnSave();
	BOOL			IsValidInput();	
	afx_msg void	OnBnClickedAddProcess();
	afx_msg void	OnBnClickedAdvancedButton();
	void			ShowAdvancedOption(BOOL bShow);
	BOOL			PrasePortValues(CString sPort,CLOISessionStringFormat &sLOIFormat);
	BOOL			AddApplicationNames(CComboBox &CbnItems,CLOISessionStringFormat &sLOIFormat);
public:
	afx_msg void	OnBnClickedAdd();
};
