#include "StdAfx.h"
#include "LOICert.h"
#include "../crypt/EncryptFile.h"
#include "cmnresource.h"

HCERTSTORE			CTempFileStore::m_hTempStore=NULL;
LONG volatile       CTempFileStore::m_lTempStoreRefCount=0;
/***********************************
initialise const string
***********************************/
TCHAR				CLOICert::CERTXMLTITLE[]=_T("LOICertParam");
TCHAR				CLOICert::CERTPARAMNAME[]=_T("CertData");

CLOICert::CLOICert(void)
{
	m_pCertContext=NULL;
	//initialise the XML
	m_Xml.Create(CERTXMLTITLE);
}
/*******************
initialise the object with cert context
*************************/
CLOICert::CLOICert(PCCERT_CONTEXT	pCertContext)
{
	m_pCertContext=CertDuplicateCertificateContext(pCertContext);
	//initialise the XML
	m_Xml.Create(CERTXMLTITLE);
}

CLOICert::~CLOICert(void)
{
	CloseCertificate();
}


/***************************************
main function which creates the client certificate if CA cert is passed
otherwise creates a CA cert(issuer & issued name is same)
****************************************/
BOOL CLOICert::CreateCertificate(LPCTSTR lpszSubjectName,FILETIME ftNotValidAfterTime,PCCERT_CONTEXT pCACert)
{
	BOOL					bRet=FALSE;
	CERT_NAME_BLOB			IssuerNameBlob={0};
	HCRYPTPROV				hCryptProv=NULL,hIssuerProv=NULL;
	HCRYPTKEY				hPubKey=NULL;
	CERT_INFO				CertInfo={0};
	CRYPT_DATA_BLOB 		CertKeyIdentifier={0};
	BYTE					CertSerialNumber[8]={0};
	PCRYPT_DATA_BLOB		pCryptIssueKeyIdBlob=NULL;
	DWORD					dwIssuerKeyType=0;
	PBYTE					pbyEncodedCert=NULL;
	DWORD					dwEncodedCertSize=0;
	CERT_EXTENSION			CertExtension[5]={0};
	PCERT_PUBLIC_KEY_INFO	pPublicKeyInfo=NULL;
	FILETIME				ftTime={0};
	SYSTEMTIME				stTime={0};


	//convert subjectname to blob data
	if(!GetSubjectNameBlob((LPTSTR)lpszSubjectName,IssuerNameBlob.pbData,IssuerNameBlob.cbData))
	{
		bRet=FALSE;
		goto cleanup;
	}

	//get the crypto context to perform cryptographic fucntions
	if(!CreateCryptoContext(hCryptProv,hPubKey))
	{
		bRet=FALSE;
		goto cleanup;
	}

	//this creates a public keys for the cert
	if(!FillCERTStructAndPublicKeyInfo(hCryptProv,
										CertSerialNumber,
										&pPublicKeyInfo,
										CertKeyIdentifier.pbData,
										CertKeyIdentifier.cbData ))
	{
		bRet=FALSE;
		goto cleanup;
	}
	// Set Public Key info of Certificate
	CertInfo.SubjectPublicKeyInfo = *pPublicKeyInfo;

	// Set Version of Certificate
	CertInfo.dwVersion = CERT_V3;
	// Set Subject of Certificate
	CertInfo.Subject = IssuerNameBlob;
	// Set Serial Number of Certificate
	CertInfo.SerialNumber.cbData = 8;
	CertInfo.SerialNumber.pbData = CertSerialNumber;

	// Set Signature Algorithm of Certificate
	CertInfo.SignatureAlgorithm.pszObjId = szOID_RSA_SHA1RSA;

	// set NotBefore date
	GetSystemTime(&stTime);
	SystemTimeToFileTime(&stTime, &ftTime);
	CertInfo.NotBefore = ftTime;

	//
	// Set After Date
	//
	CertInfo.NotAfter = ftNotValidAfterTime;

	// Set Issuer of Certificate
	if (pCACert)
	{
		if(!GetIssuerCertInfo( pCACert,
								&hIssuerProv,
								&pCryptIssueKeyIdBlob,
							    &dwIssuerKeyType))
		{
			bRet=FALSE;
			goto cleanup;
		}
		CertInfo.Issuer = pCACert->pCertInfo->Subject;
	}
	else
	{
		CertInfo.Issuer = IssuerNameBlob;
	}
	//encodes & creates the subject blob
	if(!FillSubjectInfo(pCACert,CertKeyIdentifier,
						CertExtension[CertInfo.cExtension].Value.pbData,
						CertExtension[CertInfo.cExtension].Value.cbData)
						)
	{
		bRet=FALSE;
		goto cleanup;
	}
	// Set Subject Key Identifier
	CertExtension[CertInfo.cExtension].pszObjId = szOID_SUBJECT_KEY_IDENTIFIER;
	CertExtension[CertInfo.cExtension].fCritical = FALSE;
	// Increase extension count
	CertInfo.cExtension++;




	//need to fill the authority key identifier
	if(!FillAuthorityKeyIdInfo(pCACert,
							CertKeyIdentifier,
							*pCryptIssueKeyIdBlob,
							CertInfo,
							CertExtension[CertInfo.cExtension].Value.pbData,
							CertExtension[CertInfo.cExtension].Value.cbData))
	{
		bRet=FALSE;
		goto cleanup;
	}

	// Set Authority Key Id extension
	CertExtension[CertInfo.cExtension].pszObjId = szOID_AUTHORITY_KEY_IDENTIFIER;
	CertExtension[CertInfo.cExtension].fCritical = FALSE;
	// Increase extension count
	CertInfo.cExtension++;


	CertInfo.rgExtension = CertExtension;
	

	//final stage of cert creation
	if(!SignAndEncodeCertificate(pCACert,hCryptProv,
										  hIssuerProv,
										   CertInfo,
										   dwIssuerKeyType,
										   pbyEncodedCert,
										   dwEncodedCertSize
											))
	{
		bRet=FALSE;
		goto cleanup;
	}

	//convert the bytes to context
	m_pCertContext=GetCertContext(pbyEncodedCert,dwEncodedCertSize);
	if(m_pCertContext==NULL)
	{
		bRet=FALSE;
		goto cleanup;
	}
	bRet=TRUE;

cleanup:
	//free all buffers allocated for cert creation
	if(IssuerNameBlob.pbData)
	{
		delete IssuerNameBlob.pbData;
		IssuerNameBlob.pbData=NULL;
	}
	if(hPubKey)
	{
		CryptDestroyKey (hPubKey);
		hPubKey=NULL;
	}


	if(CertKeyIdentifier.pbData)
	{
		delete CertKeyIdentifier.pbData;
		CertKeyIdentifier.pbData=NULL;
	}
	//free all extensions created
	while(CertInfo.cExtension)
	{
		delete CertExtension[CertInfo.cExtension].Value.pbData;
		CertExtension[CertInfo.cExtension].Value.pbData=NULL;
		CertInfo.cExtension--;
	}
	if(pCryptIssueKeyIdBlob)
	{
		delete pCryptIssueKeyIdBlob;
		pCryptIssueKeyIdBlob=NULL;
	}
	if(pbyEncodedCert)
	{
		delete pbyEncodedCert;
		pbyEncodedCert=NULL;
	}
	if(pPublicKeyInfo)
	{
		delete pPublicKeyInfo;
		pPublicKeyInfo=NULL;
	}
	if(hCryptProv)
	{
		CryptReleaseContext(hCryptProv,0);
		hCryptProv=NULL;
	}
	if(hIssuerProv)
	{
		CryptReleaseContext(hIssuerProv,0);
		hCryptProv=NULL;
	}
	return bRet;
}
/*****************************************************************************
this closes all handle resource allocated
******************************************************************************/
void CLOICert::CloseCertificate()
{
	if(m_pCertContext)
	{
		CertFreeCertificateContext(m_pCertContext);
		m_pCertContext=NULL;
	}
}
/****************************************
Add cert user defined params in bytes format to the certificate
****************************************/
BOOL CLOICert::AddCertParam(LPCTSTR lpszParamName,PBYTE pParam,DWORD dwParamLen)
{
	m_Xml.WriteGroupBegin(lpszParamName);
	m_Xml.Write(lpszParamName,pParam,dwParamLen);
	m_Xml.WriteGroupEnd();	
	return TRUE;
}
/**********************************************
overloaded to handle string
***********************************************/
BOOL CLOICert::AddCertParam(LPCTSTR lpszParamName,CString sParam)
{
	m_Xml.WriteGroupBegin(lpszParamName);
	m_Xml.Write(lpszParamName,(LPCTSTR)sParam);
	m_Xml.WriteGroupEnd();	
	return TRUE;
}
/**************************************
Gets the parameter from the XML
dwOutParamLen should be contain the size of the parameter passed previously to addcert param
**************************************/
BOOL CLOICert::GetCertparam(LPCTSTR lpszParamName,PBYTE pParam,DWORD &dwParamLen)
{
	PBYTE		pTemp=NULL;
	DWORD		dwLen=0;
	BOOL		bRet=FALSE;

	if(m_Xml.ReadGroupBegin(lpszParamName))
	{
		m_Xml.Read(lpszParamName,pTemp,dwLen);
		if(dwLen && pParam)
		{
			memcpy(pParam,pTemp,dwLen<dwParamLen?dwLen:dwParamLen);
			bRet=TRUE;
			dwParamLen=dwLen;
			FreeCertparamBytes(pTemp);
		}
		m_Xml.ReadGroupEnd();
	}
	return bRet;
}
/**************************************
overloaded to get string params
******************************************/
CString CLOICert::GetCertparam(LPCTSTR lpszParamName)
{
	PBYTE	pParam=NULL;
	DWORD	dwOutInParamLen=0;
	CString	sRet;

	m_Xml.ReadGroupBegin(lpszParamName);
	m_Xml.Read(lpszParamName,sRet);
	m_Xml.ReadGroupEnd();
	return sRet;
}
/**************************************
Free memory allocated while param read
***************************************/
VOID CLOICert::FreeCertparamBytes(PBYTE pParam)
{
	if(pParam)
	{
		m_Xml.FreeBytes(pParam);
	}
}
/*****************************************
gets the subject name of the cert
******************************************/
CString CLOICert::GetCertSubjectName()
{
	if(m_pCertContext)
	{
		DWORD		dwSubjectSize;
		CString		sSubjectName;
		LPTSTR		pszName = NULL;

		dwSubjectSize = CertGetNameString(m_pCertContext, 
                                         CERT_NAME_SIMPLE_DISPLAY_TYPE,
                                         0,
                                         NULL,
                                         NULL,
                                         0);
		 // Get Subject name size.
        if (dwSubjectSize)
        {
           // Allocate memory for subject name.
		   pszName = (LPTSTR)new BYTE[dwSubjectSize * sizeof(TCHAR)];

	       // Get subject name.
           CertGetNameString(m_pCertContext, 
                                CERT_NAME_SIMPLE_DISPLAY_TYPE,
                                0,
                                NULL,
                                pszName,
                                dwSubjectSize);
		   sSubjectName=pszName;
		   delete pszName;
		   return sSubjectName;
		}
			
	}
	SetLastErrorMessage(IDS_FAILED_GET_SUBJECT_NAME,GetLastError()); 
	DEBUGLOG(LOI_ERROR,(LPTSTR)(LPCTSTR)GetLastErrorMessage());
	return CString(_T(""));
}

/**********************************************************
converts a subject name to blob data
***********************************************************/
BOOL CLOICert::GetSubjectNameBlob(LPTSTR lpszSubjectName,PBYTE &pbyBlob,DWORD &dwBlobSize)
{
	LPBYTE		pbNameBlob = NULL;
	DWORD		dwSize;
	BOOL		bResult = TRUE;

	// Get X509 Name and convert it to a Name Blob
	bResult = CertStrToName(X509_ASN_ENCODING,
							  lpszSubjectName,
							  CERT_X500_NAME_STR,
							  NULL,
							  NULL,
							  &dwSize,
							  NULL);
	if (!bResult)
	{
		SetLastErrorMessage(IDS_ERR_FAILED_TO_GET_BLOB,GetLastError()); 
		DEBUGLOG(LOI_ERROR,(LPTSTR)(LPCTSTR)GetLastErrorMessage());
		return FALSE;
	}

	// Allocate memory for Name Blob
	pbNameBlob = (LPBYTE)new BYTE[dwSize];

	// Convert X509 Name to Name Blob
	bResult = CertStrToName(X509_ASN_ENCODING,
						  lpszSubjectName,
						  CERT_X500_NAME_STR,
						  NULL,
						  (LPBYTE)pbNameBlob,
						  &dwSize,
						  NULL);
	if (!bResult)
	{
		SetLastErrorMessage(IDS_ERR_FAILED_TO_GET_BLOB,GetLastError()); 
		DEBUGLOG(LOI_ERROR,(LPTSTR)(LPCTSTR)GetLastErrorMessage());
		delete pbNameBlob;
		return FALSE;
	}
	pbyBlob=pbNameBlob;
	dwBlobSize=dwSize;

	
	return bResult;
}

/*****************************************
crytpto context for all encryption process like hashing etc 
*****************************************/
BOOL CLOICert::CreateCryptoContext(HCRYPTPROV &hCryptProv,HCRYPTKEY &hPubKey)
{
	BOOL		bResult = TRUE;
	UUID		Uuid;
	LPTSTR		lpszContainer = NULL;
	RPC_STATUS	Status;

	UuidCreate(&Uuid);

	// convert random uuid to a string, we will use it as a container
	Status = UuidToString(&Uuid, (RPC_WSTR *)&lpszContainer);
	if (Status != RPC_S_OK)
	{
		return FALSE;
	}

	_stprintf_s(m_szContainer,MAX_PATH, L"%s",lpszContainer);
	RpcStringFree((RPC_WSTR *)&lpszContainer);
	// Create new crypto context
	bResult = CryptAcquireContext(&hCryptProv,
									m_szContainer,
									MS_ENHANCED_PROV ,
									PROV_RSA_FULL,
									CRYPT_NEWKEYSET );
	if(!bResult)
	{
		SetLastErrorMessage(IDS_FAILED_TO_ACQUIRE_CONTEXT,GetLastError()); 
		DEBUGLOG(LOI_ERROR,(LPTSTR)(LPCTSTR)GetLastErrorMessage());
		return FALSE;
	}

	// Generate Private/Public key pair
	bResult = CryptGenKey(hCryptProv, AT_KEYEXCHANGE, CRYPT_EXPORTABLE, &hPubKey);
	if(!bResult)
	{
		CryptReleaseContext(hCryptProv,0);
		SetLastErrorMessage(IDS_FAILED_TO_ACQUIRE_CONTEXT,GetLastError()); 
		DEBUGLOG(LOI_ERROR,(LPTSTR)(LPCTSTR)GetLastErrorMessage());
		return FALSE;
	}
	
	return TRUE;
}
/**********************************************
Creates the public key & calculates the hash of it
It returs the key identifier blob data & size
*********************************************/
BOOL CLOICert::FillCERTStructAndPublicKeyInfo(HCRYPTPROV hCryptProv,
												 BYTE		*pCertSerialNumber,
												 PCERT_PUBLIC_KEY_INFO		*ppPublicKeyInfo,
												 PBYTE &pbyKeyBlobData,
												 DWORD &dwBlobSize)
{
	BOOL						bResult = TRUE;
	DWORD						dwSize;
	LPBYTE						pbKeyIdentifier = NULL;
	HCRYPTHASH					hHash=NULL;




	// Create Random Serial Number
	bResult = CryptGenRandom(hCryptProv, 8, pCertSerialNumber);
	if (!bResult)
	{
		SetLastErrorMessage(IDS_ERR_FAILED_TO_GET_PUBLIC_KEY,GetLastError());  
		DEBUGLOG(LOI_ERROR,(LPTSTR)(LPCTSTR)GetLastErrorMessage());
		return FALSE;
	}



	// Get Public Key Info size
	bResult = CryptExportPublicKeyInfo(hCryptProv, AT_KEYEXCHANGE,
							   X509_ASN_ENCODING, NULL, &dwSize);
	if (!bResult)
	{
		SetLastErrorMessage(IDS_ERR_FAILED_TO_GET_PUBLIC_KEY,GetLastError());  
		DEBUGLOG(LOI_ERROR,(LPTSTR)(LPCTSTR)GetLastErrorMessage());
		return FALSE;
	}

	// Allocate memory for Public Key Info
	*ppPublicKeyInfo = (PCERT_PUBLIC_KEY_INFO)new BYTE[dwSize];

	// Get Public Key Info
	bResult = CryptExportPublicKeyInfo(hCryptProv, AT_KEYEXCHANGE,
							   X509_ASN_ENCODING,
							   *ppPublicKeyInfo, &dwSize);
	if (!bResult)
	{
		SetLastErrorMessage(IDS_ERR_FAILED_TO_GET_PUBLIC_KEY,GetLastError());  
		DEBUGLOG(LOI_ERROR,(LPTSTR)(LPCTSTR)GetLastErrorMessage());
		delete *ppPublicKeyInfo; 
		return bResult;
	}



	// Create Hash
	bResult = CryptCreateHash(hCryptProv, CALG_SHA1, 0, 0, &hHash);
	if (!bResult)
	{
		SetLastErrorMessage(IDS_ERR_FAILED_TO_GET_PUBLIC_KEY,GetLastError());  
		DEBUGLOG(LOI_ERROR,(LPTSTR)(LPCTSTR)GetLastErrorMessage());
		delete *ppPublicKeyInfo; 
		return FALSE;
	}

	// Hash Public Key Info
	bResult = CryptHashData(hHash, (LPBYTE)*ppPublicKeyInfo, dwSize, 0);
	if (!bResult)
	{
		SetLastErrorMessage(IDS_ERR_FAILED_TO_GET_PUBLIC_KEY,GetLastError());  
		DEBUGLOG(LOI_ERROR,(LPTSTR)(LPCTSTR)GetLastErrorMessage());
		delete *ppPublicKeyInfo; 
		CryptDestroyHash(hHash);
		return FALSE;
	}

	// Get Size of Hash
	bResult = CryptGetHashParam(hHash, HP_HASHVAL, NULL, &dwSize, 0);
	if (!bResult)
	{
		SetLastErrorMessage(IDS_ERR_FAILED_TO_GET_PUBLIC_KEY,GetLastError());  
		DEBUGLOG(LOI_ERROR,(LPTSTR)(LPCTSTR)GetLastErrorMessage());
		delete *ppPublicKeyInfo; 
		CryptDestroyHash(hHash);
		return FALSE;
	}

	// Allocate Memory for Key Identifier (hash of Public Key info)
	pbKeyIdentifier = new BYTE[dwSize];

	// Get Hash of Public Key Info
	bResult = CryptGetHashParam(hHash, HP_HASHVAL, pbKeyIdentifier, &dwSize, 0);
	if (!bResult)
	{
		SetLastErrorMessage(IDS_ERR_FAILED_TO_GET_PUBLIC_KEY,GetLastError());  
		DEBUGLOG(LOI_ERROR,(LPTSTR)(LPCTSTR)GetLastErrorMessage());
		CryptDestroyHash(hHash);
		delete *ppPublicKeyInfo; 
		delete pbKeyIdentifier;
		return FALSE;
	}

	CryptDestroyHash(hHash);

	// We will use this to set the Key Identifier extension
	pbyKeyBlobData=pbKeyIdentifier;
	dwBlobSize = dwSize;



	return TRUE;
}
/*****************************************
Fills the subject information
*****************************************/
BOOL CLOICert::FillSubjectInfo(PCCERT_CONTEXT  pCACert,
								  CRYPT_DATA_BLOB 	&CertKeyIdentifier,
								  PBYTE &pbySubjectKeyBlobData,
								  DWORD &dwBlobDataSize)
{
	BOOL bResult = TRUE;
	DWORD dwSize = 0;
	LPBYTE SubjectKeyIdentifier = NULL;


	// Get Subject Key Identifier Extension size
	bResult = CryptEncodeObject(X509_ASN_ENCODING,
					  szOID_SUBJECT_KEY_IDENTIFIER,
					  (LPVOID)&CertKeyIdentifier,
					  NULL, &dwSize);
	if (!bResult)
	{
		SetLastErrorMessage(IDS_ERR_FAILED_TO_FILL_SUBJECT,GetLastError()) ;
		DEBUGLOG(LOI_ERROR,(LPTSTR)(LPCTSTR)GetLastErrorMessage());
		return FALSE;
	}

	// Allocate Memory for Subject Key Identifier Blob
	SubjectKeyIdentifier = (LPBYTE)new BYTE[dwSize];


	// Get Subject Key Identifier Extension
	bResult = CryptEncodeObject(X509_ASN_ENCODING,
					  szOID_SUBJECT_KEY_IDENTIFIER,
					  (LPVOID)&CertKeyIdentifier,
					  SubjectKeyIdentifier, &dwSize);
	if (!bResult)
	{
		SetLastErrorMessage(IDS_ERR_FAILED_TO_FILL_SUBJECT,GetLastError()) ;
		DEBUGLOG(LOI_ERROR,(LPTSTR)(LPCTSTR)GetLastErrorMessage());
		delete SubjectKeyIdentifier; 
		return FALSE;
	}

	pbySubjectKeyBlobData= SubjectKeyIdentifier;
	dwBlobDataSize=dwSize;

	return TRUE;
}
/***********************************************
Authority key information for client cert
***********************************************/
BOOL CLOICert::FillAuthorityKeyIdInfo(PCCERT_CONTEXT pIssuerCert,
										 CRYPT_DATA_BLOB 	&CertKeyIdentifier,
										 CRYPT_DATA_BLOB &CryptIssuerKeyIdBlob,
										 CERT_INFO &IssuerCertInfo,
										 PBYTE  &pbAuthKeyBlobData,
										 DWORD  &dwBlobDataSize
										 )
{
	BOOL							bResult = TRUE;
	DWORD							dwSize = 0;
	CERT_AUTHORITY_KEY_ID_INFO		AuthorityKeyId;
	BOOL							bAddAuthorityExtension = FALSE;
	LPBYTE							pbAuthorityKeyId = NULL;

	if (pIssuerCert)
	{
		AuthorityKeyId.KeyId = CryptIssuerKeyIdBlob;
		AuthorityKeyId.CertIssuer = pIssuerCert->pCertInfo->Issuer;
		AuthorityKeyId.CertSerialNumber = pIssuerCert->pCertInfo->SerialNumber;
	}
	else
	{
		AuthorityKeyId.KeyId = CertKeyIdentifier;
		AuthorityKeyId.CertIssuer = IssuerCertInfo.Issuer;
		AuthorityKeyId.CertSerialNumber = IssuerCertInfo.SerialNumber;
	}

	// Get Authority Key Id blob size
	bResult = CryptEncodeObject(X509_ASN_ENCODING,
						 X509_AUTHORITY_KEY_ID,
						 (LPVOID)&AuthorityKeyId,
						 NULL, &dwSize);
	if (!bResult)
	{
		
		SetLastErrorMessage(IDS_ERR_FAILED_TO_SET_KEY,GetLastError()) ;
		DEBUGLOG(LOI_ERROR,(LPTSTR)(LPCTSTR)GetLastErrorMessage());
		return FALSE;
	}

	// Allocate Authority Key Id Blob
	pbAuthorityKeyId = (LPBYTE)new BYTE[dwSize];
	if (!pbAuthorityKeyId)
	{
		SetLastErrorMessage(IDS_ERR_FAILED_TO_SET_KEY,GetLastError()) ;
		DEBUGLOG(LOI_ERROR,(LPTSTR)(LPCTSTR)GetLastErrorMessage());
		return FALSE;
	}
	// Get Authority Key Id blob
	bResult = CryptEncodeObject(X509_ASN_ENCODING,
						 X509_AUTHORITY_KEY_ID,
						 (LPVOID)&AuthorityKeyId,
						 pbAuthorityKeyId, &dwSize);
	if (!bResult)
	{
		SetLastErrorMessage(IDS_ERR_FAILED_TO_SET_KEY,GetLastError()) ;
		DEBUGLOG(LOI_ERROR,(LPTSTR)(LPCTSTR)GetLastErrorMessage());
		delete pbAuthorityKeyId;
		return bResult;
	}

	pbAuthKeyBlobData=pbAuthorityKeyId;
	dwBlobDataSize=dwSize;
	return TRUE;
}
/**********************************
signs & encodes the certificate with hash
***********************************/
BOOL CLOICert::SignAndEncodeCertificate(PCCERT_CONTEXT pCACert,
										   HCRYPTPROV hCryptProv,
										   HCRYPTPROV hIssuerProv,
										   CERT_INFO  &CertInfo,
										   DWORD	dwIssuerKeyType,
										   LPBYTE	&pbyEncodedCert,
										   DWORD    &dwEncodedCertSize
											)
{
	BOOL			bResult = FALSE;

	dwEncodedCertSize = 0;
	
	if (pCACert)
	{
		// Get Encoded Certificate Size
		bResult = CryptSignAndEncodeCertificate(hIssuerProv, dwIssuerKeyType,
											 X509_ASN_ENCODING, X509_CERT_TO_BE_SIGNED,
											 (LPVOID)&CertInfo,
											 &(pCACert->pCertInfo->SignatureAlgorithm),
											 NULL, NULL, &dwEncodedCertSize);
		if (!bResult)
		{
			SetLastErrorMessage(IDS_ERR_SIGN_FAILED,GetLastError()) ;
			DEBUGLOG(LOI_ERROR,(LPTSTR)(LPCTSTR)GetLastErrorMessage());
			return FALSE;
		}

		// Allocate memory for encoded certificate
		pbyEncodedCert = (LPBYTE)new BYTE[dwEncodedCertSize];

		// Sign and Encode certificate
		bResult = CryptSignAndEncodeCertificate(hIssuerProv, dwIssuerKeyType,
											 X509_ASN_ENCODING, X509_CERT_TO_BE_SIGNED,
											 (LPVOID)&CertInfo,
											 &(pCACert->pCertInfo->SignatureAlgorithm),
											 NULL, pbyEncodedCert, &dwEncodedCertSize);
	}
	else
	{
		// Get Encoded Certificate Size
		bResult = CryptSignAndEncodeCertificate(hCryptProv, AT_KEYEXCHANGE,
											 X509_ASN_ENCODING, X509_CERT_TO_BE_SIGNED,
											 (LPVOID)&CertInfo,
											 &(CertInfo.SignatureAlgorithm),
											 NULL, NULL, &dwEncodedCertSize);
		if (!bResult)
		{
			SetLastErrorMessage(IDS_ERR_SIGN_FAILED,GetLastError());
			DEBUGLOG(LOI_ERROR,(LPTSTR)(LPCTSTR)GetLastErrorMessage());
			return FALSE;
		}

		// Allocate memory for encoded certificate
		pbyEncodedCert = (LPBYTE)new BYTE[dwEncodedCertSize];

	
		// Sign and Encode certificate
		bResult = CryptSignAndEncodeCertificate(hCryptProv, AT_KEYEXCHANGE,
											 X509_ASN_ENCODING, X509_CERT_TO_BE_SIGNED,
											 (LPVOID)&CertInfo,
											 &(CertInfo.SignatureAlgorithm),
											 NULL, pbyEncodedCert, 
											 &dwEncodedCertSize);
	
	}
	return bResult;
}
/*******************************************
Converts cert encoded byte stream to cert context
*******************************************/
PCCERT_CONTEXT CLOICert::GetCertContext(LPBYTE	bpEncodedCert,DWORD dwEncodedCertSize)
{
	PCCERT_CONTEXT			pCertContext = NULL;
	HCERTSTORE				hStore = NULL;
	CRYPT_KEY_PROV_INFO		CryptKeyProvInfo={0};


	// Place Certificate in store
	if(!CertAddEncodedCertificateToStore((HCERTSTORE)m_TempFileStore, X509_ASN_ENCODING,
							bpEncodedCert, dwEncodedCertSize,
							CERT_STORE_ADD_REPLACE_EXISTING,
							&pCertContext))
	{
		return NULL;
	}

	// Initialize CRYPT_KEY_PROV_INFO structure
	ZeroMemory(&CryptKeyProvInfo, sizeof(CryptKeyProvInfo));
	CryptKeyProvInfo.pwszContainerName = m_szContainer;
	CryptKeyProvInfo.pwszProvName = MS_ENHANCED_PROV;
	CryptKeyProvInfo.dwProvType = PROV_RSA_FULL;
	CryptKeyProvInfo.dwKeySpec = AT_KEYEXCHANGE;

	// Set Certificate's Key Provider info
	if(!CertSetCertificateContextProperty(pCertContext,
								CERT_KEY_PROV_INFO_PROP_ID,
								0, (LPVOID)&CryptKeyProvInfo))
	{
		//free cert context
		CertFreeCertificateContext(pCertContext);
		return NULL;
	}
	
	return pCertContext;
}

/*************************************************
issuer certificate cryptocontext & keyid info is required to create client 
certificate with this parameters
*************************************************/
BOOL CLOICert::GetIssuerCertInfo( PCCERT_CONTEXT pCertContext,
									 HCRYPTPROV *phIssuerProv,
								     PCRYPT_DATA_BLOB *pCryptKeyIdBlob,
									 LPDWORD pdwIssuerKeyType)
{
   PCRYPT_KEY_PROV_INFO					KeyProvInfo;
   PCERT_EXTENSION						pExtension = NULL;
   TCHAR								szContainer[160]={0};
   TCHAR								szProvider[160]={0};
   BOOL									bResult;
   DWORD								dwSize;

   *pCryptKeyIdBlob=NULL;
   *pdwIssuerKeyType=NULL;


     // Get Key Provider Info size
     bResult = CertGetCertificateContextProperty(pCertContext,
                                         CERT_KEY_PROV_INFO_PROP_ID,
                                         NULL,
                                         &dwSize);
     if (!bResult)
     {
		 SetLastErrorMessage(IDS_ERR_CA_CERT_INFO_FAILED,GetLastError());
		 DEBUGLOG(LOI_ERROR,(LPTSTR)(LPCTSTR)GetLastErrorMessage());
        return FALSE;
     }

     // Allocate Key Provider Info
     KeyProvInfo = (PCRYPT_KEY_PROV_INFO)new BYTE[dwSize];

     // Get Key Provider Info
     bResult = CertGetCertificateContextProperty(pCertContext,
                                         CERT_KEY_PROV_INFO_PROP_ID,
                                         (LPVOID)KeyProvInfo,
                                         &dwSize);
     if (!bResult)
     {
		  SetLastErrorMessage(IDS_ERR_CA_CERT_INFO_FAILED,GetLastError());
		  DEBUGLOG(LOI_ERROR,(LPTSTR)(LPCTSTR)GetLastErrorMessage());
		delete KeyProvInfo;
        return FALSE;
     }

	 if(KeyProvInfo->pwszContainerName)
		StringCbCopy(szContainer,sizeof(szContainer),KeyProvInfo->pwszContainerName);
	 if(KeyProvInfo->pwszProvName)
 		StringCbCopy(szProvider,sizeof(szProvider),KeyProvInfo->pwszProvName);

       // Get Crypto Context of Certificate
     bResult = CryptAcquireContext(phIssuerProv, 
									szContainer,
									MS_ENHANCED_PROV,
                                   PROV_RSA_FULL, 
								   0);
     if (!bResult)
     {
		SetLastErrorMessage(IDS_ERR_CA_CERT_INFO_FAILED,GetLastError());
		DEBUGLOG(LOI_ERROR,(LPTSTR)(LPCTSTR)GetLastErrorMessage());
        delete KeyProvInfo;
        return FALSE;
     }
     // Return Key Spec
     *pdwIssuerKeyType = KeyProvInfo->dwKeySpec;


     // Find Subject Key Identifier Extension
     pExtension = CertFindExtension(szOID_SUBJECT_KEY_IDENTIFIER,
                                    pCertContext->pCertInfo->cExtension,
                                    pCertContext->pCertInfo->rgExtension);

     // If Subject Key Identifier Extension Exists
     if (!pExtension)
     {
		 SetLastErrorMessage(IDS_ERR_CA_CERT_INFO_FAILED,GetLastError());
		 DEBUGLOG(LOI_ERROR,(LPTSTR)(LPCTSTR)GetLastErrorMessage());
		 delete KeyProvInfo;
		 return FALSE;
	 }

    // Get Size of Data Blob
    bResult = CryptDecodeObject(X509_ASN_ENCODING,
                                szOID_SUBJECT_KEY_IDENTIFIER,
                                pExtension->Value.pbData,
                                pExtension->Value.cbData,
                                0, NULL, &dwSize);
    if (!bResult)
    {
	   SetLastErrorMessage(IDS_ERR_CA_CERT_INFO_FAILED,GetLastError());
	   DEBUGLOG(LOI_ERROR,(LPTSTR)(LPCTSTR)GetLastErrorMessage());
	   delete KeyProvInfo;
       return FALSE;
    }

    // Allocate Data Blob
    *pCryptKeyIdBlob = (PCRYPT_DATA_BLOB)new BYTE[dwSize];

    // Get Key Identifier Data Blob
    bResult = CryptDecodeObject(X509_ASN_ENCODING,
                                szOID_SUBJECT_KEY_IDENTIFIER,
                                pExtension->Value.pbData,
                                pExtension->Value.cbData,
                                0, (LPVOID)*pCryptKeyIdBlob, &dwSize);
    if (!bResult)
    {
	   SetLastErrorMessage(IDS_ERR_CA_CERT_INFO_FAILED,GetLastError());
	   DEBUGLOG(LOI_ERROR,(LPTSTR)(LPCTSTR)GetLastErrorMessage());
      delete KeyProvInfo;
	  delete *pCryptKeyIdBlob;
      return FALSE;
    }


	return TRUE;
}

/**********************
serialises all xml data to a file
**********************/
BOOL CLOICert::Serialise(LPCTSTR lpszFilePath,LPCTSTR lpszPassword)
{
	//convert cert to bytes 
	if(m_pCertContext)
	{
		PBYTE		pbyEncodedCert=NULL;
		DWORD		dwEncodedCertSize=0;
/*		if(CertSerializeCertificateStoreElement(m_pCertContext,      // The existing certificate.
												0,                 // Accept default for dwFlags, 
												NULL,              // NULL for the first function call.
												&dwEncodedCertSize))       // Address where the length of the 
												// serialized element will be placed.
		{
			//-------------------------------------------------------------------
			// Allocate memory for the serialized element.
			pbyEncodedCert = new BYTE[dwEncodedCertSize];
			
			//-------------------------------------------------------------------
			// Create the serialized element from a certificate context.
			CertSerializeCertificateStoreElement(m_pCertContext,        // The certificate context source for the // serialized element.
												 0,                   // dwFlags. Accept the default.
												 pbyEncodedCert,           // A pointer to where the new element will // be stored.
												 &dwEncodedCertSize);         // The length of the serialized element,
*/
			//convert to PFX
			if(!(pbyEncodedCert=GetPFXData(m_pCertContext,lpszPassword,dwEncodedCertSize)))
			{
				return FALSE;
			}

			//add cert data to xml
			AddCertParam(CERTPARAMNAME,pbyEncodedCert,dwEncodedCertSize);
			FreePFXData(pbyEncodedCert);

//			delete pbyEncodedCert;
			CEncryptFile		Encrypt;
			CString				sXmlData;

			if(!Encrypt.Create(lpszFilePath,lpszPassword))
			{
				SetLastErrorMessage(Encrypt.GetLastErrorMessage());
				DEBUGLOG(LOI_ERROR,(LPTSTR)(LPCTSTR)GetLastErrorMessage());
				return FALSE;
			}
			//file created successfully write data
			sXmlData=m_Xml.GetXml();
			//write the xml file data
			Encrypt.Write((LPBYTE)(LPCTSTR)sXmlData, (sXmlData.GetLength()*sizeof(TCHAR))+sizeof(TCHAR));
			return TRUE;
//		}
	}
											
	
	return FALSE;
}
/*******************************
deserilaises cert data from a file
*******************************/
BOOL CLOICert::Deserialise(LPCTSTR lpszFilePath,LPCTSTR lpszPassword)
{
	CEncryptFile		Decrypt;
	CString				sXmlData;
	PBYTE				pFileData=NULL;
	DWORD				dwDataRead;

	//open the file
	if(!Decrypt.Open(lpszFilePath,lpszPassword))
	{
		SetLastErrorMessage(Decrypt.GetLastErrorMessage());
		DEBUGLOG(LOI_ERROR,(LPTSTR)(LPCTSTR)GetLastErrorMessage());
		return FALSE;
	}
	dwDataRead=Decrypt.GetSize();
	pFileData=new BYTE[dwDataRead];
	//decrypt the data
	Decrypt.Read(pFileData,dwDataRead);

	sXmlData=CString((LPCTSTR)pFileData);
	delete pFileData;

	//close existing xml data
	m_Xml.Close();

	//open the xml file & load the data
	if(m_Xml.LoadXML (sXmlData,CERTXMLTITLE))
	{
		DWORD		dwCertSize=60*1024;
		PBYTE		pCertBytes=new BYTE[dwCertSize];
		//load the cert data
		GetCertparam(CERTPARAMNAME,pCertBytes,dwCertSize);
		if(dwCertSize)
		{
			m_pCertContext=ConvertPFXToCert(lpszPassword,pCertBytes,dwCertSize);
			//cert exists
			//convert cert bytes to store context
			/*
			CertAddSerializedElementToStore((HCERTSTORE)m_TempFileStore, 
							pCertBytes, dwCertSize,
							CERT_STORE_ADD_REPLACE_EXISTING,
							0,
							CERT_STORE_CERTIFICATE_CONTEXT_FLAG,
							NULL,
							(const void**)&m_pCertContext);
			*/
		}
		delete pCertBytes;
	}

	return m_pCertContext?TRUE:FALSE;
}
/***************************************
gets the pfx data if size if not enough then fills the data
****************************************/
PBYTE CLOICert::GetPFXData( PCCERT_CONTEXT pCertContext,CString sPassword,DWORD &dwDataSize)
{
	CRYPT_DATA_BLOB			Blob={0};
	HCERTSTORE				hTempStore = NULL;

	hTempStore = CertOpenStore(CERT_STORE_PROV_MEMORY, 0, NULL, CERT_STORE_CREATE_NEW_FLAG, 0);
	if(hTempStore==NULL)
	{
		SetLastErrorMessage( IDS_ERR_PFX_CONVERSION_FAILED,GetLastError()); 
		return NULL;
	}

	//add the cert to the temp store
	if(!CertAddCertificateContextToStore(hTempStore,
										  pCertContext,
										  CERT_STORE_ADD_NEW, 
										  NULL)
										  )
	{
		CertCloseStore(hTempStore, 0);
		SetLastErrorMessage( IDS_ERR_PFX_CONVERSION_FAILED,GetLastError()); 
		DEBUGLOG(LOI_ERROR,(LPTSTR)(LPCTSTR)GetLastErrorMessage());
		return NULL;
	}

	if(!PFXExportCertStoreEx(
							hTempStore, 
							&Blob, 
							(LPCTSTR)sPassword, 
							NULL, 
							EXPORT_PRIVATE_KEYS | REPORT_NO_PRIVATE_KEY | REPORT_NOT_ABLE_TO_EXPORT_PRIVATE_KEY)
							)
	{
		CertCloseStore(hTempStore, 0);
		SetLastErrorMessage( IDS_ERR_PFX_CONVERSION_FAILED,GetLastError()); 
		DEBUGLOG(LOI_ERROR,(LPTSTR)(LPCTSTR)GetLastErrorMessage());
		return NULL;
	}
              
    Blob.pbData = (BYTE *)CryptMemAlloc(Blob.cbData);

    if (!PFXExportCertStoreEx(
							hTempStore,
							&Blob,
							(LPCTSTR)sPassword, 
							NULL,
							EXPORT_PRIVATE_KEYS)
							)
	{
		CryptMemFree(Blob.pbData);
		CertCloseStore(hTempStore, 0);
		SetLastErrorMessage( IDS_ERR_PFX_CONVERSION_FAILED,GetLastError()); 
		DEBUGLOG(LOI_ERROR,(LPTSTR)(LPCTSTR)GetLastErrorMessage());
		return NULL;
	}
	
	dwDataSize=Blob.cbData;
    CertCloseStore(hTempStore, 0);
	return  Blob.pbData ;

}
/***************************
Convert PFX data to certcontext
*****************************/
PCCERT_CONTEXT CLOICert::ConvertPFXToCert(CString sPassword,PBYTE pPFXData,DWORD dwDataSize)
{
	CRYPT_DATA_BLOB			Blob={0};
	HCERTSTORE				hTempStore = NULL;
	BYTE					*pbyCryptData=NULL;
	PCCERT_CONTEXT			pCertContext=NULL;

	pbyCryptData=(LPBYTE)CryptMemAlloc(dwDataSize);
	memcpy(pbyCryptData,pPFXData,dwDataSize);
	Blob.pbData = (BYTE *)pbyCryptData;
    Blob.cbData = dwDataSize;	

	hTempStore = PFXImportCertStore(&Blob,(LPCTSTR) sPassword, CRYPT_EXPORTABLE);
    if (!hTempStore)
	{
		SetLastErrorMessage( IDS_ERR_IMPORT_PFX,GetLastError()); 
		DEBUGLOG(LOI_ERROR,(LPTSTR)(LPCTSTR)GetLastErrorMessage());
		return NULL;
	}
	PCCERT_CONTEXT pctx   = 0;
    pCertContext = CertEnumCertificatesInStore(hTempStore, pctx);
	
	CertCloseStore(hTempStore,0); 
	return pCertContext;

}
/*********************
PFX data free
************************/
void CLOICert::FreePFXData(PVOID pByte)
{
	if(pByte)
	{
		CryptMemFree(pByte);
	}
}
/*******************************************
Verify the client certificate with the CAStore
********************************************/
BOOL CLOICert::VerifySSLClientCert(PCCERT_CONTEXT pClientCert,PCCERT_CONTEXT pServerCert)
{
	HTTPSPolicyCallbackData			polHttps={0};
	CERT_CHAIN_POLICY_PARA			PolicyPara={0};
	CERT_CHAIN_POLICY_STATUS		PolicyStatus={0};
	CERT_CHAIN_PARA					ChainPara={0};
	PCCERT_CHAIN_CONTEXT			pChainContext = NULL;
	LPSTR							pszUsage;
	DWORD							dwStatus;
	PCCERT_CONTEXT					pCACert=NULL;
	HCERTSTORE                      hServerCertStore=NULL;

    hServerCertStore = CertOpenStore(CERT_STORE_PROV_MEMORY, 0, NULL, CERT_STORE_CREATE_NEW_FLAG, 0);
    if(hServerCertStore==NULL)
    {
        SetLastErrorMessage( IDS_ERR_SSL_CERTIFICATE_VALIDATE_FAILED,GetLastError()); 
        DEBUGLOG(LOI_ERROR,(LPTSTR)(LPCTSTR)GetLastErrorMessage());
        goto cleanup;
    }

    //add the cert to the temp store
    if(!CertAddCertificateContextToStore(hServerCertStore,
                                          pServerCert,
                                          CERT_STORE_ADD_NEW, 
                                          NULL)
                                          )
    {
        SetLastErrorMessage( IDS_ERR_SSL_CERTIFICATE_VALIDATE_FAILED,GetLastError()); 
        DEBUGLOG(LOI_ERROR,(LPTSTR)(LPCTSTR)GetLastErrorMessage());
        goto cleanup;
    }

	//
	// Build certificate chain.
	//
	pszUsage = szOID_PKIX_KP_CLIENT_AUTH;

	ZeroMemory(&ChainPara, sizeof(ChainPara));
	ChainPara.cbSize = sizeof(ChainPara);
	ChainPara.RequestedUsage.dwType = USAGE_MATCH_TYPE_OR;
	ChainPara.RequestedUsage.Usage.cUsageIdentifier     = 1;
	ChainPara.RequestedUsage.Usage.rgpszUsageIdentifier = &pszUsage;

	if(!CertGetCertificateChain(
							NULL,
							pClientCert,
							NULL,
							hServerCertStore,
							&ChainPara,
							0,
							NULL,
							&pChainContext))
	{
		dwStatus = GetLastError();
		SetLastErrorMessage(IDS_ERR_SSL_CERTIFICATE_VALIDATE_FAILED,dwStatus); 
		DEBUGLOG(LOI_ERROR,(LPTSTR)(LPCTSTR)GetLastErrorMessage());
		goto cleanup;
	}


	//
	// Validate certificate chain.
	// 

	ZeroMemory(&polHttps, sizeof(HTTPSPolicyCallbackData));
	polHttps.cbStruct           = sizeof(HTTPSPolicyCallbackData);
	polHttps.dwAuthType         = AUTHTYPE_CLIENT;
	polHttps.fdwChecks          = 0x00000100|0x00001000|0x00000080|0x00000200;//SECURITY_FLAG_IGNORE_UNKNOWN_CA
	polHttps.pwszServerName     = NULL;

	memset(&PolicyPara, 0, sizeof(PolicyPara));
	PolicyPara.cbSize            = sizeof(PolicyPara);
	PolicyPara.pvExtraPolicyPara = &polHttps;

	memset(&PolicyStatus, 0, sizeof(PolicyStatus));
	PolicyStatus.cbSize = sizeof(PolicyStatus);

	if(!CertVerifyCertificateChainPolicy(
							CERT_CHAIN_POLICY_SSL,
							pChainContext,
							&PolicyPara,
							&PolicyStatus))
	{
		dwStatus = GetLastError();
		SetLastErrorMessage(IDS_ERR_SSL_CERTIFICATE_VALIDATE_FAILED,dwStatus); 
		DEBUGLOG(LOI_ERROR,(LPTSTR)(LPCTSTR)GetLastErrorMessage());
		goto cleanup;
	}

	if(PolicyStatus.dwError)
	{
		dwStatus = PolicyStatus.dwError;
		SetLastErrorMessage(IDS_ERR_SSL_CERTIFICATE_VALIDATE_FAILED,dwStatus); 
		DEBUGLOG(LOI_ERROR,(LPTSTR)(LPCTSTR)GetLastErrorMessage());
		goto cleanup;
	}

	if(CertVerifyTimeValidity (NULL,pClientCert->pCertInfo )==1)//means expired certificate
	{
		dwStatus=SEC_E_CERT_EXPIRED;
		SetLastErrorMessage(IDS_ERR_SSL_CERTIFICATE_VALIDATE_FAILED,dwStatus); 
		DEBUGLOG(LOI_ERROR,(LPTSTR)(LPCTSTR)GetLastErrorMessage());
		goto cleanup;
	}


	dwStatus = SEC_E_OK;

cleanup:
	if(hServerCertStore)
    {
         CertCloseStore(hServerCertStore, 0);
         hServerCertStore=NULL;
    }
	if(pChainContext)
	{
		CertFreeCertificateChain(pChainContext);
		pChainContext=NULL;
	}
	if(dwStatus!=SEC_E_OK)
	{
		SetLastErrorMessage(IDS_ERR_SSL_CERTIFICATE_VALIDATE_FAILED,dwStatus); 
		DEBUGLOG(LOI_ERROR,(LPTSTR)(LPCTSTR)GetLastErrorMessage());
	}
	return dwStatus==SEC_E_OK?TRUE:FALSE;
}
/*******************************************
get the certs expiry time 
********************************************/
BOOL CLOICert::GetCertExpiryTime(FILETIME &ftExpiryTime)
{
	if(m_pCertContext)
	{
		memcpy(&ftExpiryTime,&m_pCertContext->pCertInfo->NotAfter,sizeof(FILETIME));
		return TRUE;
	}
	return FALSE;
}
/*************************
check whether cert is expired
**************************/
BOOL CLOICert::IsCertExpired()
{
	return m_pCertContext?CertVerifyTimeValidity (NULL,m_pCertContext->pCertInfo )==1:0;
}