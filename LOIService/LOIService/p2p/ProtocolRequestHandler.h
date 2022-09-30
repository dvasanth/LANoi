#pragma once

#pragma pack(push)
#pragma pack(1)
	typedef struct __PROTO_CMD {
			USHORT  usCmd;
			DWORD   dwLength;
	}PROTO_CMD,*LPPROTO_CMD;


	//user data
    typedef struct _PEER_INFORMATION {
			DWORD			dwIPv4Address;
			in_addr6		IPv6TeredoAddress;
			USHORT			usReverseProxyPort;
			TCHAR			szPeerName[MAX_PATH];
			DWORD			dwFlag;
			USHORT			usVersion;
			TCHAR			szPeerID[MAX_PATH];
	}PEER_INFORMATION, *LPPEER_INFORMATION;

	#define    PEER_FLAG_ADMIN						0x1

	//auth validate packet
	typedef struct __AUTH_VALIDATE {
		PEER_INFORMATION   PeerInfo;
	}AUTH_VALIDATE,*LPAUTH_VALIDATE;

	//add/remove user list
	typedef struct __PEER_LISTS {
		USHORT 				usPeerCount;
		PEER_INFORMATION	PeerInfo[1];
	}PEER_LISTS,*LPPEER_LISTS;


	//add port list
	typedef struct __PORT_LIST {
		WCHAR			wPortString[1];
	}PORT_LISTS ,*LPPORT_LISTS;

	//send dynamic update port information sent only from clients
	typedef struct __DYNAMIC_SESSION_LISTS {
		WCHAR			wPortString[1];
	}DYNAMIC_SESSION_LISTS ,*LPDYNAMIC_SESSION_LISTS;

	//send keepalive
	typedef struct __KEEP_ALIVE{
		DWORD      dwKeepAliveSeconds;
	}KEEP_ALIVE ,*LPKEEP_ALIVE;


	//proxy tcp data
	typedef struct __PROXY_CONNECT{
		USHORT      usPort;
	}PROXY_CONNECT ,*LPPROXY_CONNECT;

#pragma pack(pop)

	enum PROTO_CMD_TYPE{
		WAIT_FOR_COMMAND=0,
		AUTH_REQUEST,
		ADD_PEER_LISTS,
		REMOVE_PEER_LISTS,
		PEER_KEEP_ALIVE,
		ALIVE_OK,
		REVERSE_PROXY_CONNECT,
		REVERSE_PROXY_CLIENT_RECV,
		REVERSE_PROXY_REMOTE_RECV,
		SESSION_PORTS_INFO,
		DYNAMIC_SESSION_INFO,
		HANDSHAKE_COMPLETE
	};

#define BEGIN_PROTOCOL_HANDLER \
							DWORD CmdDispatcher(BYTE *byRecBuffer,DWORD dwBytesRecevied,PROTO_CMD_TYPE &CurrentCmd,T &pContext)\
							{\
								if(CurrentCmd == WAIT_FOR_COMMAND)\
								{\
									if(dwBytesRecevied<sizeof(PROTO_CMD))return sizeof(PROTO_CMD);\
									LPPROTO_CMD lpCmd=(LPPROTO_CMD)byRecBuffer;\
									DEBUGLOG(LOI_INFO,_T("Received Command %d, State %d, Len %d"),lpCmd->usCmd,CurrentCmd,lpCmd->dwLength);\
									CurrentCmd =PROTO_CMD_TYPE( lpCmd->usCmd);\
									if(lpCmd->dwLength)\
									{\
										return lpCmd->dwLength;\
									}\
								}\
								switch(CurrentCmd)\
								{
#define ON_CMD(CmdType,FunctionToExecute) \
									case CmdType:\
										FunctionToExecute(pContext,byRecBuffer,dwBytesRecevied);\
										break;
#define ON_REVERSE_PROXY_CMD(CmdType,FunctionToExecute) \
									case CmdType:\
										return FunctionToExecute(pContext,byRecBuffer,dwBytesRecevied,CurrentCmd);\
										break;
									
#define END_PROTOCOL_HANDLER 	}\
								CurrentCmd = WAIT_FOR_COMMAND;\
								return sizeof(PROTO_CMD);\
							}

#define EXECUTE_PROTOCOL_HANDLER(byRecBuffer,dwBytesRecevied,CurrentCmd,pContext)\
										CmdDispatcher(byRecBuffer,dwBytesRecevied,CurrentCmd,pContext)

template <class T> 
class IProtocolCommandNotify{
public:
	virtual void OnPeerJoin(T &t,PEER_INFORMATION PeerInformation){}
	virtual void OnPeerLeave(T &t,in_addr6 PeerIPv6Addr){}
	virtual void OnSessionInfoRecv(T &t,LPCTSTR lpszPortList){}
	virtual void OnServerHandshakeComplete(T &t){}
	virtual void OnDynamicSessionUpdate(T &t,LPCTSTR lpszPortList){}
	virtual void OnClientAuthenticate(T &t,PEER_INFORMATION PeerInformation)
	{
	}
	/***************************
	reverse proxy related command
	***************************/
	virtual	DWORD OnReverseProxyConnect(T &t,USHORT usPort,PROTO_CMD_TYPE &NextState){return 0;}
	virtual DWORD OnLocalClientRecv(T &t,BYTE *ByValidateData,DWORD dwBytesReceived,PROTO_CMD_TYPE &NextState){return 0;}
	virtual DWORD OnRemoteClientRecv(T &t,BYTE *ByValidateData,DWORD dwBytesReceived,PROTO_CMD_TYPE &NextState){return 0;}
 
};

template <typename T> 
class CProtocolRequestHandler
{
private:
	IProtocolCommandNotify<T>		*m_pProtocolNotify;
public:
	CProtocolRequestHandler(IProtocolCommandNotify<T> *pProtocolNotify)
	{
		m_pProtocolNotify=pProtocolNotify;
	}
	virtual ~CProtocolRequestHandler(void){}


	DWORD HandleCmd(BYTE *pByteRecv,DWORD dwLen,PROTO_CMD_TYPE &State,T &Context)
	{
		
		return EXECUTE_PROTOCOL_HANDLER( pByteRecv,dwLen,State,Context);
	}	
protected:

BEGIN_PROTOCOL_HANDLER
	ON_CMD(AUTH_REQUEST,OnAuthValidate)
	ON_CMD(ADD_PEER_LISTS,OnAddPeer)
	ON_CMD(REMOVE_PEER_LISTS,OnRemovePeer)
	ON_CMD(ALIVE_OK,OnAliveResponse)
	ON_CMD(SESSION_PORTS_INFO,OnPortLists)
	ON_CMD(HANDSHAKE_COMPLETE,OnHandShakeComplete)
	ON_CMD(DYNAMIC_SESSION_INFO,OnDynamicSessionUpdate)
	ON_REVERSE_PROXY_CMD(REVERSE_PROXY_CONNECT,OnReverseProxyConnect)
	ON_REVERSE_PROXY_CMD(REVERSE_PROXY_CLIENT_RECV,OnLocalClientRecv)
	ON_REVERSE_PROXY_CMD(REVERSE_PROXY_REMOTE_RECV,OnRemoteClientRecv)
END_PROTOCOL_HANDLER

public:
	static const USHORT				PROTOCOL_VERSION=3;
	/******************************************************
	Form the dynamic session update packet,caller sends the dynamic string
	******************************************************/
	BOOL FormDynamicSessionPacket(LPCTSTR lpszDynamicString,BYTE *pProtocolData,DWORD &dwInOutLen)
	{
		//form the portlist command & send data
		DWORD					 dwProtolen = sizeof(PROTO_CMD)+sizeof(PORT_LISTS)+(_tcslen(lpszDynamicString)*sizeof(TCHAR))+sizeof(TCHAR);
		LPPROTO_CMD			     pCmd=(LPPROTO_CMD)pProtocolData;

		//check if the caller send enough size else update the actual size required
		if(dwInOutLen<dwProtolen)
		{
			dwInOutLen=dwProtolen;
			return FALSE;
		}
		
		pCmd->usCmd = DYNAMIC_SESSION_INFO;
		pCmd->dwLength = dwProtolen-sizeof(PROTO_CMD);

		//fill the structlist
		StringCbCopy(LPTSTR(pProtocolData+sizeof(PROTO_CMD)),dwProtolen-sizeof(PROTO_CMD),lpszDynamicString);
		dwInOutLen=dwProtolen;
		return TRUE;
	}
	/*****************************************************
	form the port string -- its same as FormDynamicSessionPacket only cmd is differnt
	*****************************************************/
	BOOL FormSessionPacket(LPCTSTR lpszPortString,BYTE *pProtocolData,DWORD &dwInOutLen)
	{
		if(FormDynamicSessionPacket(lpszPortString,pProtocolData,dwInOutLen))
		{
			((LPPROTO_CMD)pProtocolData)->usCmd = SESSION_PORTS_INFO;
		}
		return FALSE;


	}
	/************************************************************
	Form the client keep alive packet, we  need keep alive interval
	************************************************************/
	BOOL FormKeepAlivePacket(DWORD dwKeepAliveTime,BYTE *pProtocolData,DWORD &dwInOutLen)
	{
		DWORD					 dwProtolen = sizeof(PROTO_CMD)+sizeof(KEEP_ALIVE);
		LPPROTO_CMD			     pCmd=(LPPROTO_CMD)pProtocolData;
		LPKEEP_ALIVE			 pKeepAlive=(LPKEEP_ALIVE)(pProtocolData+sizeof(PROTO_CMD));

		//check if the caller send enough size else update the actual size required
		if(dwInOutLen<dwProtolen)
		{
			dwInOutLen=dwProtolen;
			return FALSE;
		}
		pCmd->usCmd = PEER_KEEP_ALIVE;
		pCmd->dwLength = dwProtolen-sizeof(PROTO_CMD);	

		pKeepAlive->dwKeepAliveSeconds =dwKeepAliveTime;
		dwInOutLen=dwProtolen;
		return TRUE;

	}
	////////////////helpers to form protocol packets
	/******************************************************
	Form the authentication packet,caller fills the peer information & sends it
	******************************************************/
	BOOL FormClientAuthPacket(PEER_INFORMATION ClientPeerInfo,BYTE *pProtocolData,DWORD &dwInOutLen)
	{
		DWORD					 dwProtolen = sizeof(PROTO_CMD)+sizeof(AUTH_VALIDATE);
		LPPROTO_CMD			     pCmd=(LPPROTO_CMD)pProtocolData;
		LPAUTH_VALIDATE			 pAuthReq=(LPAUTH_VALIDATE)(pProtocolData+sizeof(PROTO_CMD));

		//check if the caller send enough size else update the actual size required
		if(dwInOutLen<dwProtolen)
		{
			dwInOutLen=dwProtolen;
			return FALSE;
		}
		pCmd->usCmd = AUTH_REQUEST;
		pCmd->dwLength = dwProtolen-sizeof(PROTO_CMD);

		memcpy(pAuthReq,&ClientPeerInfo,sizeof(PEER_INFORMATION));
		//fill the protocol information ipv6 address
		pAuthReq->PeerInfo.usVersion =   CProtocolRequestHandler::PROTOCOL_VERSION;
		dwInOutLen=dwProtolen;
		return TRUE;
	}

	/********************************************************************
	Updates the peer about client addition or removal
	**********************************************************************/
	BOOL FormPeerAddorRemovePacket(BOOL bAddOrRemove,PEER_INFORMATION *pPeerList,DWORD dwPeerListCount,BYTE *pProtocolData,DWORD &dwInOutLen)
	{
		DWORD				dwProtolen=(sizeof(PEER_INFORMATION)*dwPeerListCount)+sizeof(PROTO_CMD)+sizeof(PEER_LISTS);
		LPPROTO_CMD			pCmd=(LPPROTO_CMD)pProtocolData;
		LPPEER_LISTS		pUserList=(LPPEER_LISTS)(pProtocolData+sizeof(PROTO_CMD));

		//check if the caller send enough size else update the actual size required
		if(dwInOutLen<dwProtolen)
		{
			dwInOutLen=dwProtolen;
			return FALSE;
		}

		//send the connected peer listfirst
		pCmd->usCmd =bAddOrRemove? ADD_PEER_LISTS:REMOVE_PEER_LISTS;
		pCmd->dwLength = dwProtolen-sizeof(PROTO_CMD);

		//fill struct list
		pUserList->usPeerCount = (USHORT)dwPeerListCount;
		memcpy(&pUserList->PeerInfo[0] ,pPeerList,sizeof(PEER_INFORMATION)*dwPeerListCount);

		return TRUE;
	}

	/********************************************************************
	Forming the handshake packet to say client all information is sent
	**********************************************************************/
	BOOL FormHandshakePacket(BYTE *pProtocolData,DWORD &dwInOutLen)
	{
		DWORD				dwProtolen=sizeof(PROTO_CMD);
		LPPROTO_CMD			pCmd=(LPPROTO_CMD)pProtocolData;

		//check if the caller send enough size else update the actual size required
		if(dwInOutLen<dwProtolen)
		{
			dwInOutLen=dwProtolen;
			return FALSE;
		}

		//send the connected peer listfirst
		pCmd->usCmd =HANDSHAKE_COMPLETE;
		pCmd->dwLength = 0;

		return TRUE;
	}
protected:
	/************************************************************
	This command recvies  peers in the group
	************************************************************/
	void  OnAddPeer(T &t,BYTE *ByValidateData,DWORD dwBytesReceived)
	{
		LPPEER_LISTS    pPeerLists=(LPPEER_LISTS)ByValidateData;

		for(int i=0;i<pPeerLists->usPeerCount ;i++)
		{
			//update one by one to application
			m_pProtocolNotify->OnPeerJoin(t,pPeerLists->PeerInfo[i]);
		}
	}
	/*********************************************************
	This command is received when any peer leaves the group
	*********************************************************/
	void OnRemovePeer(T &t,BYTE *ByValidateData,DWORD dwBytesReceived)
	{
		LPPEER_LISTS    pPeerLists=(LPPEER_LISTS)ByValidateData;
		for(int i=0;i<pPeerLists->usPeerCount ;i++)
		{
			//update one by one to the caller
			m_pProtocolNotify->OnPeerLeave( t,pPeerLists->PeerInfo[i].IPv6TeredoAddress);
		}

	}
	/***********************************************************
	auth handling command
	***********************************************************/
	void OnAuthValidate(T &t,BYTE *ByValidateData,DWORD dwBytesReceived)
	{
		m_pProtocolNotify->OnClientAuthenticate(t,*((PEER_INFORMATION*)(ByValidateData)));
	}
	/*******************************************************************
	This command sents about the port & app for  the session
	*******************************************************************/
	void OnPortLists(T &t,BYTE *ByValidateData,DWORD dwBytesReceived)
	{
		LPPORT_LISTS  pPortString=(LPPORT_LISTS)ByValidateData;
		//server send the ports for the session
		m_pProtocolNotify->OnSessionInfoRecv(t,&pPortString->wPortString[0]); 
	}
	/*******************************************************************
	This command sent by the server to indicate that all conneted peer list & session informatiom
	is  pushed to the client
	*******************************************************************/
	void OnHandShakeComplete(T &t, BYTE *ByValidateData,DWORD dwBytesReceived)
	{
		m_pProtocolNotify->OnServerHandshakeComplete(t); 
	}
	/*****************************
	To keepalive message, server sends the repsonse
	******************************/
	void OnAliveResponse(T &t, BYTE *ByValidateData,DWORD dwBytesReceived)
	{
	}
	/**************************************************************************
	This sent by the peer to the server to indicate the dynamic port updation required
	***************************************************************************/
	void OnDynamicSessionUpdate(T &t, BYTE *ByValidateData,DWORD dwBytesReceived)
	{
		LPPORT_LISTS  pPortString=(LPPORT_LISTS)ByValidateData;
		m_pProtocolNotify->OnDynamicSessionUpdate(t,pPortString->wPortString );
	}
	/****************************************************
	reverse proxy related
	****************************************************/
	DWORD	OnReverseProxyConnect(T &t,BYTE *ByValidateData,DWORD dwBytesReceived,PROTO_CMD_TYPE &NextState)
	{
		return m_pProtocolNotify->OnReverseProxyConnect(t,((LPPROXY_CONNECT)ByValidateData)->usPort,NextState); 
	}
	DWORD	OnLocalClientRecv(T &t,BYTE *ByValidateData,DWORD dwBytesReceived,PROTO_CMD_TYPE &NextState)
	{
		return m_pProtocolNotify->OnLocalClientRecv(t,ByValidateData,dwBytesReceived,NextState) ;
	}
	DWORD	OnRemoteClientRecv(T &t,BYTE *ByValidateData,DWORD dwBytesReceived,PROTO_CMD_TYPE &NextState)
	{
		return m_pProtocolNotify->OnRemoteClientRecv(t,ByValidateData,dwBytesReceived,NextState);
	}
};
