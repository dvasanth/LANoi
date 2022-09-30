#include "StdAfx.h"
#include "EncryptFile.h"
#include <strsafe.h>
#include "cmnresource.h"

TCHAR const CEncryptFile::FILE_ENCRYPT_HEADER[]=_T("Encrypt.header.1.0");


CEncryptFile::CEncryptFile(void)
{
	m_hFile=NULL;
}

CEncryptFile::~CEncryptFile(void)
{
	Close();
}

/***********************************************
creates the file & writes the header
************************************************/
BOOL CEncryptFile::Create(LPCTSTR lpszFilepath,LPCTSTR lpszPassword)
{
	TCHAR		szHeader[MAX_PATH]={0};

	//create the file
	m_hFile = CreateFile(lpszFilepath,                // name of the write
                       GENERIC_WRITE,          // open for writing
                       0,                      // do not share
                       NULL,                   // default security
                       CREATE_ALWAYS,          // overwrite existing
                       FILE_ATTRIBUTE_NORMAL,  // normal file
                       NULL);                  // no attr. template

    if (m_hFile == INVALID_HANDLE_VALUE) 
    { 
		DEBUGLOG(LOI_ERROR,_T(" CEncryptFile::Create not create file (error %d)"), GetLastError());
		m_hFile=NULL;
		SetLastErrorMessage(IDS_FAILED_TO_CREATE_FILE, GetLastError());
        return FALSE;
    }

	m_EncryptDecrypt.Initialize(lpszPassword);
	
	StringCbCopy(szHeader,sizeof(szHeader),FILE_ENCRYPT_HEADER);
	//write the encrypted file header
	Write((PBYTE)szHeader,(_tcslen(szHeader)*sizeof(TCHAR))+sizeof(TCHAR));

	return TRUE;
}
/*******************************************
Opens the encrypted file , verifies the header 
*******************************************/
BOOL CEncryptFile::Open(LPCTSTR lpszFilepath,LPCTSTR lpszPassword)
{
	DWORD		dwBytesRead=0;
	TCHAR		szHeader[MAX_PATH]={0};

	//open the file
	m_hFile = CreateFile(lpszFilepath,                // name of the write
                       GENERIC_READ,          // open for writing
                       FILE_SHARE_READ,                      // do not share
                       NULL,                   // default security
                       OPEN_EXISTING,          // overwrite existing
                       FILE_ATTRIBUTE_NORMAL,  // normal file
                       NULL);                  // no attr. template

    if (m_hFile == INVALID_HANDLE_VALUE) 
    { 
		DEBUGLOG(LOI_ERROR,_T(" CEncryptFile::Open not create file (error %d)\n"), GetLastError());
		SetLastErrorMessage(IDS_FAILED_OPEN_FILE,GetLastError()); 
		m_hFile=NULL;
        return FALSE;
    }
	m_EncryptDecrypt.Initialize(lpszPassword);

	//decrypts the header & verify it
	dwBytesRead=sizeof(FILE_ENCRYPT_HEADER);
	Read ((PBYTE)szHeader,dwBytesRead);
	if(dwBytesRead)
	{
		if(_tcsicmp(szHeader,FILE_ENCRYPT_HEADER)==0)
		{
			return TRUE;
		}
		SetLastErrorMessage(IDS_WRONG_FILE_PASSWORD);
	}

	return FALSE;
}

/*******************************
Close & free the resource
*******************************/
void CEncryptFile::Close()
{
	//close the handle
	if(m_hFile)
	{
		CloseHandle(m_hFile); 
		m_hFile=NULL;
	}
	m_EncryptDecrypt.Close(); 
}

/*******************************
writes the file in encrypted format
*********************************/
BOOL CEncryptFile::Write(PBYTE pStreamData,DWORD dwWriteLen)
{
	DWORD  dwBytesWritten=0;

	//encrypt the data
	m_EncryptDecrypt.EncryptStream(pStreamData,dwWriteLen);
	//write the encrypted file header
	WriteFile(m_hFile,pStreamData,dwWriteLen,&dwBytesWritten,NULL);

	return dwBytesWritten;
}
/********************************
read file in encrypted format
********************************/
BOOL CEncryptFile::Read(PBYTE pReadStream,DWORD &dwReadInOutLen)
{
	DWORD		dwBytesRead=0;

	ReadFile (m_hFile,pReadStream,dwReadInOutLen,&dwBytesRead,NULL);
	m_EncryptDecrypt.DecryptStream(pReadStream,dwReadInOutLen); 
	dwReadInOutLen=dwBytesRead;
	return dwBytesRead;
}
/****************************************
Gets size of the file without header
****************************************/
DWORD CEncryptFile::GetSize()
{
	return GetFileSize(m_hFile,NULL)-sizeof(FILE_ENCRYPT_HEADER); 
}