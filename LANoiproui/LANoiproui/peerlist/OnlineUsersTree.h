#if !defined(AFX_CBUDDYTREE_H__6511A6EE_992D_487E_9D29_51BD0ECE39E0__INCLUDED_)
#define AFX_CBUDDYTREE_H__6511A6EE_992D_487E_9D29_51BD0ECE39E0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// COnlineUsersTree.h : header file
//
#include "loisession/ui/UILOISession.h"
#include "IPC/IPCCommandHandler.h"

class CPeerItemInfo{
public:
	CPeerItemInfo(CString sSessionID)
	{
		m_sSessionID=sSessionID;
	}
	~CPeerItemInfo()
	{
	}
	CString GetSessionID()
	{
		return m_sSessionID; 
	}
private:
	CString		m_sSessionID;
};
/////////////////////////////////////////////////////////////////////////////
// COnlineUsersTree window

class COnlineUsersTree : public CTreeCtrl
{
// Construction
public:
	COnlineUsersTree();


// Attributes
public:
	void SetDefaultCursor() ;
// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COnlineUsersTree)
	//}}AFX_VIRTUAL

//*********** CURSORS  *********************
	HCURSOR cursor_hand ;
	HCURSOR cursor_arr	;
	HCURSOR cursor_no	;
	bool	isCursorArrow;
//******************************************
	//imagelist for items
	CImageList		m_ImgListNode;


//*********************************************
	//load peer list & notify
	CUILOISession			m_LOISession;
	static const UINT		PEER_CHANGE_NOTIFY;

	
	afx_msg LRESULT			OnClientInfoChanged(WPARAM wParam, LPARAM lParam);
// Implementation
public:
	COLORREF memDC_bgColor_bitmap;
	bool	 isImageTiled;
	void		Init(CFont& CustomFont,IRPCProxyStub *pIPCServiceExecute);


	virtual		~COnlineUsersTree();
	void					UpdatePeerList();
	BOOL SetBkImage(LPCTSTR lpszResourceName) ;

	// Generated message map functions
protected:
	//{{AFX_MSG(cBuddyTree)
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	
	afx_msg void OnPaint();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnItemExpanding(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg BOOL OnQueryNewPalette();
	afx_msg void OnPaletteChanged(CWnd* pFocusWnd);
	afx_msg void OnRClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint ptMousePos);
	afx_msg void OnDestroy();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:
	CPalette m_pal;
	CBitmap m_bitmap;
	int m_cxBitmap, m_cyBitmap ;
	IRPCProxyStub		*m_pIPCServiceExecute;
	HTREEITEM			m_hMenuClickedItem;
	afx_msg void		OnClosesession();
	afx_msg void		OnTvnDeleteitemOnlinePeers(NMHDR *pNMHDR, LRESULT *pResult);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CBUDDYTREE_H__6511A6EE_992D_487E_9D29_51BD0ECE39E0__INCLUDED_)
