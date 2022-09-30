#include "StdAfx.h"
#include "SessionTypes.h"

const UINT			CSessionTypes::HASH_TABLE_SIZE=997;
const CString		CSessionTypes::SESSION_FILE_NAME=_T("\\SessionTypes.dat");
const CString       CSessionTypes::SESSION_FOLDER_PATH=_T("\\crowsoft\\LANoiPro\\sessions");

CSessionTypes::CSessionTypes(void)
{
	m_SessionTypesMap.InitHashTable(HASH_TABLE_SIZE);
	TCHAR		szPath[MAX_PATH]={0};

	SHGetFolderPath(NULL,CSIDL_COMMON_APPDATA ,NULL,SHGFP_TYPE_CURRENT,szPath);
	m_sAppDataFilepath+=szPath;
	m_sAppDataFilepath+=SESSION_FOLDER_PATH;
	SHCreateDirectoryEx(NULL,(LPCTSTR)m_sAppDataFilepath,NULL);


	m_sAppDataFilepath+=SESSION_FILE_NAME;
	LoadPersistentSessionTypes();
}

CSessionTypes::~CSessionTypes(void)
{
	SavePersistentSessionTypes();
}

BOOL CSessionTypes::AddSessionType(CString sSessionTypeName, CString sSessionValue)
{
	CString  sValue;

	if(m_SessionTypesMap.Lookup(sSessionTypeName,sValue))
	{
		//with that name already there is session
		return FALSE;
	}

	m_SessionTypesMap.SetAt(sSessionTypeName,sSessionValue);  
	return TRUE;
}

CString CSessionTypes::GetSessionValue(CString sSessionName)
{
	CString  sSessionValue;
	m_SessionTypesMap.Lookup(sSessionName,sSessionValue);
	return sSessionValue;
}

void CSessionTypes::LoadPersistentSessionTypes(void)
{
	CFile				DataFile; 
	CFileException		exception;
	
	if(DataFile.Open(m_sAppDataFilepath,CFile::modeRead,&exception))
	{

		CArchive			ar(&DataFile, CArchive::load);
		m_SessionTypesMap.Serialize(ar);

		m_sSessionNames.RemoveAll(); 
		//load the string array of names
		for (POSITION pos = m_SessionTypesMap.GetStartPosition(); pos; )
		{
			CString  sSessionName,sSessionValue;
			m_SessionTypesMap.GetNextAssoc(pos, sSessionName, sSessionValue);
			m_sSessionNames.Add (sSessionName);
		}
	}
}

void CSessionTypes::SavePersistentSessionTypes(void)
{
	CFile				DataFile; 
	CFileException		exception;	

	if(DataFile.Open (m_sAppDataFilepath,CFile::modeReadWrite|CFile::modeCreate,&exception))
	{
		CArchive			ar(&DataFile, CArchive::store);
	
		m_SessionTypesMap.Serialize(ar);
	}
}

INT CSessionTypes::GetLength()
{
	return (INT) m_SessionTypesMap.GetSize();
}

CString CSessionTypes::operator[](int iElement)
{
	return m_sSessionNames[iElement];
}

void CSessionTypes::Reload()
{
	LoadPersistentSessionTypes();
}