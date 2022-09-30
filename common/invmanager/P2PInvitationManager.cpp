#include "StdAfx.h"
#include "P2PInvitationManager.h"

//////////
///consts--
/////////
///all are relative application data paths
const TCHAR CP2PInvitationManager::INVITATION_STORE_FOLDER[]=_T("\\invitation\\");
const TCHAR	CP2PInvitationManager::CACHED_CERT_FOLDER_PATH[]=_T("\\createdcerts\\");
const TCHAR CP2PInvitationManager::CA_FILEPATH[]=_T("\\invitation\\CA.cert");

CP2PInvitationManager::CP2PInvitationManager(void)
{
	TCHAR		szPath[MAX_PATH]={0};
	CString		sApplicationDataFolder;

	SHGetFolderPath(NULL,CSIDL_COMMON_APPDATA ,NULL,SHGFP_TYPE_CURRENT,szPath);
	sApplicationDataFolder+=szPath;
	sApplicationDataFolder+=	_T("\\crowsoft\\LOI\\");;
	SHCreateDirectoryEx(NULL,sApplicationDataFolder,NULL);

	//create the invitation root
	::CreateDirectory(NULL,(LPCTSTR)(sApplicationDataFolder+INVITATION_STORE_FOLDER));
	//create the folders store
	::CreateDirectory(NULL,(LPCTSTR) (sApplicationDataFolder+CACHED_CERT_FOLDER_PATH));

	m_pCACertContext=NULL;
}

CP2PInvitationManager::~CP2PInvitationManager(void)
{
}

/****************************************
this creates the self-signed certificated required to created
invitattion certs
****************************************/
BOOL CP2PInvitationManager::Initialise()
{
	//create the CA cert if it doesn't exists
	if(PathFileExists( (LPCTSTR)(theApp.GetApplicationDataPath()+CA_FILEPATH))
	{
		//serialise the existing CA cert & return the cert context
	}

	//create new CA cert

	//serialise the CA cert to the file

	return TRUE;
}

/**************************************
returns the certificate with which invitation
certs are created
*************************************/
PCCERT_CONTEXT CP2PInvitationManager::GetCACert()
{
	return m_pCACertContext;
}
/*********************************************
this functions creates the invitation object with the specified
expiry time
*********************************************/
CP2PInvitation* CP2PInvitationManager::CreateInvitation(FILETIME ftNotValidAfterTime)
{


	

	//create invitation object  

	//prepare the invitation with the CA cert & expiry time to create the client cert

}


/****************************************************
Add a invitation to the store so session parameters can be retrieved later
****************************************************/
void CP2PInvitationManager::AddToStore(CP2PInvitation* pInvitation)
{
	
	//get the unique ID of the invitation 

	//ask save the invitation with the subject name

}
/**************************************************
Get the stored invitation previously from store
*************************************************/
CP2PInvitation*  CP2PInvitationManager::GetInvitationFromStore(PCCERT_CONTEXT pClientCertContext)
{

	//form the file path with invitation unique ID

	//create the invitation object

	//load the invitation from the file path


}