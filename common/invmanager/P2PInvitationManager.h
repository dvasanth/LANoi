#pragma once
#include <wincrypt.h>
#include "P2PInvitation.h"
#include "error.h"
class CP2PInvitationManager:public CError
{
public:
	CP2PInvitationManager(void);
	~CP2PInvitationManager(void);
private:
	static const  TCHAR			INVITATION_STORE_FOLDER[];
	static const  TCHAR			CACHED_CERT_FOLDER_PATH[];
	static const  TCHAR			CA_FILEPATH[];

	PCCERT_CONTEXT				m_pCACertContext;
};
