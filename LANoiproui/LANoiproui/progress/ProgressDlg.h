#pragma once
#include "afxcmn.h"
#include "../resource.h"
// CProgressDlg dialog

class CProgressDlg : public CDialog
{
	DECLARE_DYNAMIC(CProgressDlg)

public:
	CProgressDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CProgressDlg();

// Dialog Data
	enum { IDD = IDD_PROGRESS_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	void		Close();
	void SetStatusText(UINT uiStringID);
private:
	virtual   BOOL		OnInitDialog() ;
	virtual   void		OnCancel();

	afx_msg void OnBnClickedCancel();
	CString m_sStatusText;

	CProgressCtrl m_prgProgressUpdate;
};
