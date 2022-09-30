// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (C) 2004  Microsoft Corporation.  All Rights Reserved.
//
// Module Name: overlap.cpp
//
// Description:
//
//    This module is responsible for handling the overlapped IO 
//    passed to us by the upper layer. If the LSP is on NT, then
//    an IO completion port (IOCP) is created and all overlapped
//    IO initiated by the upper layer passes through our IOCP.
//    If this is Win9x then we create a worker thread and execute
//    the overlapped IO via asynchronous procedure calls (APC).
//
#include "lspdef.h"

#include <stdio.h>
#include <stdlib.h>

#pragma warning(disable:4127)       // disable "conditional expression is constant" warning
#pragma warning(disable:4702)       // disable "unreacheable code" warning





