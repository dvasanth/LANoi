#include "StdAfx.h"
#include "ISSLSocket.h"

/****************************************************
const definition
****************************************************/
const DWORD		ISSLSocket::STREAM_CONST_LEN=16*1024;
///invoked for accepted socket 
ISSLSocket::ISSLSocket(LPTSTR lpszServername,PCCERT_CONTEXT  pCertContext,SOCKET socket,ISSLSocketNotify *pSSLNotify):ISocket(socket,pSSLNotify)
{
	//initiate ssl  initialisation
	ConstructInit(lpszServername,pCertContext,pSSLNotify);
	if(m_SSPI.SetInboundAuthCertificate (pCertContext))//set ssl server cert
	{
		//change the state as SSL server cert initialisation is done
		m_SSLState =INBOUND_HANDSHAKE_PENDING;
		return;
	}

}
//invoked before socket creation
ISSLSocket::ISSLSocket(LPTSTR lpszServername,PCCERT_CONTEXT  pCertContext,ISSLSocketNotify *pSSLNotify):ISocket(pSSLNotify)  
{
	ConstructInit(lpszServername,pCertContext,pSSLNotify);
}

void ISSLSocket::ConstructInit(LPTSTR lpszServername,PCCERT_CONTEXT  pCertContext,ISSLSocketNotify *pSSLNotify)
{
	//initiate ssl  initialisation
	m_sServerName= lpszServername;
	m_pCertContext= pCertContext;
	m_SSPI.Initialise();  
	m_SSLState=SSL_CONNECTION_CLOSE;
	ZeroMemory(&m_DecryptedBuffer,sizeof(m_DecryptedBuffer));
	ZeroMemory(&m_EncryptedBuffer,sizeof(m_EncryptedBuffer));
	m_pSSLNotify=pSSLNotify;
}
ISSLSocket::~ISSLSocket(void)
{
	//free all stream buffer
	FreeStreamData();
}

/***********************************************************
FD_ACCEPT & FD_CONNECT is used to identify the socket is inbound or outbound
**********************************************************/
void ISSLSocket::OnSockEvent(LONG lNetworkEvents)
{

	
	if( lNetworkEvents&FD_CONNECT)
	{
		BYTE				*pSendPendingBuff=NULL;
		DWORD				dwSendLen=0;

		m_SSLState=OUTBOUND_HANDSHAKE_PENDING;
		
		if(m_dwErrcode[FD_CONNECT_BIT])//connection related error
		{
			//SetLastErrorMessage(
			m_pSockNotify->OnConnectComplete(this,m_dwErrcode[FD_CONNECT_BIT] );
			ShutDown();
			return;
		}
		if(!m_SSPI.SetOutboundAuthCertificate(m_pCertContext))
		{
			//set the last error message
			ShutDown();
			return;
		}
		m_SSLState=m_SSPI.InitiateOutboundHandshake((LPCTSTR)m_sServerName,pSendPendingBuff,dwSendLen);
		if(m_SSLState==SSL_CONNECTION_CLOSE)
		{
			ShutDown();
			return;
		}
		if(pSendPendingBuff)
		{
			AllocateAndSend(pSendPendingBuff,dwSendLen);
			m_SSPI.FreeContextBuffer( pSendPendingBuff); 
		}
	}

	//default execution
	ISocket::OnSockEvent( lNetworkEvents);
}

/*************************************************************************
Recv is notified to the caller only after handshake is complete.
Encryption record can be decrypted only after receiveing the full record
*************************************************************************/
INT ISSLSocket::Recv(BYTE *pByBuffer,DWORD dwReqRecvData,	DWORD	&dwBytesReceived,DWORD &dwLastError)
{	
	PBYTE				pSendPendingBuff=NULL;
	DWORD				dwSendLen=0;
	DWORD				dwRetValue=0;
	PBYTE			    pEncStream=NULL;
	DWORD				dwEncStreamSize;
	INT					iRetErr=NO_ERROR;
	DWORD				dwEncryptedBytesReceived=0;

	dwBytesReceived=0;
	//check the stack for requested recv size 
	if( GetDecryptStreamQueueDataSize() && //if only some data available
		GetDecryptStreamQueueDataSize()>= dwReqRecvData) //and if it is greater then the requested bytes
	{
		goto completed;
	}

	//get encrypt stream buffer
	pEncStream=GetEncryptStream(dwEncStreamSize); 

	//recv as much as possible & in encrypted stream
	iRetErr=ISocket ::Recv(pEncStream,dwEncStreamSize,dwEncryptedBytesReceived,dwLastError);
	if(iRetErr==SOCKET_ERROR && dwLastError!=WSAEWOULDBLOCK)
	{
		//error connection closed most probably
		ShutDown();
		return iRetErr;
	}
	DWORD dwBytesTobeProcessed=m_EncryptedBuffer.dwByteOffset + dwEncryptedBytesReceived;
	if(dwBytesTobeProcessed == 0)
	{
		goto completed;
	}
	switch(m_SSLState)
	{
		case INBOUND_HANDSHAKE_PENDING:
			{
				m_SSLState=m_SSPI.PerformInboundHandshake(
														m_EncryptedBuffer.buf ,//currently received + previously received
														dwBytesTobeProcessed,//total bytes in the stream
														m_EncryptedBuffer.dwByteOffset,//remaining encrypted bytes 
														pSendPendingBuff,//bytes needs to be sent to the client
														dwSendLen);//send len
				//send any handshake data if pending
				if(pSendPendingBuff)
				{
					AllocateAndSend( pSendPendingBuff, dwSendLen);
					m_SSPI.FreeContextBuffer(pSendPendingBuff);  
				}
				if(m_SSLState==ENCRYPT_DECRYPT)
				{
					//raise handshake complete
					m_pSSLNotify->OnInboundHandshakeComplete( this,m_SSPI.GetRemotePartyCert());
					dwEncryptedBytesReceived=0;// received bytes is processed
				}

			}
			break;
		case OUTBOUND_HANDSHAKE_PENDING:
			{
				m_SSLState=m_SSPI.PerformOutboundHandshake(
															m_EncryptedBuffer.buf,
															dwBytesTobeProcessed,
															m_EncryptedBuffer.dwByteOffset,
															pSendPendingBuff,
															dwSendLen
															);
				//send any handshake data if pending
				if(pSendPendingBuff)
				{
					AllocateAndSend( pSendPendingBuff, dwSendLen);
					m_SSPI.FreeContextBuffer(pSendPendingBuff);  
				}
				if(m_SSLState==ENCRYPT_DECRYPT)
				{
					//raise handshake complete
					m_pSSLNotify->OnOutboundHandshakeComplete(this );   
					dwEncryptedBytesReceived=0;// received bytes is processed
				}
			}
			break;
		

	}//switch

	
	dwBytesTobeProcessed=m_EncryptedBuffer.dwByteOffset + dwEncryptedBytesReceived;
	//start decrypt if hanshake is complete
	if(m_SSLState== ENCRYPT_DECRYPT &&
	  dwBytesTobeProcessed//only if we have some bytes to process
	   )
	{
		PBYTE  pbyDecryptedBuf;
		DWORD  dwDecryptLen;
		do{
			pbyDecryptedBuf=NULL;
			dwDecryptLen=0;

			m_SSLState= m_SSPI.DecryptMessage(
												m_EncryptedBuffer.buf,
												dwBytesTobeProcessed,
												pbyDecryptedBuf,
												dwDecryptLen,
												m_EncryptedBuffer.dwByteOffset
												);
			dwBytesTobeProcessed=m_EncryptedBuffer.dwByteOffset;
			if(m_SSLState==ENCRYPT_DECRYPT)
			{
				if(dwDecryptLen)
				{
					QueueDecryptStream(pbyDecryptedBuf,dwDecryptLen);
					m_SSPI.FreeStreamBuffer(pbyDecryptedBuf); 
				}
			}
		}while(m_SSLState ==ENCRYPT_DECRYPT &&//decryption is success & no connection close requested
			   dwBytesTobeProcessed &&     // tell there is some bytes to be processed
			   dwDecryptLen                //current bytes processed is decrypted successfully
				);

	}



completed: 

	if(GetDecryptStreamQueueDataSize() ) //only if data available
	{
		//copy data of size whichever is minimum
		DWORD dwDataTobeCopied=GetDecryptStreamQueueDataSize()<dwReqRecvData?GetDecryptStreamQueueDataSize():dwReqRecvData;

		//data available just copy the data
		if(DequeueDecryptStream(pByBuffer,dwDataTobeCopied))
		{
			dwBytesReceived=dwDataTobeCopied;
			dwLastError=0;
		}
		//if still data in stream then raise the recv event to copy more data
		if(GetDecryptStreamQueueDataSize())
		{
			//raise the event so app requests more data to recv
			RaiseEvents(FD_READ); 
		}
	}
	//the other party requested for close
	if(m_SSLState==SSL_CONNECTION_CLOSE)
	{
		ShutDown();
	}

	return iRetErr;
}
/**************************************
helper function for sending
**************************************/
BOOL ISSLSocket::AllocateAndSend(BYTE *byEncBuffer,DWORD dwEncDataLength)
{
	PBYTE  pSockioSendStream;

	pSockioSendStream=GetSendBuffer( dwEncDataLength);
	memcpy(pSockioSendStream,byEncBuffer,dwEncDataLength);
	ISocket::SendData( pSockioSendStream, dwEncDataLength);
	return TRUE;
}
/**************************************************************
Send function which encrypts the buffer with SSL & send it with ISocket
***************************************************************/
void ISSLSocket::SendData(BYTE *bySendBuffer,DWORD dwDataLength)
{
	PBYTE			    pEncryptedStream=NULL;
	DWORD				dwEncStreamLen;
	DWORD				dwBytesEncrypted=0;
	SSPI_STATE			RetState=SSL_CONNECTION_CLOSE;
	//create the encrypted buffer & send

	do{
		DWORD  dwBytesProcessed=0;
		RetState=m_SSPI.EncryptMessage(&bySendBuffer[dwBytesEncrypted],
											dwDataLength-dwBytesEncrypted,
											pEncryptedStream,
											dwEncStreamLen,
											dwBytesProcessed); 
		if(RetState!=SSL_CONNECTION_CLOSE )
		{
			AllocateAndSend( pEncryptedStream, dwEncStreamLen);
			m_SSPI.FreeStreamBuffer(pEncryptedStream); 
			dwBytesEncrypted+=dwBytesProcessed;
		}
	}while(RetState!=SSL_CONNECTION_CLOSE && dwBytesEncrypted<dwDataLength);

	return ;
}

/**********************************************
Encrypted stream buffer where the stream is stored
**********************************************/
PBYTE ISSLSocket::GetEncryptStream(DWORD &dwEncryptSteamSize)
{
	//if the stream is filled then reallocate the stream
	if(m_EncryptedBuffer.dwTotalLen <= m_EncryptedBuffer.dwByteOffset)
	{
		BYTE		*pNewBuff;
		pNewBuff  = new BYTE[m_EncryptedBuffer.dwTotalLen+STREAM_CONST_LEN];
		if(m_DecryptedBuffer.dwByteOffset)
		{
			memcpy(pNewBuff,m_EncryptedBuffer.buf, m_EncryptedBuffer.dwByteOffset);
		}
		m_EncryptedBuffer.dwTotalLen+=STREAM_CONST_LEN;
		if(m_EncryptedBuffer.buf) delete m_EncryptedBuffer.buf;
		m_EncryptedBuffer.buf=pNewBuff;
	}
	dwEncryptSteamSize=m_EncryptedBuffer.dwTotalLen-m_EncryptedBuffer.dwByteOffset;
	return &m_EncryptedBuffer.buf[m_EncryptedBuffer.dwByteOffset];
}
/*************************************
return the size of the decryptstream
*************************************/
DWORD ISSLSocket::GetDecryptStreamQueueDataSize()
{
	return m_DecryptedBuffer.dwByteOffset;
}

/********************************
stores the decrypted buffer in a stream
********************************/
void ISSLSocket::QueueDecryptStream(PBYTE pDecryptedStream,DWORD dwLen)
{
	
	if((m_DecryptedBuffer.dwByteOffset+dwLen)>=m_DecryptedBuffer.dwTotalLen)
	{
		//if decrypt stream is not able to hold data reallocate the data
		BYTE		*pNewBuff;
		DWORD		 dwLenreq=m_DecryptedBuffer.dwTotalLen+dwLen+STREAM_CONST_LEN;
		pNewBuff  = new BYTE[dwLenreq];
		if(m_DecryptedBuffer.dwByteOffset)
		{
			memcpy(pNewBuff,m_DecryptedBuffer.buf, m_DecryptedBuffer.dwByteOffset);
		}
		
		ZeroMemory(pNewBuff,dwLenreq);
		if(m_DecryptedBuffer.buf) delete m_DecryptedBuffer.buf;
		m_DecryptedBuffer.buf=pNewBuff;
		m_DecryptedBuffer.dwTotalLen = dwLenreq;
	}
	memcpy(&m_DecryptedBuffer.buf[m_DecryptedBuffer.dwByteOffset],pDecryptedStream, dwLen);
	m_DecryptedBuffer.dwByteOffset+=dwLen;//new offset
}

/********************************
restores  the decrypted buffer of specified in the stream
********************************/
BOOL ISSLSocket::DequeueDecryptStream(PBYTE pDecryptBuffer,DWORD dwGetByteLen)
{
	if(m_DecryptedBuffer.dwByteOffset>=dwGetByteLen)
	{
		DWORD dwRemainingBytes;
		//copy the requested size
		memcpy(pDecryptBuffer,m_DecryptedBuffer.buf,dwGetByteLen);
		//new read offset
		dwRemainingBytes =(int)m_DecryptedBuffer.dwByteOffset-(int)dwGetByteLen;
		//shift the remanining bytes to the start position
		if(dwRemainingBytes)
		{
			MoveMemory( m_DecryptedBuffer.buf,&m_DecryptedBuffer.buf[dwGetByteLen],dwRemainingBytes);
			
		}
		m_DecryptedBuffer.dwByteOffset=dwRemainingBytes;
		return TRUE;
	}
	return FALSE;
}
/****************************
frees all stream data allocated for encrypt & decrypt
****************************/
void ISSLSocket::FreeStreamData()
{
	if(m_DecryptedBuffer.buf) 
	{
		delete m_DecryptedBuffer.buf;
		m_DecryptedBuffer.buf=0;
	}

	if(m_EncryptedBuffer.buf)
	{
		delete m_EncryptedBuffer.buf;
		m_EncryptedBuffer.buf=NULL;
	}
}