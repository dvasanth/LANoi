#pragma once
#include "debugprivilege.h"
#include "../firewall/Firewall.h"
#include "error.h"
#include "PortForwarding.h"
#include "RegOperations.h"
#include "thread.h"
class CTeredoHlpr:public CError,
				  public IThread
{
public:
	enum ADAPTER_STATE{
		DOWN,
		UP
	};
	CTeredoHlpr(void);
	~CTeredoHlpr(void);
	BOOL			StartTeredoAdapter();
	void			CloseAdapter();
	BOOL			NotifyAdapterChange(HANDLE hAdapterChangeNotify);
	void			CancelNotifyAdapterChange();
	CString			GetTeredoStatus();
	BOOL			IsAdapterUp();
	DWORD			GetTeredoIpv6Address(in_addr6 &TeredoIPv6Addr);	
	BOOL			IsAddressChanged();
	inline BOOL IsSymmetricNAT()
	{
		return m_bSymmetricNAT;
	}
private:
	BOOL					m_bSymmetricNAT;
	CDebugPrivilege			m_Privilege;
	in_addr6				m_IPv6Addr;
	///overlapp struct to notify adapter change
	SOCKET					m_AdapterNotifySock;
	OVERLAPPED				*m_pAdapterNotifyOverlap;
	ADAPTER_STATE			m_AdapterState;
	BOOL					m_bInitialFirewallState;
	CPortForwarding			m_TeredoPortForward;
	virtual DWORD			OnThreadRun();
	BOOL					ExecuteNetCommand(LPCTSTR lpszCommandLine,LPTSTR lpszOutput,DWORD &dwSizeInBytes);
	BOOL					GetTeredoStatus(LPTSTR lpszOutput,DWORD &dwSizeInBytes);
	BOOL					IsInternetAvailable();
	
	inline void KillProcess(HANDLE hProcess)
	{
		DWORD dwExitCode;

		GetExitCodeProcess(hProcess,&dwExitCode);
		TerminateProcess(hProcess,dwExitCode); 
	}
};
