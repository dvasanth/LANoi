#pragma once
#include "loisession.h"
#include "../threadlock.h"
#include "../mtsmap.h"
//win32 don't have cstring
#ifdef CString
	#include "loisession\LOISessionStringFormat.h"
#endif

typedef struct __UI_TO_LSP_IPC_INFO {
		TCHAR      szIPCName[MAX_PATH];
		DWORD	   dwMaxSize;
} UI_TO_LSP_IPC_INFO ,*LPUI_TO_LSP_IPC_INFO;

enum IPC_ITEM{
		SESSION_ITEM=0,
		DYNAMIC_SESSION_UPDATE_ITEM,
		TOTAL_ITEMS
	};

typedef struct __SESSION_HEADER {
		TCHAR      szSessionID[MAX_PATH];
} SESSION_HEADER ,*LPSESSION_HEADER;


typedef struct __SESSION_INFORMATION {
		TCHAR				szSessionName[MAX_PATH];
		PEER_INFORMATION	*pPeerList;
		USHORT				 usShortList;
} SESSION_INFORMATION ,*LPSESSION_INFORMATION;


class ISessionChangesNotify{
	DWORD OnDynamicSessionAdd(LPCTSTR lpszSessionID,LPCTSTR lpszSessionString);
};

class ISessionContainer:public ISessionChangeNotify
{
public:
	ISessionContainer(void)
	{
		m_hUIToLSPEvent=m_hLSPToUIEvent=NULL;
		m_hLSPToUIWaitHandler=m_hUIToLSPWaitHandler=NULL;
		m_pSessionListIPC=m_pDynamicUpdateIPC=NULL;
		m_hLOIEnableEvent=NULL;
	}
	virtual ~ISessionContainer(void)
	{
		UnRegisterEventNotifiers();
		ClearSessionData();
		if(m_hLOIEnableEvent)
		{
			CloseHandle(m_hLOIEnableEvent);
			m_hLOIEnableEvent=NULL;
		}
	}
private:
	ThreadLock							m_SessionObjCreationLock;
	HANDLE								m_hLOIEnableEvent;

protected:
	static const TCHAR					UI_TO_LSP_NOTIFY_NAME[];
	static const TCHAR					LSP_TO_UI_NOTIFY_NAME[];
	static const   TCHAR				ENABLE_LOI_IPC[];
	static UI_TO_LSP_IPC_INFO			m_IpcInfo[];
	CDynamicSessionInfoUpdate			m_DynamicUpdate;
	HANDLE								m_hUIToLSPEvent;
	HANDLE								m_hLSPToUIEvent;
	HANDLE								m_hUIToLSPWaitHandler;
	HANDLE								m_hLSPToUIWaitHandler;
	CMTSMap <std::string,CLOISession*>	m_StringToSessionMapList;
	CIPCFileMap							*m_pDynamicUpdateIPC;
	CIPCFileMap							*m_pSessionListIPC;
public:
	/****************************
	This initailises the event so can be event to notify LSP abt changes 
	Also intialises the map for session data sharing
	****************************/
	void  RegisterUIToLSPNotifier()
	{
		//one ui instance to many lsp instance notify
		GetUIToLSPEvent(m_hUIToLSPEvent);
	
		m_pSessionListIPC = new  CIPCFileMap(m_IpcInfo[SESSION_ITEM].szIPCName,CIPCFileMap::store);
		m_pSessionListIPC->Open(m_IpcInfo[SESSION_ITEM].dwMaxSize );
		/*if(m_hUIToLSPEvent && EventCallback)
		{
			RegisterWaitForSingleObject(&m_hUIToLSPWaitHandler,m_hUIToLSPEvent,EventCallback,Context,INFINITE,WT_EXECUTEDEFAULT|WT_EXECUTEINWAITTHREAD);//need change can result in dead lock
		}*/
	}
	/********************
	Get UI to lsp event handle
	*********************/
	void GetUIToLSPEvent(HANDLE &hUIToLSPEvent)
	{
		if(!(hUIToLSPEvent =OpenEvent(SYNCHRONIZE|EVENT_MODIFY_STATE ,FALSE, UI_TO_LSP_NOTIFY_NAME)))
		{
			CSecurityObject Sa;
			hUIToLSPEvent=CreateEvent( Sa.GetAllAccessSA() ,TRUE,FALSE,UI_TO_LSP_NOTIFY_NAME);
			CSecurityObject::SetObjectToLowIntegrity(hUIToLSPEvent);
		}
	}
	/*********************************
	Register notification for LSP to UI communiction
	**********************************/
	void  RegisterLSPToUINotifier()
	{
		//many lsp instance to one ui instance notify
		if(!(m_hLSPToUIEvent =OpenEvent(SYNCHRONIZE|EVENT_MODIFY_STATE ,FALSE, LSP_TO_UI_NOTIFY_NAME)))
		{
			CSecurityObject Sa;
			m_hLSPToUIEvent=CreateEvent( Sa.GetAllAccessSA() ,FALSE,FALSE,LSP_TO_UI_NOTIFY_NAME);
			CSecurityObject::SetObjectToLowIntegrity(m_hLSPToUIEvent);
		}
		m_pDynamicUpdateIPC = new CIPCFileMap(m_IpcInfo[DYNAMIC_SESSION_UPDATE_ITEM].szIPCName ,CIPCFileMap::store);
		m_pDynamicUpdateIPC->Open( m_IpcInfo[DYNAMIC_SESSION_UPDATE_ITEM].dwMaxSize); 
		m_DynamicUpdate.Initialise(*m_pDynamicUpdateIPC);//intialise so LSP can write new data
		if(m_hLSPToUIEvent )//just keep the event open to be set from another process
		{
			RegisterWaitForSingleObject(&m_hLSPToUIWaitHandler,m_hLSPToUIEvent,DynamicUpdate,this,INFINITE,WT_EXECUTEDEFAULT|WT_EXECUTEINWAITTHREAD);//need change can result in dead lock
		}
	}
	/***********************************
	Callback invoked when LSP needs data to be received from UI -- its dynamic port information update
	***********************************/
	static VOID WINAPI DynamicUpdate(PVOID lpParameter,BOOLEAN TimerOrWaitFired)
	{
		ISessionContainer *pThis =(ISessionContainer *)lpParameter;

		TCHAR							szDynamicItems[2048]={0};
		//get the new dynamic  data
		DWORD							dwCount=0;
		DYNAMIC_SESSION_UPDATE_INFO		*pItems = pThis->m_DynamicUpdate.ClearDynamicUpdatedItems(*pThis->m_pDynamicUpdateIPC,dwCount);


		StringCbCopy(szDynamicItems,sizeof(szDynamicItems),_T(""));
		for(int iCount=0;iCount<(int)dwCount;iCount++)
		{
#ifdef CSTRING_DEFINED
			CLOISessionStringFormat			 SessionXmlFormat;

			SessionXmlFormat.AddDynamicBoundedAddress(pItems[iCount].dwBindedLocalAddress,
													  pItems[iCount].usBoundPort,
													  pItems[iCount].dwVirtualPeerAddress
													  );
			/*
			TCHAR			szBoundAddr[MAX_PATH]={0},szVirtualAddr[MAX_PATH]={0};
			TCHAR			szSessionStr[MAX_PATH]={0};
			in_addr			BoundAddr,VirtualAddr;

			BoundAddr.S_un.S_addr=  pItems[iCount].dwBindedLocalAddress;
			VirtualAddr.S_un.S_addr= pItems[iCount].dwVirtualPeerAddress;


			StringCbPrintf(szBoundAddr,sizeof(szBoundAddr),_T("%hs"),inet_ntoa(BoundAddr));
			StringCbPrintf(szVirtualAddr,sizeof(szVirtualAddr),_T("%hs"),inet_ntoa(VirtualAddr));

			StringCbPrintf(szSessionStr,sizeof(szSessionStr),_T("I%s@%d@%s"),
												szBoundAddr,
												pItems[iCount].usBoundPort,
												szVirtualAddr
												);*/
			//notify the caller about the change
			CLOISession		*pLOISession =pThis->GetExistingSession( pItems[iCount].szSessionID);
			if(pLOISession)
			{
				//tell the session abt the new session string 
				pLOISession->AddDynamicPortString((LPCTSTR)SessionXmlFormat.GetXMLString());
			}
#endif 
		}
		pThis->m_DynamicUpdate.FreeDynamicUpdatedItems(pItems);	
	}
	void UnRegisterEventNotifiers()
	{
		//unregister event handlers 
		if(m_hUIToLSPWaitHandler)
		{
			UnregisterWait(m_hUIToLSPWaitHandler);
			if(GetLastError()==ERROR_IO_PENDING )
				Sleep(200);
			m_hUIToLSPWaitHandler=NULL;
		}
			//unregister event handlers 
		if(m_hLSPToUIWaitHandler)
		{
			UnregisterWait(m_hLSPToUIWaitHandler);
			if(GetLastError()==ERROR_IO_PENDING )
				Sleep(200);
			m_hLSPToUIWaitHandler=NULL;
		}
		//close notify events
		if(m_hUIToLSPEvent )
		{
			CloseHandle(m_hUIToLSPEvent );
			m_hUIToLSPEvent=NULL;
		}
		if(m_hLSPToUIEvent )
		{
			CloseHandle(m_hLSPToUIEvent );
			m_hLSPToUIEvent=NULL;
		}
		//remove maps 
		if(m_pSessionListIPC)
		{
			delete m_pSessionListIPC;
			m_pSessionListIPC=NULL;
		}
		if(m_pDynamicUpdateIPC)
		{
			delete m_pDynamicUpdateIPC;
			m_pDynamicUpdateIPC=NULL;
		}
	}
	void  NotifyLSPInstances()
	{
		if(m_hUIToLSPEvent)
		{
			SetEvent( m_hUIToLSPEvent);
			Sleep(50);
			ResetEvent(m_hUIToLSPEvent);
		}
	}
	void  NotifyUIInstance()
	{
		if(!m_hLSPToUIEvent)
		{
			m_hLSPToUIEvent =OpenEvent(SYNCHRONIZE|EVENT_MODIFY_STATE ,FALSE, LSP_TO_UI_NOTIFY_NAME);
			
		}
		if(m_hLSPToUIEvent)
				SetEvent( m_hLSPToUIEvent);
	}	
	/*****************************
	session object creation lock -- this will the objects thread safe
	******************************/
	void ObjectCreationLock()
	{
		m_SessionObjCreationLock.Lock();
	}
	void ObjectCreationUnLock()
	{
		m_SessionObjCreationLock.UnLock();
	}
	
	/**************************************************
	Session handling functions
	**************************************************/
	CLOISession *GetSession(LPCTSTR lpszSessionID,IDynamicSessionUpdateNotify *pDynamicNotify)
	{
		CLOISession		*pLOISession=NULL;

		m_SessionObjCreationLock.Lock(); 

		if(m_StringToSessionMapList.IsKeyExists(GetStdString(lpszSessionID)))
		{	
			//check if the session already exists
			pLOISession=m_StringToSessionMapList [GetStdString(lpszSessionID)];
			//if already exits then just increment the reference
			pLOISession->AddRef();
			m_SessionObjCreationLock.UnLock ();
			return pLOISession;
		}
		//session doesn't exists create it
		pLOISession= new CLOISession(this,pDynamicNotify); 
		m_StringToSessionMapList.Insert (GetStdString(lpszSessionID), pLOISession);//add the item to the list 
		m_SessionObjCreationLock.UnLock ();
		return pLOISession;
	}
	/*****************************************
	Gets the already existing session doesn't create it
	*****************************************/
	CLOISession *GetExistingSession(LPCTSTR lpszSessionID)
	{
		CLOISession		*pLOISession=NULL;

		m_SessionObjCreationLock.Lock(); 

		//check if the session already exists
		if(m_StringToSessionMapList.IsKeyExists(GetStdString(lpszSessionID)))
		{
			pLOISession=m_StringToSessionMapList [GetStdString(lpszSessionID)];
			//if already exits then just increment the reference
			pLOISession->AddRef();
			m_SessionObjCreationLock.UnLock ();
			return pLOISession;
		}
		m_SessionObjCreationLock.UnLock ();
		return NULL;
	}
	/***************************************************
	Gets all the peer information with session name	
	***************************************************/
	SESSION_INFORMATION* GetActiveSessionLists(USHORT &usSessionCount)
	{
		SESSION_INFORMATION								*pSessionInfo=NULL;
		multimap<std::string,CLOISession*>::iterator	items;

		usSessionCount=0;
		if(m_StringToSessionMapList.GetSize()==0)
									return pSessionInfo;


		m_SessionObjCreationLock.Lock(); 
		pSessionInfo=new SESSION_INFORMATION[m_StringToSessionMapList.GetSize()];
		m_StringToSessionMapList.ThreadSafeLock();
		for( items=m_StringToSessionMapList.begin();items!=m_StringToSessionMapList.end();items++)
		{
			TCHAR  szSessionName[MAX_PATH]={0};
			(*items).second->GetSessionName(szSessionName,sizeof(szSessionName)); 
			StringCbCopy( pSessionInfo[usSessionCount].szSessionName,sizeof(pSessionInfo[usSessionCount].szSessionName),szSessionName);
			pSessionInfo[usSessionCount].pPeerList = (*items).second->GetConnectedClientList(pSessionInfo[usSessionCount].usShortList);   
			usSessionCount++;
		}
		m_StringToSessionMapList.ThreadSafeUnLock();	
		m_SessionObjCreationLock.UnLock ();
		

		return pSessionInfo;
	}
	/**********************************
	Get  the list of session which are idle
	session referende has to be freed
	***********************************/
	CLOISession**  GetIdleSessions(const DWORD dwMinMinnutesTimeout,DWORD &dwIdleSessionCount)
	{
		CLOISession				**ppLOISession=NULL;
		CLOISession				**ppIDLELOISession=NULL;
		DWORD					 dwItem=0;
		

		if(m_StringToSessionMapList.GetSize()==0)
											return  NULL;

		m_SessionObjCreationLock.Lock(); 
		ppLOISession= m_StringToSessionMapList.GetItems(dwItem);
		if(ppLOISession)
		{
			ppIDLELOISession= new CLOISession*[dwItem]; 
			dwIdleSessionCount=0;
			while(dwItem!=0)
			{
				if(ppLOISession[dwItem-1]->GetLastActivity()>=dwMinMinnutesTimeout  )
				{
					ppIDLELOISession[dwIdleSessionCount]=ppLOISession[dwItem-1];
					ppIDLELOISession[dwIdleSessionCount]->AddRef(); 
					dwIdleSessionCount++;
				}
				
				dwItem--;
			}
			m_StringToSessionMapList.FreeItems(ppLOISession);  
		}
		m_SessionObjCreationLock.UnLock ();
		return ppIDLELOISession;
	}
	/**********************
	free session list
	************************/
	void FreeSessionList(CLOISession **ppIdleSessions,DWORD dwItemCount)
	{
		for(DWORD dwCount=0; dwCount<dwItemCount;dwCount++)
		{
			Release(ppIdleSessions[dwCount]);
		}
		if(ppIdleSessions)
		{
			delete ppIdleSessions;
		}
	}

	/******************************************
	free all session information
	*******************************************/
	void FreeActiveSessionList(SESSION_INFORMATION* pSessionInfo,USHORT usSessionCount)
	{
		if(pSessionInfo)
		{
			//free all inner peerlist first
			while(usSessionCount)
			{
				CLOISession::FreeConnectedClientList(pSessionInfo[usSessionCount-1].pPeerList);
				usSessionCount--;
			}
			delete pSessionInfo;
		}
	}
	/*****************************************
	Remove the sessions
	tru -if the object is destroyed
	******************************************/
	DWORD Release(CLOISession *pLOISession)
	{
		DWORD	dwRefCount;

		m_SessionObjCreationLock.Lock(); 
		//first release
		dwRefCount=pLOISession->Release();
		if(dwRefCount==0)//confirm that nobody is using it)
		{
			m_StringToSessionMapList.EraseByValue(pLOISession);  
		}
		m_SessionObjCreationLock.UnLock ();

		//notify about object destroyed
		if(dwRefCount==0)
			OnSessionValueChange();
		//if session count is zero then reset LOIevent 
		if(m_StringToSessionMapList.IsEmpty() )
		{
			EnableLOI(FALSE);
		}
		return dwRefCount;

	}
	/******************************************
	Checks the session exists
	*******************************************/
	BOOL IsSessionExists(LPCTSTR lpszSessionID)
	{
		BOOL   bRet=FALSE;
		m_SessionObjCreationLock.Lock(); 
		bRet=m_StringToSessionMapList.IsKeyExists (GetStdString(lpszSessionID) );
		m_SessionObjCreationLock.UnLock(); 
		return bRet;
	}

	/*******************************
	traverses all session objects & checks whether the client is valid
	*****************************/
	BOOL IsValidClient(in_addr6 ClientIPv6Addr)
	{
		BOOL		bRet=FALSE;
		m_SessionObjCreationLock.Lock(); 
		DWORD			 dwItemCount;
		CLOISession		**ppSessions=NULL;
		ppSessions=m_StringToSessionMapList.GetItems(dwItemCount);
		if(dwItemCount)
		{
			for( DWORD dwCount=0;dwCount<dwItemCount;dwCount++)
			{
				 if(ppSessions[dwCount]->GetIpv4Address( ClientIPv6Addr))
				 {
					//found it in the list means the client exists
					bRet=TRUE;
					break;
				 }
			}
			m_StringToSessionMapList.FreeItems(ppSessions);
		}
		m_SessionObjCreationLock.UnLock(); 
		return bRet;
	}
	/*******************************
	traverses all session objects & checks whether the client is allowed to proxy in this port
	*****************************/
	CLOISession* IsProxyAllowed(in_addr6 ClientIPv6Addr,USHORT usPort)
	{
		BOOL		bRet=FALSE;
		m_SessionObjCreationLock.Lock(); 
		DWORD			 dwItemCount;
		CLOISession		**ppSessions=NULL;
		CLOISession		 *pSessionRet=NULL;

		ppSessions=m_StringToSessionMapList.GetItems(dwItemCount);
		if(dwItemCount)
		{
			for( DWORD dwCount=0;dwCount<dwItemCount;dwCount++)
			{
				 if(ppSessions[dwCount]->GetIpv4Address( ClientIPv6Addr) &&
					 ppSessions[dwCount]->IsPortRequireProxy(usPort) 
					 )
				 {
					//found it in the list means the client exists
					pSessionRet=ppSessions[dwCount];
					bRet=TRUE;
					break;
				 }
			}
			m_StringToSessionMapList.FreeItems(ppSessions);
		}
		m_SessionObjCreationLock.UnLock(); 
		return pSessionRet;
	}
	/***********************
	Clear all sessions
	************************/
	void ClearSessionData()
	{
		m_SessionObjCreationLock.Lock(); 
		DWORD			 dwItemCount;
		CLOISession		**ppSessions=NULL;
		ppSessions=m_StringToSessionMapList.GetItems(dwItemCount);
		if(dwItemCount)
		{
			for( DWORD dwCount=0;dwCount<dwItemCount;dwCount++)
			{
				delete ppSessions[dwCount];
			}
			m_StringToSessionMapList.FreeItems(ppSessions);
			m_StringToSessionMapList.Clear();  //clear all items
		}
		m_SessionObjCreationLock.UnLock(); 
	}

public:
	void EnableLOI(BOOL bEnable)
	{
		if(bEnable)
		{
			if(!m_hLOIEnableEvent)
			{
				CSecurityObject Sa;
				m_hLOIEnableEvent =CreateEvent( Sa.GetAllAccessSA() ,TRUE,TRUE,ENABLE_LOI_IPC);
				CSecurityObject::SetObjectToLowIntegrity(m_hLOIEnableEvent);
			}
			SetEvent(m_hLOIEnableEvent);
		}
		//if object 
		if(!bEnable)
		{
			if(m_hLOIEnableEvent)
			{
				CloseHandle(m_hLOIEnableEvent);
				m_hLOIEnableEvent=NULL;
			}
		}
	}
	static BOOL	IsLOIEnabled(LPCTSTR lpszAppName)
	{
		HANDLE				hLOIEnableEvent;


		if(_tcsstr(lpszAppName,_T("svchost.exe")) ||
			_tcsstr(lpszAppName,_T("winlogon.exe")) ||
			_tcsstr(lpszAppName,_T("csrss.exe")) ||
			_tcsstr(lpszAppName,_T("lanoiservice.exe"))
			)
			return FALSE;
		hLOIEnableEvent= OpenEvent(SYNCHRONIZE,FALSE,ENABLE_LOI_IPC);
		if(hLOIEnableEvent)
		{
			BOOL bEnabled=(WaitForSingleObject( hLOIEnableEvent,0)==WAIT_OBJECT_0);
			CloseHandle(hLOIEnableEvent);
			return bEnabled;
		}
		return FALSE;
	}


private:
	/*******************************
	Notified when any of the sessions data is changed
	********************************/
	void OnSessionValueChange()
	{
		//serialise the session values
		if(m_pSessionListIPC)
		{
		 	Serialize(*m_pSessionListIPC);
		}
		EnableLOI(TRUE);//set the loi event to notify LSP abt new session creation
		NotifyLSPInstances(); 
	}
	/*****************************
	convert TCHAR to std string
	*****************************/
	std::string GetStdString(LPCTSTR lpszString)
	{
		CHAR			szStringA[MAX_PATH];
		std::string		stemp;

		StringCbPrintfA(szStringA,sizeof(szStringA),"%S",lpszString);
		stemp =  szStringA;
		return stemp;
	}
public:
	/**************************
	Serialize the data to the MAP
	**************************/
	void Serialize(CIPCFileMap& FileMap)
	{
		FileMap.SingleUseLock();
		if(FileMap.IsStoring())
		{
			m_SessionObjCreationLock.Lock(); 
			SESSION_HEADER									SessionHeader={0};
			multimap<std::string,CLOISession*>::iterator	items;
			TCHAR											szSessionID[MAX_PATH]={0};

			m_StringToSessionMapList.ThreadSafeLock();
			//initialise to the start
			FileMap.ClearMap();//clear all previous map data
			FileMap.SetFilePointer( FILE_BEGIN);
			for( items=m_StringToSessionMapList.begin();items!=m_StringToSessionMapList.end();items++)
			{	
				StringCbPrintf(szSessionID,sizeof(szSessionID),_T("%S"),items->first.c_str());
				StringCbCopy(SessionHeader.szSessionID,sizeof(SessionHeader.szSessionID),szSessionID ); 
				//write the header information about session unique string
				FileMap.Write((LPBYTE)&SessionHeader,sizeof(SessionHeader)); 
				//followed by the session object serialisation
				items->second->Serialize(FileMap); 
			}
			m_StringToSessionMapList.ThreadSafeUnLock();
			m_SessionObjCreationLock.UnLock(); 
		}
		else
		{
			SESSION_HEADER		SessionHeader={0};
			ClearSessionData();

			//initialise to the start
			FileMap.SetFilePointer( FILE_BEGIN);
			if(FileMap.GetMapSize())//only if any valid data is in map then serialise
			{
				m_SessionObjCreationLock.Lock(); 
				while(FileMap.Read((LPBYTE)&SessionHeader,sizeof(SessionHeader)))
				{
					//add create a new session object 
					CLOISession *pSession=NULL;
					//add the item to the map
					pSession=GetSession(SessionHeader.szSessionID,NULL); //creates the session if it doesn't exists
					//deserialise the session data from map
					pSession->Serialize( FileMap);

				}
				m_SessionObjCreationLock.UnLock();
			}
		}
		FileMap.SingleUseUnLock();
	}



};

class CLOISessionContainer:public ISessionContainer{
public:
	CLOISessionContainer();
	~CLOISessionContainer();
	void Init();
	void Close();
};