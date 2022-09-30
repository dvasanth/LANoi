#include "StdAfx.h"
#include "SSPISchannel.h"
#include <strsafe.h>
CSSPISchannel::CSSPISchannel(void)
{
	m_hSecurity=NULL;
	m_pSSPI=NULL;
	ZeroMemory(&m_hSecContext,sizeof(m_hSecContext));
	ZeroMemory(&m_hSecCredHandle,sizeof(m_hSecCredHandle));
	m_pRemoteCertContext=NULL;
	m_pSendStreamBuffer=NULL;
	ZeroMemory(&m_StreamSizes,sizeof(m_StreamSizes));
	m_bInitContext=TRUE;
}

CSSPISchannel::~CSSPISchannel(void)
{
	CtxtHandle		hZeroSecContext={0};
	CredHandle		hZeroSecCredHandle={0};

	//free resources
	if(memcmp(&m_hSecCredHandle,&hZeroSecCredHandle,sizeof(hZeroSecCredHandle))==0)
	{
		m_pSSPI->FreeCredentialsHandle(&m_hSecCredHandle);
		ZeroMemory(&hZeroSecCredHandle,sizeof(hZeroSecCredHandle));
	}
	if(memcmp(&m_hSecContext,&hZeroSecContext,sizeof(hZeroSecContext))==0)
	{
		m_pSSPI->DeleteSecurityContext(&m_hSecContext);
		ZeroMemory(&m_hSecContext,sizeof(m_hSecContext));
	}

	FreeRemoteCert();

	if(m_hSecurity)
	{
		FreeLibrary(m_hSecurity);
		m_hSecurity = NULL;
	}
	if(m_pSendStreamBuffer)
	{
		delete m_pSendStreamBuffer;
		m_pSendStreamBuffer=NULL;
	}
}

BOOL CSSPISchannel::Initialise()
{
	INIT_SECURITY_INTERFACE_A				pInitSecurityInterface;

	m_hSecurity=LoadLibrary(TEXT("Secur32.dll"));
	if(m_hSecurity == NULL)
	{
		//log error
		DEBUGLOG(LOI_ERROR,_T("CSSPISchannel::Initialise 0x%x loading "), GetLastError());
		return FALSE;
	}
	pInitSecurityInterface = (INIT_SECURITY_INTERFACE_A)GetProcAddress(m_hSecurity,"InitSecurityInterfaceA");

	if(pInitSecurityInterface == NULL)
	{
		//log error
		DEBUGLOG(LOI_ERROR,_T("CSSPISchannel::Initialise 0x%x reading InitSecurityInterface entry point"), GetLastError());
		return FALSE;
	}

	m_pSSPI = pInitSecurityInterface();

	if(m_pSSPI == NULL)
    {
		//logerror
		DEBUGLOG(LOI_ERROR,_T("CSSPISchannel::Initialise 0x%x reading security interface.\n"), GetLastError());
		return FALSE;
	}
	return TRUE;
}
/****************************************************
initialises the server auth certificates & parameters
****************************************************/
BOOL CSSPISchannel::SetInboundAuthCertificate( PCCERT_CONTEXT  pCertContext)
{
	BOOL				bRet=FALSE;
    TimeStamp			tsExpiry;
    SECURITY_STATUS		Status;
	SCHANNEL_CRED		SchannelCred;

    //
    // Build Schannel credential structure. Currently, this sample only
    // specifies the protocol to be used (and optionally the certificate, 
    // of course). Real applications may wish to specify other parameters 
    // as well.
    //

    ZeroMemory(&SchannelCred, sizeof(SchannelCred));

    SchannelCred.dwVersion = SCHANNEL_CRED_VERSION;

    SchannelCred.cCreds = 1;
    SchannelCred.paCred = &pCertContext;

    SchannelCred.grbitEnabledProtocols = SP_PROT_SSL3;


    //
    // Create an SSPI credential.
    //

    Status = m_pSSPI->AcquireCredentialsHandleA(
                        NULL,                   // Name of principal
                        UNISP_NAME_A,           // Name of package
                        SECPKG_CRED_INBOUND,    // Flags indicating use
                        NULL,                   // Pointer to logon ID
                        &SchannelCred,          // Package specific data
                        NULL,                   // Pointer to GetKey() func
                        NULL,                   // Value to pass to GetKey()
                        &m_hSecCredHandle,                // (out) Cred Handle
                        &tsExpiry);             // (out) Lifetime (optional)
    if(Status != SEC_E_OK)
    {
		DEBUGLOG(LOI_ERROR,_T("AcquireCredentialsHandleA 0x%x returned by AcquireCredentialsHandle\n"), Status);
        return FALSE;
    }

	return TRUE;
}

//////////////////\
////Initialise the SSL client parameters
////////////////////////
BOOL CSSPISchannel::SetOutboundAuthCertificate( PCCERT_CONTEXT  pCertContext)
{
	BOOL				bRet=FALSE;
    TimeStamp			tsExpiry;
    SECURITY_STATUS		Status;
	SCHANNEL_CRED		SchannelCred;
 
    //
    // Build Schannel credential structure. Currently, this sample only
    // specifies the protocol to be used (and optionally the certificate, 
    // of course). Real applications may wish to specify other parameters 
    // as well.
    //

    ZeroMemory(&SchannelCred, sizeof(SchannelCred));

    SchannelCred.dwVersion  = SCHANNEL_CRED_VERSION;
    if(pCertContext)
    {
        SchannelCred.cCreds     = 1;
        SchannelCred.paCred     = &pCertContext;
    }

    SchannelCred.grbitEnabledProtocols = SP_PROT_SSL3;
    SchannelCred.dwFlags |= SCH_CRED_NO_DEFAULT_CREDS;

    // The SCH_CRED_MANUAL_CRED_VALIDATION flag is specified because
    // this sample verifies the server certificate manually. 
    // Applications that expect to run on WinNT, Win9x, or WinME 
    // should specify this flag and also manually verify the server
    // certificate. Applications running on newer versions of Windows can
    // leave off this flag, in which case the InitializeSecurityContext
    // function will validate the server certificate automatically.
	SchannelCred.dwFlags |= SCH_CRED_MANUAL_CRED_VALIDATION;
	//Prevent Schannel from comparing the supplied target name
	//with the subject names in server certificates.
	SchannelCred.dwFlags |= SCH_CRED_NO_SERVERNAME_CHECK;

    //
    // Create an SSPI credential.
    //

    Status = m_pSSPI->AcquireCredentialsHandleA(
                        NULL,                   // Name of principal    
                        UNISP_NAME_A,           // Name of package
						SECPKG_CRED_OUTBOUND,   // Flags indicating use
                        NULL,                   // Pointer to logon ID
                        &SchannelCred,          // Package specific data
                        NULL,                   // Pointer to GetKey() func
                        NULL,                   // Value to pass to GetKey()
                        &m_hSecCredHandle,                // (out) Cred Handle
                        &tsExpiry);             // (out) Lifetime (optional)
    if(Status != SEC_E_OK)
    {
		//log error
		DEBUGLOG(LOI_ERROR,_T("AcquireCredentialsHandleA 0x%x returned by AcquireCredentialsHandle\n"), Status);
        goto cleanup;
    }
	bRet=TRUE;
cleanup:
	return bRet;
}

SSPI_STATE CSSPISchannel::PerformInboundHandshake(BYTE *pRecvEncBuffer,DWORD dwEncLen,DWORD &dwExtraBytesCopied,PBYTE &pOutHandShakeBuf,DWORD &dwOutHandShakeBufLen)
{
    TimeStamp            tsExpiry;
    SECURITY_STATUS      scRet;
    SecBufferDesc        InBuffer;
    SecBufferDesc        OutBuffer;
    SecBuffer            InBuffers[2];
	DWORD				 dwSSPIFlags,dwSSPIOutFlags;
    SecBuffer            OutBuffers[1];
    DWORD                err = 0;
	SSPI_STATE			 RetState=INBOUND_HANDSHAKE_PENDING;

	dwExtraBytesCopied=0;
	dwOutHandShakeBufLen=0;
	dwSSPIFlags =   ASC_REQ_SEQUENCE_DETECT        |
                    ASC_REQ_REPLAY_DETECT      |
                    ASC_REQ_CONFIDENTIALITY  |
                    ASC_REQ_EXTENDED_ERROR    |
                    ASC_REQ_ALLOCATE_MEMORY  |
                    ASC_REQ_STREAM;

	//client auth enabled
    dwSSPIFlags |= ASC_REQ_MUTUAL_AUTH;


	//
    //  set OutBuffer for InitializeSecurityContext call
    //

    OutBuffer.cBuffers = 1;
    OutBuffer.pBuffers = OutBuffers;
    OutBuffer.ulVersion = SECBUFFER_VERSION;

	scRet = SEC_I_CONTINUE_NEEDED;
	//
    // InBuffers[1] is for getting extra data that
    //  SSPI/SCHANNEL doesn't proccess on this
    //  run around the loop.
    //

    InBuffers[0].pvBuffer = pRecvEncBuffer;
    InBuffers[0].cbBuffer = dwEncLen;
    InBuffers[0].BufferType = SECBUFFER_TOKEN;

    InBuffers[1].pvBuffer   = NULL;
    InBuffers[1].cbBuffer   = 0;
    InBuffers[1].BufferType = SECBUFFER_EMPTY;

    InBuffer.cBuffers        = 2;
    InBuffer.pBuffers        = InBuffers;
    InBuffer.ulVersion       = SECBUFFER_VERSION;


    //
    // Initialize these so if we fail, pvBuffer contains NULL,
    // so we don't try to free random garbage at the quit
    //

    OutBuffers[0].pvBuffer   = NULL;
    OutBuffers[0].BufferType = SECBUFFER_TOKEN;
    OutBuffers[0].cbBuffer   = 0;
	dwOutHandShakeBufLen=0;
	dwExtraBytesCopied=0;


    scRet = m_pSSPI->AcceptSecurityContext(
                    &m_hSecCredHandle,
					m_bInitContext?NULL:&m_hSecContext,
                    &InBuffer,
                    dwSSPIFlags,
                    SECURITY_NATIVE_DREP,
					m_bInitContext?&m_hSecContext:NULL,
                    &OutBuffer,
                    &dwSSPIOutFlags,
                    &tsExpiry);

	m_bInitContext=FALSE;
    if (  scRet == SEC_I_CONTINUE_NEEDED)
    {
		 if(OutBuffers[0].cbBuffer != 0 && OutBuffers[0].pvBuffer != NULL )
         {
			pOutHandShakeBuf=(PBYTE)OutBuffers[0].pvBuffer;
			dwOutHandShakeBufLen=	OutBuffers[0].cbBuffer ; 
			
		 }
         if ( InBuffers[1].BufferType == SECBUFFER_EXTRA )
         {
            memcpy(pRecvEncBuffer,(LPBYTE) (pRecvEncBuffer + (dwEncLen - InBuffers[1].cbBuffer)),InBuffers[1].cbBuffer);
            dwExtraBytesCopied = InBuffers[1].cbBuffer;
         }
		 RetState=INBOUND_HANDSHAKE_PENDING;
		 goto completed;
	}

	if(scRet==SEC_E_INCOMPLETE_MESSAGE)
	{
		RetState=INBOUND_HANDSHAKE_PENDING;
		dwExtraBytesCopied=dwEncLen;//we need more data
		goto completed;
	}	

    if ( scRet == SEC_E_OK )
    {
		IntialiseStreamBuffers();

        if ( InBuffers[1].BufferType == SECBUFFER_EXTRA )
        {
            memcpy(pRecvEncBuffer,(LPBYTE) (pRecvEncBuffer + (dwEncLen - InBuffers[1].cbBuffer)),InBuffers[1].cbBuffer);
            dwExtraBytesCopied = InBuffers[1].cbBuffer;
        }
		if(OutBuffers[0].cbBuffer != 0 && OutBuffers[0].pvBuffer != NULL )
        {
			pOutHandShakeBuf=(PBYTE)OutBuffers[0].pvBuffer;
			dwOutHandShakeBufLen=	OutBuffers[0].cbBuffer ; 
		}
		RetState=ENCRYPT_DECRYPT;

		
		goto completed;
    }
  

    if (FAILED(scRet))
    {
		//logerror
		DEBUGLOG(LOI_ERROR,_T("AcceptSecurityContext Failed with error code %lx\n"), scRet);
		RetState=SSL_CONNECTION_CLOSE;
    }

completed:
	return RetState;

}
SSPI_STATE CSSPISchannel::InitiateOutboundHandshake(LPCTSTR pszServerName,PBYTE &pOutHandShakeBuf,DWORD &dwOutHandShakeBufLen)
{
	SecBufferDesc			OutBuffer;
    SecBuffer				OutBuffers[1];
    DWORD					dwSSPIFlags;
    DWORD					dwSSPIOutFlags;
    TimeStamp				tsExpiry;
    SECURITY_STATUS			scRet;
	CHAR					szServerNameA[MAX_PATH]={0};

	dwOutHandShakeBufLen=0;
    dwSSPIFlags = ISC_REQ_SEQUENCE_DETECT   |
                  ISC_REQ_REPLAY_DETECT     |
                  ISC_REQ_CONFIDENTIALITY   |
                  ISC_RET_EXTENDED_ERROR    |
                  ISC_REQ_ALLOCATE_MEMORY   |
                  ISC_REQ_STREAM;

    //
    //  Initiate a ClientHello message and generate a token.
    //

    OutBuffers[0].pvBuffer   = NULL;
    OutBuffers[0].BufferType = SECBUFFER_TOKEN;
    OutBuffers[0].cbBuffer   = 0;

    OutBuffer.cBuffers = 1;
    OutBuffer.pBuffers = OutBuffers;
    OutBuffer.ulVersion = SECBUFFER_VERSION;

	StringCbPrintfA( szServerNameA,sizeof(szServerNameA),"%S",pszServerName);
    scRet = m_pSSPI->InitializeSecurityContextA(
                    &m_hSecCredHandle,
                    NULL,
                    NULL,
                    dwSSPIFlags,
                    0,
                    SECURITY_NATIVE_DREP,
                    NULL,
                    0,
                    &m_hSecContext,
                    &OutBuffer,
                    &dwSSPIOutFlags,
                    &tsExpiry);

  	if(scRet == SEC_I_CONTINUE_NEEDED)
	{
		if(OutBuffers[0].cbBuffer != 0 && OutBuffers[0].pvBuffer != NULL )
         {
			pOutHandShakeBuf=(PBYTE)OutBuffers[0].pvBuffer;
			dwOutHandShakeBufLen=	OutBuffers[0].cbBuffer ; 
		 }
		 return OUTBOUND_HANDSHAKE_PENDING;
	}

	if(scRet != SEC_I_CONTINUE_NEEDED)
    {
		//log error
		DEBUGLOG(LOI_ERROR,_T("InitializeSecurityContextA %d returned by InitializeSecurityContext (1)"), scRet);
    
    }
	return SSL_CONNECTION_CLOSE;
}
SSPI_STATE CSSPISchannel::PerformOutboundHandshake(BYTE *pRecvEncBuffer,DWORD dwEncLen,DWORD &dwExtraBytesCopied,PBYTE &pOutHandShakeBuf,DWORD &dwOutHandShakeBufLen)
{
	SecBufferDesc			OutBuffer;
    SecBuffer				OutBuffers[1];
    DWORD					dwSSPIFlags;
    DWORD					dwSSPIOutFlags=0;
    TimeStamp				tsExpiry;
    SECURITY_STATUS			scRet;
	SecBufferDesc			InBuffer;
    SecBuffer				InBuffers[2];

	dwOutHandShakeBufLen=0;
	dwExtraBytesCopied=0;
    dwSSPIFlags = ISC_REQ_SEQUENCE_DETECT   |
                  ISC_REQ_REPLAY_DETECT     |
                  ISC_REQ_CONFIDENTIALITY   |
                  ISC_RET_EXTENDED_ERROR    |
                  ISC_REQ_ALLOCATE_MEMORY   |
                  ISC_REQ_STREAM			|
				  ISC_REQ_USE_SUPPLIED_CREDS;
	//
    // Set up the input buffers. Buffer 0 is used to pass in data
    // received from the server. Schannel will consume some or all
    // of this. Leftover data (if any) will be placed in buffer 1 and
    // given a buffer type of SECBUFFER_EXTRA.
    //

    InBuffers[0].pvBuffer   = pRecvEncBuffer;
    InBuffers[0].cbBuffer   = dwEncLen;
    InBuffers[0].BufferType = SECBUFFER_TOKEN;

    InBuffers[1].pvBuffer   = NULL;
    InBuffers[1].cbBuffer   = 0;
    InBuffers[1].BufferType = SECBUFFER_EMPTY;

    InBuffer.cBuffers       = 2;
    InBuffer.pBuffers       = InBuffers;
    InBuffer.ulVersion      = SECBUFFER_VERSION;

    //
    // Set up the output buffers. These are initialized to NULL
    // so as to make it less likely we'll attempt to free random
    // garbage later.
    //

    OutBuffers[0].pvBuffer  = NULL;
    OutBuffers[0].BufferType= SECBUFFER_TOKEN;
    OutBuffers[0].cbBuffer  = 0;

    OutBuffer.cBuffers      = 1;
    OutBuffer.pBuffers      = OutBuffers;
    OutBuffer.ulVersion     = SECBUFFER_VERSION;

    //
    // Call InitializeSecurityContext.
    //

    scRet = m_pSSPI->InitializeSecurityContextA(
										&m_hSecCredHandle,
                                      &m_hSecContext,
                                      NULL,
                                      dwSSPIFlags,
                                      0,
                                      SECURITY_NATIVE_DREP,
                                      &InBuffer,
                                      0,
                                      NULL,
                                      &OutBuffer,
                                      &dwSSPIOutFlags,
                                      &tsExpiry);

    //
    // If InitializeSecurityContext was successful (or if the error was 
    // one of the special extended ones), send the contends of the output
    // buffer to the server.
    //
	if(scRet == SEC_I_CONTINUE_NEEDED )
	{
		if(OutBuffers[0].cbBuffer != 0 && OutBuffers[0].pvBuffer != NULL )
		{
			pOutHandShakeBuf=(PBYTE)OutBuffers[0].pvBuffer;
			dwOutHandShakeBufLen=	OutBuffers[0].cbBuffer ; 
		}
		if ( InBuffers[1].BufferType == SECBUFFER_EXTRA )
        {
            memcpy(pRecvEncBuffer,(LPBYTE) (pRecvEncBuffer + (dwEncLen - InBuffers[1].cbBuffer)),InBuffers[1].cbBuffer);
            dwExtraBytesCopied = InBuffers[1].cbBuffer;
        }
		DEBUGLOG(LOI_INFO,_T("PerformOutboundHandshake SEC_I_CONTINUE_NEEDED"));
		return OUTBOUND_HANDSHAKE_PENDING;
	}
	if(scRet==SEC_E_INCOMPLETE_MESSAGE)
	{
		 DEBUGLOG(LOI_INFO,_T("PerformOutboundHandshake SEC_E_INCOMPLETE_MESSAGE"));
		dwExtraBytesCopied=dwEncLen;
		return OUTBOUND_HANDSHAKE_PENDING;
	}	

	if(scRet == SEC_E_OK)
    {
		IntialiseStreamBuffers();

        if ( InBuffers[1].BufferType == SECBUFFER_EXTRA )
        {
            memcpy(pRecvEncBuffer,(LPBYTE) (pRecvEncBuffer + (dwEncLen - InBuffers[1].cbBuffer)),InBuffers[1].cbBuffer);
            dwExtraBytesCopied = InBuffers[1].cbBuffer;
        }
		if(OutBuffers[0].cbBuffer != 0 && OutBuffers[0].pvBuffer != NULL )
        {
			pOutHandShakeBuf=(PBYTE)OutBuffers[0].pvBuffer;
			dwOutHandShakeBufLen=	OutBuffers[0].cbBuffer ; 
		}
		
		return ENCRYPT_DECRYPT;
    }

	if (FAILED(scRet))
    {
		//logerror
		DEBUGLOG(LOI_ERROR,_T("InitializeSecurityContextA Failed with error code %lx"), scRet);
			
		
    }
	return SSL_CONNECTION_CLOSE;
}

///
///copies the decrypted buffer in pDecryptedBuffer & len in dwDecryptedLen
///sometime decrypted buffer len may be of zero bytes
//
SSPI_STATE CSSPISchannel::DecryptMessage(BYTE *pRecvEncBuffer,DWORD dwEncLen,PBYTE &pDecryptedBuffer,DWORD &dwDecryptedLen,DWORD &dwExtraBytesCopied)
{
    SecBufferDesc			Message;
    SecBuffer				Buffers[4];
    SECURITY_STATUS			scRet;
	SSPI_STATE				RetState=ENCRYPT_DECRYPT;
    SecBuffer *				pDataBuffer;
    SecBuffer *				pExtraBuffer;


	dwExtraBytesCopied=0;
	dwDecryptedLen=0;
	pDecryptedBuffer=NULL;
    // 
    // Attempt to decrypt the received data.
    //
    Buffers[0].pvBuffer = pRecvEncBuffer;
	Buffers[0].cbBuffer = dwEncLen;
	Buffers[0].BufferType = SECBUFFER_DATA;

	Buffers[1].BufferType = SECBUFFER_EMPTY;
	Buffers[2].BufferType = SECBUFFER_EMPTY;
	Buffers[3].BufferType = SECBUFFER_EMPTY;

    Message.ulVersion       = SECBUFFER_VERSION;
    Message.cBuffers        = 4;
    Message.pBuffers        = Buffers;

	scRet = m_pSSPI->DecryptMessage(&m_hSecContext, &Message, 0, NULL);

	if(scRet == SEC_E_OK )
	{	
		RetState=ENCRYPT_DECRYPT;
		 // Locate data and (optional) extra buffers.
        pDataBuffer  = NULL;
        pExtraBuffer = NULL;
        for(int i = 1; i < 4; i++)
        {

            if(pDataBuffer == NULL && Buffers[i].BufferType == SECBUFFER_DATA)
            {
                pDataBuffer = &Buffers[i];
            }
            if(pExtraBuffer == NULL && Buffers[i].BufferType == SECBUFFER_EXTRA)
            {
                pExtraBuffer = &Buffers[i];
            }
        }

        if(pDataBuffer == NULL)
        {
            goto completed;
        }
		
		if(pDataBuffer)
		{
			pDecryptedBuffer=new BYTE[pDataBuffer->cbBuffer];
			memcpy(pDecryptedBuffer,pDataBuffer->pvBuffer,pDataBuffer->cbBuffer);
			dwDecryptedLen=pDataBuffer->cbBuffer;
		}

		if(pExtraBuffer)
		{
			MoveMemory(pRecvEncBuffer, pExtraBuffer->pvBuffer, pExtraBuffer->cbBuffer);
            dwExtraBytesCopied = pExtraBuffer->cbBuffer;
		}
		RetState=ENCRYPT_DECRYPT;
		goto completed;
	}
	if(scRet == SEC_E_INCOMPLETE_MESSAGE)
	{
		RetState=ENCRYPT_DECRYPT;
		dwExtraBytesCopied=dwEncLen;
		goto completed;
	}

    // Server signalled end of session
	//or any ssl state error
	RetState=SSL_CONNECTION_CLOSE;

	DEBUGLOG(LOI_ERROR,_T("DecryptMessage Failed with error code %lx"), scRet);
completed:
	return RetState;
}

BOOL CSSPISchannel::IntialiseStreamBuffers()
{
	SECURITY_STATUS				scRet;

	scRet = m_pSSPI->QueryContextAttributesA(&m_hSecContext,SECPKG_ATTR_STREAM_SIZES,&m_StreamSizes);
    if(scRet != SEC_E_OK)
    {
		//log error
        //printf("**** Error 0x%x reading SECPKG_ATTR_STREAM_SIZES\n", scRet);
        return FALSE;
    }

	//alloc max buffer that can be send in one shot
	m_pSendStreamBuffer=new BYTE[m_StreamSizes.cbHeader + 
								 m_StreamSizes.cbMaximumMessage +
								 m_StreamSizes.cbTrailer];
	

	return TRUE;
}
DWORD CSSPISchannel::GetMaxStreamSize()
{
	return m_StreamSizes.cbMaximumMessage;
}
//
//Encrypted buffer is stored in pEncryptedBuffer with len in dwEncryptBuffLen
//total bytes process in this call is updated in dwBytesProcessed
//caller needs to invoke multiple times if needed

SSPI_STATE CSSPISchannel::EncryptMessage(BYTE *pSendBuffer,DWORD dwEncLen,PBYTE &pEncryptedBuffer,DWORD &dwEncryptBuffLen,DWORD &dwBytesProcessed)
{
	DWORD				dwEncryptChunkLen;
    SecBufferDesc		Message;
    SecBuffer			Buffers[4];
    SECURITY_STATUS		scRet;

	dwEncryptChunkLen=dwEncLen<m_StreamSizes.cbMaximumMessage?dwEncLen:m_StreamSizes.cbMaximumMessage;

	pEncryptedBuffer=new BYTE[dwEncryptChunkLen+m_StreamSizes.cbHeader+m_StreamSizes.cbTrailer];

	memcpy(pEncryptedBuffer+ m_StreamSizes.cbHeader,pSendBuffer,dwEncryptChunkLen);

    //
    // Encrypt the HTTP request.
    //

    Buffers[0].pvBuffer     = pEncryptedBuffer;
    Buffers[0].cbBuffer     = m_StreamSizes.cbHeader;
    Buffers[0].BufferType   = SECBUFFER_STREAM_HEADER;

    Buffers[1].pvBuffer     = pEncryptedBuffer+ m_StreamSizes.cbHeader;
    Buffers[1].cbBuffer     = dwEncryptChunkLen;
    Buffers[1].BufferType   = SECBUFFER_DATA;

    Buffers[2].pvBuffer     =  pEncryptedBuffer+ m_StreamSizes.cbHeader + dwEncryptChunkLen;
    Buffers[2].cbBuffer     = m_StreamSizes.cbTrailer;
    Buffers[2].BufferType   = SECBUFFER_STREAM_TRAILER;

    Buffers[3].BufferType   = SECBUFFER_EMPTY;

    Message.ulVersion       = SECBUFFER_VERSION;
    Message.cBuffers        = 4;
    Message.pBuffers        = Buffers;

	scRet = m_pSSPI->EncryptMessage(&m_hSecContext, 0, &Message, 0);

    if(FAILED(scRet))
    {
		//log error
		DEBUGLOG(LOI_ERROR,_T("EncryptMessage Error 0x%x returned by EncryptMessage\n"), scRet);
		FreeStreamBuffer(pEncryptedBuffer);
        return SSL_CONNECTION_CLOSE;
    }	
	dwBytesProcessed=dwEncryptChunkLen;
	dwEncryptBuffLen=Buffers[0].cbBuffer + Buffers[1].cbBuffer + Buffers[2].cbBuffer;
	return ENCRYPT_DECRYPT;
}

PCCERT_CONTEXT CSSPISchannel::GetRemotePartyCert()
{
	SECURITY_STATUS		scRet;

	scRet = m_pSSPI->QueryContextAttributesA(&m_hSecContext,
                                            SECPKG_ATTR_REMOTE_CERT_CONTEXT,
                                            (PVOID)&m_pRemoteCertContext);
    if(scRet != SEC_E_OK)
    {
		//log error
		DEBUGLOG(LOI_ERROR,_T("QueryContextAttributesA 0x%lx querying client certificate\n"), scRet);
		return NULL;
    }
    
	return m_pRemoteCertContext;
}

void CSSPISchannel::FreeRemoteCert()
{
	if(m_pRemoteCertContext)
	{
		// Free the server certificate context.
		CertFreeCertificateContext(m_pRemoteCertContext);
		m_pRemoteCertContext = NULL;
	}
}

void CSSPISchannel::FreeContextBuffer(PVOID pBuffer)
{
	if(pBuffer)
	{
		m_pSSPI->FreeContextBuffer( pBuffer );
	}
}

void CSSPISchannel::FreeStreamBuffer(PVOID pBuffer)
{
	if(pBuffer)
	{
		delete pBuffer;
	}

}