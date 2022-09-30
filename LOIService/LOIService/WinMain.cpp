#include "StdAfx.h"
#include "WinMain.h"
#include "LOIApp.h"

// LOIService.cpp : Defines the entry point for the application.
//



CLOIApp     theApp;

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{


	theApp.InitInstance(hInstance,lpCmdLine);
	theApp.ExitInstance(); 
	return (int) 0;
}


