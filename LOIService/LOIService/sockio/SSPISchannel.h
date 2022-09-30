#pragma once
#include <windows.h>
#include <wincrypt.h>
#include <wintrust.h>
#include <schannel.h>

#define SECURITY_WIN32
#include <security.h>
#include <sspi.h>
	enum SSPI_STATE{
		INBOUND_HANDSHAKE_PENDING=0,
		OUTBOUND_HANDSHAKE_PENDING,
		NEED_MORE_DATA,
		ENCRYPT_DECRYPT,
		SSL_CONNECTION_CLOSE
	};
class CSSPISchannel
{
public:
	CSSPISchannel(void);
	~CSSPISchannel(void);

private:
	PSecurityFunctionTableA			m_pSSPI;
	HMODULE							m_hSecurity;
	CtxtHandle						m_hSecContext;
	CredHandle						m_hSecCredHandle;
	PCCERT_CONTEXT					m_pRemoteCertContext;
	PBYTE							m_pSendStreamBuffer;
	SecPkgContext_StreamSizes		m_StreamSizes;
	BOOL							m_bInitContext;
	BOOL							IntialiseStreamBuffers();
public:
	BOOL			Initialise();
	BOOL			SetInboundAuthCertificate( PCCERT_CONTEXT  pCertContext);
	BOOL			SetOutboundAuthCertificate( PCCERT_CONTEXT  pCertContext);
	SSPI_STATE		PerformInboundHandshake(BYTE *pRecvEncBuffer,DWORD dwEncLen,DWORD &dwExtraBytesCopied,PBYTE &pOutHandShakeBuf,DWORD &dwOutHandShakeBufLen);
	SSPI_STATE		InitiateOutboundHandshake(LPCTSTR pszServerName,PBYTE &pOutHandShakeBuf,DWORD &dwOutHandShakeBufLen);
	SSPI_STATE		PerformOutboundHandshake(BYTE *pRecvEncBuffer,DWORD dwEncLen,DWORD &dwExtraBytesCopied,PBYTE &pOutHandShakeBuf,DWORD &dwOutHandShakeBufLen);
	SSPI_STATE		DecryptMessage(BYTE *pRecvEncBuffer,DWORD dwEncLen,PBYTE &pDecryptedBuffer,DWORD &dwDecryptedLen,DWORD &dwExtraBytesCopied);
	SSPI_STATE		EncryptMessage(BYTE *pSendBuffer,DWORD dwEncLen,PBYTE &pEncryptedBuffer,DWORD &dwEncryptBuffLen,DWORD &dwBytesProcessed);
	PCCERT_CONTEXT	GetRemotePartyCert();
	void			FreeRemoteCert();
	void			FreeContextBuffer(PVOID pBuffer);
	void			FreeStreamBuffer(PVOID pBuffer);
	void			FreeBuffer(PVOID pBuffer);
	DWORD			GetMaxStreamSize();
};
