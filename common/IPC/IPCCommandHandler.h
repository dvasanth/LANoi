#pragma once
#include "error.h"
#include "XmlSerializer.h"
#include "cmnresource.h"
#include "eventnotify.h"
#include "../loisession/ipcfilemap.h"
#include "../processlock.h"
class CIPC:public CError{
public:
	enum IPC_STATE{
		WAITING_FOR_CLIENT,
		CLIENT_ACCEPT_PENDING,
		CLIENT_READ_PENDING,
		IPC_ERROR,
		CONNECTED_TO_SERVER
	};
	enum IPC_SESSION_TYPE{
		NONE,
		SERVER,
		CLIENT
	};
private:
	OVERLAPPED   *m_pOverlap;
	HANDLE		 m_hPipe;
	PBYTE		 m_pRecvStream;
	DWORD		 m_dwTotalRecvLen;
	IPC_STATE	 m_PipeState;		
	IPC_SESSION_TYPE m_SessionType;
	
public:
	CIPC()
	{
		m_pOverlap = new OVERLAPPED;
		m_hPipe=NULL;
		m_dwTotalRecvLen=20*1024;
		m_pRecvStream=new BYTE[m_dwTotalRecvLen];
		m_PipeState=IPC_ERROR;
		m_SessionType=NONE;
	}
	~CIPC()
	{
		Close();
		if(m_pRecvStream)
		{
			delete m_pRecvStream;
			m_pRecvStream=NULL;
		}
	}
	void Close()
	{
		if(m_hPipe && m_hPipe!=INVALID_HANDLE_VALUE)
		{
			CancelIo (m_hPipe);
			CloseHandle(m_hPipe);
			m_hPipe=NULL;
		}
		if(m_pOverlap)
		{
			delete m_pOverlap;
			m_pOverlap=NULL;
		}

	}
	BOOL InitServer(LPCTSTR lpszIPCName,HANDLE hOverlappedRecvEvent)
	{
		CSecurityObject Sa;
		m_SessionType=SERVER;
		m_hPipe= CreateNamedPipe( 
								 lpszIPCName,            // pipe name 
								 PIPE_ACCESS_DUPLEX |     // read/write access 
								 FILE_FLAG_OVERLAPPED,    // overlapped mode 
								 PIPE_TYPE_MESSAGE |      // message-type pipe 
								 PIPE_READMODE_MESSAGE |  // message-read mode 
								 PIPE_WAIT,               // blocking mode 
								 PIPE_UNLIMITED_INSTANCES,               // number of instances 
								 m_dwTotalRecvLen,   // output buffer size 
								 m_dwTotalRecvLen,   // input buffer size 
								 5000,            // client time-out 
								  Sa.GetAllAccessSA() );                   // default security attributes 
		if(m_hPipe== INVALID_HANDLE_VALUE)
		{
			SetLastErrorMessage(IDS_ERR_IPC_FAILED_TO_START,GetLastError()); 
			return FALSE;
		}
		//access permission to all
		CSecurityObject::SetObjectToLowIntegrity(m_hPipe);
		//initialise overlapped event
		ZeroMemory(m_pOverlap,sizeof(*m_pOverlap));
		m_pOverlap->hEvent=hOverlappedRecvEvent;
		return TRUE;
	}

	BOOL PendClientAccept()
	{
		BOOL		bRet;
		
		//disconnect old clients if exists
		DisconnectNamedPipe (m_hPipe);

		bRet=ConnectNamedPipe(m_hPipe, m_pOverlap); 
		//failure check th reason
		if(!bRet && GetLastError()==ERROR_IO_PENDING)
		{
			//operation queued successfully
			m_PipeState=CLIENT_ACCEPT_PENDING;
			return TRUE;
		}
		if(bRet || GetLastError()==ERROR_PIPE_CONNECTED)
		{
			//connect successful 
			SetEvent(m_pOverlap->hEvent);
			//so the caller invokes/queues read
			m_PipeState=CLIENT_ACCEPT_PENDING;
			
			return TRUE;
		}
		
		if(!bRet)
		{
			SetLastErrorMessage(IDS_ERR_IPC_FAILED_TO_START,GetLastError()); 
			return FALSE;
		}
		return TRUE;
	}
	BOOL ConnectToServer(LPCTSTR lpszIPCName,HANDLE hOverlappedRecvEvent)
	{
		int			iRetryCount=3;

		m_SessionType=CLIENT;
		while(iRetryCount>0)
		{
			 m_hPipe = CreateFile( 
								 lpszIPCName,   // pipe name 
								 GENERIC_READ |  // read and write access 
								 GENERIC_WRITE, 
								 0,              // no sharing 
								 NULL,           // default security attributes
								 OPEN_EXISTING,  // opens existing pipe 
								 FILE_FLAG_OVERLAPPED,//0,//one way communication//              // default attributes 
								 NULL);          // no template file 
			 
			 if (m_hPipe != INVALID_HANDLE_VALUE) 
			 {
				 DWORD   dwMode = PIPE_READMODE_MESSAGE; 
				 SetNamedPipeHandleState( 
										  m_hPipe,    // pipe handle 
										  &dwMode,  // new pipe mode 
										  NULL,     // don't set maximum bytes 
										  NULL);    // don't set maximum time 
				 m_PipeState=CONNECTED_TO_SERVER;
				//initialise overlapped event
				ZeroMemory(m_pOverlap,sizeof(*m_pOverlap));
				m_pOverlap->hEvent=hOverlappedRecvEvent;
				
				 return TRUE;
			 }

         	  // Break if the pipe handle is valid. 
			  if (GetLastError() != ERROR_PIPE_BUSY) 
			  {
				  SetLastErrorMessage(IDS_ERR_SERVICE_CONNECT_FAILED,GetLastError()); 
				  return FALSE;
			  }

			  //wait for some time before giving up
			  if (!WaitNamedPipe(lpszIPCName, 30000)) 
			  { 
				 SetLastErrorMessage(IDS_ERR_SERVICE_CONNECT_FAILED,GetLastError());  
				 return FALSE;
			  } 
			  iRetryCount--;
		
		}//while
		return FALSE;
	}//function end

	BOOL ReconnectToServer(LPCTSTR lpszIPCName)
	{
		return ConnectToServer(lpszIPCName,m_pOverlap->hEvent);
	}
	IPC_STATE GetPendingState()
	{
		return m_PipeState;
	}
	IPC_SESSION_TYPE GetSessionType()
	{
		return m_SessionType;
	}
	//only one pend recv request is allowed
	BOOL PendSingleRead()
	{
		BOOL		bRet;
		DWORD		dwDataRead;
		bRet=ReadFile(m_hPipe,m_pRecvStream,m_dwTotalRecvLen,&dwDataRead,m_pOverlap);
		
		if(!bRet && GetLastError ()!= ERROR_IO_PENDING)
		{
			m_PipeState=IPC_ERROR;
			return FALSE;
		}
		m_PipeState=CLIENT_READ_PENDING;
		return TRUE;//read is pend
	}
	void CancelPendingRead()
	{
		CancelIo(m_hPipe);
	}
	BOOL IsPipeActive()
	{
		return (m_PipeState==CLIENT_READ_PENDING)|| (m_PipeState==CONNECTED_TO_SERVER);
				
	}
	//if true then requested bytes copied else wait for the event to complete the recv
	///invoke this only after pending a read & the event is set
	BOOL ReadStream(PBYTE &pReceivedStream,DWORD &dwReadLen)
	{
		BOOL		bRet;
		
	
		if(!IsPipeActive())
			return FALSE;
		dwReadLen=0;
		bRet=GetOverlappedResult(m_hPipe,m_pOverlap,&dwReadLen,FALSE);
		if (bRet && dwReadLen) 
		{
			pReceivedStream=m_pRecvStream;
			return TRUE;
		}
		if(GetLastError()==ERROR_IO_INCOMPLETE)
		{
			return TRUE;
		}
		//failure
		m_PipeState=IPC_ERROR;
		SetLastErrorMessage(IDS_ERR_IPC_READ_WRITE_ERROR,GetLastError());
		return FALSE;
	}
	BOOL ReadStreamSync(PBYTE &pReceivedStream,DWORD &dwReadLen)
	{
		BOOL		  bRet;

		if(!IsPipeActive())
			return FALSE;

		dwReadLen=0;
		bRet=ReadFile(m_hPipe,m_pRecvStream,m_dwTotalRecvLen,&dwReadLen,NULL);
		if(bRet && dwReadLen )
		{
			pReceivedStream=m_pRecvStream;
			return TRUE;
		}
		return FALSE;
	}
	//write is always synchrnous write call
	BOOL WriteStream(BYTE *pWriteStream,DWORD dwWriteLen)
	{
		OVERLAPPED    WriteIO={0};
		DWORD		  dwWrittenLen;
		BOOL		  bRet;

		if(!IsPipeActive())
					return FALSE;
		WriteIO.hEvent=WSACreateEvent();
		bRet=WriteFile(m_hPipe,pWriteStream,dwWriteLen,&dwWrittenLen,&WriteIO);
		//immediate failure
		if(!bRet && GetLastError ()!= ERROR_IO_PENDING)
		{
			m_PipeState=IPC_ERROR;
			SetLastErrorMessage(IDS_ERR_IPC_READ_WRITE_ERROR,GetLastError());
			CloseHandle(WriteIO.hEvent);
			return FALSE;
		}
		//immediate success
		if(bRet && dwWriteLen==dwWrittenLen)
		{
			CloseHandle(WriteIO.hEvent);
			return TRUE;
		}
		bRet=GetOverlappedResult(m_hPipe,&WriteIO,&dwWrittenLen,TRUE);
		if(!bRet||dwWriteLen!=dwWrittenLen)
		{
			SetLastErrorMessage(IDS_ERR_IPC_READ_WRITE_ERROR,GetLastError()); 
			m_PipeState=IPC_ERROR;
			CloseHandle(WriteIO.hEvent);
			return FALSE;
		}

		CloseHandle(WriteIO.hEvent);
		return TRUE;
	}
};

class IRPCExecute:public CError{
public:
	virtual BOOL CreateInvitation(CString sNickName,CString sPassword,CString sSessionName,CString sSessionXMLString,FILETIME ftNotValidAfterTime,CString sFilepath,BYTE *pbyAppdata,DWORD dwAppDataLen){return FALSE;};
	virtual BOOL JoinInvitation(CString sPeerName,CString sFilePath,CString sPassword){return FALSE;}
	virtual BOOL FirewallAdd(DWORD dwProcessID){return FALSE;}
	virtual BOOL GetP2PStatus(){return FALSE;}
	virtual void CloseSession(CString sSessionID){}
	virtual BOOL PostP2PStatusChange(BOOL bEnabled){return FALSE;}
	virtual BOOL PostCreateInvitationStatus(CString sSessionName,BOOL bSuccess,CString sErrorMessage){return FALSE;};
	virtual BOOL PostJoinInvitationStatus(BOOL bConnectSuccess,CString sErrorMessage,CString sSessionName,CString sInviterName){return FALSE;}
	virtual BOOL PostInvitationAppData(BYTE *pbyAppData,DWORD dwAppDataLen){return FALSE;}
};
class IRPCProxyStub:public IRPCExecute{

};
class CIPCCommandHandler:public IRPCProxyStub
{
public:
	enum IPC_COMMAND{
		CREATE_INVITATION,
		JOIN_INVITATION,
		JOIN_INVITATION_STATUS,
		ADD_FIREWALL,
		GET_P2PSTATUS,
		ON_P2P_STATUS_CHANGE,
		CREATE_INVITATION_STATUS,
		INVITATION_APP_DATA,
		CLOSE_SERVER_SESSION
	};
	CIPCCommandHandler(void);
	~CIPCCommandHandler(void);
	BOOL				NotifyUIRequest(HANDLE hCmdNotify);
	BOOL				NotifyServiceRequest(HWND hNotifyWnd,UINT uiNotifyMsg);
	BOOL				HandleCommand(IRPCExecute *pRPCExecute);
	virtual BOOL		CreateInvitation(CString sNickName,CString sPassword,CString sSessionName,CString sSessionXMLString,FILETIME ftNotValidAfterTime,CString sFilepath,BYTE *pbyAppdata,DWORD dwAppDataLen);
	virtual BOOL		JoinInvitation(CString sPeerName,CString sFilePath,CString sPassword);
	virtual BOOL		FirewallAdd(DWORD dwProcessID);
	virtual BOOL		GetP2PStatus();
	virtual void		CloseSession(CString sSessionID);
	virtual BOOL		PostP2PStatusChange(BOOL bEnabled);
	virtual BOOL		PostJoinInvitationStatus(BOOL bConnectSuccess,CString sErrorMessage,CString sSessionName,CString sInviterName);
	virtual BOOL		PostCreateInvitationStatus(CString sSessionName,BOOL bSuccess,CString sErrorMessage);
	virtual BOOL		PostInvitationAppData(BYTE *pbyAppData,DWORD dwAppDataLen);
private:
	static const TCHAR		IPC_NAME[];
	static const TCHAR		IPC_NAME_XML[];
////xml nodes
	static const TCHAR		COMMAND_NODE[];
	static const TCHAR		PARAM_NODE[];
	static const TCHAR		RETURN_VAL_NODE[];
	static const TCHAR		LAST_ERROR_MSG_NODE[];

	HANDLE					m_hCmdNotify;
	CEventNotify			m_ServiceEventNotify;

//xml attributes
	CIPC					m_IPC;

	DWORD					ExecuteStub(CXmlSerializer      &XmlReq,BOOL bGetReturnValue);
	void					SetDWORDReturnValue(CString sErrorMsg,DWORD dwRet);
	BOOL					ExecuteRPCFunction(IRPCExecute *pRPCExecute,BYTE *pByXMLData,DWORD dwDataLen);
	IPC_COMMAND				GetCommand(CXmlSerializer  &XmlReq);
	void					SetCommand(CXmlSerializer  &XmlReq,IPC_COMMAND Command);
	void					SetStringParam(CXmlSerializer  &XmlReq,CString sParam);
	CString					GetStringParam(CXmlSerializer  &XmlReq);
	DWORD					GetDWORDParam(CXmlSerializer  &XmlReq);
	void					SetDWORDParam(CXmlSerializer  &XmlReq,DWORD dwValue);
	DWORD					GetBytesParam(CXmlSerializer  &XmlReq,PBYTE pParam,DWORD dwParamLen);
	void					SetBytesParam(CXmlSerializer  &XmlReq,PBYTE pParam,DWORD dwParamLen);

};
