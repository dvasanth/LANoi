// MediaStreamingDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "..\resource.h"
#include "RemoteMediaStreaming.h"
// CMediaStreamingDlg dialog
class CMediaStreamingDlg : public CDialog
{
// Construction
public:
	CMediaStreamingDlg(CRemoteMediaStreaming *pMediaStreaming,CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_MEDIASTREAMING_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CStringArray& GetSelectedFileList();
	CString		 GetSourceType();
	CString		 GetVideoCaptureDeviceSelected();
	CString		 GetAudioCaptureDeviceSelected();
	CString		 GetBitrateSelected();
private:
	//streaming class to get stream parameters
	CRemoteMediaStreaming      *m_pMediaStreaming;
	//selected items 
	CString						m_sSelectedSourceType;
	// Browse button
	CButton						m_BtnBrowse;
	// source type selection combo
	CComboBox m_CbnSourceType;
	// contains the list of files selected by user
	CStringArray	 m_sSelectedFileList;
	//contains video/audio capture device selected
	CString			 m_sVideoCaptureDeviceSelected;
	CString          m_sAudioCaptureDeviceSelected;
	//contains bitrate selected
	CString			 m_sBitrateSelected;
	//contains source type
	CString			 m_sSourceType;
	// contains the capture device selected by user
	CComboBox m_CbnVideoCaptureDeviceSelect;
	// user selected bitrate is stored
	CComboBox m_CbnBitrateSelection;
	//edit window text
	CString		m_sSelectedFiles;
	// store the default setting & enumerate capture devices & bitrate available
	void OnLoad(void);
	//expand combo item
	void	SetMaxHorizontalComboTxt(CComboBox *pComboBox);
	//perform validataions
	void ValidateSelections();
	//enable the file or capture selection
	void EnableFileOptions(BOOL bEnable);
	//validates the bitrates for the selections
	void	UpdateBitrates();
	//on browse
	afx_msg void OnBnClickedBtBrowse();
	afx_msg void OnBnClickedBtnStream();
	// Combo for audio capture
	CComboBox m_CbnAudioCaptureDevice;

	afx_msg void OnCbnSelchangeCbnSourceType();
	CString		GetSelectedSourceType();
};
