#pragma once

class CDebugPrivilege{
private:
	HANDLE m_hProcessToken;
public:
	CDebugPrivilege()
	{
	}
	~CDebugPrivilege()
	{
		DisableDebugPrivilege();
	}

	/*************************************
	Enables debug privilege to get module names
	*************************************/
	BOOL EnableDebugPrivilege()
	{
		if(!OpenProcessToken(GetCurrentProcess(),  TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &m_hProcessToken))
		{
			if (GetLastError() == ERROR_NO_TOKEN)
			{
				if (!ImpersonateSelf(SecurityImpersonation))
				return FALSE;

				if(!OpenProcessToken(GetCurrentProcess(),  TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &m_hProcessToken))
				{
					DEBUGLOG(LOI_ERROR,_T("OpenProcessToken"));
					return FALSE;
				}
			 }
			else
				return FALSE;
		 }

		// enable SeDebugPrivilege
		if(!SetPrivilege(m_hProcessToken, SE_DEBUG_NAME, TRUE))
		{
			DEBUGLOG(LOI_ERROR,_T("SetPrivilege"));

			// close token handle
			CloseHandle(m_hProcessToken);
			m_hProcessToken=NULL;
			// indicate failure
			return FALSE;
		}

		return TRUE;
	}
	/**************************************************
	Enabling debug privilege
	**************************************************/
	BOOL SetPrivilege(HANDLE hToken,          // token handle
											LPCTSTR Privilege,      // Privilege to enable/disable
											BOOL bEnablePrivilege   // TRUE to enable.  FALSE to disable
											)
	{
		TOKEN_PRIVILEGES tp;
		LUID luid;
		TOKEN_PRIVILEGES tpPrevious;
		DWORD cbPrevious=sizeof(TOKEN_PRIVILEGES);

		if(!LookupPrivilegeValue( NULL, Privilege, &luid )) return FALSE;

		// 
		// first pass.  get current privilege setting
		// 
		tp.PrivilegeCount           = 1;
		tp.Privileges[0].Luid       = luid;
		tp.Privileges[0].Attributes = 0;

		AdjustTokenPrivileges(
				hToken,
				FALSE,
				&tp,
				sizeof(TOKEN_PRIVILEGES),
				&tpPrevious,
				&cbPrevious
				);

		if (GetLastError() != ERROR_SUCCESS) return FALSE;

		// 
		// second pass.  set privilege based on previous setting
		// 
		tpPrevious.PrivilegeCount       = 1;
		tpPrevious.Privileges[0].Luid   = luid;

		if(bEnablePrivilege) {
			tpPrevious.Privileges[0].Attributes |= (SE_PRIVILEGE_ENABLED);
		}
		else {
			tpPrevious.Privileges[0].Attributes ^= (SE_PRIVILEGE_ENABLED &
				tpPrevious.Privileges[0].Attributes);
		}

		AdjustTokenPrivileges(
				hToken,
				FALSE,
				&tpPrevious,
				cbPrevious,
				NULL,
				NULL
				);

		if (GetLastError() != ERROR_SUCCESS) return FALSE;

		return TRUE;
	}
	/********************************************
	Disable the debug privilege
	********************************************/
	BOOL DisableDebugPrivilege()
	{
		if(m_hProcessToken)
		{
			CloseHandle(m_hProcessToken);
			m_hProcessToken=NULL;
		}
		return TRUE;
	}
};
