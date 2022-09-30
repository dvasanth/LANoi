#include "StdAfx.h"
#include "Hash.h"

CHash::CHash(void)
{
}

CHash::~CHash(void)
{
}


CString CHash::HashString(CString strString)
{
	MD5_CTX			ctx;
	MD5				md5;
	//init md5
	md5.MD5Init(&ctx);
	//update with our string
	md5.MD5Update(&ctx,
		 (unsigned char*)(LPCTSTR)strString,
		 strString.GetLength());
	
	//create the hash
	unsigned char buff[16] = "";	
	md5.MD5Final((unsigned char*)buff,&ctx);

	//converte the hash to a string and return it
	CString  sHash;

	for(int i=0; i<16; i++)
	{
		CString sTemp;
		sTemp.Format(_T("%02x"),buff[i]);
		sHash+=sTemp;
	}	

	return sHash;	

}