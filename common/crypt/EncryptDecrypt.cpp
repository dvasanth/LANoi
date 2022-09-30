#include "stdafx.h"
#include "EncryptDecrypt.h"
#include<strsafe.h>

 const ALG_ID CEncryptDecrypt::AlgorithmID=CALG_RC4;

CEncryptDecrypt::CEncryptDecrypt(void)
{
	m_hKey=m_hProv=NULL;
	ZeroMemory(m_szPassword,sizeof(m_szPassword));
}

CEncryptDecrypt::~CEncryptDecrypt(void)
{
	Close();

}

void CEncryptDecrypt::Close()
{
 
}
BOOL CEncryptDecrypt::Initialize(LPCTSTR pszPassword)
{
	StringCbCopy(m_szPassword,sizeof(m_szPassword),pszPassword);
	return TRUE;
}

//---------------------------------------------------------------
BOOL CEncryptDecrypt::Encrypt(LPBYTE pData, LPDWORD pdwDataSize, 
                        DWORD dwBufferSize, BOOL bFinal)
{
   return TRUE;
}

//---------------------------------------------------------------
BOOL CEncryptDecrypt::Decrypt(LPBYTE pData, LPDWORD pdwDataSize,
                        BOOL bFinal)
{

	return TRUE;
}
BOOL CEncryptDecrypt::EncryptStream(PVOID pData, DWORD dwSizeToEncrypt)
{

	rc4_key rc4key;

	prepare_key((BYTE*)m_szPassword, INT((_tcslen(m_szPassword)*sizeof(TCHAR))), &rc4key);

	rc4((BYTE*)pData,dwSizeToEncrypt, &rc4key);
	return TRUE;
    
}


//---------------------------------------------------------------
BOOL CEncryptDecrypt::DecryptStream(LPBYTE pData,DWORD dwSizeToDecrypt)
{
   return EncryptStream(pData,dwSizeToDecrypt); 
}










///////////////////////////////////////////
/////////RC4--------
//////////////////////////////////////////

static void swap_byte(unsigned char *a, unsigned char *b);

void prepare_key(unsigned char *key_data_ptr, int key_data_len, rc4_key *key)
{
    unsigned char index1;
    unsigned char index2;
    unsigned char* state;
    short counter;     
    
    state = &key->state[0];         
    for(counter = 0; counter < 256; counter++)              
		state[counter] =(CHAR) counter;  
	
    key->x = 0;     
    key->y = 0;     
    index1 = 0;     
    index2 = 0;    
	
    for(counter = 0; counter < 256; counter++)      
    {               
         index2 = (key_data_ptr[index1] + state[counter] +
            index2) % 256;                
         swap_byte(&state[counter], &state[index2]);            

         index1 = (index1 + 1) % key_data_len;  
    }       
}

void rc4(unsigned char *buffer_ptr, UINT buffer_len, rc4_key *key)
{ 
    unsigned char x;
    unsigned char y;
    unsigned char* state;
    unsigned char xorIndex;
    UINT	 counter;              
    
    x = key->x;     
    y = key->y;     
    
    state = &key->state[0];         
    for(counter = 0; counter < buffer_len; counter ++)      
    {               
         x = (x + 1) % 256;                      
         y = (state[x] + y) % 256;               
         swap_byte(&state[x], &state[y]);                        
              
         xorIndex = (state[x] + state[y]) % 256;                 
              
         buffer_ptr[counter] ^= state[xorIndex];         
     }               
     key->x = x;     
     key->y = y;
}

static void swap_byte(unsigned char *a, unsigned char *b)
{
    unsigned char swapByte; 
    
    swapByte = *a; 
    *a = *b;      
    *b = swapByte;
}