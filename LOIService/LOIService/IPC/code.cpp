

class IFunctionImpl{
		DWORD GetFunctionID(IIPC *pIpc)
		{
		}
		void SetFucntionID(IIPC *pIpc)
		{

		}
		void GetNextParameter(IIPC *pIpc,PVOID &pVoid,DWORD &dwSize)
		{

		}
		void AddParameter(IIPC *pIpc,PVOID pByte,DWORD dwSize)
		{

		}
		DWORD GetReturnValue(IIPC *pIpc)
		{
		}
	};

class IIPCUIToServiceCmdHandler{
	IIPCCmdHandler()
	{
	}
	~IIPCCmdHandler()
	{
	}
	
	class CCreateInvitation {
	{
		protected:
		//deserialiaze & invoke the dispatch the function
		void Dispatch(IIPC *pIpc, IIPCUIToServiceCmdHandler pHandler)
		{
			//read parameters
			//callc
			pHandler->OnCreateInvitation
			//write the return value
			
		}
		//
		DWORD Execute(IIPC *pIpc,LPCTSTR lpszNickName,LPCTSTR lpszSessionName,LPCTSTR lpszSessionString)
		{
			
		}


	};

	

	virtual void OnCreateInvitation(LPCTSTR lpszNickName,LPCTSTR lpszSessionName,LPCTSTR lpszSessionString)
	{
		 
	}
	void CreateInvitation(LPCTSTR lpszNickName,LPCTSTR lpszSessionName,LPCTSTR lpszSessionString)
	{
		CCreateInvitation Send;

		Send.Execute(
		 
	}


	void OnJoinInvitation(LPCTSTR lpszFilePath,LPCTSTR lpszPassword)
	{
		
	}
	void UpdateInviterConnectionStatus(P2PEVENT )
	{
	}
	
	void UpdataApplicationSessionInformation(LPTSTR lpszXmlSession)
	{
	}
};

class IIPCServiceToUICmdHandler{

	void CreateInvitation(LPCTSTR lpszNickName,LPCTSTR lpszSessionName,LPCTSTR lpszSessionString)
	{
		 
	}
	void JoinInvitation(LPCTSTR lpszFilePath,LPCTSTR lpszPassword)
	{
		
	}
	void OnInviterConnectionStatus(P2PEvent)
	{
	}

	//sessiondata
};