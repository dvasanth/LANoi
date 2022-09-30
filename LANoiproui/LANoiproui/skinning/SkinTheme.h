#pragma once

class CSkinTheme
{
public:
	CSkinTheme(void);
	~CSkinTheme(void);
	COLORREF GetWindowBackgroundColor(void);
	COLORREF GetControlBackgroundColor();
	COLORREF GetControlTextColor(BOOL bSelected);
	COLORREF GetControlBackgroundTextColor(BOOL bSelected);
	void	 GetTitleBarColor(COLORREF &crDarkColor,COLORREF &crLightColor);
	UINT	 GetTitleIcon();
private:
	static	const COLORREF	 WINDOW_BACKGROUND_COLOR;		
	static 	const COLORREF	 CONTROL_BACKGROUND_COLOR;		
	static 	const COLORREF	 CONTROL_TEXT_COLOR;		
	static 	const COLORREF	 CONTROL_TEXT_BACKGROUND_COLOR;		
	static 	const COLORREF	 CONTROL_SELECTED_TEXT_COLOR;		
	static 	const COLORREF	 CONTROL_SELECTED_TEXT_BACKGROUND_COLOR;	
	static 	const COLORREF	 TITLEBAR_DARK_COLOR;
	static 	const COLORREF   TITLEBAR_LIGHT_COLOR;
	static  const UINT		 TITLE_ICON_RECOURCE_ID;
	COLORREF			m_clrWndBackground;
	COLORREF			m_clrCtrlBackground;
	COLORREF			m_clrText;
	COLORREF			m_clrTextBackground;
	COLORREF			m_clrSelectedText;
	COLORREF			m_clrSelectedBackground;
	COLORREF			m_clrTitleBarDark;
	COLORREF			m_clrTitleBarLight;
	UINT				m_uiTitleIconID;
};
