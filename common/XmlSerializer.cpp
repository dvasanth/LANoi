#include "StdAfx.h"


#include "XmlSerializer.h"
#include "Base64.h"
#include <comutil.h>
#include <strsafe.h>
#ifndef PROGID_XMLDOMDocument
   #define PROGID_XMLDOMDocument L"MSXML2.DOMDocument"
#endif


/////////////////////////////////////////////////////////////////
// CXmlSerializer
CXmlSerializer::CXmlSerializer()
{
	CoInitialize(NULL); 
}
CXmlSerializer::~CXmlSerializer()
{
	
   Close();
  // CoUninitialize();
}

BOOL CXmlSerializer::Create(LPCTSTR pstrTitle)
{
   ATLASSERT(!::IsBadStringPtr(pstrTitle,-1));

   Close();

   // Create XML document
   HRESULT Hr = m_spDoc.CoCreateInstance(PROGID_XMLDOMDocument);
   ATLASSERT(SUCCEEDED(Hr));
   if( FAILED(Hr) ) return FALSE;

   // Create preprocessing stuff (for foreign characters)
   CComPtr<IXMLDOMProcessingInstruction> spPI;
   Hr = m_spDoc->createProcessingInstruction(CComBSTR(L"xml"), CComBSTR(L"version=\"1.0\" encoding=\"UTF-8\""), &spPI);
   ATLASSERT(SUCCEEDED(Hr));
   if( FAILED(Hr) ) return FALSE;
   CComPtr<IXMLDOMNode> spOut;
   if( FAILED( m_spDoc->appendChild(spPI, &spOut) ) ) return FALSE;



   m_spNode = m_spDoc;
   return WriteGroupBegin(pstrTitle);
}

BOOL CXmlSerializer::Open(LPCTSTR pstrTitle, LPCTSTR pstrFilename)
{
   ATLASSERT(!::IsBadStringPtr(pstrTitle,-1));
   ATLASSERT(!::IsBadStringPtr(pstrFilename,-1));

   // Create XML document
   CComPtr<IXMLDOMDocument> spDoc;
   HRESULT Hr = spDoc.CoCreateInstance(PROGID_XMLDOMDocument);
   ATLASSERT(SUCCEEDED(Hr));
   if( FAILED(Hr) ) return FALSE;

   // Load from file
   VARIANT_BOOL vbSuccess;
   spDoc->put_async(VARIANT_FALSE);
   if( FAILED( spDoc->load(CComVariant(pstrFilename), &vbSuccess) ) ) return FALSE;
   if( vbSuccess == VARIANT_FALSE ) return FALSE;

   return Open(spDoc, pstrTitle, pstrFilename);
}

BOOL CXmlSerializer::Open(IXMLDOMDocument* pDoc, LPCTSTR pstrTitle, LPCTSTR pstrFilename)
{
   ATLASSERT(pDoc);
   ATLASSERT(!::IsBadStringPtr(pstrTitle,-1));
   ATLASSERT(!::IsBadStringPtr(pstrFilename,-1));

   if( pDoc == NULL ) return FALSE;

   // Close previous archive
   Close();

   // Assign XML Document reference
   m_spDoc = pDoc;

   // Prepare scan...
   // Get its attributes
   CComPtr<IXMLDOMElement> spRoot;
   if( FAILED( m_spDoc->get_documentElement(&spRoot) ) ) return FALSE;
   ATLASSERT(spRoot);
   if( spRoot == NULL ) return FALSE;

   m_spNode = spRoot;
   if( m_spNode == NULL ) return FALSE;

   // Make sure it's the right document type (checks title)...
   CComBSTR bstr;
   m_spNode->get_nodeName(&bstr);
   if( bstr.Length() == 0 ) return FALSE;
   if( wcscmp(bstr, pstrTitle) != 0 ) return FALSE;

   // Reference attributes now
   m_spNode->get_attributes(&m_spAttribs);

   m_iLevel = 0;
   m_sFilename = pstrFilename;
   return TRUE;
}
BOOL CXmlSerializer::LoadXML(CString sXmlString,LPCTSTR pstrTitle)
{
   ATLASSERT(!::IsBadStringPtr(pstrTitle,-1));

   // Close previous archive
   Close();
   // Create XML document
   CComPtr<IXMLDOMDocument> spDoc;
   HRESULT Hr = spDoc.CoCreateInstance(PROGID_XMLDOMDocument);
   ATLASSERT(SUCCEEDED(Hr));
   if( FAILED(Hr) ) return FALSE;

   // Load from file
   VARIANT_BOOL vbSuccess;
   spDoc->put_async(VARIANT_FALSE);
   if( FAILED( spDoc->loadXML(CComBSTR(sXmlString), &vbSuccess) ) ) return FALSE;
   if( vbSuccess == VARIANT_FALSE ) return FALSE;

   if( spDoc == NULL ) return FALSE;
   // Close previous archive
   Close();

   // Assign XML Document reference
   m_spDoc = spDoc;

   // Prepare scan...
   // Get its attributes
   CComPtr<IXMLDOMElement> spRoot;
   if( FAILED( m_spDoc->get_documentElement(&spRoot) ) ) return FALSE;
   ATLASSERT(spRoot);
   if( spRoot == NULL ) return FALSE;

   m_spNode = spRoot;
   if( m_spNode == NULL ) return FALSE;

   // Make sure it's the right document type (checks title)...
   CComBSTR bstr;
   m_spNode->get_nodeName(&bstr);
   if( bstr.Length() == 0 ) return FALSE;
   if( wcscmp(bstr, pstrTitle) != 0 ) return FALSE;

   // Reference attributes now
   m_spNode->get_attributes(&m_spAttribs);

   m_iLevel = 0;
   return TRUE;
}

BOOL CXmlSerializer::Save(LPCTSTR pstrFilename)
{
	m_sFilename=pstrFilename;
   ATLASSERT(!m_sFilename.IsEmpty());
   if( FAILED( m_spDoc->save(CComVariant(m_sFilename)) ) ) {
      ::SetLastError(ERROR_WRITE_FAULT);
      return FALSE;
   }
   return TRUE;
}
CString CXmlSerializer::GetXml()
{
	 BSTR				bstr = NULL;
	 CString			sXmlString;

	 m_spDoc->get_xml(&bstr);
	 if (bstr) 
	 {
		 sXmlString=CString(bstr);
		 SysFreeString(bstr); 
	 }
	return sXmlString;
}
void CXmlSerializer::Close()
{
   m_spDoc.Release();
   m_spNode.Release();
   m_spAttribs.Release();
}

// ISerialiable

BOOL CXmlSerializer::ReadGroupBegin(LPCTSTR pstrName)
{
   ATLASSERT(!::IsBadStringPtr(pstrName,-1));
   if( m_spNode == NULL ) return FALSE;
   // Check to see if we're iterating a repeating group.
   // The node-name should already be on the context-stack then.
   CComBSTR bstrName = pstrName;
   CComPtr<IXMLDOMNode> spNext;
   CComPtr<IXMLDOMNodeList> spList;
   int i;
   for( i = m_aContexts.GetSize() - 1; i >= 0 ; --i ) {
      CONTEXT& Context = m_aContexts[i];
      if( Context.iLevel == m_iLevel && Context.bstrName == bstrName ) {
         Context.spList->get_item(++Context.iIndex, &spNext);
         if( spNext == NULL ) return FALSE;
         break;
      }
   }
   if( i == -1 ) {
      // Get the collection of children of this type
      m_spNode->selectNodes(bstrName, &spList);
      if( spList == NULL ) return FALSE;
      spList->get_item(0, &spNext);
      if( spNext == NULL ) return FALSE;
      CONTEXT Context;
      Context.bstrName = bstrName;
      Context.spList = spList;
      Context.iIndex = 0;
      Context.iLevel = m_iLevel;
      m_aContexts.Add(Context);
   }
   // Pick the next node
   m_spNode = spNext;
   // Get its attributes
   m_spAttribs.Release();
   m_spNode->get_attributes(&m_spAttribs);
   // We're going into next level
   m_iLevel++;
   return TRUE;
}

BOOL CXmlSerializer::ReadGroupEnd()
{
   m_spAttribs.Release();
   if( m_spNode == NULL ) return FALSE;
   // Clear the stack
   if( m_iLevel == 0 ) {
      m_aContexts.RemoveAll();
      return TRUE;
   }
   CComPtr<IXMLDOMNode> spParent;
   m_spNode->get_parentNode(&spParent);
   ATLASSERT(spParent);
   if( spParent == NULL ) return FALSE;
   m_spNode = spParent;
   // Pop contexts from higher level off stack...
   for( int i = 0; i < m_aContexts.GetSize(); i++ ) {
      if( m_aContexts[i].iLevel == m_iLevel ) {
         m_aContexts.RemoveAt(i);
         i = -1;
         continue;
      }
   }
   m_iLevel--;
   return TRUE;
}

BOOL CXmlSerializer::ReadItem(LPCTSTR pstrName)
{
   ATLASSERT(!::IsBadStringPtr(pstrName,-1));
   m_spAttribs.Release();
   if( m_spNode == NULL ) return FALSE;
   CComPtr<IXMLDOMNode> spNode;
   HRESULT Hr = m_spNode->selectSingleNode(CComBSTR(pstrName), &spNode);
   if( Hr != S_OK ) return FALSE;
   // Get its attributes
   m_spAttribs.Release();
   spNode->get_attributes(&m_spAttribs);
   return TRUE;
}
/*
BOOL CXmlSerializer::Read(LPCTSTR pstrName, LPTSTR pstrValue, UINT cchMax)
{
   ATLASSERT(!::IsBadStringPtr(pstrName,-1));
   ATLASSERT(!::IsBadWritePtr(pstrValue,cchMax*sizeof(WCHAR)));
   ATLASSERT(cchMax>0);
   ATLASSERT(m_spAttribs);
   USES_CONVERSION;
   *pstrValue = '\0';
   if( m_spAttribs == NULL ) return FALSE;
   // Get the attribute
   CComPtr<IXMLDOMNode> spNode;
   HRESULT Hr = m_spAttribs->getNamedItem(T2OLE(const_cast<LPTSTR>(pstrName)), &spNode);
   if( Hr != S_OK ) return FALSE;
   // Read the value
   CComBSTR bstr;
   if( FAILED( spNode->get_text(&bstr) ) ) return FALSE;
   StringCbCopyN(pstrValue,cchMax, OLE2CT(bstr),cchMax );
   return TRUE;
}*/
BOOL CXmlSerializer::Read(LPCTSTR pstrName, CString& sValue)
{
/*   const int		MAX_TCHAR_READ=60*1024;
   LPTSTR pszValue=new TCHAR[MAX_TCHAR_READ];

   if( !Read(pstrName, pszValue,MAX_TCHAR_READ*sizeof(TCHAR) ) )
   {
      sValue = _T("");
	  delete pszValue;
      return FALSE;
   }
   sValue = pszValue;
   delete pszValue;
   return TRUE;
   */
	ATLASSERT(!::IsBadStringPtr(pstrName,-1));
   //ATLASSERT(!::IsBadWritePtr(pstrValue,cchMax*sizeof(WCHAR)));
  // ATLASSERT(cchMax>0);
   ATLASSERT(m_spAttribs);
   USES_CONVERSION;
//   *pstrValue = '\0';
   if( m_spAttribs == NULL ) return FALSE;
   // Get the attribute
   CComPtr<IXMLDOMNode> spNode;
   HRESULT Hr = m_spAttribs->getNamedItem(T2OLE(const_cast<LPTSTR>(pstrName)), &spNode);
   if( Hr != S_OK ) return FALSE;
   // Read the value
   CComBSTR bstr;
   if( FAILED( spNode->get_text(&bstr) ) ) return FALSE;
  _bstr_t bstrIntermediate(bstr);
   sValue.Format(_T("%s"),(LPCTSTR)bstrIntermediate);
  // StringCbCopyN(pstrValue,cchMax, OLE2CT(bstr),cchMax );
   return TRUE;
}
/******************************************
reads base64 encoded string & converts to bytes
******************************************/
BOOL CXmlSerializer::Read(LPCTSTR pstrName,PBYTE &pBytes,DWORD &dwByteLen)
{
	CString    sValue;


	//read the base64 TCHAR string & decode it to convert it to bytes
	if(Read(pstrName,sValue))
	{
		//decode the base64 string
		CBase64		DecodeToBytes;
		DWORD		dwDecodedLen=0;

		DecodeToBytes.Decode(sValue,dwDecodedLen);
		if(dwDecodedLen )
		{
			pBytes = new BYTE[dwDecodedLen];
			memcpy(pBytes,DecodeToBytes.DecodedMessage(),dwDecodedLen  );
			dwByteLen=dwDecodedLen;
			return TRUE;
		}
	}
	
	return FALSE;
}
/**************************************
deallocation routine for freeing the buffer alllocated in read bytes
****************************************/
void CXmlSerializer::FreeBytes(BYTE *pAllocatedBytes)
{
	if(pAllocatedBytes)
	{
		delete pAllocatedBytes;
		pAllocatedBytes=NULL;
	}
}



BOOL CXmlSerializer::Read(LPCTSTR pstrName, SYSTEMTIME& stValue)
{
   ::ZeroMemory(&stValue, sizeof(stValue));
   CString sValue;
   if( !Read(pstrName, sValue) ) return FALSE;
   // TODO: Write it in actual XML date format!
   CComVariant v = (LPCTSTR)sValue;
   if( FAILED( v.ChangeType(VT_DATE) ) ) return FALSE;
   if( ::VariantTimeToSystemTime(v.date, &stValue) == 0 ) return FALSE;
   return TRUE;
}

BOOL CXmlSerializer::Read(LPCTSTR pstrName, DWORD& dwValue)
{
   TCHAR szValue[20] = { 0 };
   CString sValue;
   if( !Read(pstrName, sValue ))
   {
      dwValue = 0;
      return FALSE;
   }
   dwValue = (DWORD)_ttol((LPCTSTR)sValue);
   return TRUE;
}

BOOL CXmlSerializer::Read(LPCTSTR pstrName, BOOL& bValue)
{
   CString sValue;
   if( !Read(pstrName, sValue) ) {
      bValue = FALSE;
      return FALSE;
   }
   bValue = (_tcsicmp((LPCTSTR)sValue, _T("true")) == 0);
   return TRUE;
}

BOOL CXmlSerializer::WriteGroupBegin(LPCTSTR pstrName)
{
   ATLASSERT(!::IsBadStringPtr(pstrName,-1));
   m_spAttribs.Release();
   CComPtr<IXMLDOMElement> spElement;
   if( FAILED( m_spDoc->createElement(CComBSTR(pstrName), &spElement) ) ) return FALSE;
   CComPtr<IXMLDOMNode> spNode;
   if( FAILED( m_spNode->appendChild(spElement, &spNode) ) ) return FALSE;
   m_spNode = spNode;
   // Get its attributes
   m_spNode->get_attributes(&m_spAttribs);
   return TRUE;
}

BOOL CXmlSerializer::WriteGroupEnd()
{
   CComPtr<IXMLDOMNode> spNode;
   m_spNode->get_parentNode(&spNode);
   m_spNode = spNode;
   //
   m_spAttribs.Release();
   return TRUE;
}

BOOL CXmlSerializer::WriteItem(LPCTSTR pstrName)
{
   ATLASSERT(!::IsBadStringPtr(pstrName,-1));
   m_spAttribs.Release();
   CComPtr<IXMLDOMElement> spElement;
   if( FAILED( m_spDoc->createElement(CComBSTR(pstrName), &spElement) ) ) return FALSE;
   CComPtr<IXMLDOMNode> spNode;
   if( FAILED( m_spNode->appendChild(spElement, &spNode) ) ) return FALSE;
   // Get its attributes
   spNode->get_attributes(&m_spAttribs);
   return TRUE;
}
/************************
Allow to write bytes just base64 & convert it to string & use standatrd routine to write it
*************************/
BOOL CXmlSerializer::Write(LPCTSTR pstrName, LPBYTE pBytes,DWORD dwByteLen)
{
	CBase64  StringEncode;
	//just convert the bytes to string & write it
	StringEncode.Encode(pBytes,dwByteLen);
	return Write(pstrName,(LPCTSTR)StringEncode.EncodedMessage());
}

BOOL CXmlSerializer::Write(LPCTSTR pstrName, LPCTSTR pstrValue)
{
   ATLASSERT(!::IsBadStringPtr(pstrName,-1));
   ATLASSERT(!::IsBadStringPtr(pstrValue,-1));
   ATLASSERT(m_spAttribs);
   if( m_spAttribs == NULL ) return FALSE;
   CComPtr<IXMLDOMAttribute> spAttrib;
   if( FAILED( m_spDoc->createAttribute(CComBSTR(pstrName), &spAttrib) ) ) return FALSE;
   CComBSTR bstr = pstrValue;
   if( FAILED( spAttrib->put_text(bstr) ) ) return FALSE;
   CComPtr<IXMLDOMNode> spNode;
   if( FAILED( m_spAttribs->setNamedItem(spAttrib, &spNode) ) ) return FALSE;
   return TRUE;
}

BOOL CXmlSerializer::Write(LPCTSTR pstrName, SYSTEMTIME stValue)
{
   DATE dDate = 0.0;
   if( ::SystemTimeToVariantTime(&stValue, &dDate) == 0 ) return FALSE;
   CComVariant v = dDate;
   v.vt = VT_DATE;
   if( FAILED( v.ChangeType(VT_BSTR) ) ) return FALSE;
   USES_CONVERSION;
   return Write(pstrName, OLE2CT(v.bstrVal));
}

BOOL CXmlSerializer::Write(LPCTSTR pstrName, DWORD dwValue)
{
   TCHAR szValue[20];
   StringCbPrintf(szValue,sizeof(szValue) ,_T("%ld"), dwValue);
   return Write(pstrName, szValue);
}

BOOL CXmlSerializer::Write(LPCTSTR pstrName, BOOL bValue)
{
   return Write(pstrName, bValue ? _T("true") : _T("false"));
}

BOOL CXmlSerializer::WriteExternal(LPCTSTR /*pstrName*/)
{
   // Clients use this to persist the contents of the object,
   // such as the text of a text editor. Usually the provider
   // just ignores this (like this one does), but it could be
   // usefull for advanced clipboard operations, etc.
   return TRUE;
}

BOOL CXmlSerializer::Delete(LPCTSTR pstrName)
{
   ATLASSERT(!::IsBadStringPtr(pstrName,-1));
   CComPtr<IXMLDOMNode> spNode;
   HRESULT Hr = m_spNode->selectSingleNode(CComBSTR(pstrName), &spNode);
   if( Hr != S_OK ) return FALSE;
   if( spNode == NULL ) return FALSE;
   CComPtr<IXMLDOMNode> spOutput;
   if( FAILED( m_spNode->removeChild(spNode, &spOutput) ) ) return FALSE;
   return TRUE;
}
