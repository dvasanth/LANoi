// SuptEdit.cpp : implementation file
//

#include "stdafx.h"
#include "SuptEdit.h"


// CSuptEdit

IMPLEMENT_DYNAMIC(CSuptEdit, CEdit)

CSuptEdit::CSuptEdit()
{

}

CSuptEdit::~CSuptEdit()
{
}


BEGIN_MESSAGE_MAP(CSuptEdit, CEdit)
	ON_WM_PAINT()
END_MESSAGE_MAP()



// CSuptEdit message handlers
void CSuptEdit::OnPaint()
{
	CPaintDC dc(this);
	CString strText;
	GetWindowText(strText);
	if((GetFocus()->GetSafeHwnd() != this->GetSafeHwnd()) && (strText.GetLength() <=0))
	{
		CRect rect;
		GetClientRect(rect);
		dc.SetBkMode(TRANSPARENT);
		CBrush br;
		br.CreateSolidBrush(RGB(255,255,255));
		dc.FillRect(rect,&br);
		dc.SelectObject(GetFont());
		rect.InflateRect(-1,-1);
		dc.SetTextColor(RGB(170,170,170));
		dc.DrawText(m_strText.GetBuffer(0),rect,DT_SINGLELINE);
	}
}

void CSuptEdit::InitString(CString strText)
{
	m_strText = strText;
}
