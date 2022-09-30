#include "StdAfx.h"
#include "IPCCommandHandler.h"


const TCHAR		CIPCCommandHandler::COMMAND_NODE[]=_T("command");
const TCHAR		CIPCCommandHandler::PARAM_NODE[]=_T("parameters");
const TCHAR		CIPCCommandHandler::RETURN_VAL_NODE[]=_T("returnvalue");
const TCHAR		CIPCCommandHandler::LAST_ERROR_MSG_NODE[]=_T("lasterror");

const TCHAR		CIPCCommandHandler::IPC_NAME[]=TEXT("\\\\.\\pipe\\{3FAB0660-208D-4898-AA25-14D0DD1B5E3B}.LANoiIPC"); 
const TCHAR		CIPCCommandHandler::IPC_NAME_XML[]=TEXT("LANoiIPC"); 

CIPCCommandHandler::CIPCCommandHandler(void)
{
	m_hCmdNotify=NULL;
}

CIPCCommandHandler::~CIPCCommandHandler(void)
{
	m_ServiceEventNotify.UnRegisterEvent();
	if(m_hCmdNotify)
	{
		CloseHandle(m_hCmdNotify); 
		m_hCmdNotify=NULL;
	}
}

/***************************************
Invoke this to get notification from UI
***************************************/
BOOL	CIPCCommandHandler::NotifyUIRequest(HANDLE hCmdNotify)
{
	if(!m_IPC.InitServer(IPC_NAME,hCmdNotify))
	{
		SetLastErrorMessage(m_IPC.GetLastErrorMessage()); 
	}
	return m_IPC.PendClientAccept(); 
}
/******************************************
Invoke IPC to service - can be used to invoke  RPC functions
*******************************************/
BOOL	CIPCCommandHandler::NotifyServiceRequest(HWND hNotifyWnd,UINT uiNotifyMsg)
{
	m_hCmdNotify=CreateEvent(NULL,FALSE,FALSE,NULL); 


	if(m_IPC.ConnectToServer(IPC_NAME,m_hCmdNotify))
	{	
		m_ServiceEventNotify.RegisterEvent(m_hCmdNotify,hNotifyWnd,uiNotifyMsg); 
	//	return TRUE;//only one way communication
		return m_IPC.PendSingleRead();
	}
	SetLastErrorMessage(m_IPC.GetLastErrorMessage()); 
	return FALSE;
}
/************************************************
Called after hCmdNotify event is set
************************************************/
BOOL	CIPCCommandHandler::HandleCommand(IRPCExecute *pRPCExecute)
{
	BOOL			bRet=FALSE;

	switch(m_IPC.GetPendingState())
	{
		//case CIPC::WAITING_FOR_CLIENT:
		//	bRet=m_IPC.PendSingleRead();//client has been connected try read the data
		//	break;
		case CIPC::CLIENT_ACCEPT_PENDING:
			bRet=m_IPC.PendSingleRead();//client has been connected try read the data
			break;
		case CIPC::CLIENT_READ_PENDING:
			{
				BYTE		*pReceivedStream=NULL;
				DWORD		dwDataLen=0;
				//read the data pipe
				if(m_IPC.ReadStream(pReceivedStream,dwDataLen) )
				{
					//Execute the function command 
					if(dwDataLen)
					{
						ExecuteRPCFunction(pRPCExecute,pReceivedStream,dwDataLen);
						m_IPC.PendSingleRead();//pend another read
						bRet= TRUE;
					}
					break;
				}
				//connection dropped wait for another client
				if(m_IPC.GetSessionType()==CIPC::SERVER)  
				{
					DEBUGLOG(LOI_INFO,_T("IPC_ERROR PendClientAccept"));
					bRet=m_IPC.PendClientAccept();
				}
				//connection to server failed retry again
				if(m_IPC.GetSessionType()==CIPC::CLIENT)
				{
					DEBUGLOG(LOI_INFO,_T("IPC_ERROR ReconnectToServer"));
					bRet=m_IPC.ReconnectToServer(IPC_NAME); 
				}
			}
			break;
			//default restart the connection
		default:
				//connection dropped wait for another client
				if(m_IPC.GetSessionType()==CIPC::SERVER)  
				{
					DEBUGLOG(LOI_INFO,_T("IPC_ERROR PendClientAccept"));
					bRet=m_IPC.PendClientAccept();
				}
				//connection to server failed retry again
				if(m_IPC.GetSessionType()==CIPC::CLIENT)
				{
					DEBUGLOG(LOI_INFO,_T("IPC_ERROR ReconnectToServer"));
					bRet=m_IPC.ReconnectToServer(IPC_NAME); 
				}
				break;
	}

	//set the error message for the last ipc call
	SetLastErrorMessage( m_IPC.GetLastErrorMessage()); 
	return bRet;
}
/***************************************
get the command from the XML request & invoke the functions
***************************************/
BOOL CIPCCommandHandler::ExecuteRPCFunction(IRPCExecute *pRPCExecute,BYTE *pByXMLData,DWORD dwDataLen)
{
	CXmlSerializer      XmlReq;
	IPC_COMMAND			IpcCommand;
	DWORD				dwRet=0;


	XmlReq.LoadXML (CString((LPCTSTR)pByXMLData),IPC_NAME_XML);
	IpcCommand=GetCommand( XmlReq);
	

	switch(IpcCommand)
	{
		case CREATE_INVITATION:
			{
				CString		 sNickName,sPassword,sSessionName,sSessionXMLString,sFilepath;
				FILETIME	 ftNotValidAfterTime;
				BYTE		 *pbyAppdata=new BYTE[20*1024];//max appdata
				DWORD		  dwAppDataLen=20*1024;

				sNickName=GetStringParam(XmlReq);
				sPassword=GetStringParam(XmlReq);
				sSessionName=GetStringParam(XmlReq);
				sSessionXMLString=GetStringParam(XmlReq);
				GetBytesParam(XmlReq,(PBYTE)&ftNotValidAfterTime,sizeof(ftNotValidAfterTime)); 
				sFilepath=GetStringParam(XmlReq);
				dwAppDataLen=GetBytesParam(XmlReq,pbyAppdata,dwAppDataLen);
				if(dwAppDataLen)
				{
					dwAppDataLen= GetDWORDParam(XmlReq);
				}
					
				dwRet=pRPCExecute->CreateInvitation(sNickName,sPassword,sSessionName,sSessionXMLString,
													ftNotValidAfterTime,
													sFilepath,
													dwAppDataLen==0?NULL:pbyAppdata,//pas null if len zero
													dwAppDataLen);
				delete pbyAppdata;
				
			}
			break;
		case JOIN_INVITATION:
			{
				CString			sPeerName,sFilePath,sPassword;

				DEBUGLOG(LOI_INFO,_T("Executing JOIN_INVITATION"));
				sPeerName=GetStringParam(XmlReq);
				sFilePath=GetStringParam(XmlReq);
				sPassword=GetStringParam(XmlReq);
				dwRet=pRPCExecute->JoinInvitation(sPeerName,sFilePath,sPassword);
				
			}
			break;

		case ADD_FIREWALL:
			{
				DWORD    dwProcessID;


				dwProcessID=GetDWORDParam(XmlReq);
				dwRet=pRPCExecute->FirewallAdd(dwProcessID);
				
			}
			break;
		case GET_P2PSTATUS:
			{
				dwRet= pRPCExecute->GetP2PStatus(); 
			}
			break;
		case ON_P2P_STATUS_CHANGE:
			{
				BOOL  bEnabled;
				bEnabled=(BOOL)GetDWORDParam(XmlReq);
				pRPCExecute->PostP2PStatusChange( bEnabled);
			}
			break;
		case JOIN_INVITATION_STATUS:
			{
				BOOL		bConnectSuccess;
				CString		sErrorMessage;
				CString		sSessionName;
				CString		sInviterName;

				DEBUGLOG(LOI_INFO,_T("Executing JOIN_INVITATION_STATUS"));
				bConnectSuccess=GetDWORDParam(XmlReq);
				sErrorMessage=GetStringParam(XmlReq);
				sSessionName=GetStringParam(XmlReq);
				sInviterName=GetStringParam(XmlReq);

				dwRet=pRPCExecute->PostJoinInvitationStatus(bConnectSuccess,sErrorMessage,sSessionName,sInviterName);
				DEBUGLOG(LOI_INFO,_T("Execute Exit JOIN_INVITATION_STATUS"));
			}
			break;
		case CREATE_INVITATION_STATUS:
			{
				BOOL		bSuccess;
				CString		sErrorMessage;
				CString		sSessionName;

				DEBUGLOG(LOI_INFO,_T("Executing CREATE_INVITATION_STATUS"));
				sSessionName=GetStringParam(XmlReq);
				bSuccess=GetDWORDParam(XmlReq);
				sErrorMessage=GetStringParam(XmlReq);
				
				dwRet=pRPCExecute->PostCreateInvitationStatus(sSessionName,bSuccess,sErrorMessage);
				DEBUGLOG(LOI_INFO,_T("Execute Exit CREATE_INVITATION_STATUS"));
			}
			break;
		case INVITATION_APP_DATA:
			{
				BYTE		*pbyAppData=new BYTE[20*1024];
				DWORD		dwAppDataLen=20*1024;

				DEBUGLOG(LOI_INFO,_T("Executing INVITATION_APP_DATA"));
				GetBytesParam(XmlReq,pbyAppData,dwAppDataLen );
				dwAppDataLen=GetDWORDParam(XmlReq);
				
				dwRet=pRPCExecute->PostInvitationAppData (pbyAppData,dwAppDataLen);
				DEBUGLOG(LOI_INFO,_T("Execute Exit INVITATION_APP_DATA"));
			}
			break;
		case CLOSE_SERVER_SESSION:
			{
				CString		sSessionID;
			
				sSessionID=GetStringParam(XmlReq);
				DEBUGLOG(LOI_INFO,_T("Executing CLOSE_SERVER_SESSION %s"),(LPCTSTR)sSessionID);
				pRPCExecute->CloseSession(sSessionID);
				DEBUGLOG(LOI_INFO,_T("Execute Exit CLOSE_SERVER_SESSION"));
			}
			break;

	}
	
	return TRUE;
}
/***********************************************
this writes the return value to the pipe
***********************************************/
void CIPCCommandHandler::SetDWORDReturnValue(CString sErrorMsg,DWORD dwRet)
{
	CXmlSerializer      XmlRet;
	CString				sXmlRet;

	//write the return value
	XmlRet.Create( IPC_NAME_XML);
	XmlRet.WriteGroupBegin(RETURN_VAL_NODE);
	XmlRet.Write(RETURN_VAL_NODE,dwRet);
	XmlRet.WriteGroupEnd();

	//write error message
	XmlRet.WriteGroupBegin(LAST_ERROR_MSG_NODE);
	XmlRet.Write(LAST_ERROR_MSG_NODE,(LPCTSTR)sErrorMsg);
	XmlRet.WriteGroupEnd();


	sXmlRet=XmlRet.GetXml(); 
	m_IPC.WriteStream((LPBYTE)(LPCTSTR)sXmlRet,sXmlRet.GetLength()*sizeof(TCHAR)+sizeof(TCHAR));
}
/********************************************
Reads the return value from the pipe
*******************************************/
DWORD CIPCCommandHandler::ExecuteStub(CXmlSerializer      &XmlReq,BOOL bGetReturnValue)
{
	CString				sXmlRPC;
	BYTE				*pReceivedStream=NULL;
	DWORD				dwDataLen=0;


	sXmlRPC=XmlReq.GetXml(); 

	m_IPC.WriteStream((LPBYTE)(LPCTSTR)sXmlRPC,sXmlRPC.GetLength()*sizeof(TCHAR)+sizeof(TCHAR));
	
	//get the return value synchronously
	//m_IPC.PendSingleRead();
	//read the data pipe
//	if(m_IPC.ReadStream(pReceivedStream,dwDataLen))
	if(bGetReturnValue &&
		//m_IPC.ReadStream(pReceivedStream,dwDataLen))
		m_IPC.ReadStreamSync(pReceivedStream,dwDataLen)
		)
	{
		CXmlSerializer      XmlReturn;	
		DWORD				dwRet=0;
		CString				sErrorMsg;

		XmlReq.LoadXML (CString((LPCTSTR)pReceivedStream),IPC_NAME_XML);
		XmlReq.ReadGroupBegin(RETURN_VAL_NODE);
		XmlReq.Read(RETURN_VAL_NODE,dwRet);
		XmlReq.ReadGroupEnd();

		//get last error value
		XmlReq.ReadGroupBegin(LAST_ERROR_MSG_NODE);
		XmlReq.Read(LAST_ERROR_MSG_NODE,sErrorMsg);
		XmlReq.ReadGroupEnd();
		SetLastErrorMessage(sErrorMsg); 

		return dwRet;
	}



	SetLastErrorMessage(  m_IPC.GetLastErrorMessage ());
	return 0;

}
/************************************************************************
Invokes the createinvitation via ipc & gets the ret value synchrosously
************************************************************************/
BOOL CIPCCommandHandler::CreateInvitation(CString sNickName,CString sPassword,CString sSessionName,CString sSessionXMLString,FILETIME ftNotValidAfterTime,CString sFilepath,BYTE *pbyAppdata,DWORD dwAppDataLen)
{
	CXmlSerializer      XmlReq;
	BOOL				bRet;


	XmlReq.Create(IPC_NAME_XML);
	SetCommand(XmlReq,CREATE_INVITATION);
	//sets the parameter in the same order
	SetStringParam(XmlReq,sNickName);
	SetStringParam(XmlReq,sPassword);
	SetStringParam (XmlReq,sSessionName);
	SetStringParam (XmlReq,sSessionXMLString);
	SetBytesParam(XmlReq,(PBYTE)&ftNotValidAfterTime,sizeof(ftNotValidAfterTime)); 
	SetStringParam(XmlReq,sFilepath);
	if(pbyAppdata)
	{
		SetBytesParam(XmlReq,pbyAppdata,dwAppDataLen);
		SetDWORDParam(XmlReq,dwAppDataLen); 
	}
	bRet=ExecuteStub(XmlReq,FALSE);
	return bRet;
}
/******************************************************************************
Invokes the join invitation & get the ret value from RPC
******************************************************************************/
BOOL CIPCCommandHandler::JoinInvitation(CString sPeerName,CString sFilePath,CString sPassword)
{
	CXmlSerializer      XmlReq;
	BOOL				bRet;

	DEBUGLOG(LOI_INFO,_T("Calling JoinInvitation"));
	XmlReq.Create(IPC_NAME_XML);
	SetCommand(XmlReq,JOIN_INVITATION);
	//sets the parameter in the same order
	SetStringParam(XmlReq,sPeerName);
	SetStringParam(XmlReq,sFilePath);
	SetStringParam(XmlReq,sPassword);
	bRet=ExecuteStub(XmlReq,FALSE);
	DEBUGLOG(LOI_INFO,_T("Call Exit JoinInvitation"));
	return bRet;
}

/*********************************
firewall add
*********************************/
BOOL CIPCCommandHandler::FirewallAdd(DWORD dwProcessID)
{
	CXmlSerializer      XmlReq;
	BOOL				bRet;

	XmlReq.Create(IPC_NAME_XML);
	SetCommand(XmlReq,ADD_FIREWALL);
	//sets the parameter in the same order
	SetDWORDParam(XmlReq,dwProcessID);
	bRet=ExecuteStub(XmlReq,FALSE);
	return bRet;
}
/**********************************
retrieves p2p status
**********************************/
BOOL CIPCCommandHandler::GetP2PStatus()
{
	CXmlSerializer      XmlReq;
	BOOL				bRet;

	XmlReq.Create(IPC_NAME_XML);
	SetCommand(XmlReq,GET_P2PSTATUS);
	bRet=ExecuteStub(XmlReq,FALSE);
	return bRet;
}
/**********************************
Close the particualt session id
**********************************/
void CIPCCommandHandler::CloseSession(CString sSessionID)
{
	CXmlSerializer      XmlReq;
	BOOL				bRet;

	XmlReq.Create(IPC_NAME_XML);
	SetCommand(XmlReq,CLOSE_SERVER_SESSION);
	SetStringParam(XmlReq,sSessionID);
	bRet=ExecuteStub(XmlReq,FALSE);
	
}
/**********************
Updates Invitaation creation Status
***********************/
BOOL CIPCCommandHandler::PostCreateInvitationStatus(CString sSessionName,BOOL bSuccess,CString sErrorMessage)
{
	CXmlSerializer      XmlReq;
	BOOL				bRet;

	DEBUGLOG(LOI_INFO,_T("Calling PostCreateInvitationStatus"));
	XmlReq.Create(IPC_NAME_XML);
	SetCommand(XmlReq,CREATE_INVITATION_STATUS);
	//sets the parameter in the same order
	SetStringParam(XmlReq,sSessionName);
	SetDWORDParam(XmlReq,bSuccess);
	SetStringParam(XmlReq,sErrorMessage);

	bRet=ExecuteStub(XmlReq,FALSE);
	DEBUGLOG(LOI_INFO,_T("Call Exit PostCreateInvitationStatus"));
	return bRet;
}
/***********************************
updates the status about the network invitation joined
*************************************/
BOOL CIPCCommandHandler::PostJoinInvitationStatus(BOOL bConnectSuccess,CString sErrorMessage,CString sSessionName,CString sInviterName)
{
	CXmlSerializer      XmlReq;
	BOOL				bRet;

	DEBUGLOG(LOI_INFO,_T("Calling PostJoinInvitationStatus"));
	XmlReq.Create(IPC_NAME_XML);
	SetCommand(XmlReq,JOIN_INVITATION_STATUS);
	//sets the parameter in the same order
	SetDWORDParam(XmlReq,bConnectSuccess);
	SetStringParam(XmlReq,sErrorMessage);
	SetStringParam(XmlReq,sSessionName);
	SetStringParam(XmlReq,sInviterName);
	
	bRet=ExecuteStub(XmlReq,FALSE);
	DEBUGLOG(LOI_INFO,_T("Call Exit PostJoinInvitationStatus"));
	return bRet;
}
/*****************************************
Post app data associated with a invitation 
******************************************/
BOOL CIPCCommandHandler::PostInvitationAppData(BYTE *pbyAppData,DWORD dwAppDataLen)
{
	CXmlSerializer      XmlReq;
	BOOL				bRet;

	DEBUGLOG(LOI_INFO,_T("Calling PostInvitationAppData"));
	XmlReq.Create(IPC_NAME_XML);
	SetCommand(XmlReq,INVITATION_APP_DATA);
	//sets the parameter in the same order
	SetBytesParam(XmlReq,pbyAppData,dwAppDataLen); 
	SetDWORDParam(XmlReq,dwAppDataLen);
	bRet=ExecuteStub(XmlReq,FALSE);
	DEBUGLOG(LOI_INFO,_T("Call Exit PostInvitationAppData"));
	return bRet;
}
/*********************************
notiify p2p status to the other party
**********************************/
BOOL	CIPCCommandHandler::PostP2PStatusChange(BOOL bEnabled)
{
	CXmlSerializer      XmlReq;

	DEBUGLOG(LOI_INFO,_T("Call  PostP2PStatusChange %d"),bEnabled);
	XmlReq.Create(IPC_NAME_XML);
	SetCommand(XmlReq,ON_P2P_STATUS_CHANGE);
	//sets the parameter in the same order
	SetDWORDParam(XmlReq,(DWORD)bEnabled);
	return ExecuteStub(XmlReq,FALSE);
}
/************************************************
Gets the command from the XML 
************************************************/
CIPCCommandHandler::IPC_COMMAND CIPCCommandHandler::GetCommand(CXmlSerializer  &XmlReq)
{
	DWORD    dwCommand;

	XmlReq.ReadGroupBegin(COMMAND_NODE);
	XmlReq.Read( COMMAND_NODE,dwCommand);
	XmlReq.ReadGroupEnd(); 

	return IPC_COMMAND(dwCommand);
}

/*********************************************
Sets the command in the XML
*********************************************/
void CIPCCommandHandler::SetCommand(CXmlSerializer  &XmlReq,IPC_COMMAND Command)
{
	XmlReq.WriteGroupBegin(COMMAND_NODE);
	XmlReq.Write(COMMAND_NODE,DWORD(Command));
	XmlReq.WriteGroupEnd(); 
}
/*********************************************************
Sets the string param on the xml file
**********************************************************/
void CIPCCommandHandler::SetStringParam(CXmlSerializer  &XmlReq,CString sParam)
{
	XmlReq.WriteGroupBegin(PARAM_NODE);
	XmlReq.Write(PARAM_NODE,(LPCTSTR)sParam);
	XmlReq.WriteGroupEnd(); 
}
/*******************************************************
Gets the string param from the xml request
*******************************************************/
CString CIPCCommandHandler::GetStringParam(CXmlSerializer  &XmlReq)
{
	CString sRet;
	XmlReq.ReadGroupBegin(PARAM_NODE);
	XmlReq.Read(PARAM_NODE, sRet);
	XmlReq.ReadGroupEnd(); 
	return sRet;

}
/***************************************
Gets DWORD from the XML
****************************************/
DWORD CIPCCommandHandler::GetDWORDParam(CXmlSerializer  &XmlReq)
{
	DWORD dwRet;
	XmlReq.ReadGroupBegin(PARAM_NODE);
	XmlReq.Read(PARAM_NODE,dwRet);
	XmlReq.ReadGroupEnd(); 
	return dwRet;
}
/*************************************
Sets the DWORD to the xml
*************************************/
void CIPCCommandHandler::SetDWORDParam(CXmlSerializer  &XmlReq,DWORD dwValue)
{
	XmlReq.WriteGroupBegin(PARAM_NODE);
	XmlReq.Write(PARAM_NODE,dwValue);
	XmlReq.WriteGroupEnd();
}
/***********************************************
Gets BYTES
& return the bytes copied
************************************************/
DWORD CIPCCommandHandler::GetBytesParam(CXmlSerializer  &XmlReq,PBYTE pParam,DWORD dwParamLen)
{
	PBYTE		pTemp=NULL;
	DWORD		dwLen=0;
	DWORD		dwBytesCopied=0;

	if(XmlReq.ReadGroupBegin(PARAM_NODE))
	{
		XmlReq.Read(PARAM_NODE,pTemp,dwLen);
		if(dwLen)
		{
			if(dwParamLen>=dwLen)
			{
				memcpy(pParam,pTemp,dwLen);
				dwBytesCopied=dwLen;
			}
			XmlReq.FreeBytes(pTemp);
		}
		XmlReq.ReadGroupEnd();
	}

	return dwBytesCopied;
}
/**********************************************
sets the bytes
**********************************************/
void CIPCCommandHandler::SetBytesParam(CXmlSerializer  &XmlReq,PBYTE pParam,DWORD dwParamLen)
{
	XmlReq.WriteGroupBegin(PARAM_NODE);
	XmlReq.Write(PARAM_NODE,pParam,dwParamLen);
	XmlReq.WriteGroupEnd();
}