#include "StdAfx.h"
#include "LOISessionStringFormat.h"

const TCHAR		CLOISessionStringFormat::NAME_XML[]=TEXT("LOISessionStringFormat"); 
const TCHAR		CLOISessionStringFormat::GROUP_PORT_RANGE[]=_T("PortRange");
const TCHAR		CLOISessionStringFormat::ATTR_PORT_START[]=_T("PortStart");
const TCHAR		CLOISessionStringFormat::ATTR_PORT_STOP[]=_T("PortStop");
const TCHAR		CLOISessionStringFormat::ATTR_PORT_SERVICE[]=_T("PortService");
const TCHAR		CLOISessionStringFormat::GROUP_APPLICATION_NAME[]=_T("AppName");
const TCHAR		CLOISessionStringFormat::ATTR_APPLICATION_NAME[]=_T("AppNamAttr");
const TCHAR		CLOISessionStringFormat::GROUP_FORCED_ADDRES_BIND[]=_T("ForceAddressBInd");
const TCHAR		CLOISessionStringFormat::ATTR_FORCED_ADDRESS[]=_T("ForceIP");
const TCHAR		CLOISessionStringFormat::ATTR_FORCED_PORT[]=_T("ForcePort");
const TCHAR		CLOISessionStringFormat::ATTR_DIVERTED_VIRTUAL[]=_T("DivertedIP");

CLOISessionStringFormat::CLOISessionStringFormat(void)
{
	m_XmlSerialize.Create(NAME_XML);
}


CLOISessionStringFormat::~CLOISessionStringFormat(void)
{
}
/**************************************
Load the xml string for parsing the values
***************************************/
CLOISessionStringFormat::CLOISessionStringFormat(CString sXmlString)
{
	m_sXmlString=sXmlString;
	m_XmlSerialize.LoadXML(sXmlString,NAME_XML);
}
/**********************************
get the xml string from the format
***********************************/
CString CLOISessionStringFormat::GetXMLString()
{
	return m_XmlSerialize.GetXml();
}

/****************************************
Add port information
*****************************************/
BOOL CLOISessionStringFormat::AddPortRange(USHORT usPortStart,USHORT usPortStop,BOOL bService)
{
	m_XmlSerialize.WriteGroupBegin(GROUP_PORT_RANGE);
	m_XmlSerialize.Write(ATTR_PORT_START,(DWORD)usPortStart);
	m_XmlSerialize.Write(ATTR_PORT_STOP,(DWORD)usPortStop);
	m_XmlSerialize.Write(ATTR_PORT_SERVICE,bService);

	m_XmlSerialize.WriteGroupEnd();

	return TRUE;
}
/********************************************
Extract the port  information from loaded xml string
 **********************************************/
BOOL CLOISessionStringFormat::GetNextPortValue(USHORT &usPortStart,USHORT &usPortStop,BOOL &bService)
{
	if(m_XmlSerialize.ReadGroupBegin(GROUP_PORT_RANGE))
	{
		DWORD dwPortStart,dwPortStop;
		m_XmlSerialize.Read(ATTR_PORT_START,dwPortStart);
		m_XmlSerialize.Read(ATTR_PORT_STOP,dwPortStop);
		usPortStart=(USHORT)dwPortStart;
		usPortStop=(USHORT)dwPortStop;
		m_XmlSerialize.Read(ATTR_PORT_SERVICE,bService);
		m_XmlSerialize.ReadGroupEnd();
		return TRUE;
	}
	return FALSE;
}
/***************************************
Add the application name
****************************************/
BOOL CLOISessionStringFormat::AddApplication(CString sAppName)
{
	m_XmlSerialize.WriteGroupBegin(GROUP_APPLICATION_NAME);
	m_XmlSerialize.Write(ATTR_APPLICATION_NAME,sAppName);
	m_XmlSerialize.WriteGroupEnd();

	return TRUE;
}
/******************************
Extract all application name
*******************************/
BOOL CLOISessionStringFormat::GetNextApplicationName(CString &sApplicationPath)
{
	if(m_XmlSerialize.ReadGroupBegin(GROUP_APPLICATION_NAME))
	{
		m_XmlSerialize.Read(ATTR_APPLICATION_NAME,sApplicationPath);
		m_XmlSerialize.ReadGroupEnd();
		return TRUE;
	}
	return FALSE;
}
/**********************************************
dynamic notification about forced binded ipaddress & port
Also divertion virtual ipaddress, translate to teredo address
**********************************************/
BOOL CLOISessionStringFormat::AddDynamicBoundedAddress(DWORD dwBoundAddress,USHORT usBoundPort,DWORD dwVirtualAddress)
{
	m_XmlSerialize.WriteGroupBegin(GROUP_FORCED_ADDRES_BIND);
	m_XmlSerialize.Write(ATTR_FORCED_ADDRESS,dwBoundAddress);
	m_XmlSerialize.Write(ATTR_FORCED_PORT,(DWORD)usBoundPort);
	m_XmlSerialize.Write(ATTR_DIVERTED_VIRTUAL,dwVirtualAddress);
	m_XmlSerialize.WriteGroupEnd();
	return TRUE;
}
/**********************************************
gets the dynamic bound address information
***********************************************/
BOOL CLOISessionStringFormat::GetNextDynamicBoundValues(DWORD &dwBoundAddress,USHORT &usBoundPort,DWORD &dwVirtualAddress)
{
	if(m_XmlSerialize.ReadGroupBegin(GROUP_FORCED_ADDRES_BIND))
	{
		DWORD dwBoundPort;
		m_XmlSerialize.Read(ATTR_FORCED_ADDRESS,dwBoundAddress);
		m_XmlSerialize.Read(ATTR_FORCED_PORT,(DWORD)dwBoundPort);
		usBoundPort=(USHORT)dwBoundPort;
		m_XmlSerialize.Read(ATTR_DIVERTED_VIRTUAL,dwVirtualAddress);
		m_XmlSerialize.ReadGroupEnd();
		return TRUE;
	}
	return FALSE;
}
