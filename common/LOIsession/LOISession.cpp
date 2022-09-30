#include "StdAfx.h"
#include "LOISession.h"




CLOISession::CLOISession(ISessionChangeNotify *pSessionChange,IDynamicSessionUpdateNotify *pDynamicUpdate)
{
	m_State=ACTIVE;
	m_lReferenceCount=1;
	m_PSessionChangeNotify=pSessionChange;
	m_pDynamicSessionNotify=pDynamicUpdate;
	m_pszSessionString=NULL;
	ZeroMemory(m_szSessionName,sizeof(m_szSessionName));
	m_dwLastSessionActivityTime=GetTickCount(); 
}

CLOISession::~CLOISession(void)
{
	if(m_pszSessionString)
	{
		delete m_pszSessionString;
		m_pszSessionString=NULL;
	}
}


DWORD CLOISession::AddRef()
{
	InterlockedIncrement(&m_lReferenceCount );
	return m_lReferenceCount;
}


DWORD CLOISession::Release()
{
	InterlockedDecrement(&m_lReferenceCount );
	if(m_lReferenceCount==0)
	{

		delete this;
		return 0;
	}
	return m_lReferenceCount;
}
void CLOISession::SetState(SESSION_STATE State)
{
	m_State=State;
}

CLOISession::SESSION_STATE CLOISession::GetState()
{
	return m_State;
}
/*************************************************************************
Session name for readable value for the session
**************************************************************************/
void CLOISession::SetSessionName(LPCTSTR lpszSessionName)
{
	StringCbCopy(m_szSessionName,sizeof(m_szSessionName),lpszSessionName);
}
/*****************************
gets the sessionname & copies
*******************************/
void CLOISession::GetSessionName(LPTSTR lpszSessioName,DWORD dwSizeInBytes)
{
	StringCbCopy(lpszSessioName,dwSizeInBytes,m_szSessionName);
}
//cstring is not available in win32 project
#ifdef CSTRING_DEFINED
/****************************************************
parses the session xml string & adds the port resource information
****************************************************/
void CLOISession::ParseSessionXML(LPCTSTR lpszSessionXML)
{
	CLOISessionStringFormat  XMLSession(lpszSessionXML);
	USHORT					usPortStart=0,usPortStop=0;
	BOOL					bService=FALSE;

	//clear the port list first
	m_PortConf.Initialise();
	//port information
	while(XMLSession.GetNextPortValue(usPortStart,usPortStop,bService))
	{
		for(USHORT usPort=usPortStart;usPort<=usPortStop;usPort++)
			m_PortConf.AddPort(htons(usPort),bService);
	}

	//application information
	CString sApplication;
	while(XMLSession.GetNextApplicationName(sApplication))
	{
		m_PortConf.AddPort(0,0,(LPCTSTR)sApplication);
	}

	//dynamic port information
	DWORD		dwBoundAddress=0,dwVirtualAddress=0;
	USHORT		usBoundPort=0;
	while(XMLSession.GetNextDynamicBoundValues(dwBoundAddress,usBoundPort,dwVirtualAddress))
	{
		m_PortConf.AddBindedPort(dwBoundAddress,usBoundPort,dwVirtualAddress) ;
	}
	m_sSessionXML=lpszSessionXML;

	m_dwLastSessionActivityTime=GetTickCount(); 
	//notify the invoker abt the parameter change
	if(m_PSessionChangeNotify)
		m_PSessionChangeNotify->OnSessionValueChange();
}
/******************************
Session xml string for this session
*******************************/
CString CLOISession::GetSessionString()
{
	return m_sSessionXML;
}
#endif

void CLOISession::AddDynamicPortString(LPCTSTR lpszPortString)
{
	if(m_pDynamicSessionNotify)
		m_pDynamicSessionNotify->OnDynamicUpdateChange(this,lpszPortString); 
	m_dwLastSessionActivityTime=GetTickCount();
}

BOOL CLOISession::IsValidPortEntries(LPCTSTR lpszPortList)
{	
	USHORT			  usPort=0;
try{
	LPTSTR			  token,Context=NULL;
	TCHAR			  delimiter[]   =_T(" ,;:");
	LPTSTR			  pszPortList= new TCHAR[_tcslen(lpszPortList)+4];


	
	StringCbCopy(pszPortList,(_tcslen(lpszPortList)+4)*sizeof(TCHAR),lpszPortList);
	
	//parse the port numbers from the string
	token = _tcstok_s(pszPortList, delimiter,&Context );
    while( token != NULL )
	{
	  BOOL        bProxyPortConnect=FALSE;
 	  if(token[0] >='A' && token[0] <='Z')
	  {
		  BOOL  bGotoNextToken=FALSE;
		  //check if the listening app is a service where LSP can't bind
		  switch(token[0])
		  {
			case 'S':
				{
					token++;
				}
				break;
			case 'A'://user don't have idea abt then identify by applicationname
				{
					bGotoNextToken=TRUE;
					usPort=TRUE;
				}
				break;
			default: //command not understood get next
				bGotoNextToken=TRUE;
				break;
		  }
		  if(bGotoNextToken)
		  {
			token = _tcstok_s( NULL, delimiter,&Context );
			continue;
		  }
	  }
	  if(_tcsstr(token,_T("-")))//user asked for port ranges
	  {
		  TCHAR			szStartPort[10]={0},szEndPort[10]={0};
		  _tcsncpy_s(szStartPort,10,token, _tcsstr(token,_T("-"))-token);
		  _tcscpy_s(szEndPort,10,_tcsstr(token,_T("-"))+1);
		  usPort= (_ttoi(szEndPort)>_ttoi(szStartPort));
		  
	  }
	  else
	  {
		usPort=_ttoi(token);
		if(!usPort || usPort>65534)
		{
			usPort=0;
			break;	
		}
	  }
	    /* Get next token: */
	  token = _tcstok_s( NULL, delimiter,&Context );
	}
	if(pszPortList)delete pszPortList;
}
catch(...)
{
	return FALSE;
}
	return usPort;
}
void CLOISession::AddSelfTeredoAddress(in_addr6 IPv6Addr)
{
	m_PeerConf.AddSelfPeerAddress(IPv6Addr);
}
void CLOISession::UpdateSessionPeerInfo(BOOL bAddPeer,LPPEER_INFORMATION pPeerInfo,in_addr6	*pIPv6ClosedTeredoAddress)
{
	if(bAddPeer)
	{
		m_PeerConf.AddPeer(*pPeerInfo);
	}
	else
	{
		m_PeerConf.RemovePeer(*pIPv6ClosedTeredoAddress);
	}
	m_dwLastSessionActivityTime=GetTickCount();
	//notify the invoker abt the parameter change
	if(m_PSessionChangeNotify)
		m_PSessionChangeNotify->OnSessionValueChange();
}


PEER_INFORMATION* CLOISession::GetConnectedClientList(USHORT &usTotalClients)
{
	return m_PeerConf.GetConnectedClientList(usTotalClients);
}
void CLOISession::FreeConnectedClientList(PEER_INFORMATION* pConnectList)
{
	CConnectedPeerListConf::FreeConnectedClientList(pConnectList);
}
/*******************
get the inviter peername from the list of connected users
*******************/
void CLOISession::GetInviterName(LPTSTR lpszServerName,DWORD dwSizeInBytes)
{
	PEER_INFORMATION		*pPeerList;
	USHORT					usPeerListCount=0;

	pPeerList=GetConnectedClientList ( usPeerListCount);
	if(usPeerListCount)
	{
		for(USHORT usCount=0;usCount<usPeerListCount;usCount++)
		{
			if(pPeerList[usCount].dwFlag &PEER_FLAG_ADMIN)
			{
				StringCbCopy(lpszServerName,dwSizeInBytes,pPeerList[usCount].szPeerName); 
				break;
			}
		}
		FreeConnectedClientList(pPeerList); 
	}
}
DWORD CLOISession::GetIpv4Address(in_addr6 IPv6TeredoAddr)
{
	return m_PeerConf.GetIpv4Address(IPv6TeredoAddr);
}

void CLOISession::Serialize(CIPCFileMap& FileMap)
{
	//serialise the sessioname 
	FileMap.SingleUseLock();
	if(FileMap.IsStoring())
	{
		FileMap.Write((LPBYTE)m_szSessionName,sizeof(m_szSessionName));
	}
	else
	{
		FileMap.Read((LPBYTE)m_szSessionName,sizeof(m_szSessionName));
	}
	FileMap.SingleUseUnLock();

	//serialise all other objects in the session
	m_PeerConf.Serialize( FileMap);
	m_PortConf.Serialize(FileMap);  
}

/**********************************************************
checks whether connection needs to be diverted 
**********************************************************/
CONNECT_TYPE CLOISession::IsConnectDivertRequired(LPCTSTR lpszAppName,DWORD dwIPV4AddressToConnect,USHORT usPortToConnect,in_addr6 &IPv6TeredoAddress,USHORT &usDivertedPort)
{
	USHORT          usProxyServerPort;
	DWORD			dwVirtualPeerAddressToConnect=0;

	//check if this a teredo mapped address
	if( m_PortConf.IsPortNeedsTunnelExpose(usPortToConnect,lpszAppName) && //perform divertion only for app & port we are insterested
		m_PeerConf.IsTeredoMappedIpv4Address(dwIPV4AddressToConnect,
											IPv6TeredoAddress,
											usProxyServerPort))
	{
		//is this port require proxy connection
		if(m_PortConf.IsPortRequiresProxyTunnel(usPortToConnect))
		{
			usDivertedPort=usProxyServerPort;
			return PROXY;
		}
		usDivertedPort=usPortToConnect;
		return DIRECT;
	}
	//check if it is a dynamic binded port information
	if(m_PortConf.IsBindedPeerAddress(dwIPV4AddressToConnect,usPortToConnect, dwVirtualPeerAddressToConnect ))
	{
		//chcek whether the client still exists
		if(m_PeerConf.IsTeredoMappedIpv4Address(dwVirtualPeerAddressToConnect, IPv6TeredoAddress ,usProxyServerPort))
		{
			return DIRECT;
		}
	}

	//no change just pass as it is
	return DEFAULT;
}
/******************************************************
checks whether UDP sentto needs to be diverted
******************************************************/
CONNECT_TYPE CLOISession::IsSentoDivertRequired(LPCTSTR lpszAppName,DWORD dwIPV4SentToAddress,USHORT usSentToPort,in_addr6 &IPv6TeredoAddress)
{
	USHORT          usProxyServerPort;
	DWORD			dwVirtualPeerAddressToConnect=0;
	//otherwise check if it is teredo mapped adddres
	//if it is our ipv4 virtual address
	if(m_PortConf.IsPortNeedsTunnelExpose(usSentToPort,lpszAppName) && //perform divertion only for app & port we are insterested
		m_PeerConf.IsTeredoMappedIpv4Address(dwIPV4SentToAddress,
											IPv6TeredoAddress,
											usProxyServerPort))
											return DIRECT;
	//check if it is broadcast address
	if(dwIPV4SentToAddress == 0xffffffff)
	{
		return BROADCAST;
	}
	//check if it is a dynamic binded port information
	if(m_PortConf.IsBindedPeerAddress(dwIPV4SentToAddress,usSentToPort, dwVirtualPeerAddressToConnect ))
	{
		//chcek whether the client still exists
		if(m_PeerConf.IsTeredoMappedIpv4Address(dwVirtualPeerAddressToConnect, IPv6TeredoAddress ,usProxyServerPort))
		{
			return DIRECT;
		}
	}
	return DEFAULT;
}
/**********************************************************
checks whether bind call needs to be forced to teredo address
**********************************************************/
BOOL CLOISession::IsBindToIPv6Required(USHORT usPort,LPCTSTR lpszAppName)
{
	return m_PortConf.IsPortNeedsTunnelExpose(usPort,lpszAppName);
}
/*****************************************
checks whether port exits in this session
******************************************/
BOOL CLOISession::IsPortRequireProxy(USHORT usPort)
{
	return m_PortConf.IsPortRequiresProxyTunnel(usPort);
}
/************************************************
gets the teredo address for binding
************************************************/
in_addr6 CLOISession::GetBindTeredoAddress()
{
	return m_PeerConf.GetSelfTeredoAddress();
}

/*************************************
get the ipv4 address of the name
************************************/
DWORD CLOISession::IsPeername(LPCTSTR lpszPeername,DWORD & dwIpV4Address)
{
	return m_PeerConf.IsPeername(lpszPeername,dwIpV4Address);
}
/******************************
Last client activity
returns minutes from last activity
*******************************/
DWORD CLOISession::GetLastActivity()
{
	return (GetTickCount()- m_dwLastSessionActivityTime)/(60*1000);
}