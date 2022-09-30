#pragma once
#include "../xmlserializer.h"

class CLOISessionStringFormat
{
public:
	CLOISessionStringFormat(void);
	~CLOISessionStringFormat(void);
	CLOISessionStringFormat(CString sXmlString);
	CString				GetXMLString();
	BOOL				AddPortRange(USHORT usPortStart,USHORT usPortStop,BOOL bService);
	BOOL				GetNextPortValue(USHORT &usPortStart,USHORT &usPortStop,BOOL &bService);
	BOOL				AddApplication(CString sAppName);
	BOOL				GetNextApplicationName(CString &sApplicationPath);
	BOOL				AddDynamicBoundedAddress(DWORD dwBoundAddress,USHORT usBoundPort,DWORD dwVirtualAddress);
	BOOL				GetNextDynamicBoundValues(DWORD &dwBoundAddress,USHORT &usBoundPort,DWORD &dwVirtualAddress);
private:
	static	const TCHAR		NAME_XML[]; 
	static  const TCHAR		GROUP_PORT_RANGE[];
	static  const TCHAR		ATTR_PORT_START[];
	static  const TCHAR		ATTR_PORT_STOP[];
	static  const TCHAR		GROUP_APPLICATION_NAME[];
	static  const TCHAR		ATTR_APPLICATION_NAME[];
	static  const TCHAR		ATTR_PORT_SERVICE[];
	static  const TCHAR		GROUP_FORCED_ADDRES_BIND[];
	static  const TCHAR		ATTR_FORCED_ADDRESS[];
	static  const TCHAR		ATTR_FORCED_PORT[];
	static  const TCHAR		ATTR_DIVERTED_VIRTUAL[];

	CString				m_sXmlString;
	CXmlSerializer		m_XmlSerialize;


};
