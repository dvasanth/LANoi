#pragma once

class CSessionTypes
{
public:
	CSessionTypes(void);
	~CSessionTypes(void);

	BOOL						AddSessionType(CString sSessionTypeName, CString sSessionValue);
	CString						GetSessionValue(CString sSessionName);
	CString						operator[](int iElement);
	INT							GetLength();
	void						Reload();
private:
	CMapStringToString			 m_SessionTypesMap;//key is session type name & value is port setting
	static const UINT			HASH_TABLE_SIZE;
	static const CString		SESSION_FILE_NAME;
	static const CString		SESSION_FOLDER_PATH;
	CString						m_sAppDataFilepath;
	CStringArray				m_sSessionNames;
	void						LoadPersistentSessionTypes(void);
	void						SavePersistentSessionTypes(void);

};
