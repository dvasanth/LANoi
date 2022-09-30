#include "StdAfx.h"
#include "RemoteMediaStreaming.h"
#include "MediaStreamingDlg.h"
#include "../resource.h"
#include "RegOperations.h"
const SOURCE_TYPE_INFO CRemoteMediaStreaming::SOURCE_TYPES[]={
								{IDS_STREAM_SOURCE_CAPTURE,MEDIA_CAPTURE_DEVICE},
								{IDS_STRING_SOURCE_VIDEO_FILE,MEDIA_VIDEO_FILE},
								{IDS_STREAM_SOURCE_AUDIO_FILE,MEDIA_AUDIO_FILE}
								};
const	TCHAR	CRemoteMediaStreaming::VIDEO_PROFILE_IGNORE[][MAX_PATH]={
											{_T("vbr for broadband")},
											{_T("screen video")},
											{_T("dial-up modem")}
										};
CRemoteMediaStreaming::CRemoteMediaStreaming(void):m_pEncoder(NULL)
{
	m_pSrcGrpColl=NULL;
	m_pBitrateProfile=NULL;
	m_pEncBroadcast=NULL;
}

CRemoteMediaStreaming::~CRemoteMediaStreaming(void)
{
	CloseEncoder();
}


CString CRemoteMediaStreaming::GetServerPort()
{
	CRegOperations    PortConfig;
	CString			  sPort;

	sPort.Format(_T("%d"),PortConfig.GetMediaServerListeningPort());

	return sPort;
}


HRESULT CRemoteMediaStreaming::InitialiseEncoder()
{
	HRESULT			hResult=E_FAIL;

	// Initialize the COM library and retrieve a pointer
    // to an IWMEncoder interface.
    hResult = CoInitialize(NULL);
    if ( SUCCEEDED( hResult ) )
    {
        hResult = CoCreateInstance(CLSID_WMEncoder,
									NULL,
									CLSCTX_INPROC_SERVER,
									IID_IWMEncoder2,
									(void**) &m_pEncoder);
    }

	return  hResult ;
}

void CRemoteMediaStreaming::CloseEncoder()
{
	
	if ( m_pEncoder )
    {
		m_pEncoder->Stop();
    }


	if(m_pSrcGrpColl)
	{
		m_pSrcGrpColl->Release();
		m_pSrcGrpColl=NULL;
	}
	if(m_pBitrateProfile)
	{
		m_pBitrateProfile->Release();
		m_pBitrateProfile=NULL;
	}
	if(m_pEncBroadcast)
	{
		m_pEncBroadcast->Release();
		m_pEncBroadcast=NULL;
	}

	//remove all interfaces in list
	POSITION pos = m_ComPtrList.GetHeadPosition();
	while(pos != NULL)
	{
		void * pPtr=m_ComPtrList.GetNext(pos);
		IUnknown* pUnknown  = reinterpret_cast<IUnknown*> (pPtr);
		if(pUnknown)pUnknown->Release(); 
	}
	m_ComPtrList.RemoveAll();

	if ( m_pEncoder )
    {
        m_pEncoder->Release();
        m_pEncoder = NULL;
    }
}

BOOL CRemoteMediaStreaming::GetBroadCastParams()
{
	CMediaStreamingDlg        StreamDlg(this);
	HRESULT					  hResult=E_FAIL;

	hResult=InitialiseEncoder();
	if(FAILED(hResult))
	{
		//show error message
		if(hResult==REGDB_E_CLASSNOTREG)
		{
			if(AfxMessageBox(IDS_MEDIA_ENCODER_INSTALL, MB_YESNO|MB_ICONSTOP)==IDYES)
			{
				//lauch the link
				SHELLEXECUTEINFO 	 shInfo={0};
				CString				 sLink;

				sLink.LoadString(IDS_MEDIA_ENCODER_LINK_DOWNLOAD); 
				shInfo.cbSize =sizeof(shInfo);
				shInfo.lpVerb = _T("open");
				shInfo.lpFile = _T("iexplore.exe");
				shInfo.lpParameters= (LPCTSTR)sLink;
				shInfo.nShow = SW_SHOW;
				ShellExecuteEx(&shInfo);
			}
		}
		return FALSE;
	}

	///Get the streaming settings from ui
	StreamDlg.DoModal();

	m_pBitrateProfile = GetBitrateProfile(StreamDlg.GetBitrateSelected ());
	if(!m_pBitrateProfile) return FALSE;


	//launch broadcast server with this parameter
	if(IsSourceLocalFileType(StreamDlg.GetSourceType()))
	{
		//associate the encoder with  local files
		AssociateFilesSource(StreamDlg.GetSelectedFileList());
	}
	else
	{
		//associate with the capture device
		AssociateCaptureStreamSource(StreamDlg.GetVideoCaptureDeviceSelected(),StreamDlg.GetAudioCaptureDeviceSelected());
	}

	
	return TRUE;
}

BOOL CRemoteMediaStreaming::StartTheBroadcast(CString sStreamingPort)
{
	HRESULT							hResult=S_OK;
	USHORT							usStreamingPort=0;

	usStreamingPort=(USHORT)_ttol((LPCTSTR)sStreamingPort);

	// Create an IWMEncBroadcast object and specify a port and a protocol.
	hResult = m_pEncoder->get_Broadcast(&m_pEncBroadcast);
	
    if ( SUCCEEDED( hResult ) )
    {
        hResult = m_pEncBroadcast->put_PortNumber(WMENC_PROTOCOL_HTTP, usStreamingPort);
    }

    // Start the encoding process.
    if ( SUCCEEDED( hResult ) )
    {
        hResult = m_pEncoder->PrepareToEncode(VARIANT_TRUE);
    }

    if ( SUCCEEDED( hResult ) )
    {
        hResult = m_pEncoder->Start();
	}

	
	if(FAILED(hResult))
	{
		if(m_pEncBroadcast)
		{
			m_pEncBroadcast->Release();
			m_pEncBroadcast=NULL;
		}
		//show error message
		SetLastErrorMessage(IDS_MEDIA_ENCODER_FAILED,hResult);
		AfxMessageBox(GetLastErrorMessage(),MB_OK|MB_ICONSTOP); 
	}
	return SUCCEEDED(hResult);


}

void CRemoteMediaStreaming::StopTheBroadcastServer()
{
	CloseEncoder();
}

void CRemoteMediaStreaming::PlayStream(CString sNickname,CString sPort)
{
	SHELLEXECUTEINFO 	 shInfo={0};
	CString				 sCmdLine;

	sCmdLine.Format(_T("http://%s:%s"),(LPCTSTR)sNickname,(LPCTSTR)sPort);  
	shInfo.cbSize =sizeof(shInfo);
	shInfo.lpVerb = _T("open");
	shInfo.lpFile = _T("wmplayer.exe");
	shInfo.lpParameters= (LPCTSTR)sCmdLine;
	shInfo.nShow = SW_SHOW;
	ShellExecuteEx(&shInfo);
}
void CRemoteMediaStreaming::SetDisplay(CString sAuthor,CString sTitle)
{
	HRESULT							hResult=S_OK;
	IWMEncDisplayInfo*				pDispInfo=NULL;

	hResult = m_pEncoder->get_DisplayInfo(&pDispInfo);

	// Set the data members of the IWMEncDisplayInfo interface.
	if ( SUCCEEDED( hResult ) )
	{
		hResult = pDispInfo->put_Author(CComBSTR(sAuthor));
	}
	if ( SUCCEEDED( hResult ) )
	{
		hResult = pDispInfo->put_Description(CComBSTR(sTitle));
	}

	if(pDispInfo)
	{
		pDispInfo->Release();
		pDispInfo=NULL;
	}
}
void CRemoteMediaStreaming::GetSourceTypes(CStringArray &sArrSourceTypes)
{

	for(int iCount =0;iCount<sizeof(SOURCE_TYPES)/sizeof(SOURCE_TYPES[0]);iCount++)
	{
		CString sRes;
		sRes.LoadString( SOURCE_TYPES[iCount].iResID );
		sArrSourceTypes.Add(sRes);
	}

}

BOOL CRemoteMediaStreaming::IsSourceLocalFileType(CString sSelectedSource)
{
	CString			sAudioRes,sVideoRes;

	sAudioRes.LoadString(IDS_STREAM_SOURCE_AUDIO_FILE );
	sVideoRes.LoadString(IDS_STRING_SOURCE_VIDEO_FILE); 
	return (sAudioRes==sSelectedSource)||
			(sVideoRes==sSelectedSource);
}
BOOL CRemoteMediaStreaming::IsVideoFile(CString sFileName)
{

	sFileName.MakeLower(); 
	return !(
				(sFileName.Find(_T(".wmv")) == -1) &&
				(sFileName.Find(_T(".avi")) == -1) &&
				(sFileName.Find(_T(".mpg")) == -1) &&
				(sFileName.Find(_T(".vob")) == -1) &&
				(sFileName.Find(_T(".mp4")) == -1) &&
				(sFileName.Find(_T(".flv")) == -1) &&
				(sFileName.Find(_T(".dat")) == -1) &&
				(sFileName.Find(_T(".mpg")) == -1) 
			);
}
BOOL CRemoteMediaStreaming::AssociateFilesSource(CStringArray &sArrLocalFiles)
{
	HRESULT							hResult=E_FAIL;
	BOOL							bRet=FALSE;

	hResult = m_pEncoder->get_SourceGroupCollection(&m_pSrcGrpColl);
	if(FAILED(hResult))goto cleanup;

	for(int iFileCount =0;iFileCount<=sArrLocalFiles.GetUpperBound();iFileCount++)
	{
		CString					sGroupName;
		IWMEncSourceGroup *		pSrcGrp=NULL;
		IWMEncSourceGroup2 *	pSrcGrp2=NULL;

		sGroupName.Format(_T("SG_%d"),iFileCount);
		hResult=m_pSrcGrpColl->Add(CComBSTR(sGroupName),(IWMEncSourceGroup**)&pSrcGrp);
		// Retrieve a pointer to an IWMEncSourceGroup2 interface.
		if ( SUCCEEDED( hResult ) )
		{
			hResult = pSrcGrp->QueryInterface(IID_IWMEncSourceGroup2, (void**)&pSrcGrp2);
			m_ComPtrList.AddHead( pSrcGrp);
		}

		if ( SUCCEEDED( hResult ) )
		{
			IWMEncSource*		pSrcAud=NULL;
			IWMEncSource*		pSrcVid=NULL;

			m_ComPtrList.AddHead( pSrcGrp2);
			// Create a source object for each type of content in the source group.
			// Create a source object for each type of content in the source group.
			hResult = pSrcGrp2->AddSource(WMENC_AUDIO, &pSrcAud);
			if ( SUCCEEDED( hResult ) )
			{
				m_ComPtrList.AddHead( pSrcAud);
				pSrcAud->SetInput(CComBSTR(sArrLocalFiles[iFileCount]));
			}


			if(IsVideoFile(sArrLocalFiles[iFileCount]))
			{
				hResult = pSrcGrp2->AddSource(WMENC_VIDEO, &pSrcVid);
				// Specify the input for the sources in the first source group.
				if ( SUCCEEDED( hResult ) )
				{
					m_ComPtrList.AddHead( pSrcVid);
					pSrcVid->SetInput(CComBSTR(sArrLocalFiles[iFileCount]));
				}
			}
	
			//set the bitrate
			pSrcGrp2->put_Profile(CComVariant(m_pBitrateProfile));
			
			//loopback to first on complete
			sGroupName.Format( _T("SG_%d"),iFileCount==sArrLocalFiles.GetUpperBound()?0:iFileCount+1);
			pSrcGrp2->SetAutoRollover(-1, CComBSTR(sGroupName));
			bRet=TRUE;
		}
	}
cleanup:
	return bRet;
}

BOOL CRemoteMediaStreaming::AssociateCaptureStreamSource(CString sVideoCaptureDevice,CString sAudioCaptureDevice)
{
	HRESULT							hResult=E_FAIL;
	BOOL							bRet=FALSE;
	IWMEncSource*					pSrcAud=NULL;
	IWMEncSource*					pSrcVid=NULL;
	IWMEncSourceGroup*				pSrcGrp=NULL;

	hResult = m_pEncoder->get_SourceGroupCollection(&m_pSrcGrpColl);
	if(FAILED(hResult))goto cleanup;

    hResult = m_pSrcGrpColl->Add(CComBSTR("SG_1"), &pSrcGrp);
	if(FAILED(hResult))goto cleanup;

	m_ComPtrList.AddHead( pSrcGrp);
	hResult = pSrcGrp->AddSource(WMENC_AUDIO, &pSrcAud);
	if ( SUCCEEDED( hResult ))
	{
		m_ComPtrList.AddHead( pSrcAud);
		pSrcAud->SetInput(CComBSTR(CString(_T("Device://"))+sAudioCaptureDevice));
	}

	hResult = pSrcGrp->AddSource(WMENC_VIDEO, &pSrcVid);
	if ( SUCCEEDED( hResult ))
	{
		m_ComPtrList.AddHead( pSrcVid);
		hResult=pSrcVid->SetInput(CComBSTR(CString(_T("Device://"))+sVideoCaptureDevice));
		if(SUCCEEDED( hResult ))
		{
			bRet=TRUE;
		}
	}
	//set the bitrate
	pSrcGrp->put_Profile(CComVariant(m_pBitrateProfile));

cleanup:
	return bRet;

}
void CRemoteMediaStreaming::GetCaptureDevices(CStringArray &sArrCaptureDevices,CStringArray &sArrAudioDevices)
{
	HRESULT							hResult=E_FAIL;
	CComBSTR						sScheme;
	IWMEncPluginInfo*				pPlugInfo=NULL;
	IWMEncSourcePluginInfoManager*  pSrcPlugMgr=NULL;
	LONG							lCapCount;

	// Retrieve source and device plug-in info manager objects from WMEncoder.
    hResult = m_pEncoder->get_SourcePluginInfoManager(&pSrcPlugMgr);
	if(FAILED(hResult))return;

    // Loop through all the audio and video devices on the system.
    hResult = pSrcPlugMgr->get_Count(&lCapCount);
    if(FAILED(hResult))goto cleanup;

	for (int iDevices = 0; iDevices < lCapCount; iDevices++)
	{
		// Set the IWMEncPluginInfo object to the current plug-in.
		hResult = pSrcPlugMgr->Item(iDevices, &pPlugInfo);
		// Find the plug-ins that support resources.
		if ( SUCCEEDED( hResult ) )
		{
			hResult = pPlugInfo->get_SchemeType(&sScheme);
			if (_wcsicmp(sScheme, CComBSTR("DEVICE"))==0)
    			EnumerateDevices(pPlugInfo,sArrCaptureDevices,sArrAudioDevices);
			pPlugInfo->Release();
			pPlugInfo=NULL;
		}
	}
cleanup:
	if(pSrcPlugMgr)
	{
		pSrcPlugMgr->Release();
		pSrcPlugMgr=NULL;
	}
	
}

void CRemoteMediaStreaming::EnumerateDevices(IWMEncPluginInfo*	pPlugInfo,CStringArray &sArrVideoCaptureDevices,CStringArray &sArrAudioCaptureDevices)
{
    CComBSTR				sName;
    VARIANT_BOOL			bResources;
	HRESULT					hResult=E_FAIL;
	LONG					lResrcCount=0;
	WMENC_SOURCE_TYPE 		type;
	// Find the devices.
	hResult = pPlugInfo->get_Resources(&bResources);
	if (SUCCEEDED( hResult ) && bResources==VARIANT_TRUE)
	{
	   // Loop through the resources in the current plug-in.
	   hResult = pPlugInfo->get_Count(&lResrcCount);
	   if(FAILED(hResult))return;
	   hResult= pPlugInfo->get_MediaType( &type);

	   for (int iRes = 0; iRes < lResrcCount; iRes++)
		{
			// Display the name of the plug-in.
			hResult = pPlugInfo->Item(iRes, &sName);
			if (SUCCEEDED( hResult )) 
			{
				if(type == WMENC_VIDEO)
					sArrVideoCaptureDevices.Add (sName);
				if(type == WMENC_AUDIO)
					sArrAudioCaptureDevices.Add(sName);
			}
		}//for
	}//bResources
}

void CRemoteMediaStreaming::GetBitrates(CString sSourceType,CStringArray &sArrBitrates)
{
	HRESULT						hResult=E_FAIL;
	CComBSTR					bstrName("");
	IWMEncProfileCollection*	pProColl=NULL;
    IWMEncProfile*				pPro=NULL;
	LONG						lProFileCount=0;
	MEDIA_SOURCE_TYPE			SourceType;


	// Loop through the profile collection and retrieve a specific
	// profile.
	hResult = m_pEncoder->get_ProfileCollection(&pProColl);
	if(FAILED(hResult))goto cleanup;

	hResult = pProColl->get_Count(&lProFileCount);
	if(FAILED(hResult))goto cleanup;

	SourceType=GetSourceType(sSourceType);

	for (int iProfiles=0; iProfiles<lProFileCount; iProfiles++)
	{
		hResult = pProColl->Item(iProfiles, &pPro);
		if ( SUCCEEDED( hResult ) )
		{
			BOOL		bAudioOnlyProfile=IsAudioOnlyProfile(pPro);	

			hResult = pPro->get_Name(&bstrName);
			if ( SUCCEEDED( hResult ) )
			{
				if(SourceType==MEDIA_AUDIO_FILE)
				{
					//add only pure audio bitrate profiles
					if(bAudioOnlyProfile)
							sArrBitrates.Add(bstrName);
				}
				else
				{
					if(!bAudioOnlyProfile && !IsVideoBitrateIgnore(CString(bstrName)))
											sArrBitrates.Add(bstrName);
				}
			}
			pPro->Release();
			pPro=NULL;
		}
	}

cleanup:
	if(pProColl)
	{
		pProColl->Release();
		pProColl=NULL;
	}
}
BOOL CRemoteMediaStreaming::IsVideoBitrateIgnore(CString sBitrateProfile)
{
	

	for(int iList=0;iList<sizeof(VIDEO_PROFILE_IGNORE)/sizeof(VIDEO_PROFILE_IGNORE[0]);iList++)
	{
		sBitrateProfile.MakeLower(); 
		if(_tcsstr((LPCTSTR)sBitrateProfile,VIDEO_PROFILE_IGNORE[iList]))
		{
			//found the string ignore it
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CRemoteMediaStreaming::IsAudioOnlyProfile(IWMEncProfile* pPro)
{
	SHORT						usVideoSupported=0;
	HRESULT						hResult=E_FAIL;

	hResult=pPro->get_MediaCount(WMENC_VIDEO,&usVideoSupported);
	if ( SUCCEEDED( hResult ) && usVideoSupported )
		return FALSE;

	return TRUE;
}

MEDIA_SOURCE_TYPE CRemoteMediaStreaming::GetSourceType(CString sSourceType)
{
	for(int iCount =0;iCount<sizeof(SOURCE_TYPES)/sizeof(SOURCE_TYPES[0]);iCount++)
	{
		CString sRes;
		sRes.LoadString( SOURCE_TYPES[iCount].iResID );
		if(sRes==sSourceType)
		{
			return SOURCE_TYPES[iCount].SourceType;
		}
	}
	return MEDIA_VIDEO_FILE;//default
}

IWMEncProfile*  CRemoteMediaStreaming::GetBitrateProfile(CString sBitrate)
{
	HRESULT						hResult=E_FAIL;
	CComBSTR					bstrName("");
	IWMEncProfileCollection*	pProColl=NULL;
    IWMEncProfile*				pPro=NULL;
	LONG						lProFileCount=0;

	// Loop through the profile collection and retrieve a specific
	// profile.
	hResult = m_pEncoder->get_ProfileCollection(&pProColl);
	if(FAILED(hResult))goto cleanup;

	hResult = pProColl->get_Count(&lProFileCount);
	if(FAILED(hResult))goto cleanup;


	for (int iProfiles=0; iProfiles<lProFileCount; iProfiles++)
	{
		hResult = pProColl->Item(iProfiles, &pPro);
		if ( SUCCEEDED( hResult ) )
		{
			hResult = pPro->get_Name(&bstrName);
			
			if ( SUCCEEDED( hResult ) && 
				sBitrate == bstrName
				)
				break;//found the bitrate
			pPro->Release();
			pPro=NULL;
		}
	}

cleanup:
	if(pProColl)
	{
		pProColl->Release();
		pProColl=NULL;
	}
	return pPro;

}