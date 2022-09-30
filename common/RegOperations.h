// RegOperations.h: interface for the CRegOperations class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_REGOPERATIONS_H__0ABFE516_CA6E_489C_80A2_DB55783FB266__INCLUDED_)
#define AFX_REGOPERATIONS_H__0ABFE516_CA6E_489C_80A2_DB55783FB266__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <atlbase.h>
#include <atlstr.h>
class CRegOperations  
{
public:
	CRegOperations();
	virtual ~CRegOperations();
	void			SetNickname(CString sNickName);	
	CString			GetNickname();
	DWORD			GetStartVirtualAddress(CString sSessionName);
	CString			GetInstallPath();
	DWORD			EnableRA(BOOL bValue);
	DWORD			EnableRAUnSolicitPolicy(BOOL bValue);
	DWORD			GetMediaServerListeningPort();
	DWORD			GetIdleTimeout();
	void			SetIdleTimeOut(DWORD dwTimeOut);
	USHORT			GetTeredoForwardPort();
	CString			GetPNRPSeedServers();
	void			SetPNRPSeedServer(CString sSeedServer);
private:
	CRegKey			m_RegCurrentUser;
	CRegKey			m_RegLocalMachine;
	static const TCHAR	PNRP_REG_PATH[];

};

#endif // !defined(AFX_REGOPERATIONS_H__0ABFE516_CA6E_489C_80A2_DB55783FB266__INCLUDED_)
