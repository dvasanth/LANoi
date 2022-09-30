#pragma once
#include "../resource.h"
#include "SkinTheme.h"

#define DHT_CLOSE		0x01
#define DHT_CAPTION		0x02
// CSkinDialog dialog

class CSkinDialog : public CDialog
{
	DECLARE_DYNAMIC(CSkinDialog)

public:
	CSkinDialog();   // standard constructor
	CSkinDialog(UINT nIDTemplate, CWnd* pParentWnd = NULL);
	CSkinDialog(LPCTSTR lpszTemplateName, CWnd* pParentWnd = NULL);
	virtual ~CSkinDialog();
	void		SetBkImage(CString sFilePath);
// Dialog Data
//	enum { IDD = IDD_SKINDIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	CSkinTheme		m_Theme;
	CBrush			m_BackgroundBrush;
	CBitmap			m_BackgroundBitmap;

	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	void		 PreSubclassWindow() ;
/*************
client area move handling
*************/
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
};
