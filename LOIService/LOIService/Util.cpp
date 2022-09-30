#include "StdAfx.h"
#include "Util.h"
#include "LOIApp.h"

int LoadString(UINT uID,LPTSTR lpszBuffer,int iMaxChars)
{
	return LoadString(theApp.GetHInstance(),uID,lpszBuffer,iMaxChars);
}


/********************
Get formatted string in ipv6
********************/
CString GetFormattedIPv6(in_addr6 ipv6addr)
{
	CString sIPv6;

	sIPv6.Format(_T("%X:%X:%X:%X:%X:%X:%X:%X"),
				htons(ipv6addr.u.Word[0]),htons(ipv6addr.u.Word[1]),
				htons(ipv6addr.u.Word[2]),htons(ipv6addr.u.Word[3]),
				htons(ipv6addr.u.Word[4]),htons(ipv6addr.u.Word[5]),
				htons(ipv6addr.u.Word[6]),htons(ipv6addr.u.Word[7])
				);
	return sIPv6;
														
}