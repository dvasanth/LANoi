/*****************************************************************************
Module :     URLEncode.H
Notices:     Written 2002 by ChandraSekar Vuppalapati
Description: H URL Encoder
*****************************************************************************/
#ifndef __CURLENCODE_H_
#define __CURLENCODE_H_



class CURLEncode
{
private:
	static CStringA csUnsafeString;
	CStringA decToHex(char num, int radix);
	bool isUnsafe(char compareChar);
	CStringA convert(char val);

public:
	CURLEncode() { };
	virtual ~CURLEncode() { };
	CStringA URLEncode(CString vData);
};

#endif //__CURLENCODE_H_