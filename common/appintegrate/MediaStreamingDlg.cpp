// MediaStreamingDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MediaStreamingDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMediaStreamingDlg dialog




CMediaStreamingDlg::CMediaStreamingDlg(CRemoteMediaStreaming *pMediaStreaming,CWnd* pParent /*=NULL*/)
	: CDialog(CMediaStreamingDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pMediaStreaming=pMediaStreaming;
}

void CMediaStreamingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CBN_SOURCE_TYPE, m_CbnSourceType);
	DDX_Control(pDX, IDC_CB_CAPTURE_DEVICE_SELECTION, m_CbnVideoCaptureDeviceSelect);
	DDX_Control(pDX, IDC_CB_BITRATE_SELECTION, m_CbnBitrateSelection);
	DDX_Control(pDX, IDC_BT_BROWSE, m_BtnBrowse);
	DDX_Text(pDX, IDC_ED_FILE_LIST ,m_sSelectedFiles);

	DDX_Control(pDX, IDC_CBN_AUDIO_CAPTURE, m_CbnAudioCaptureDevice);
}

BEGIN_MESSAGE_MAP(CMediaStreamingDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BT_BROWSE, &CMediaStreamingDlg::OnBnClickedBtBrowse)
	ON_BN_CLICKED(ID_BTN_STREAM, &CMediaStreamingDlg::OnBnClickedBtnStream)
	ON_CBN_SELCHANGE(IDC_CBN_SOURCE_TYPE, &CMediaStreamingDlg::OnCbnSelchangeCbnSourceType)
END_MESSAGE_MAP()


// CMediaStreamingDlg message handlers

BOOL CMediaStreamingDlg::OnInitDialog()
{
	CDialog::OnInitDialog();



	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	OnLoad();
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMediaStreamingDlg::SetMaxHorizontalComboTxt(CComboBox *pComboBox)
{
	// Find the longest string in the combo box.
	CString      str;
	CSize      sz;
	int      dx = 0;
	TEXTMETRIC   tm;
	CDC*      pDC = pComboBox->GetDC();
	CFont*      pFont = pComboBox->GetFont();

	// Select the listbox font, save the old font
	CFont* pOldFont = pDC->SelectObject(pFont);
	// Get the text metrics for avg char width
	pDC->GetTextMetrics(&tm);

	for (int i = 0; i < pComboBox->GetCount(); i++)
	{
	   pComboBox->GetLBText(i, str);
	   sz = pDC->GetTextExtent(str);

	   // Add the avg width to prevent clipping
	   sz.cx += tm.tmAveCharWidth;
	   
	   if (sz.cx > dx)
		  dx = sz.cx;
	}
	// Select the old font back into the DC
	pDC->SelectObject(pOldFont);
	pComboBox->ReleaseDC(pDC);

	// Set the horizontal extent so every character of all strings can 
	// be scrolled to.
	pComboBox->SetHorizontalExtent(dx);

	// Adjust the width for the vertical scroll bar and the left and right border.
	dx += ::GetSystemMetrics(SM_CXVSCROLL) + 2*::GetSystemMetrics(SM_CXEDGE);

	// Set the width of the list box so that every item is completely visible.
	pComboBox->SetDroppedWidth(dx);
}
void CMediaStreamingDlg::OnSysCommand(UINT nID, LPARAM lParam)
{

}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMediaStreamingDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMediaStreamingDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


// store the default setting & enumerate capture devices & bitrate available
void CMediaStreamingDlg::OnLoad(void)
{
	CStringArray     sArrSourceTypes;
	CStringArray	 sArrVideoCaptureDevices,sArrAudioCaptureDevices;


	//add the type of sources available
	m_pMediaStreaming->GetSourceTypes(sArrSourceTypes);
	for(int iTypes =0;iTypes<=sArrSourceTypes.GetUpperBound();iTypes++)
				m_CbnSourceType.AddString( sArrSourceTypes.GetAt(iTypes));
	m_CbnSourceType.SetCurSel (0 ); 

	//enumerate the type of capture devices available & add it
	m_pMediaStreaming->GetCaptureDevices(sArrVideoCaptureDevices,sArrAudioCaptureDevices);
	for(int iCapDevices =0;iCapDevices<=sArrVideoCaptureDevices.GetUpperBound();iCapDevices++)
			m_CbnVideoCaptureDeviceSelect.AddString( sArrVideoCaptureDevices.GetAt(iCapDevices));
	m_CbnVideoCaptureDeviceSelect.SetCurSel (0); 

	//audio capture devices
	for(int iCapDevices =0;iCapDevices<=sArrAudioCaptureDevices.GetUpperBound();iCapDevices++)
			m_CbnAudioCaptureDevice.AddString( sArrAudioCaptureDevices.GetAt(iCapDevices));
	m_CbnAudioCaptureDevice.SetCurSel (0); 

	//validate the selections
	ValidateSelections();
}

void CMediaStreamingDlg::ValidateSelections()
{
	EnableFileOptions(m_pMediaStreaming->IsSourceLocalFileType(GetSelectedSourceType()));
	UpdateBitrates();
}

void CMediaStreamingDlg::EnableFileOptions(BOOL bEnable)
{
	//enable/disable the file options alone
	GetDlgItem(IDC_ED_FILE_LIST)->EnableWindow(bEnable);
	m_BtnBrowse.EnableWindow(bEnable);

	//disable/enable the capture options
	m_CbnVideoCaptureDeviceSelect.EnableWindow(!bEnable);
	m_CbnAudioCaptureDevice.EnableWindow(!bEnable); 

}
void CMediaStreamingDlg::UpdateBitrates()
{
	CStringArray	 sArrBitrates;
	
	m_CbnBitrateSelection.ResetContent(); 
	//slected bitrate depending on source type
	m_pMediaStreaming->GetBitrates(GetSelectedSourceType(),sArrBitrates);
	for(int iBitRates =0;iBitRates<=sArrBitrates.GetUpperBound();iBitRates++)
			m_CbnBitrateSelection .AddString( sArrBitrates.GetAt(iBitRates));
	m_CbnBitrateSelection.SetCurSel(0); 
	int iIndex=m_CbnBitrateSelection.FindString(0,_T("Windows"));
	if(iIndex!=CB_ERR )
	{
		m_CbnBitrateSelection.SetCurSel(iIndex); 
	}

	SetMaxHorizontalComboTxt(&m_CbnBitrateSelection);

}
CString CMediaStreamingDlg::GetSelectedSourceType()
{
	int iSelectedIndex;

	iSelectedIndex=m_CbnSourceType.GetCurSel();
	if(iSelectedIndex!= LB_ERR)
		m_CbnSourceType.GetLBText(iSelectedIndex, m_sSelectedSourceType);
	return m_sSelectedSourceType;
}
void CMediaStreamingDlg::OnBnClickedBtBrowse()
{	
	CString		sFileExt,sTitle;
	const int	iFileListMaxChars=20*1024;

	// TODO: Add your control notification handler code here
	if(m_pMediaStreaming->GetSourceType( GetSelectedSourceType()) == MEDIA_AUDIO_FILE)
		sFileExt.LoadString(IDS_STREAM_AUDIO_FILE_TYPES);
	else
		sFileExt.LoadString(IDS_STREAM_VIDEO_FILE_TYPES);  
	CFileDialog		fOpenDlg(TRUE, 
							_T("Media Files"),
							_T(""), 
							OFN_ALLOWMULTISELECT,
							(LPCTSTR)sFileExt,
							this);


	// Initializes m_pOFN structure
	sTitle.LoadString(IDS_STREAM_FILE_SELECT_TITLE); 
	fOpenDlg.m_pOFN->lpstrTitle=(LPCTSTR)sTitle;
	fOpenDlg.m_pOFN->lpstrFile =   new TCHAR[iFileListMaxChars];
	ZeroMemory(fOpenDlg.m_pOFN->lpstrFile,iFileListMaxChars);
	fOpenDlg.m_pOFN->nMaxFile = iFileListMaxChars;

	if(fOpenDlg.DoModal()==IDOK)
	{
		POSITION        pos;

		m_sSelectedFileList.RemoveAll(); 
		pos = fOpenDlg.GetStartPosition();
		m_sSelectedFiles=_T("");
        while (pos != NULL)
        {
			 CString sFilepath=fOpenDlg.GetNextPathName(pos);
             m_sSelectedFileList.Add(sFilepath);
			 m_sSelectedFiles+=sFilepath;
	    }
		UpdateData(FALSE); 
	}

	if(fOpenDlg.m_pOFN->lpstrFile)
	{
		delete fOpenDlg.m_pOFN->lpstrFile;
		fOpenDlg.m_pOFN->lpstrFile=NULL;
	}
}

CStringArray& CMediaStreamingDlg::GetSelectedFileList()
{
	return m_sSelectedFileList;
}

CString CMediaStreamingDlg::GetSourceType()
{
	return m_sSelectedSourceType;
}

CString CMediaStreamingDlg::GetVideoCaptureDeviceSelected()
{
	return m_sVideoCaptureDeviceSelected;
}
CString CMediaStreamingDlg::GetAudioCaptureDeviceSelected()
{
	return m_sAudioCaptureDeviceSelected;
}
CString CMediaStreamingDlg::GetBitrateSelected()
{
	return m_sBitrateSelected;
}
void CMediaStreamingDlg::OnBnClickedBtnStream()
{

	//check the files are stored
	if(m_pMediaStreaming->IsSourceLocalFileType(GetSelectedSourceType()))
	{
		if(m_sSelectedFileList.IsEmpty())
		{
			AfxMessageBox( IDS_STREAM_NO_FILE_SELECTED,MB_OK  |MB_ICONSTOP|MB_TOPMOST );
			return;
		}
	}
	else
	{
		if(m_CbnVideoCaptureDeviceSelect.GetCurSel()== CB_ERR ||
			m_CbnAudioCaptureDevice.GetCurSel()==CB_ERR
			)
		{
			AfxMessageBox( IDS_STREAM_NO_CAPTURE_SELECTED,MB_OK  |MB_ICONSTOP|MB_TOPMOST );
			return;
		}
		//get the video capture device
		m_CbnVideoCaptureDeviceSelect.GetLBText(m_CbnVideoCaptureDeviceSelect.GetCurSel(),m_sVideoCaptureDeviceSelected);
		//get the audio capture device
		m_CbnAudioCaptureDevice.GetLBText(m_CbnAudioCaptureDevice.GetCurSel(),m_sAudioCaptureDeviceSelected);
	
	}

	if(m_CbnBitrateSelection.GetCurSel()== CB_ERR)
	{
		AfxMessageBox( IDS_STREAM_SELECT_BITRATE,MB_OK  |MB_ICONSTOP|MB_TOPMOST );
		
		return;
	}
	//get the bitrate
	m_CbnBitrateSelection.GetLBText( m_CbnBitrateSelection.GetCurSel(),m_sBitrateSelected);

	CDialog::OnOK(); 
}

void CMediaStreamingDlg::OnCbnSelchangeCbnSourceType()
{
	// TODO: Add your control notification handler code here
	ValidateSelections();
}
