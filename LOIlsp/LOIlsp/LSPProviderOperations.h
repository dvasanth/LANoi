#pragma once

class CLSPProviderOperations
{
public:
	CLSPProviderOperations(void);
	~CLSPProviderOperations(void);


	PROVIDER *			GetProvider( int af,int type,int protocol,LPWSAPROTOCOL_INFOW lpProtocolInfo,GROUP g,DWORD dwFlags,WSAPROTOCOL_INFOW& InfoCopy,LPINT  lpErrno);
	PROVIDER *			GetProvider(LPWSAPROTOCOL_INFOW lpProtocolInfo,LPINT lpErrno);


	INT					Initialize(WORD  wVersion,LPWSPDATA   lpWSPData,LPWSAPROTOCOL_INFOW lpProtocolInfo,WSPUPCALLTABLE  UpCallTable,LPWSPPROC_TABLE lpProcTable);
	INT					DeInitialize(  LPINT lpErrno  );
	PROVIDER*           GetProviderByIndex(int i);
	int					GetProviderCount();
	void				FreeAllInformation(BOOL bProcessDetached,LPINT lpErrno);
private:

	INT					m_iLayerCount;           // Number of layered protocol entries for LSP
	PROVIDER			*m_BaseInfo;   			   // Provider structures for each layered protocol


	BOOL				FindLspEntries(PROVIDER  **lspProviders,int  *lspProviderCount,int *lpErrno);
	PROVIDER *			FindMatchingLspEntryForProtocolInfo(WSAPROTOCOL_INFOW *inInfo,PROVIDER *lspProviders,int lspCount,BOOL fromStartup);
	BOOL				LoadProviderPath(PROVIDER    *loadProvider,int *lpErrno);
	int					InitializeProvider(PROVIDER *provider,WORD wVersion,WSAPROTOCOL_INFOW *lpProtocolInfo,WSPUPCALLTABLE UpCallTable,int *Error);
	int					EnumerateProvidersExisting(WINSOCK_CATALOG     Catalog,WSAPROTOCOL_INFOW  *ProtocolInfo,LPDWORD  ProtocolInfoSize);

	void				FreeProviders(LPWSAPROTOCOL_INFOW ProtocolInfo);
	int					VerifyProcTable(LPWSPPROC_TABLE lpProcTable);
	LPWSAPROTOCOL_INFOW EnumerateProviders(WINSOCK_CATALOG Catalog, LPINT TotalProtocols);

	void				CloseAndFreeSocketInfo(PROVIDER *provider,BOOL      processDetach );
	void				FreeSocketsAndMemory(BOOL processDetach,int *lpErrno);


};
