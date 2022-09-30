#include "StdAfx.h"
#include "Firewall.h"
#include <strsafe.h>
TCHAR    CFirewall::UI_APP_NAME[]=_T("LANOnInternet.exe");
TCHAR    CFirewall::APP_REG_PATH[]=_T("SOFTWARE\\CrowSoft\\Lan On Internet");

CFirewall::CFirewall(void)
{
	CoInitialize(NULL);
}

CFirewall::~CFirewall(void)
{
	CoUninitialize();
} 
INetFwProfile* CFirewall::WindowsFirewallInitialize()
{
    HRESULT					 hr = S_OK;
    INetFwMgr				*fwMgr = NULL;
    INetFwPolicy			*fwPolicy = NULL;
	INetFwProfile			*pFirewallProfile;

    pFirewallProfile = NULL;

    // Create an instance of the firewall settings manager.
    hr = CoCreateInstance(
            __uuidof(NetFwMgr),
            NULL,
            CLSCTX_INPROC_SERVER,
            __uuidof(INetFwMgr),
            (void**)&fwMgr
            );
    if (FAILED(hr))
    {
        DEBUGLOG(LOI_ERROR,_T("CoCreateInstance failed: 0x%08lx"), hr);
        goto error;
    }

    // Retrieve the local firewall policy.
    hr = fwMgr->get_LocalPolicy(&fwPolicy);
    if (FAILED(hr))
    {
        DEBUGLOG(LOI_ERROR,_T("get_LocalPolicy failed: 0x%08lx"), hr);
        goto error;
    }

    // Retrieve the firewall profile currently in effect.
    hr = fwPolicy->get_CurrentProfile(&pFirewallProfile);
    if (FAILED(hr))
    {
        DEBUGLOG(LOI_ERROR,_T("get_CurrentProfile failed: 0x%08lx"), hr);
        goto error;
    }

error:

    // Release the local firewall policy.
    if (fwPolicy != NULL)
    {
        fwPolicy->Release();
    }

    // Release the firewall settings manager.
    if (fwMgr != NULL)
    {
        fwMgr->Release();
    }

    return pFirewallProfile;
}

HRESULT CFirewall::EnableWindowsFirewall(BOOL bEnable,BOOL  &bPreviousState)
{
    HRESULT				hr = S_OK;
    VARIANT_BOOL		fwEnabled;
	INetFwProfile		*pFirewallProfile=NULL;

	
    bPreviousState= FALSE;
	pFirewallProfile=WindowsFirewallInitialize();
	if(!pFirewallProfile)
	{
		hr=E_FAIL;
		goto error;
	}

    // Get the current state of the firewall.
    hr = pFirewallProfile->get_FirewallEnabled(&fwEnabled);
    if (FAILED(hr))
    {
        DEBUGLOG(LOI_ERROR,_T("get_FirewallEnabled failed: 0x%08lx"), hr);
        goto error;
    }
    // Check to see if the firewall is on.
	bPreviousState=fwEnabled != VARIANT_FALSE?TRUE:FALSE;

    if (bEnable )
    {
        // If it is not on & enable flag, turn it on.
		if(!bPreviousState)
		{ 
			// Turn the firewall on.
			hr = pFirewallProfile->put_FirewallEnabled(VARIANT_TRUE);
			if (FAILED(hr))
			{
				DEBUGLOG(LOI_ERROR,_T("put_FirewallEnabled failed: 0x%08lx"), hr);
				goto error;
			}
		}
    }
	
	if(!bEnable )
	{
		//if it on &  then turn it off
		if(bPreviousState)
		{
			hr = pFirewallProfile->put_FirewallEnabled(VARIANT_FALSE);
			if (FAILED(hr))
	        {
		        DEBUGLOG(LOI_ERROR,_T("put_FirewallEnabled failed: 0x%08lx"), hr);
			    goto error;
			}
		}
	}

error:
	// Release the firewall profile.
    if (pFirewallProfile != NULL)
    {
        pFirewallProfile->Release();
    }
    return hr;
}

HRESULT CFirewall::AddFirewallRule(LPCTSTR lpszAppPath ) 
{ 
	TCHAR			szAppName[MAX_PATH]={0};
	HRESULT			hrResult=E_FAIL;

	if(IsRuleAdded( lpszAppPath))
	{
		return S_OK;
	}

	GetModuleBaseName(lpszAppPath,szAppName,sizeof(szAppName)); 


	if(IsWindowsVistaAndAbove())
	{
		hrResult=CheckNatTraversalAndAdd(lpszAppPath,szAppName);
	}
	else
	{
		hrResult= WindowsXPFirewallAddApp(lpszAppPath,szAppName);
	}
	return hrResult;

}

BOOL CFirewall::IsRuleAdded(LPCTSTR lpszAppPath)
{	
	BOOL				 bAppEnabled=FALSE;
	if(IsWindowsVistaAndAbove())
	{
		return IsNatTraversalAdded(lpszAppPath);
	}

	//default xp operation
	INetFwProfile		*pFirewallProfile=NULL;
	pFirewallProfile=WindowsFirewallInitialize();
	if(pFirewallProfile)
	{
		WindowsXPFirewallAppIsEnabled(pFirewallProfile,  lpszAppPath,&bAppEnabled);
		pFirewallProfile->Release();
	}
	return bAppEnabled; 

}

void CFirewall::GetModuleBaseName(LPCTSTR lpszFilePath,LPTSTR lpszAppName,DWORD dwSizeInBytes)
{
	if(_tcsrchr(lpszFilePath,'\\'))
	StringCbCopy(lpszAppName,dwSizeInBytes,_tcsrchr(lpszFilePath,'\\')+1);
	_tcslwr_s(lpszAppName,dwSizeInBytes/sizeof(lpszFilePath[0]));
}
HRESULT CFirewall::WindowsXPFirewallAddApp(
												IN const wchar_t* fwProcessImageFileName,
												IN const wchar_t* fwName
												)
{
    HRESULT hr = S_OK;
    BOOL fwAppEnabled;
    BSTR fwBstrName = NULL;
    BSTR fwBstrProcessImageFileName = NULL;
    INetFwAuthorizedApplication* fwApp = NULL;
    INetFwAuthorizedApplications* fwApps = NULL;
	INetFwProfile		*pFirewallProfile=NULL;

    _ASSERT(fwProcessImageFileName != NULL);
    _ASSERT(fwName != NULL);

	pFirewallProfile=WindowsFirewallInitialize();
	if(!pFirewallProfile)
	{
		hr=E_FAIL;
		goto error; 
	}

    // First check to see if the application is already authorized.
    hr = WindowsXPFirewallAppIsEnabled(
			pFirewallProfile,
            fwProcessImageFileName,
            &fwAppEnabled
            );
    if (FAILED(hr))
    {
        DEBUGLOG(LOI_ERROR,_T("WindowsFirewallAppIsEnabled failed: 0x%08lx"), hr);
        goto error;
    }

    // Only add the application if it isn't already authorized.
    if (!fwAppEnabled)
    {
        // Retrieve the authorized application collection.
        hr = pFirewallProfile->get_AuthorizedApplications(&fwApps);
        if (FAILED(hr))
        {
             DEBUGLOG(LOI_ERROR,_T("get_AuthorizedApplications failed: 0x%08lx"), hr);
            goto error;
        }

        // Create an instance of an authorized application.
        hr = CoCreateInstance(
                __uuidof(NetFwAuthorizedApplication),
                NULL,
                CLSCTX_INPROC_SERVER,
                __uuidof(INetFwAuthorizedApplication),
                (void**)&fwApp
                );
        if (FAILED(hr))
        {
             DEBUGLOG(LOI_ERROR,_T("CoCreateInstance failed: 0x%08lx"), hr);
            goto error;
        }

        // Allocate a BSTR for the process image file name.
        fwBstrProcessImageFileName = SysAllocString(fwProcessImageFileName);
        if (SysStringLen(fwBstrProcessImageFileName) == 0)
        {
            hr = E_OUTOFMEMORY;
             DEBUGLOG(LOI_ERROR,_T("SysAllocString failed: 0x%08lx"), hr);
            goto error;
        }

        // Set the process image file name.
        hr = fwApp->put_ProcessImageFileName(fwBstrProcessImageFileName);
        if (FAILED(hr))
        {
            DEBUGLOG(LOI_ERROR,_T("put_ProcessImageFileName failed: 0x%08lx"), hr);
            goto error;
        }

        // Allocate a BSTR for the application friendly name.
        fwBstrName = SysAllocString(fwName);
        if (SysStringLen(fwBstrName) == 0)
        {
            hr = E_OUTOFMEMORY;
            DEBUGLOG(LOI_ERROR,_T("SysAllocString failed: 0x%08lx"), hr);
            goto error;
        }

        // Set the application friendly name.
        hr = fwApp->put_Name(fwBstrName);
        if (FAILED(hr))
        {
            DEBUGLOG(LOI_ERROR,_T("put_Name failed: 0x%08lx"), hr);
            goto error;
        }

        // Add the application to the collection.
        hr = fwApps->Add(fwApp);
        if (FAILED(hr))
        {
            DEBUGLOG(LOI_ERROR,_T("Add failed: 0x%08lx"), hr);
            goto error;
        }

        
    }

error:

    // Free the BSTRs.
    SysFreeString(fwBstrName);
    SysFreeString(fwBstrProcessImageFileName);

    // Release the authorized application instance.
    if (fwApp != NULL)
    {
        fwApp->Release();
    }

    // Release the authorized application collection.
    if (fwApps != NULL)
    {
        fwApps->Release();
    }
	// Release the firewall profile.
    if (pFirewallProfile != NULL)
    {
        pFirewallProfile->Release();
    }
    return hr;
}

HRESULT CFirewall::WindowsXPFirewallAppIsEnabled(
														INetFwProfile		*pFirewallProfile,
														IN const wchar_t* fwProcessImageFileName,
														OUT BOOL* fwAppEnabled
														)
{
    HRESULT hr = S_OK;
    BSTR fwBstrProcessImageFileName = NULL;
    VARIANT_BOOL fwEnabled;
    INetFwAuthorizedApplication* fwApp = NULL;
    INetFwAuthorizedApplications* fwApps = NULL;

    _ASSERT(fwProcessImageFileName != NULL);
    _ASSERT(fwAppEnabled != NULL);

    *fwAppEnabled = FALSE;
	if(!pFirewallProfile)
	{
		hr=E_FAIL;
		goto error; 
	}

    // Retrieve the authorized application collection.
    hr = pFirewallProfile->get_AuthorizedApplications(&fwApps);
    if (FAILED(hr))
    {
        //printf("get_AuthorizedApplications failed: 0x%08lx\n", hr);
        goto error;
    }

    // Allocate a BSTR for the process image file name.
    fwBstrProcessImageFileName = SysAllocString(fwProcessImageFileName);
    if (SysStringLen(fwBstrProcessImageFileName) == 0)
    {
        hr = E_OUTOFMEMORY;
        //printf("SysAllocString failed: 0x%08lx\n", hr);
        goto error;
    }

    // Attempt to retrieve the authorized application.
    hr = fwApps->Item(fwBstrProcessImageFileName, &fwApp);
    if (SUCCEEDED(hr))
    {
        // Find out if the authorized application is enabled.
        hr = fwApp->get_Enabled(&fwEnabled);
        if (FAILED(hr))
        {
          //  printf("get_Enabled failed: 0x%08lx\n", hr);
            goto error;
        }

        if (fwEnabled != VARIANT_FALSE)
        {
            // The authorized application is enabled.
            *fwAppEnabled = TRUE;
        }

    }
    else
    {
        // The authorized application was not in the collection.
        hr = S_OK;

   
    }

error:

    // Free the BSTR.
    SysFreeString(fwBstrProcessImageFileName);

    // Release the authorized application instance.
    if (fwApp != NULL)
    {
        fwApp->Release();
    }

    // Release the authorized application collection.
    if (fwApps != NULL)
    {
        fwApps->Release();
    }

    return hr;
}

BOOL CFirewall::IsNatTraversalAdded(LPCTSTR lpszAppPath)
{

	BOOL				bExists = FALSE;

try 
{
	HRESULT			hResult = S_OK; 
	ULONG			cFetched = 0; 
	CComVariant		var;
	BSTR			bstrVal;
    IUnknown		*pEnumerator = NULL;
	IEnumVARIANT*	pVariant = NULL;
	INetFwPolicy2	*pNetFwPolicy2 = NULL;
	INetFwRules		*pFwRules = NULL;
	INetFwRule		*pFwRule = NULL;

	

	hResult = CoCreateInstance(__uuidof(NetFwPolicy2),
									NULL, 
									CLSCTX_INPROC_SERVER, 
									__uuidof(INetFwPolicy2), 
									(void **)&pNetFwPolicy2);

	if(FAILED (hResult))
				goto Cleanup;
	// Retrieve INetFwRules
	hResult = pNetFwPolicy2->get_Rules(&pFwRules);
	if (FAILED(hResult))
		goto Cleanup;
	
	// Iterate through all of the rules in pFwRules
	pFwRules->get__NewEnum(&pEnumerator);
	if(pEnumerator)
	{
		hResult = pEnumerator->QueryInterface(__uuidof(IEnumVARIANT), (void **) &pVariant);
	}
	while(SUCCEEDED(hResult) && hResult != S_FALSE)
	{
		var.Clear();
		hResult = pVariant->Next(1, &var, &cFetched);
		if (S_FALSE != hResult)
		{
			if (SUCCEEDED(hResult))
			{
				hResult = var.ChangeType(VT_DISPATCH);
			}
			if (SUCCEEDED(hResult))
			{
				hResult = (V_DISPATCH(&var))->QueryInterface(__uuidof(INetFwRule), reinterpret_cast<void**>(&pFwRule));
			}
			if (SUCCEEDED(hResult))
			{
				if (SUCCEEDED(pFwRule->get_Name(&bstrVal)))
				{
					if(_tcsicmp(bstrVal,lpszAppPath) == 0)
					{
						bExists = TRUE;
						VARIANT_BOOL vbEdgeTrav = VARIANT_FALSE;
						pFwRule->get_EdgeTraversal(&vbEdgeTrav);
						if(vbEdgeTrav == VARIANT_TRUE)
										bExists=TRUE;

						goto Cleanup;
					}
				}
			}
		}
	}
Cleanup:
	SysFreeString(bstrVal);
	if(pNetFwPolicy2 != NULL)
		pNetFwPolicy2->Release();
	if(pFwRules != NULL)
		pFwRules->Release();
	if(pFwRule != NULL)
		pFwRule->Release();
} 
catch(...) 
{
	return bExists;
} 
return bExists;
}

HRESULT CFirewall::CheckNatTraversalAndAdd(LPCTSTR lpszAppPath,LPCTSTR lpszRuleName)
{
	HRESULT				hResult = S_OK; 

try 
{
	ULONG			cFetched = 0; 
	CComVariant		var;
	BSTR			bstrVal;
    IUnknown		*pEnumerator = NULL;
	IEnumVARIANT*	pVariant = NULL;
	INetFwPolicy2	*pNetFwPolicy2 = NULL;
	INetFwRules		*pFwRules = NULL;
	INetFwRule		*pFwRule = NULL;
	BOOL			bExists = FALSE;
	

	hResult = CoCreateInstance(__uuidof(NetFwPolicy2),
									NULL, 
									CLSCTX_INPROC_SERVER, 
									__uuidof(INetFwPolicy2), 
									(void **)&pNetFwPolicy2);

	if(FAILED (hResult))
	{
		DEBUGLOG(LOI_ERROR,_T("CoCreateInstance INetFwPolicy2 failed %X"),hResult);
		goto Cleanup;
	}
	// Retrieve INetFwRules
	hResult = pNetFwPolicy2->get_Rules(&pFwRules);
	if (FAILED(hResult))
	{
		DEBUGLOG(LOI_ERROR,_T("pNetFwPolicy2 get_Rules failed %X"),hResult);
		goto Cleanup;
	}
	
	// Iterate through all of the rules in pFwRules
	pFwRules->get__NewEnum(&pEnumerator);
	if(pEnumerator)
	{
		hResult = pEnumerator->QueryInterface(__uuidof(IEnumVARIANT), (void **) &pVariant);
	}
	while(SUCCEEDED(hResult) && hResult != S_FALSE)
	{
		var.Clear();
		hResult = pVariant->Next(1, &var, &cFetched);
		if (S_FALSE != hResult)
		{
			if (SUCCEEDED(hResult))
			{
				hResult = var.ChangeType(VT_DISPATCH);
			}
			if (SUCCEEDED(hResult))
			{
				hResult = (V_DISPATCH(&var))->QueryInterface(__uuidof(INetFwRule), reinterpret_cast<void**>(&pFwRule));
			}
			if (SUCCEEDED(hResult))
			{
				if (SUCCEEDED(pFwRule->get_Name(&bstrVal)))
				{
					if(_tcsicmp(bstrVal,lpszAppPath) == 0)
					{
						bExists = TRUE;
						VARIANT_BOOL vbEdgeTrav = VARIANT_FALSE;
						pFwRule->get_EdgeTraversal(&vbEdgeTrav);
						if(vbEdgeTrav != VARIANT_TRUE)
							AddVistaFirewallRule(pFwRule,pFwRules,lpszRuleName,lpszAppPath);
						goto Cleanup;
					}
				}
			}
		}
	}
	AddVistaFirewallRule(pFwRule,pFwRules,lpszRuleName,lpszAppPath);
Cleanup:
	SysFreeString(bstrVal);
	if(pNetFwPolicy2 != NULL)
		pNetFwPolicy2->Release();
	if(pFwRules != NULL)
		pFwRules->Release();
	if(pFwRule != NULL)
		pFwRule->Release();
} 
catch(...) 
{
	return hResult;
} 
return hResult;
}
HRESULT CFirewall::AddVistaFirewallRule(INetFwRule	*pFwRule, INetFwRules *pFwRules,LPCTSTR lpszRuleName,LPCTSTR lpszAppPath)
{
	INetFwRule		*sipFwRule = NULL;
	long			lCurrentProfileTypes = 0;
	HRESULT			hResult = S_OK;

	BSTR bstrRuleName = SysAllocString(lpszAppPath);
	BSTR bstrRuleDescription = SysAllocString(lpszRuleName);
	hResult =  pFwRule->get_Profiles(&lCurrentProfileTypes);

	hResult = CoCreateInstance(__uuidof(NetFwRule),
									NULL,
									CLSCTX_INPROC_SERVER,
									__uuidof(INetFwRule),
									(void**)&sipFwRule);

	if (FAILED(hResult))
	{
	   DEBUGLOG(LOI_ERROR,_T("CoCreateInstance INetFwRule failed %X"),hResult);
	   goto Cleanup;
	}
	// Set the remaining rule properties 
	sipFwRule->put_Name(bstrRuleDescription);
	sipFwRule->put_ApplicationName(bstrRuleName);
	sipFwRule->put_Description(bstrRuleName);

	//sipFwRule->put_RemoteAddresses(L"");
	sipFwRule->put_Profiles(lCurrentProfileTypes);
	sipFwRule->put_Action(NET_FW_ACTION_ALLOW);
	sipFwRule->put_Enabled(VARIANT_TRUE);
	sipFwRule->put_EdgeTraversal(VARIANT_TRUE);

	hResult =pFwRules->Add(sipFwRule);
	if(sipFwRule != NULL)
		sipFwRule->Release();

Cleanup:
	// Free BSTR's
    SysFreeString(bstrRuleName);
    SysFreeString(bstrRuleDescription);
	return hResult;
}

// Add settings for the demo server application to the Windows Firewall 
HRESULT CFirewall::CoCreateInstanceAsAdmin(HWND hwnd, REFCLSID rclsid, REFIID riid, __out void ** ppv)
{
    BIND_OPTS3 bo;
    WCHAR  wszCLSID[50];
    WCHAR  wszMonikerName[300];

    StringFromGUID2(rclsid, wszCLSID, sizeof(wszCLSID)/sizeof(wszCLSID[0])); 
    HRESULT hr = StringCchPrintf(wszMonikerName, sizeof(wszMonikerName)/sizeof(wszMonikerName[0]), L"Elevation:Administrator!new:%s", wszCLSID);
    if (FAILED(hr))
        return hr;
    memset(&bo, 0, sizeof(bo));
    bo.cbStruct = sizeof(bo);
    bo.hwnd = hwnd;
    bo.dwClassContext  = CLSCTX_LOCAL_SERVER;
    return CoGetObject(wszMonikerName, &bo, riid, ppv);
}
BOOL CFirewall::IsWindowsVistaAndAbove()
{
	OSVERSIONINFO osvi;
    
    ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    GetVersionEx(&osvi);
	//WindowsVista or Later
    if(osvi.dwMajorVersion >= 6)
		return TRUE;
	return FALSE;
}

