#include "StdAfx.h"
#include "SkinTheme.h"
#include "../resource.h"


/***************************************/
const COLORREF CSkinTheme::WINDOW_BACKGROUND_COLOR=RGB(23,33,255);		
const COLORREF CSkinTheme::CONTROL_BACKGROUND_COLOR=RGB(0,255,255);		
const COLORREF CSkinTheme::CONTROL_TEXT_COLOR=RGB(0,67,0);		
const COLORREF CSkinTheme::CONTROL_TEXT_BACKGROUND_COLOR=RGB(255,0,0);		
const COLORREF CSkinTheme::CONTROL_SELECTED_TEXT_COLOR=RGB(0,255,255);		
const COLORREF CSkinTheme::CONTROL_SELECTED_TEXT_BACKGROUND_COLOR=RGB(0,33,255);	
const COLORREF CSkinTheme::TITLEBAR_DARK_COLOR=100;//blue color start value
const COLORREF CSkinTheme::TITLEBAR_LIGHT_COLOR=250;//blue color end value
const UINT     CSkinTheme::TITLE_ICON_RECOURCE_ID=IDR_MAINFRAME;

CSkinTheme::CSkinTheme(void)
{
	m_clrWndBackground=WINDOW_BACKGROUND_COLOR;
	m_clrCtrlBackground=CONTROL_BACKGROUND_COLOR;
	m_clrText=CONTROL_TEXT_COLOR;
	m_clrTextBackground=CONTROL_TEXT_BACKGROUND_COLOR;
	m_clrSelectedText=CONTROL_SELECTED_TEXT_COLOR;
	m_clrSelectedBackground=CONTROL_SELECTED_TEXT_BACKGROUND_COLOR;

	m_clrTitleBarDark=TITLEBAR_DARK_COLOR;
	m_clrTitleBarLight=TITLEBAR_LIGHT_COLOR;

	m_uiTitleIconID=TITLE_ICON_RECOURCE_ID;
}

CSkinTheme::~CSkinTheme(void)
{
}
/**************************
gets client area color of a window
*************************/
COLORREF CSkinTheme::GetWindowBackgroundColor(void)
{
	return m_clrWndBackground;
}
/**************************************
Gets the control client area color
**************************************/
COLORREF CSkinTheme::GetControlBackgroundColor()
{
	return m_clrCtrlBackground;
}
/*****************************************
gets the control text color 
******************************************/
COLORREF CSkinTheme::GetControlTextColor(BOOL bSelected)
{
	return bSelected?m_clrSelectedText:m_clrText;
}

/**********************************
controls text backgroud color
**********************************/
COLORREF CSkinTheme::GetControlBackgroundTextColor(BOOL bSelected)
{
	return bSelected?m_clrSelectedBackground:m_clrTextBackground;
}
/***********************************
title bar fading color
***********************************/
void CSkinTheme::GetTitleBarColor(COLORREF &crDarkColor,COLORREF &crLightColor)
{
	crDarkColor=m_clrTitleBarDark;
	crLightColor=m_clrTitleBarLight;
}
/********************************
this icon is used in the titlebar
*********************************/
UINT CSkinTheme::GetTitleIcon()
{
	return m_uiTitleIconID;
}