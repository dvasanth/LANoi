// SkinDialog.cpp : implementation file
//

#include "stdafx.h"
#include "SkinDialog.h"


// CSkinDialog dialog

IMPLEMENT_DYNAMIC(CSkinDialog, CDialog)

//CSkinDialog::CSkinDialog(CWnd* pParent /*=NULL*/)
	//: CDialog(CSkinDialog::IDD, pParent)
CSkinDialog::CSkinDialog(UINT nIDTemplate, CWnd* pParentWnd)
		: CDialog(nIDTemplate, pParentWnd)
{
	m_BackgroundBrush.CreateSolidBrush(m_Theme.GetWindowBackgroundColor());
}

CSkinDialog::~CSkinDialog()
{

}

void CSkinDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CSkinDialog, CDialog)
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

void CSkinDialog::PreSubclassWindow() 
{

	CDialog::PreSubclassWindow();
}

// CSkinDialog message handlers
/******************************
Sets the background for the dialog
********************************/
void CSkinDialog::SetBkImage(CString sFilePath)
{
	HBITMAP			hBackgroundBmp;
	hBackgroundBmp=(HBITMAP)LoadImage(AfxGetInstanceHandle(),
												(LPCTSTR)sFilePath,
												IMAGE_BITMAP,
												0,0, 
												LR_CREATEDIBSECTION|LR_LOADFROMFILE|LR_SHARED );

	m_BackgroundBitmap.Attach( hBackgroundBmp);

	//set background region
	CClientDC objClientDC(this);
	objClientDC.BeginPath();
	RECT rcClientRect;
	GetWindowRect(&rcClientRect);
	objClientDC.RoundRect(0,0,rcClientRect.right-rcClientRect.left,rcClientRect.bottom-rcClientRect.top,18,18);
	objClientDC.EndPath();
	CRgn rgnSkin;
	rgnSkin.CreateFromPath(&objClientDC);
	SetWindowRgn(rgnSkin.operator HRGN(),TRUE);
}
/********************************
defines the client area color of the dialog
**********************************/
BOOL CSkinDialog::OnEraseBkgnd(CDC* pDC)
{
	
	// TODO: Add your message handler code here and/or call default
	CDC			dcMemory;

	BITMAP		bmHeader;

	
    dcMemory.CreateCompatibleDC(pDC);

    CBitmap* pOldbitmap = dcMemory.SelectObject(&m_BackgroundBitmap);
    CRect rcClient;
    GetClientRect(&rcClient);
	m_BackgroundBitmap.GetBitmap(&bmHeader);
//	pDC->StretchBlt(0,0,bmHeader.bmWidth,bmHeader.bmHeight,&dcMemory,0,0,rcClient.Width(),rcClient.Height(),SRCCOPY);
    pDC->BitBlt(0,0,bmHeader.bmWidth,bmHeader.bmHeight,&dcMemory,0,0,SRCCOPY);
    dcMemory.SelectObject(pOldbitmap);
    return TRUE;
}
/**********
capture the mouse
************/
void CSkinDialog::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	SendMessage(WM_SYSCOMMAND, SC_MOVE|0x0002);

}
