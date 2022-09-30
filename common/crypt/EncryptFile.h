#pragma once
#include "Error.h"
#include "EncryptDecrypt.h"

class CEncryptFile:public CError
{
public:
	CEncryptFile(void);
	~CEncryptFile(void);
	BOOL			Create(LPCTSTR lpszFilepath,LPCTSTR lpszPassword);
	BOOL			Open(LPCTSTR lpszFilepath,LPCTSTR lpszPassword);
	void			Close();
	BOOL			Write(PBYTE pStreamData,DWORD dwWriteLen);
	BOOL			Read(PBYTE pReadStream,DWORD &dwReadInOutLen);
	DWORD			GetSize();
private:
	static const TCHAR	FILE_ENCRYPT_HEADER[];
	HANDLE				m_hFile; 
	CEncryptDecrypt		m_EncryptDecrypt;

};
