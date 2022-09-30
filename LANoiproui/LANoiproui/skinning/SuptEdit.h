#pragma once


// CSuptEdit

class CSuptEdit : public CEdit
{
	DECLARE_DYNAMIC(CSuptEdit)

public:
	CSuptEdit();
	virtual ~CSuptEdit();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
private:
	CString m_strText;
public:
	void InitString(CString strText);
};


