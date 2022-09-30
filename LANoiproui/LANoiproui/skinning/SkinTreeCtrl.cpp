// SkinTreeCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "SkinTreeCtrl.h"


// CSkinTreeCtrl

IMPLEMENT_DYNAMIC(CSkinTreeCtrl, CTreeCtrl)

CSkinTreeCtrl::CSkinTreeCtrl()
{
	m_BackgroundBrush.CreateSolidBrush(m_Theme.GetControlBackgroundColor());
}

CSkinTreeCtrl::~CSkinTreeCtrl()
{
}


BEGIN_MESSAGE_MAP(CSkinTreeCtrl, CTreeCtrl)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnNMCustomdrawTree)
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()



// CSkinTreeCtrl message handlers


void CSkinTreeCtrl::PreSubclassWindow() 
{

	CTreeCtrl::PreSubclassWindow();
}

// CSkinTreeCtrl message handlers
/****************************************
custom draw the color of the tree parent & child items
****************************************/
void CSkinTreeCtrl::OnNMCustomdrawTree(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTVCUSTOMDRAW	pNMCD = reinterpret_cast<LPNMTVCUSTOMDRAW>(pNMHDR);
	BOOL				bItemSelected;


	// TODO: Add your control notification handler code here
	switch (pNMCD->nmcd.dwDrawStage)
	{
		case CDDS_PREPAINT:
			// Need to process this case and set pResult to CDRF_NOTIFYITEMDRAW, 
			// otherwise parent will never receive CDDS_ITEMPREPAINT notification. (GGH) 
			*pResult = CDRF_NOTIFYITEMDRAW;
			return ;
		case CDDS_ITEMPREPAINT:
			bItemSelected=(pNMCD->nmcd.uItemState == (CDIS_FOCUS | CDIS_SELECTED));
			pNMCD->clrText = m_Theme.GetControlTextColor(bItemSelected);
			pNMCD->clrTextBk=m_Theme.GetControlBackgroundTextColor(bItemSelected);
			*pResult = CDRF_SKIPDEFAULT;
			break;
	}
	*pResult = 0;

}
/***********************************************
Decide the client area background color of the treectrl here
***********************************************/
BOOL CSkinTreeCtrl::OnEraseBkgnd(CDC* pDC) 
{
	RECT			ClientArea;
	CBrush		    *pOldBrush;

	GetClientRect( &ClientArea);
	pOldBrush=pDC->SelectObject (&m_BackgroundBrush);
	pDC->Rectangle(0,0,ClientArea.right,ClientArea.bottom);  
	pDC->SelectObject(pOldBrush);
	return TRUE;
}
