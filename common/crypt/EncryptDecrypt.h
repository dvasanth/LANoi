#pragma once
#include <wincrypt.h>
typedef struct rc4_key
{      
    unsigned char state[256];       
    unsigned char x;        
    unsigned char y;
} rc4_key;

void prepare_key(unsigned char *key_data_ptr,int key_data_len,rc4_key *key);
void rc4(unsigned char *buffer_ptr,UINT buffer_len,rc4_key * key);
void make_hash(const char * data, BYTE ** hash, long * len);
void write_file(const char * filename, unsigned char * data, long length);

class CEncryptDecrypt
{
public:
	CEncryptDecrypt(void);
	~CEncryptDecrypt(void);
    BOOL Encrypt(LPBYTE pData, LPDWORD pdwDataSize, 
                 DWORD dwBufferSize, BOOL bFinal);
    BOOL Decrypt(LPBYTE pData, LPDWORD dwDataSize,
                 BOOL bFinal);
	BOOL  Initialize(LPCTSTR pszPassword);
	BOOL EncryptStream(PVOID pData, DWORD dwSizeToEncrypt);
	BOOL DecryptStream(LPBYTE pData,DWORD dwSizeToDecrypt);
	void Close();
private:
    HCRYPTPROV m_hProv;
    HCRYPTKEY  m_hKey;
	TCHAR		m_szPassword[MAX_PATH];

	//conts
	static const ALG_ID   AlgorithmID;
};
