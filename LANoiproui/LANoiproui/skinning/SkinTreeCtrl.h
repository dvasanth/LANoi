#pragma once
#include "SkinTheme.h"

// CSkinTreeCtrl

class CSkinTreeCtrl : public CTreeCtrl
{
	DECLARE_DYNAMIC(CSkinTreeCtrl)

public:
	CSkinTreeCtrl();
	virtual ~CSkinTreeCtrl();

protected:
	DECLARE_MESSAGE_MAP()
private:
	CSkinTheme		m_Theme;
	CBrush			m_BackgroundBrush;

	afx_msg void	OnNMCustomdrawTree(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg BOOL	OnEraseBkgnd(CDC* pDC);
	void			PreSubclassWindow() ;
};


