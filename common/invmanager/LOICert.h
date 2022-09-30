#pragma once
#include <wincrypt.h>
#include "error.h"
#include <strsafe.h>
#include "XmlSerializer.h"

class CTempFileStore{
private:
	TCHAR				m_szTempStoreFilePath[MAX_PATH];
	static HCERTSTORE	m_hTempStore;
	static LONG volatile m_lTempStoreRefCount;
public:
	CTempFileStore()
	{
		TCHAR  szTempPath[MAX_PATH]={0};

		GetTempPath(sizeof(  szTempPath)/sizeof(TCHAR),szTempPath);
		StringCbPrintf(m_szTempStoreFilePath,
							sizeof(m_szTempStoreFilePath),
							_T("%s\\testStor.sto"),
							szTempPath);  
		GetTempStoreHANDLE();
	}
	~CTempFileStore()
	{
		ReleaseTempStoreHANDLE();
	}
	/*******************************************
	Opens a temp store to create a certcontext handle from encoded bytes
	*******************************************/
	HCERTSTORE GetTempStoreHANDLE()
	{
		if(m_hTempStore==NULL)
		{
			// Open temp Certificate store
			m_hTempStore = CertOpenStore(
										  CERT_STORE_PROV_FILENAME,
										  PKCS_7_ASN_ENCODING | X509_ASN_ENCODING,
										  NULL,
										  CERT_FILE_STORE_COMMIT_ENABLE_FLAG ,
										  m_szTempStoreFilePath );
		}
		InterlockedIncrement(& m_lTempStoreRefCount);
		return m_hTempStore;
	}
	/***************************************
	Close the handle if no object uses it
	***************************************/
	void ReleaseTempStoreHANDLE()
	{
		InterlockedDecrement(& m_lTempStoreRefCount);
		if(m_lTempStoreRefCount==0)
		{
			//nobody is using it close it
			if(m_hTempStore)
			{
				CertCloseStore(m_hTempStore,0);
				m_hTempStore=NULL;
			}
		}
	}
	operator HCERTSTORE()
	{
		return m_hTempStore;
	}
};

class CLOICert:public CError
{
private:
	CTempFileStore		m_TempFileStore;
	CXmlSerializer      m_Xml;
	PCCERT_CONTEXT		m_pCertContext;
	TCHAR				m_szContainer[MAX_PATH];
	static TCHAR		CERTXMLTITLE[MAX_PATH];	
	static TCHAR		CERTPARAMNAME[MAX_PATH];	
public:
	CLOICert(void);
	CLOICert(PCCERT_CONTEXT	pCertContext);
	~CLOICert(void);

	BOOL			CreateCertificate(LPCTSTR lpszSubjectName,FILETIME ftNotValidAfterTime,PCCERT_CONTEXT pCACert);
	void			CloseCertificate();
	inline	PCCERT_CONTEXT DuplicateCertContext()
	{
		PCCERT_CONTEXT pCertContext=NULL;

		if(m_pCertContext)
		{
			//pass a duplicate object caller will free it
			pCertContext=CertDuplicateCertificateContext (m_pCertContext);
		}
		return pCertContext;
	}
	BOOL			AddCertParam(LPCTSTR lpszParamName,PBYTE pParam,DWORD dwParamLen);
	BOOL			AddCertParam(LPCTSTR lpszParamName,CString sParam);
	BOOL			GetCertparam(LPCTSTR lpszParamName,PBYTE pParam,DWORD &dwParamLen);
	CString			GetCertparam(LPCTSTR lpszParamName);
	BOOL			Serialise(LPCTSTR lpszFilePath,LPCTSTR lpszPassword);
	BOOL			Deserialise(LPCTSTR lpszFilePath,LPCTSTR lpszPassword);
	CString			GetCertSubjectName();
	VOID			FreeCertparamBytes(PBYTE pParam);
	BOOL			VerifySSLClientCert(PCCERT_CONTEXT pClientCert,PCCERT_CONTEXT pServerCert);
	BOOL			GetCertExpiryTime(FILETIME &ftExpiryTime);
	BOOL			IsCertExpired();
private:
	BOOL			GetSubjectNameBlob(LPTSTR lpszSubjectName,PBYTE &pbyBlob,DWORD &dwBlobSize);
	BOOL			CreateCryptoContext(HCRYPTPROV &hCryptProv,HCRYPTKEY &hPubKey);
	BOOL			FillCERTStructAndPublicKeyInfo(HCRYPTPROV hCryptProv,BYTE *pCertSerialNumber,PCERT_PUBLIC_KEY_INFO *ppPublicKeyInfo,PBYTE &pbyKeyBlobData,DWORD &dwBlobSize);
	BOOL			FillSubjectInfo(PCCERT_CONTEXT  pCACert,CRYPT_DATA_BLOB &CertKeyIdentifier,PBYTE &pbySubjectKeyBlobData,DWORD &dwBlobDataSize);
	BOOL			FillAuthorityKeyIdInfo(PCCERT_CONTEXT pIssuerCert,CRYPT_DATA_BLOB 	&CertKeyIdentifier,CRYPT_DATA_BLOB &CryptIssuerKeyIdBlob,CERT_INFO &IssuerCertInfo,PBYTE  &pbAuthKeyBlobData,DWORD  &dwBlobDataSize);
	BOOL			SignAndEncodeCertificate(PCCERT_CONTEXT pCACert,HCRYPTPROV hCryptProv, HCRYPTPROV hIssuerProv,CERT_INFO  &CertInfo, DWORD	dwIssuerKeyType,LPBYTE	&pbyEncodedCert,DWORD    &dwEncodedCertSize);
	PCCERT_CONTEXT	GetCertContext(LPBYTE	bpEncodedCert,DWORD dwEncodedCertSize);
	BOOL			GetIssuerCertInfo( PCCERT_CONTEXT pCertContext, HCRYPTPROV *phIssuerProv,PCRYPT_DATA_BLOB *pCryptKeyIdBlob,LPDWORD pdwIssuerKeyType);
	PBYTE			GetPFXData( PCCERT_CONTEXT pCertContext,CString sPassword,DWORD &dwDataSize);
	void			FreePFXData(PVOID pByte);
	PCCERT_CONTEXT	ConvertPFXToCert(CString sPassword,PBYTE pPFXData,DWORD dwDataSize);
	
};
