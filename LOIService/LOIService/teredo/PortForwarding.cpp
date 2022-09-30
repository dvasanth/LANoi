#include "StdAfx.h"
#include "PortForwarding.h"
#include <iphlpapi.h>
#include <strsafe.h>
/************************
const
**************************/
const CString	CPortForwarding::PROTOCOL_NAME=_T("UDP");
const CString	CPortForwarding::MAP_DESCRIPTION=_T("LANoi");

CPortForwarding::CPortForwarding(void)
{
	m_pUPnPNAT=NULL;
	m_PortMapCollection=NULL;
	m_usForwardingPort=0;
	m_pPortForwardedMap=NULL;
	CoInitialize(NULL); // Must be NULL 
}

CPortForwarding::~CPortForwarding(void)
{
	Close();
	CoUninitialize(); 
}
/*************************************
Initialise the NAT manager 
**************************************/
BOOL CPortForwarding::Init(USHORT  usExternalPort)
{
	HRESULT			hResult;


 	Close();//close any existing previous information
	m_usForwardingPort=usExternalPort;


    // Access the IUPnPNAT COM interface, has Windows send UPnP messages to the NAT router 
    hResult = CoCreateInstance(__uuidof(UPnPNAT), 
								NULL,
								CLSCTX_ALL, 
								__uuidof(IUPnPNAT), 
								(void **)&m_pUPnPNAT
								); 
	if(SUCCEEDED(hResult))
	{
		hResult=m_pUPnPNAT->get_StaticPortMappingCollection(&m_PortMapCollection);

		if(SUCCEEDED(hResult) && m_PortMapCollection)
		{
			hResult=ForwardForInternalAddress(m_PortMapCollection,
											  m_usForwardingPort,
											  &m_pPortForwardedMap
											  );
			if(SUCCEEDED(hResult))	return TRUE;
		}
		
	}


	return FALSE;
}
/*******************************************
remove the NAT mapping,Close the nat manager &  free resource
*******************************************/
void CPortForwarding::Close()
{
	if(m_PortMapCollection)
	{
		m_PortMapCollection->Remove(m_usForwardingPort,PROTOCOL_NAME.AllocSysString());
		m_PortMapCollection->Release();
		m_PortMapCollection=NULL;
	}
	if(m_pPortForwardedMap)
	{
		m_pPortForwardedMap->Release();
		m_pPortForwardedMap=NULL;
	}

	if(m_pUPnPNAT)
	{
		m_pUPnPNAT->Release();
		m_pUPnPNAT=NULL;
	}

}

/******************************************
Adds the forwarded the port for one by one to all adapter address 
since we don't the local address range of the NAT device incase if the system has multiple adapters
*******************************************/
HRESULT CPortForwarding::ForwardForInternalAddress(IStaticPortMappingCollection *pPortMapCollection,USHORT usForwardingPort,IStaticPortMapping	**pPortForwardedMap)
{
	PIP_ADAPTER_INFO		pAdapterInfo=NULL;
	PIP_ADAPTER_INFO		pAdapter = NULL;
	DWORD					dwRetVal = 0;
	ULONG					ulOutBufLen;
	DWORD					dwFirstLocalAddr=0;
	HRESULT					hResult;


	pAdapterInfo = (IP_ADAPTER_INFO *) malloc( sizeof(IP_ADAPTER_INFO) );
	ulOutBufLen = sizeof(IP_ADAPTER_INFO);
	// Make an initial call to GetAdaptersInfo to get
	// the necessary size into the ulOutBufLen variable
	if (GetAdaptersInfo( pAdapterInfo, &ulOutBufLen) != ERROR_SUCCESS) 
	{
		free (pAdapterInfo);
	    pAdapterInfo = (IP_ADAPTER_INFO *) malloc (ulOutBufLen);
	}
	//enumerate all adapters
	if ((dwRetVal = GetAdaptersInfo( pAdapterInfo, &ulOutBufLen)) == NO_ERROR) 
	{
		pAdapter = pAdapterInfo;
		while (pAdapter) 
		{
			TCHAR   szAddString[MAX_PATH]={0};
			StringCbPrintf(szAddString,sizeof(szAddString),_T("%hs"),pAdapter->IpAddressList.IpAddress.String); 

			if(CString(szAddString) != _T("0.0.0.0") )
			{
				hResult=pPortMapCollection->Add(usForwardingPort,
												PROTOCOL_NAME.AllocSysString(),
												usForwardingPort,
												CString(szAddString).AllocSysString(),
												TRUE,
												MAP_DESCRIPTION.AllocSysString(),
												pPortForwardedMap);
				if(SUCCEEDED(hResult))
				{
					
					break;
				}
				DEBUGLOG(LOI_ERROR,_T(" CPortForwarding::ForwardForInternalAddress failed mapped addr %s"),szAddString);
				
			}
			pAdapter = pAdapter->Next;
		}
	}

	if(pAdapterInfo)
			free (pAdapterInfo);
	

	return hResult;
	
}