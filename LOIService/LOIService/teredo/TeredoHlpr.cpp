#include "StdAfx.h"
#include "TeredoHlpr.h"

#include <winsock2.h>
#include <iphlpapi.h>
CTeredoHlpr::CTeredoHlpr(void)
{
	WSADATA					wsaData={0};
	WORD					wVersionRequested = MAKEWORD( 2, 2 );

	CoInitialize(NULL); 
	m_Privilege.EnableDebugPrivilege(); 
	///socket initialise

	WSAStartup( wVersionRequested, &wsaData );
	m_AdapterNotifySock=INVALID_SOCKET;
	m_pAdapterNotifyOverlap=NULL;
	m_AdapterState=DOWN;
	ZeroMemory(&m_IPv6Addr,sizeof(m_IPv6Addr));
	m_bSymmetricNAT=FALSE;
}

CTeredoHlpr::~CTeredoHlpr(void)
{
	CloseAdapter();
	CancelNotifyAdapterChange();
	WSACleanup(); 
	CoUninitialize();

}
/*******************************************************
This initilaises the teredo adapter after handshake with 
********************************************************/
BOOL CTeredoHlpr::StartTeredoAdapter()
{
	TCHAR		szOutput[1024];
	DWORD		dwSizeInBytes=sizeof(szOutput);

	
	if(IsAdapterUp()) 
	{
		return TRUE;
	}

	//stop the thread if running already
	
	//create the thread;
	if(!IsThreadRunning())
	{
		CreateThread();
	}
	return FALSE;
}
/*****************************
Handling the teredo starting process in this thread
*******************************/
DWORD CTeredoHlpr::OnThreadRun()
{
	TCHAR					szOutput[1024]={0};
	DWORD					dwOutLen=sizeof(szOutput);
	CFirewall				Firewall;
	USHORT					usPort=0;
	CRegOperations			TeredoForwardPort;

	if(!IsInternetAvailable())
	{
		//if no internet no use in trying
		return 0;
	}
	//set the firewall first
	Firewall.EnableWindowsFirewall(TRUE,m_bInitialFirewallState);


	//disable other ipv6 connection
	ExecuteNetCommand(_T("netsh interface ipv6 6to4 set state state=disabled"),szOutput,dwOutLen);
	ExecuteNetCommand(_T("netsh interface ipv6 isatap set state disabled"),szOutput,dwOutLen);
	
	DEBUGLOG(LOI_INFO,_T("starting teredo with default parameters"));
	//start the teredo
	ExecuteNetCommand(_T("netsh interface ipv6 set teredo client  teredo.ipv6.microsoft.com 50"),szOutput,dwOutLen);
	//we need wait for teredo to get intialised
	GetTeredoStatus(szOutput,dwOutLen);

	//fix managed unmanged problem
	if(_tcsstr(szOutput,_T("managed"))  && !_tcsstr(szOutput,_T("unmanaged")))
	{
		ExecuteNetCommand(_T("Netsh interface ipv6 set teredo enterpriseclient"),szOutput,dwOutLen);
		GetTeredoStatus(szOutput,dwOutLen);
		DEBUGLOG(LOI_ERROR,_T("teredo in managed"));
	}

	//try fixing the symmetric NAT problem with upnp
	if(_tcsstr(szOutput,_T("symmetric")))
	{
		DEBUGLOG(LOI_ERROR,_T("teredo in symmetric NAT"));
		usPort=TeredoForwardPort.GetTeredoForwardPort();
		if(m_TeredoPortForward.Init(usPort))
		{
			//enable the teredo on the forwarded port
			TCHAR  szCmd[MAX_PATH]={0};
			StringCbPrintf(szCmd,sizeof(szCmd),_T("netsh interface ipv6 set teredo client teredo.ipv6.microsoft.com. 60	%hu"),DWORD(usPort)); 
			ExecuteNetCommand(szCmd,szOutput,dwOutLen);
			GetTeredoStatus(szOutput,dwOutLen);
			DEBUGLOG(LOI_INFO,_T("Port forwarding enabled. cmd %s"),szCmd);
		}
		else
		{
			//upnp not supported no teredo support
			m_bSymmetricNAT=TRUE;
		}
		
	}
	
	//fix server not reachable
	if(_tcsstr(szOutput,_T("secondary server address unreachable")) || _tcsstr(szOutput,_T("failed")) || _tcsstr(szOutput,_T("primary server address unreachable"))) 
	{
		ExecuteNetCommand(_T("netsh interface ipv6 set teredo client 65.55.158.80"),szOutput,dwOutLen);
		GetTeredoStatus(szOutput,dwOutLen);
		DEBUGLOG(LOI_ERROR,_T("teredo in secondary server address "));
	}

	DEBUGLOG(LOI_ERROR,_T("teredo failed %s"),szOutput);
	//fix fail last try
	if(_tcsstr(szOutput,_T("failed"))) 
	{
		DEBUGLOG(LOI_ERROR,_T("teredo failed %s"),szOutput);
		ExecuteNetCommand(_T("netsh interface ipv6 renew"),szOutput,dwOutLen);
		
	}

	return 0;	
}
/*********************
fetches the teredo stable status
***********************/
BOOL CTeredoHlpr::GetTeredoStatus(LPTSTR lpszOutput,DWORD &dwSizeInBytes)
{
	int						iWaitTimes=0;

	do{
		if(WaitForExitMessage(5000))
		{
			return FALSE;
		}

		if(!ExecuteNetCommand(_T("netsh interface ipv6 show teredo"),lpszOutput,dwSizeInBytes))
		{
			return FALSE;
		}
		iWaitTimes++;
	}while(iWaitTimes<10 && _tcsstr(lpszOutput,_T("probe")));

	return TRUE;
}
/******************************************
Close the teredo adapter if requested
******************************************/
void CTeredoHlpr::CloseAdapter()
{
	TCHAR					szOutput[1024]={0};
	DWORD					dwOutLen=sizeof(szOutput);
	CFirewall				Firewall;
	BOOL					bPreviousState;

	Firewall.EnableWindowsFirewall(m_bInitialFirewallState,bPreviousState);
	//disable teredo
	//ExecuteNetCommand(_T("netsh interface ipv6 set teredo disable"),szOutput,dwOutLen);

}
/****************************************************
Notify the teredo adapter changes state enabled & disabled
****************************************************/
BOOL CTeredoHlpr::NotifyAdapterChange(HANDLE hAdapterChangeNotify)
{
	DWORD		RetBytes;

	CancelNotifyAdapterChange();//cancel any existing notifictions
	m_AdapterNotifySock = WSASocket(AF_INET6, SOCK_STREAM, IPPROTO_TCP,NULL,NULL,WSA_FLAG_OVERLAPPED);
	if (m_AdapterNotifySock == INVALID_SOCKET) 
	{

		return FALSE;
	}

	m_pAdapterNotifyOverlap = new  OVERLAPPED;
	m_pAdapterNotifyOverlap->hEvent=hAdapterChangeNotify;
	WSAResetEvent(hAdapterChangeNotify); 
	WSAIoctl(m_AdapterNotifySock,
					SIO_ADDRESS_LIST_CHANGE ,
					NULL,0,
					NULL,0,&RetBytes,
					m_pAdapterNotifyOverlap,
					NULL
					);

	ZeroMemory(&m_IPv6Addr,sizeof(m_IPv6Addr));
	//store the address
	GetTeredoIpv6Address(m_IPv6Addr);
	m_AdapterState=IsAdapterUp()?UP:DOWN;
	return TRUE;
}
/*****************************
Address change store the address when user invokes NotifyAdapterChange
*****************************/
BOOL CTeredoHlpr::IsAddressChanged()
{
	in_addr6 CurrentIPv6Addr={0};
	INT iRet;

	GetTeredoIpv6Address(CurrentIPv6Addr);
	iRet=memcmp(&m_IPv6Addr,&CurrentIPv6Addr,sizeof(in_addr6));
	if(iRet!=0)
	{
		DEBUGLOG(LOI_ERROR,_T("IPv6 new address %X:%X:%X:%X:%X:%X:%X:%X"),
														htons(CurrentIPv6Addr.u.Word[0]),htons(CurrentIPv6Addr.u.Word[1]),
														htons(CurrentIPv6Addr.u.Word[2]),htons(CurrentIPv6Addr.u.Word[3]),
														htons(CurrentIPv6Addr.u.Word[4]),htons(CurrentIPv6Addr.u.Word[5]),
														htons(CurrentIPv6Addr.u.Word[6]),htons(CurrentIPv6Addr.u.Word[7])  	
														);
		DEBUGLOG(LOI_ERROR,_T("IPv6 old address %X:%X:%X:%X:%X:%X:%X:%X"),
														htons(m_IPv6Addr.u.Word[0]),htons(m_IPv6Addr.u.Word[1]),
														htons(m_IPv6Addr.u.Word[2]),htons(m_IPv6Addr.u.Word[3]),
														htons(m_IPv6Addr.u.Word[4]),htons(m_IPv6Addr.u.Word[5]),
														htons(m_IPv6Addr.u.Word[6]),htons(m_IPv6Addr.u.Word[7])  	
														);

	}
	return (iRet!=0);
}
/************************************************
cancels any pending io for notification
************************************************/
void CTeredoHlpr::CancelNotifyAdapterChange()
{
	if(m_AdapterNotifySock!=INVALID_SOCKET)
	{
		CancelIo((HANDLE) m_AdapterNotifySock);
		closesocket(m_AdapterNotifySock);
		Sleep(200);
		m_AdapterNotifySock=INVALID_SOCKET;
		delete m_pAdapterNotifyOverlap;
		m_pAdapterNotifyOverlap=NULL;

	}
}
/*********************************
This gets the teredo status as a string returned from cmd line
********************************/
CString CTeredoHlpr::GetTeredoStatus()
{
	TCHAR					szOutput[1024]={0};
	DWORD					dwOutLen=sizeof(szOutput);

	ExecuteNetCommand(_T("netsh interface ipv6 show teredo"),szOutput,dwOutLen);
	return CString(szOutput);
}
/**************************************************************************
This can execute console command & return the output in a string
**************************************************************************/
BOOL CTeredoHlpr::ExecuteNetCommand(LPCTSTR lpszCommandLine,LPTSTR lpszOutput,DWORD &dwSizeInBytes)
{
	STARTUPINFO					sInfo={0}; 
	PROCESS_INFORMATION			pInfo={0}; 
	BOOL						bRet=FALSE;
	TCHAR						szAppPath[MAX_PATH]={0};
	SECURITY_ATTRIBUTES			sa={0};
	HANDLE						hChildStdOut,hOutputRead;


    // Set up the security attributes struct.
    sa.nLength= sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;


    // Create the child output pipe.
    CreatePipe(&hOutputRead,&hChildStdOut,&sa,0);
  
	//fill start-info
	GetStartupInfo(&sInfo);
	sInfo.cb			= sizeof(sInfo);
	sInfo.dwFlags		= STARTF_USESHOWWINDOW|STARTF_USESTDHANDLES;
	sInfo.wShowWindow	= SW_HIDE;
    sInfo.hStdOutput = hChildStdOut;
	sInfo.hStdInput= GetStdHandle(STD_INPUT_HANDLE);
    sInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);


	//get netsh-path
	GetSystemDirectory(szAppPath, MAX_PATH);
	StringCbCat(szAppPath,sizeof(szAppPath),_T("\\netsh.exe"));

	
	bRet= CreateProcess(szAppPath, (LPTSTR)lpszCommandLine, NULL, NULL, TRUE, 
    CREATE_NEW_CONSOLE, NULL, NULL, &sInfo, &pInfo); 

	if(bRet)
	{
		CHAR				szReadConsole[1024]={0};
		DWORD				dwRead;
		DWORD_PTR			dwRetVal;

		dwRetVal=MsgWaitForEvents(1,&pInfo.hProcess,20000);  
		KillProcess(pInfo.hProcess);
		if(dwRetVal == WAIT_OBJECT_0 )
		{
			BOOL bExit;
			//we have received a message
			//dispatch message
			DispatchMessage( bExit);
			if(bExit)
			{
				CloseHandle(pInfo.hProcess);
				CloseHandle(pInfo.hThread);
				CloseHandle(hOutputRead);
				CloseHandle(hChildStdOut);
				return FALSE;
			}
		}	

		//read the output
		ReadFile(hOutputRead,szReadConsole,sizeof(szReadConsole),&dwRead,NULL);	
		StringCbPrintf(lpszOutput,dwSizeInBytes,_T("%hs"),szReadConsole);
		CloseHandle(pInfo.hProcess);
		CloseHandle(pInfo.hThread);
	}

	CloseHandle(hOutputRead);
	CloseHandle(hChildStdOut);
	return TRUE;


}
/***************
check internet availbilty by resolving host name
*****************/
BOOL CTeredoHlpr::IsInternetAvailable()
{
	struct hostent *pName=NULL;
	pName=gethostbyname("microsoft.com");
	return !(pName==NULL);
}
/***************************************
Returns the adapter status
***************************************/
BOOL CTeredoHlpr::IsAdapterUp()
{
	in_addr6 TeredoIPv6Addr;

	return GetTeredoIpv6Address(TeredoIPv6Addr)==NO_ERROR;
}

/**************************************************************
this returns the teredo ip address
**************************************************************/
DWORD CTeredoHlpr::GetTeredoIpv6Address(in_addr6 &TeredoIPv6Addr)
{
	PIP_ADAPTER_ADDRESSES				pAddresses;
	ULONG								outBufLen = 0;
	DWORD								dwRetVal = 0;

	pAddresses = (IP_ADAPTER_ADDRESSES*) malloc(sizeof(IP_ADAPTER_ADDRESSES));
	// Make an initial call to GetAdaptersAddresses to get the 
	// size needed into the outBufLen variable
	if (GetAdaptersAddresses(AF_INET6, 
							   GAA_FLAG_SKIP_ANYCAST|GAA_FLAG_SKIP_MULTICAST, 
							  NULL, 
							  pAddresses, 
							  &outBufLen) == ERROR_BUFFER_OVERFLOW)
	{
	  free(pAddresses);
	  pAddresses = (IP_ADAPTER_ADDRESSES*) malloc(outBufLen);
	}

	// Make a second call to GetAdapters Addresses to get the
	// actual data we want
	if ((dwRetVal = GetAdaptersAddresses(AF_INET6, 
										  GAA_FLAG_SKIP_ANYCAST|GAA_FLAG_SKIP_MULTICAST, 
										  NULL, 
										  pAddresses, 
										  &outBufLen)) == NO_ERROR)
	{
	  // If successful, output some information from the data we received
		PIP_ADAPTER_ADDRESSES		pIPv6Addresses=pAddresses;
	    while (pIPv6Addresses)
		{
	//		if(_tcsstr(pIPv6Addresses->Description,_T("Teredo")))
			{
				//found teredo adapter
				PIP_ADAPTER_UNICAST_ADDRESS pUnicastAddress=pIPv6Addresses->FirstUnicastAddress;
				while(pUnicastAddress)
				{
					if( pUnicastAddress->Address.iSockaddrLength>=sizeof(sockaddr_in6 ) &&
						pUnicastAddress->ValidLifetime  &&
						((sockaddr_in6*)pUnicastAddress->Address.lpSockaddr)->sin6_addr.u.Word[0] == 0x0120//2001 prefix means assigned by teredo ipv6 route
						)
					{
						//got a teredo address return;
						TeredoIPv6Addr= ((sockaddr_in6*)pUnicastAddress->Address.lpSockaddr)->sin6_addr;
						free(pAddresses);
						
						return NO_ERROR;
					}//pUnicastAddress
					pUnicastAddress=pUnicastAddress->Next;
				}//while
			}//_tcsstr
			pIPv6Addresses = pIPv6Addresses->Next;
		}//while
	}//dwRetVal

	free(pAddresses);
	//give enough information abt teredo failure
	SetLastErrorMessage(IDS_TEREDO_NOT_FOUND);
	return ERROR_INVALID_ADDRESS;
}