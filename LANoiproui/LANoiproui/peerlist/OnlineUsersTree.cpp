// COnlineUsersTree.cpp : implementation file
//

#include "stdafx.h"
#include "OnlineUsersTree.h"
#include "../resource.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
const UINT COnlineUsersTree::PEER_CHANGE_NOTIFY=RegisterWindowMessage(_T("PeerNotify")); 


/////////////////////////////////////////////////////////////////////////////
// COnlineUsersTree

COnlineUsersTree::COnlineUsersTree()
{
	memDC_bgColor_bitmap = RGB(255,255,255);

	isImageTiled = FALSE ;

	SetDefaultCursor() ;
	m_pIPCServiceExecute=NULL;
}

COnlineUsersTree::~COnlineUsersTree()
{
	m_LOISession.StopChangeNotification();
}


BEGIN_MESSAGE_MAP(COnlineUsersTree, CTreeCtrl)
	//{{AFX_MSG_MAP(COnlineUsersTree)
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_PAINT()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_NOTIFY_REFLECT(TVN_ITEMEXPANDING, OnItemExpanding)
	ON_REGISTERED_MESSAGE(PEER_CHANGE_NOTIFY, OnClientInfoChanged)
	ON_WM_ERASEBKGND()
	ON_WM_QUERYNEWPALETTE()
	ON_WM_PALETTECHANGED()
	ON_NOTIFY_REFLECT(NM_RCLICK, OnRClick)
	ON_NOTIFY_REFLECT(TVN_DELETEITEM,&COnlineUsersTree::OnTvnDeleteitemOnlinePeers)
	ON_WM_CONTEXTMENU()
	//}}AFX_MSG_MAP
	ON_WM_DESTROY()
	ON_COMMAND(ID_CLOSESESSION, &COnlineUsersTree::OnClosesession)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COnlineUsersTree message handlers
/********************************************
Initialise the tree background,font,text color etc
Load the online users 
********************************************/
void COnlineUsersTree::Init(CFont& CustomFont,IRPCProxyStub *pIPCServiceExecute)
{
	COLORREF		BackColor=RGB(0,0,0);
	CBitmap			bmpImageList;
	HIMAGELIST		hTreeImageList;

	SetFont(&CustomFont,FALSE);


	SetTextColor(BackColor);

	// Set Background image
	SetBkImage(_T("images\\Tree_bg.bmp")) ;

	// normal tree images	
	hTreeImageList = ::ImageList_LoadImage(AfxGetInstanceHandle(),
											_T("images\\tree_down_arrow.bmp"),
											18, 7,
											RGB(252,254,252),
											IMAGE_BITMAP,
											LR_LOADTRANSPARENT|LR_CREATEDIBSECTION|LR_LOADFROMFILE|LR_SHARED);
	if(NULL != hTreeImageList)
	{
		m_ImgListNode.Attach(hTreeImageList);
		// ...further enjoy!...
	}
	ASSERT(m_ImgListNode.m_hImageList);

	bmpImageList.Attach( (HBITMAP)LoadImage( AfxGetInstanceHandle(),
											_T("images\tree_down_arrow.bmp"), 
											IMAGE_BITMAP,
											0,0, 
											LR_LOADTRANSPARENT|LR_CREATEDIBSECTION|LR_LOADFROMFILE|LR_SHARED ));
	//bmpImageList.LoadBitmap(IDB_TREE_NORMAL_IMAGELIST);
	m_ImgListNode.Add( &bmpImageList, RGB(252,254,252));
	bmpImageList.DeleteObject();

	SetImageList(&m_ImgListNode,TVSIL_NORMAL); //TVSIL_STATE

	/// load all session information & ask notification
	m_LOISession.RegisterChangeNotification( PEER_CHANGE_NOTIFY,m_hWnd);
	UpdatePeerList();
	m_pIPCServiceExecute=pIPCServiceExecute;

}
/***********************************************
Load all peer connectecd information
**************************************************/
void COnlineUsersTree::UpdatePeerList()
{
	USHORT						usClientList=0;
	LPCLIENT_INFORMATION		pClientInfo=NULL;

	DeleteAllItems  ();

	m_LOISession.RefreshClientInfo();
	pClientInfo=m_LOISession.GetConnectedClientsInfo(usClientList);

	if(usClientList)
	{
		CMapStringToPtr   SessionIDToTreeParentMap;

		for(int iIndex=0;iIndex<usClientList;iIndex++)
		{
			
			HTREEITEM  hSessionParent =(HTREEITEM)SessionIDToTreeParentMap[pClientInfo[iIndex].szSessionID];
			if(hSessionParent==NULL)
			{
				CPeerItemInfo    *pSessionItemInfo= new CPeerItemInfo(pClientInfo[iIndex].szSessionID);
				//tree doesn't exists add it
				hSessionParent =InsertItem(pClientInfo[iIndex].szSessionName,0,0);
				SessionIDToTreeParentMap.SetAt( pClientInfo[iIndex].szSessionID,hSessionParent);
				SetItemData(hSessionParent,(DWORD_PTR)pSessionItemInfo);
				
			}
			CPeerItemInfo		*pPeerItemInfo= new CPeerItemInfo(pClientInfo[iIndex].szSessionID);
			HTREEITEM			hTreePeerItem;

			hTreePeerItem=InsertItem(pClientInfo[iIndex].PeerInfo.szPeerName,1,1,hSessionParent);
			SetItemData(hTreePeerItem,(DWORD_PTR)pPeerItemInfo);
			Expand(hSessionParent,TVE_EXPAND   );
		}
		m_LOISession.FreeConnectedClientInfo(pClientInfo);
	}	

	 

	if(usClientList==0)
	{
		//if no client check the teredo status & update

	}

}
/**************************
Send contextenu show message
**************************/
void COnlineUsersTree::OnRClick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TRACE0("COnlineUsersTree::OnRClick()\n");
	// Send WM_CONTEXTMENU to self
	SendMessage(WM_CONTEXTMENU, (WPARAM) m_hWnd, GetMessagePos());
	// Mark message as handled and suppress default handling
	*pResult = 1;
}
/*****************************
show the context menu
********************************/
void COnlineUsersTree::OnContextMenu(CWnd* pWnd, CPoint ptMousePos) 
{
	// if Shift-F10
	if (ptMousePos.x == -1 && ptMousePos.y == -1)
		ptMousePos = (CPoint) GetMessagePos();

	ScreenToClient(&ptMousePos);

	UINT uFlags;
	HTREEITEM htItem;
	
	htItem = HitTest( ptMousePos, &uFlags );

	if( htItem == NULL )
		return;
	
	m_hMenuClickedItem = htItem;

	CMenu menu;
	CMenu* pPopup;

	// the font popup is stored in a resource
	menu.LoadMenu(IDR_ONLINE_PEER_CONTEXTMENU);
	pPopup = menu.GetSubMenu(0);
	ClientToScreen(&ptMousePos);
	pPopup->TrackPopupMenu( TPM_LEFTALIGN, ptMousePos.x, ptMousePos.y, this );
}
/********************************************
Close the network Session
*********************************************/
void COnlineUsersTree::OnClosesession()
{
	// TODO: Add your command handler code here
	CPeerItemInfo    *pItemInfo= reinterpret_cast<CPeerItemInfo*>(GetItemData(m_hMenuClickedItem));
	if(pItemInfo)
	{
		m_pIPCServiceExecute->CloseSession(pItemInfo->GetSessionID());
	}
}
/******************************************
Client list is changed
*******************************************/
LRESULT	COnlineUsersTree::OnClientInfoChanged(WPARAM wParam, LPARAM lParam)
{
	UpdatePeerList();
	return 0;
}
/***********************
delete the items
************************/
void COnlineUsersTree::OnTvnDeleteitemOnlinePeers(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
	delete reinterpret_cast<CPeerItemInfo*>(GetItemData(pNMTreeView->itemOld.hItem));
}

/***********************
Do all deinitialze funtions here
**************************/
void COnlineUsersTree::OnDestroy()
{
	CTreeCtrl::OnDestroy();

	// TODO: Add your message handler code here
	m_LOISession.StopChangeNotification();
}

void COnlineUsersTree::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	CTreeCtrl::OnLButtonDblClk(nFlags, point);
	
	HTREEITEM hItem = GetSelectedItem();
	
	// If this is a root node, return
	if (GetParentItem(hItem) == NULL) return ;

}

void COnlineUsersTree::OnRButtonDown(UINT nFlags, CPoint point) 
{

	CTreeCtrl::OnRButtonDown(nFlags, point);
}


void COnlineUsersTree::OnPaint() 
{

		// Remove comments from next five lines if you don't need any 
	// specialization beyond adding a background image
//	if( m_bitmap.m_hObject == NULL )
//	{
//		CTreeCtrl::OnPaint();
//		return;
//	}

	CPaintDC dc(this);

	CRect rcClip, rcClient;
	dc.GetClipBox( &rcClip );
	GetClientRect(&rcClient);

	// Create a compatible memory DC 
	CDC memDC;
	memDC.CreateCompatibleDC( &dc );
	
	// Select a compatible bitmap into the memory DC
	CBitmap bitmap, bmpImage;
	bitmap.CreateCompatibleBitmap( &dc, rcClient.Width(), rcClient.Height() );
	memDC.SelectObject( &bitmap );

	
	// First let the control do its default drawing.
	CWnd::DefWindowProc( WM_PAINT, (WPARAM)memDC.m_hDC, 0 );

	// Draw bitmap in the background if one has been set
	if( m_bitmap.m_hObject != NULL )
	{
		// Now create a mask
		CDC maskDC;
		maskDC.CreateCompatibleDC(&dc);
		CBitmap maskBitmap;

		// Create monochrome bitmap for the mask
		maskBitmap.CreateBitmap( rcClient.Width(), rcClient.Height(), 
						1, 1, NULL );
		maskDC.SelectObject( &maskBitmap );
		memDC.SetBkColor(RGB(255,255,255) /*::GetSysColor( COLOR_WINDOW )*/ );

		// Create the mask from the memory DC
		maskDC.BitBlt( 0, 0, rcClient.Width(), rcClient.Height(), &memDC, 
					rcClient.left, rcClient.top, SRCCOPY );

		
		CDC tempDC;
		tempDC.CreateCompatibleDC(&dc);
		tempDC.SelectObject( &m_bitmap );

		CDC imageDC;
		CBitmap bmpImage;
		imageDC.CreateCompatibleDC( &dc );
		bmpImage.CreateCompatibleBitmap( &dc, rcClient.Width(), 
						rcClient.Height() );
		imageDC.SelectObject( &bmpImage );

		if( dc.GetDeviceCaps(RASTERCAPS) & RC_PALETTE && m_pal.m_hObject != NULL )
		{
			dc.SelectPalette( &m_pal, FALSE );
			dc.RealizePalette();

			imageDC.SelectPalette( &m_pal, FALSE );
		}

		// Get x and y offset
		CRect rcRoot;
		GetItemRect( GetRootItem(), rcRoot, FALSE );
		rcRoot.left = -GetScrollPos( SB_HORZ );

	
		if (isImageTiled)
		{
			// Draw bitmap in tiled manner to imageDC
			for( int i = rcRoot.left; i < rcClient.right; i += m_cxBitmap )
			for( int j = rcRoot.top; j < rcClient.bottom; j += m_cyBitmap )
				imageDC.BitBlt( i, j, m_cxBitmap, m_cyBitmap, &tempDC, 
							0, 0, SRCCOPY );
		}else
		{
			int x=0,y=0 ;
			(m_cxBitmap > rcClient.right) ? x=0:x=(rcClient.right - m_cxBitmap);
			(m_cyBitmap > rcClient.bottom)? y=0:y=(rcClient.bottom - m_cyBitmap);
		//	imageDC.BitBlt( x, y, m_cxBitmap, m_cyBitmap, &tempDC, 
		//					0, 0, SRCCOPY );
			imageDC.StretchBlt( 0, 0,rcClient.right-rcClient.left,rcClient.bottom-rcClient.top,
								 &tempDC, 
								0, 0,m_cxBitmap, m_cyBitmap,
								SRCCOPY );

		}

		// Set the background in memDC to black. Using SRCPAINT with black and any other
		// color results in the other color, thus making black the transparent color
		memDC.SetBkColor( RGB(0,0,0)/*memDC_bgColor_bitmap*/);        
		memDC.SetTextColor(RGB(255,255,255));
		memDC.BitBlt(rcClip.left, rcClip.top, rcClip.Width(), rcClip.Height(), &maskDC, 
				rcClip.left, rcClip.top, SRCAND);

		// Set the foreground to black. See comment above.
		imageDC.SetBkColor(RGB(255,255,255));
		imageDC.SetTextColor(RGB(0,0,0));
		imageDC.BitBlt(rcClip.left, rcClip.top, rcClip.Width(), rcClip.Height(), &maskDC, 
				rcClip.left, rcClip.top, SRCAND);

		// Combine the foreground with the background
		imageDC.BitBlt(rcClip.left, rcClip.top, rcClip.Width(), rcClip.Height(), 
					&memDC, rcClip.left, rcClip.top,SRCPAINT);

		//*****************************
			/*	for( int yy = 0; yy < rcClient.Height(); yy++)
				for( int xx = 0; xx < rcClient.Width(); xx++ )
			{
				if (imageDC.GetPixel(CPoint(xx,yy)) == RGB(0,0,0))
					imageDC.SetPixel(CPoint(xx,yy),RGB(255,255,255));
			}
			 Create a compatible memory DC 48068
			CDC whiteDC;
			whiteDC.CreateCompatibleDC( &dc );
	
			// Select a compatible bitmap into the memory DC
				CBitmap cBmp;
			blankBmp.CreateCompatibleBitmap( &dc, rcClient.Width(), rcClient.Height() );
			whiteDC.SelectObject( &blankBmp );*/
		//*****************************

		// Draw the final image to the screen		
		dc.BitBlt( rcClip.left, rcClip.top, rcClip.Width(), rcClip.Height(), 
					&imageDC, rcClip.left, rcClip.top, SRCCOPY );
	}
	else
	{
		dc.BitBlt( rcClip.left, rcClip.top, rcClip.Width(), 
				rcClip.Height(), &memDC, 
				rcClip.left, rcClip.top, SRCCOPY );
	}

}

void COnlineUsersTree::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if( m_bitmap.m_hObject != NULL ) InvalidateRect(NULL);

	CTreeCtrl::OnHScroll(nSBCode, nPos, pScrollBar);

}

void COnlineUsersTree::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if( m_bitmap.m_hObject != NULL )	InvalidateRect(NULL);

	CTreeCtrl::OnVScroll(nSBCode, nPos, pScrollBar);

}

void COnlineUsersTree::OnItemExpanding(NMHDR* pNMHDR, LRESULT* pResult) 
{
	
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	if( m_bitmap.m_hObject != NULL ) InvalidateRect(NULL);
	
	*pResult = 0;

}

BOOL COnlineUsersTree::OnEraseBkgnd(CDC* pDC) 
{
	if( m_bitmap.m_hObject != NULL )	return TRUE;

	return CTreeCtrl::OnEraseBkgnd(pDC);

}

BOOL COnlineUsersTree::OnQueryNewPalette() 
{

	CClientDC dc(this);
	if( dc.GetDeviceCaps(RASTERCAPS) & RC_PALETTE && m_pal.m_hObject != NULL )
	{
		dc.SelectPalette( &m_pal, FALSE );
		BOOL result = dc.RealizePalette();
		if( result )
			Invalidate();
		return result;
	}
	
	return CTreeCtrl::OnQueryNewPalette();
}

void COnlineUsersTree::OnPaletteChanged(CWnd* pFocusWnd) 
{
	CTreeCtrl::OnPaletteChanged(pFocusWnd);
	
	if( pFocusWnd == this )
		return;

	OnQueryNewPalette();

}


BOOL COnlineUsersTree::SetBkImage(LPCTSTR lpszResourceName)
{

	// If this is not the first call then Delete GDI objects
	if( m_bitmap.m_hObject != NULL )
		m_bitmap.DeleteObject();
	if( m_pal.m_hObject != NULL )
		m_pal.DeleteObject();
	
	
	HBITMAP hBmp = (HBITMAP)::LoadImage( AfxGetInstanceHandle(), 
			lpszResourceName, IMAGE_BITMAP, 0,0, LR_CREATEDIBSECTION|LR_LOADFROMFILE|LR_SHARED );

	if( hBmp == NULL ) 
		return FALSE;

	m_bitmap.Attach( hBmp );
	BITMAP bm;
	m_bitmap.GetBitmap( &bm );
	m_cxBitmap = bm.bmWidth;
	m_cyBitmap = bm.bmHeight;


	// Create a logical palette for the bitmap
	DIBSECTION ds;
	BITMAPINFOHEADER &bmInfo = ds.dsBmih;
	m_bitmap.GetObject( sizeof(ds), &ds );

	int nColors = bmInfo.biClrUsed ? bmInfo.biClrUsed : 1 << bmInfo.biBitCount;

	// Create a halftone palette if colors > 256. 
	CClientDC dc(NULL);			// Desktop DC
	if( nColors > 256 )
		m_pal.CreateHalftonePalette( &dc );
	else
	{
		// Create the palette

		RGBQUAD *pRGB = new RGBQUAD[nColors];
		CDC memDC;
		memDC.CreateCompatibleDC(&dc);

		memDC.SelectObject( &m_bitmap );
		::GetDIBColorTable( memDC, 0, nColors, pRGB );

		UINT nSize = sizeof(LOGPALETTE) + (sizeof(PALETTEENTRY) * nColors);
		LOGPALETTE *pLP = (LOGPALETTE *) new BYTE[nSize];

		pLP->palVersion = 0x300;
		pLP->palNumEntries = nColors;

		for( int i=0; i < nColors; i++)
		{
			pLP->palPalEntry[i].peRed = pRGB[i].rgbRed;
			pLP->palPalEntry[i].peGreen = pRGB[i].rgbGreen;
			pLP->palPalEntry[i].peBlue = pRGB[i].rgbBlue;
			pLP->palPalEntry[i].peFlags = 0;
		}

		m_pal.CreatePalette( pLP );

		delete[] pLP;
		delete[] pRGB;
	}
	Invalidate();

	return TRUE;
}



void COnlineUsersTree::SetDefaultCursor()
{
	   // Get the windows directory
        CString strWndDir;
        GetWindowsDirectory(strWndDir.GetBuffer(MAX_PATH), MAX_PATH);
        strWndDir.ReleaseBuffer();

        strWndDir += _T("\\winhlp32.exe");
        // This retrieves cursor #106 from winhlp32.exe, which is a hand pointer
        HMODULE hModule = LoadLibrary(strWndDir);
        if (hModule) {
            HCURSOR hHandCursor = ::LoadCursor(hModule, MAKEINTRESOURCE(106));
            if (hHandCursor)
			{
                cursor_hand = CopyCursor(hHandCursor);
			}
			      
        }
        FreeLibrary(hModule);

		cursor_arr	= ::LoadCursor(NULL, IDC_ARROW);
		cursor_no	= ::LoadCursor(NULL, IDC_NO) ;
}



