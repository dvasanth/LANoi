#pragma once
#include <atlbase.h>
#include <comdef.h>
#include "C:\WMSDK\WMEncSDK9\include\wmencode.h"
#include "../error.h"

enum MEDIA_SOURCE_TYPE
    {	
		MEDIA_AUDIO_FILE	= 0,
		MEDIA_VIDEO_FILE,
		MEDIA_CAPTURE_DEVICE
    } 	;
typedef struct __SOURCE_TYPE_INFO{
	UINT				iResID;
	MEDIA_SOURCE_TYPE   SourceType;
}SOURCE_TYPE_INFO,*LPSOURCE_TYPE_INFO;
class CRemoteMediaStreaming:public CError
{

public:
	CRemoteMediaStreaming(void);
	~CRemoteMediaStreaming(void);
	CString				GetServerPort();
	BOOL				GetBroadCastParams();
	void				GetSourceTypes(CStringArray &sArrSourceTypes);
	MEDIA_SOURCE_TYPE	GetSourceType(CString sSourceType);
	BOOL				IsSourceLocalFileType(CString sSelectedSource);
	void				GetCaptureDevices(CStringArray &sArrCaptureDevices,CStringArray &sArrAudioDevices);
	void				GetBitrates(CString sSourceType,CStringArray &sArrBitrates);
	void				StopTheBroadcastServer();
	void				PlayStream(CString sNickname,CString sPort);
	BOOL				StartTheBroadcast(CString sStreamingPort);
private:
	//type of sources available 
	static const	SOURCE_TYPE_INFO  SOURCE_TYPES[];
	static const	TCHAR			  VIDEO_PROFILE_IGNORE[][MAX_PATH];
	IWMEncoder2*					m_pEncoder;
	IWMEncSourceGroupCollection*	m_pSrcGrpColl;
	IWMEncProfile*					m_pBitrateProfile;
	IWMEncBroadcast*				m_pEncBroadcast;

	HRESULT							InitialiseEncoder();
	void							CloseEncoder();	
	void							EnumerateDevices(IWMEncPluginInfo*	pPlugInfo,CStringArray &sArrVideoCaptureDevices,CStringArray &sArrAudioCaptureDevices);
	BOOL							AssociateFilesSource(CStringArray &sArrLocalFiles);
	BOOL							AssociateCaptureStreamSource(CString sVideoCaptureDevice,CString sAudioCaptureDevice);

	IWMEncProfile*					GetBitrateProfile(CString sBitrate);

	BOOL							IsVideoFile(CString sFileName);
	void							SetDisplay(CString sAuthor,CString sTitle);
	BOOL							IsAudioOnlyProfile(IWMEncProfile* pPro);
	BOOL							IsVideoBitrateIgnore(CString sBitrateProfile);
	CList<void* ,void*>				m_ComPtrList;						 
};
