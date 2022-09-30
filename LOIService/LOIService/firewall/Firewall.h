#pragma once
#include <netfw.h>
#include <objbase.h>
#include <oleauto.h>
#include <stdio.h>
#include <atlcomcli.h>
#import "netfw.tlb"

class CFirewall
{
public:
	CFirewall(void);
	~CFirewall(void);

	BOOL				IsRuleAdded(LPCTSTR lpszAppPath);
	HRESULT				AddFirewallRule(LPCTSTR lpszAppPath ) ;
	HRESULT				EnableWindowsFirewall(BOOL bEnable,BOOL  &bPreviousState);
private:
	static TCHAR     UI_APP_NAME[];
	static TCHAR     APP_REG_PATH[];

	INetFwProfile*		WindowsFirewallInitialize();
	void				GetModuleBaseName(LPCTSTR lpszFilePath,LPTSTR lpszAppName,DWORD dwSizeInBytes);
//	HRESULT				CoCreateInstanceAsAdmin(HWND hwnd, REFCLSID rclsid, REFIID riid, __out void ** ppv);
	HRESULT				WindowsXPFirewallAddApp(IN const wchar_t* fwProcessImageFileName,IN const wchar_t* fwName);
	HRESULT				WindowsXPFirewallAppIsEnabled(INetFwProfile		*pFirewallProfile,IN const wchar_t* fwProcessImageFileName,OUT BOOL* fwAppEnabled);
	HRESULT				CheckNatTraversalAndAdd(LPCTSTR lpszAppPath,LPCTSTR lpszRuleName);
	HRESULT				AddVistaFirewallRule(INetFwRule	*pFwRule, INetFwRules *pFwRules,LPCTSTR lpszRuleName,LPCTSTR lpszAppPath);
	HRESULT				CoCreateInstanceAsAdmin(HWND hwnd, REFCLSID rclsid, REFIID riid, __out void ** ppv);
	BOOL				IsNatTraversalAdded(LPCTSTR lpszAppPath);
	BOOL				IsWindowsVistaAndAbove();
	
};