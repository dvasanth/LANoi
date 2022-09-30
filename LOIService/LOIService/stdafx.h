// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once
#pragma warning(disable : 4995)

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <ShellApi.h>
#include <shlwapi.h>
#include <winsock2.h>
#include <in6addr.h>
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

#include <atlbase.h>
#include <atlstr.h>
#include <ATLComTime.h>
#include <ws2tcpip.h>
 #pragma warning(disable : 4995)
#include "resource.h"
// TODO: reference additional headers your program requires here
#include "ServiceLogging.h"
#include "LOIApp.h"
#include "Util.h"
