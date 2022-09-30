

/* this ALWAYS GENERATED file contains the proxy stub code */


 /* File created by MIDL compiler version 7.00.0500 */
/* at Sun Apr 19 13:53:47 2009
 */
/* Compiler settings for .\appintegrate\msra.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext, robust
    error checks: stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#if !defined(_M_IA64) && !defined(_M_AMD64)


#pragma warning( disable: 4049 )  /* more than 64k source lines */
#if _MSC_VER >= 1200
#pragma warning(push)
#endif

#pragma warning( disable: 4211 )  /* redefine extern to static */
#pragma warning( disable: 4232 )  /* dllimport identity*/
#pragma warning( disable: 4024 )  /* array to pointer mapping*/
#pragma warning( disable: 4152 )  /* function/data pointer conversion in expression */
#pragma warning( disable: 4100 ) /* unreferenced arguments in x86 call */

#pragma optimize("", off ) 

#define USE_STUBLESS_PROXY


/* verify that the <rpcproxy.h> version is high enough to compile this file*/
#ifndef __REDQ_RPCPROXY_H_VERSION__
#define __REQUIRED_RPCPROXY_H_VERSION__ 475
#endif


#include "rpcproxy.h"
#ifndef __RPCPROXY_H_VERSION__
#error this stub requires an updated version of <rpcproxy.h>
#endif // __RPCPROXY_H_VERSION__


#include "msra_h.h"

#define TYPE_FORMAT_STRING_SIZE   1123                              
#define PROC_FORMAT_STRING_SIZE   931                               
#define EXPR_FORMAT_STRING_SIZE   1                                 
#define TRANSMIT_AS_TABLE_SIZE    0            
#define WIRE_MARSHAL_TABLE_SIZE   3            

typedef struct _msra_MIDL_TYPE_FORMAT_STRING
    {
    short          Pad;
    unsigned char  Format[ TYPE_FORMAT_STRING_SIZE ];
    } msra_MIDL_TYPE_FORMAT_STRING;

typedef struct _msra_MIDL_PROC_FORMAT_STRING
    {
    short          Pad;
    unsigned char  Format[ PROC_FORMAT_STRING_SIZE ];
    } msra_MIDL_PROC_FORMAT_STRING;

typedef struct _msra_MIDL_EXPR_FORMAT_STRING
    {
    long          Pad;
    unsigned char  Format[ EXPR_FORMAT_STRING_SIZE ];
    } msra_MIDL_EXPR_FORMAT_STRING;


static RPC_SYNTAX_IDENTIFIER  _RpcTransferSyntax = 
{{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}};


extern const msra_MIDL_TYPE_FORMAT_STRING msra__MIDL_TypeFormatString;
extern const msra_MIDL_PROC_FORMAT_STRING msra__MIDL_ProcFormatString;
extern const msra_MIDL_EXPR_FORMAT_STRING msra__MIDL_ExprFormatString;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO IPCHCollection_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO IPCHCollection_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO IPCHService_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO IPCHService_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ISAFSession_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ISAFSession_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO IRASrv_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO IRASrv_ProxyInfo;


extern const USER_MARSHAL_ROUTINE_QUADRUPLE UserMarshalRoutines[ WIRE_MARSHAL_TABLE_SIZE ];

#if !defined(__RPC_WIN32__)
#error  Invalid build platform for this stub.
#endif

#if !(TARGET_IS_NT50_OR_LATER)
#error You need a Windows 2000 or later to run this stub because it uses these features:
#error   /robust command line switch.
#error However, your C/C++ compilation flags indicate you intend to run this app on earlier systems.
#error This app will fail with the RPC_X_WRONG_STUB_VERSION error.
#endif


static const msra_MIDL_PROC_FORMAT_STRING msra__MIDL_ProcFormatString =
    {
        0,
        {

	/* Procedure get__NewEnum */

			0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/*  2 */	NdrFcLong( 0x0 ),	/* 0 */
/*  6 */	NdrFcShort( 0x7 ),	/* 7 */
/*  8 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 10 */	NdrFcShort( 0x0 ),	/* 0 */
/* 12 */	NdrFcShort( 0x8 ),	/* 8 */
/* 14 */	0x45,		/* Oi2 Flags:  srv must size, has return, has ext, */
			0x2,		/* 2 */
/* 16 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 18 */	NdrFcShort( 0x0 ),	/* 0 */
/* 20 */	NdrFcShort( 0x0 ),	/* 0 */
/* 22 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter pVal */

/* 24 */	NdrFcShort( 0x13 ),	/* Flags:  must size, must free, out, */
/* 26 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 28 */	NdrFcShort( 0x2 ),	/* Type Offset=2 */

	/* Return value */

/* 30 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 32 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 34 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure get_Item */

/* 36 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 38 */	NdrFcLong( 0x0 ),	/* 0 */
/* 42 */	NdrFcShort( 0x8 ),	/* 8 */
/* 44 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 46 */	NdrFcShort( 0x8 ),	/* 8 */
/* 48 */	NdrFcShort( 0x8 ),	/* 8 */
/* 50 */	0x45,		/* Oi2 Flags:  srv must size, has return, has ext, */
			0x3,		/* 3 */
/* 52 */	0x8,		/* 8 */
			0x3,		/* Ext Flags:  new corr desc, clt corr check, */
/* 54 */	NdrFcShort( 0x9f ),	/* 159 */
/* 56 */	NdrFcShort( 0x0 ),	/* 0 */
/* 58 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter vIndex */

/* 60 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 62 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 64 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter ppEntry */

/* 66 */	NdrFcShort( 0x4113 ),	/* Flags:  must size, must free, out, simple ref, srv alloc size=16 */
/* 68 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 70 */	NdrFcShort( 0x408 ),	/* Type Offset=1032 */

	/* Return value */

/* 72 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 74 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 76 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure get_SessionState */


	/* Procedure get_Count */

/* 78 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 80 */	NdrFcLong( 0x0 ),	/* 0 */
/* 84 */	NdrFcShort( 0x9 ),	/* 9 */
/* 86 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 88 */	NdrFcShort( 0x0 ),	/* 0 */
/* 90 */	NdrFcShort( 0x24 ),	/* 36 */
/* 92 */	0x44,		/* Oi2 Flags:  has return, has ext, */
			0x2,		/* 2 */
/* 94 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 96 */	NdrFcShort( 0x0 ),	/* 0 */
/* 98 */	NdrFcShort( 0x0 ),	/* 0 */
/* 100 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter pVal */


	/* Parameter pVal */

/* 102 */	NdrFcShort( 0x2150 ),	/* Flags:  out, base type, simple ref, srv alloc size=8 */
/* 104 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 106 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Return value */


	/* Return value */

/* 108 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 110 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 112 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure Opnum7NotUsedByProtocol */

/* 114 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 116 */	NdrFcLong( 0x0 ),	/* 0 */
/* 120 */	NdrFcShort( 0x7 ),	/* 7 */
/* 122 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 124 */	NdrFcShort( 0x0 ),	/* 0 */
/* 126 */	NdrFcShort( 0x8 ),	/* 8 */
/* 128 */	0x44,		/* Oi2 Flags:  has return, has ext, */
			0x1,		/* 1 */
/* 130 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 132 */	NdrFcShort( 0x0 ),	/* 0 */
/* 134 */	NdrFcShort( 0x0 ),	/* 0 */
/* 136 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Return value */

/* 138 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 140 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 142 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure Opnum8NotUsedByProtocol */

/* 144 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 146 */	NdrFcLong( 0x0 ),	/* 0 */
/* 150 */	NdrFcShort( 0x8 ),	/* 8 */
/* 152 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 154 */	NdrFcShort( 0x0 ),	/* 0 */
/* 156 */	NdrFcShort( 0x8 ),	/* 8 */
/* 158 */	0x44,		/* Oi2 Flags:  has return, has ext, */
			0x1,		/* 1 */
/* 160 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 162 */	NdrFcShort( 0x0 ),	/* 0 */
/* 164 */	NdrFcShort( 0x0 ),	/* 0 */
/* 166 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Return value */

/* 168 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 170 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 172 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure Opnum9NotUsedByProtocol */

/* 174 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 176 */	NdrFcLong( 0x0 ),	/* 0 */
/* 180 */	NdrFcShort( 0x9 ),	/* 9 */
/* 182 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 184 */	NdrFcShort( 0x0 ),	/* 0 */
/* 186 */	NdrFcShort( 0x8 ),	/* 8 */
/* 188 */	0x44,		/* Oi2 Flags:  has return, has ext, */
			0x1,		/* 1 */
/* 190 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 192 */	NdrFcShort( 0x0 ),	/* 0 */
/* 194 */	NdrFcShort( 0x0 ),	/* 0 */
/* 196 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Return value */

/* 198 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 200 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 202 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure Opnum10NotUsedByProtocol */

/* 204 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 206 */	NdrFcLong( 0x0 ),	/* 0 */
/* 210 */	NdrFcShort( 0xa ),	/* 10 */
/* 212 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 214 */	NdrFcShort( 0x0 ),	/* 0 */
/* 216 */	NdrFcShort( 0x8 ),	/* 8 */
/* 218 */	0x44,		/* Oi2 Flags:  has return, has ext, */
			0x1,		/* 1 */
/* 220 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 222 */	NdrFcShort( 0x0 ),	/* 0 */
/* 224 */	NdrFcShort( 0x0 ),	/* 0 */
/* 226 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Return value */

/* 228 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 230 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 232 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure Opnum11NotUsedByProtocol */

/* 234 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 236 */	NdrFcLong( 0x0 ),	/* 0 */
/* 240 */	NdrFcShort( 0xb ),	/* 11 */
/* 242 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 244 */	NdrFcShort( 0x0 ),	/* 0 */
/* 246 */	NdrFcShort( 0x8 ),	/* 8 */
/* 248 */	0x44,		/* Oi2 Flags:  has return, has ext, */
			0x1,		/* 1 */
/* 250 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 252 */	NdrFcShort( 0x0 ),	/* 0 */
/* 254 */	NdrFcShort( 0x0 ),	/* 0 */
/* 256 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Return value */

/* 258 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 260 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 262 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure Opnum12NotUsedByProtocol */

/* 264 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 266 */	NdrFcLong( 0x0 ),	/* 0 */
/* 270 */	NdrFcShort( 0xc ),	/* 12 */
/* 272 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 274 */	NdrFcShort( 0x0 ),	/* 0 */
/* 276 */	NdrFcShort( 0x8 ),	/* 8 */
/* 278 */	0x44,		/* Oi2 Flags:  has return, has ext, */
			0x1,		/* 1 */
/* 280 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 282 */	NdrFcShort( 0x0 ),	/* 0 */
/* 284 */	NdrFcShort( 0x0 ),	/* 0 */
/* 286 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Return value */

/* 288 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 290 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 292 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure Opnum13NotUsedByProtocol */

/* 294 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 296 */	NdrFcLong( 0x0 ),	/* 0 */
/* 300 */	NdrFcShort( 0xd ),	/* 13 */
/* 302 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 304 */	NdrFcShort( 0x0 ),	/* 0 */
/* 306 */	NdrFcShort( 0x8 ),	/* 8 */
/* 308 */	0x44,		/* Oi2 Flags:  has return, has ext, */
			0x1,		/* 1 */
/* 310 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 312 */	NdrFcShort( 0x0 ),	/* 0 */
/* 314 */	NdrFcShort( 0x0 ),	/* 0 */
/* 316 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Return value */

/* 318 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 320 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 322 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure Opnum14NotUsedByProtocol */

/* 324 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 326 */	NdrFcLong( 0x0 ),	/* 0 */
/* 330 */	NdrFcShort( 0xe ),	/* 14 */
/* 332 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 334 */	NdrFcShort( 0x0 ),	/* 0 */
/* 336 */	NdrFcShort( 0x8 ),	/* 8 */
/* 338 */	0x44,		/* Oi2 Flags:  has return, has ext, */
			0x1,		/* 1 */
/* 340 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 342 */	NdrFcShort( 0x0 ),	/* 0 */
/* 344 */	NdrFcShort( 0x0 ),	/* 0 */
/* 346 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Return value */

/* 348 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 350 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 352 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure Opnum15NotUsedByProtocol */

/* 354 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 356 */	NdrFcLong( 0x0 ),	/* 0 */
/* 360 */	NdrFcShort( 0xf ),	/* 15 */
/* 362 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 364 */	NdrFcShort( 0x0 ),	/* 0 */
/* 366 */	NdrFcShort( 0x8 ),	/* 8 */
/* 368 */	0x44,		/* Oi2 Flags:  has return, has ext, */
			0x1,		/* 1 */
/* 370 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 372 */	NdrFcShort( 0x0 ),	/* 0 */
/* 374 */	NdrFcShort( 0x0 ),	/* 0 */
/* 376 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Return value */

/* 378 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 380 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 382 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure Opnum16NotUsedByProtocol */

/* 384 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 386 */	NdrFcLong( 0x0 ),	/* 0 */
/* 390 */	NdrFcShort( 0x10 ),	/* 16 */
/* 392 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 394 */	NdrFcShort( 0x0 ),	/* 0 */
/* 396 */	NdrFcShort( 0x8 ),	/* 8 */
/* 398 */	0x44,		/* Oi2 Flags:  has return, has ext, */
			0x1,		/* 1 */
/* 400 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 402 */	NdrFcShort( 0x0 ),	/* 0 */
/* 404 */	NdrFcShort( 0x0 ),	/* 0 */
/* 406 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Return value */

/* 408 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 410 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 412 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure Opnum17NotUsedByProtocol */

/* 414 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 416 */	NdrFcLong( 0x0 ),	/* 0 */
/* 420 */	NdrFcShort( 0x11 ),	/* 17 */
/* 422 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 424 */	NdrFcShort( 0x0 ),	/* 0 */
/* 426 */	NdrFcShort( 0x8 ),	/* 8 */
/* 428 */	0x44,		/* Oi2 Flags:  has return, has ext, */
			0x1,		/* 1 */
/* 430 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 432 */	NdrFcShort( 0x0 ),	/* 0 */
/* 434 */	NdrFcShort( 0x0 ),	/* 0 */
/* 436 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Return value */

/* 438 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 440 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 442 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure Opnum18NotUsedByProtocol */

/* 444 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 446 */	NdrFcLong( 0x0 ),	/* 0 */
/* 450 */	NdrFcShort( 0x12 ),	/* 18 */
/* 452 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 454 */	NdrFcShort( 0x0 ),	/* 0 */
/* 456 */	NdrFcShort( 0x8 ),	/* 8 */
/* 458 */	0x44,		/* Oi2 Flags:  has return, has ext, */
			0x1,		/* 1 */
/* 460 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 462 */	NdrFcShort( 0x0 ),	/* 0 */
/* 464 */	NdrFcShort( 0x0 ),	/* 0 */
/* 466 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Return value */

/* 468 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 470 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 472 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure RemoteConnectionParms */

/* 474 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 476 */	NdrFcLong( 0x0 ),	/* 0 */
/* 480 */	NdrFcShort( 0x13 ),	/* 19 */
/* 482 */	NdrFcShort( 0x1c ),	/* x86 Stack size/offset = 28 */
/* 484 */	NdrFcShort( 0x8 ),	/* 8 */
/* 486 */	NdrFcShort( 0x8 ),	/* 8 */
/* 488 */	0x47,		/* Oi2 Flags:  srv must size, clt must size, has return, has ext, */
			0x6,		/* 6 */
/* 490 */	0x8,		/* 8 */
			0x7,		/* Ext Flags:  new corr desc, clt corr check, srv corr check, */
/* 492 */	NdrFcShort( 0x1 ),	/* 1 */
/* 494 */	NdrFcShort( 0xb ),	/* 11 */
/* 496 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter bstrUserName */

/* 498 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 500 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 502 */	NdrFcShort( 0x41a ),	/* Type Offset=1050 */

	/* Parameter bstrDomainName */

/* 504 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 506 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 508 */	NdrFcShort( 0x41a ),	/* Type Offset=1050 */

	/* Parameter lSessionID */

/* 510 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 512 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 514 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter bstrUserHelpBlob */

/* 516 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 518 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 520 */	NdrFcShort( 0x41a ),	/* Type Offset=1050 */

	/* Parameter pbstrConnectionString */

/* 522 */	NdrFcShort( 0x2113 ),	/* Flags:  must size, must free, out, simple ref, srv alloc size=8 */
/* 524 */	NdrFcShort( 0x14 ),	/* x86 Stack size/offset = 20 */
/* 526 */	NdrFcShort( 0x428 ),	/* Type Offset=1064 */

	/* Return value */

/* 528 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 530 */	NdrFcShort( 0x18 ),	/* x86 Stack size/offset = 24 */
/* 532 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure RemoteUserSessionInfo */

/* 534 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 536 */	NdrFcLong( 0x0 ),	/* 0 */
/* 540 */	NdrFcShort( 0x14 ),	/* 20 */
/* 542 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 544 */	NdrFcShort( 0x0 ),	/* 0 */
/* 546 */	NdrFcShort( 0x8 ),	/* 8 */
/* 548 */	0x45,		/* Oi2 Flags:  srv must size, has return, has ext, */
			0x2,		/* 2 */
/* 550 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 552 */	NdrFcShort( 0x0 ),	/* 0 */
/* 554 */	NdrFcShort( 0x0 ),	/* 0 */
/* 556 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter pVal */

/* 558 */	NdrFcShort( 0x13 ),	/* Flags:  must size, must free, out, */
/* 560 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 562 */	NdrFcShort( 0x432 ),	/* Type Offset=1074 */

	/* Return value */

/* 564 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 566 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 568 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure Opnum21NotUsedByProtocol */

/* 570 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 572 */	NdrFcLong( 0x0 ),	/* 0 */
/* 576 */	NdrFcShort( 0x15 ),	/* 21 */
/* 578 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 580 */	NdrFcShort( 0x0 ),	/* 0 */
/* 582 */	NdrFcShort( 0x8 ),	/* 8 */
/* 584 */	0x44,		/* Oi2 Flags:  has return, has ext, */
			0x1,		/* 1 */
/* 586 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 588 */	NdrFcShort( 0x0 ),	/* 0 */
/* 590 */	NdrFcShort( 0x0 ),	/* 0 */
/* 592 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Return value */

/* 594 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 596 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 598 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure get_SessionID */

/* 600 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 602 */	NdrFcLong( 0x0 ),	/* 0 */
/* 606 */	NdrFcShort( 0x7 ),	/* 7 */
/* 608 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 610 */	NdrFcShort( 0x0 ),	/* 0 */
/* 612 */	NdrFcShort( 0x24 ),	/* 36 */
/* 614 */	0x44,		/* Oi2 Flags:  has return, has ext, */
			0x2,		/* 2 */
/* 616 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 618 */	NdrFcShort( 0x0 ),	/* 0 */
/* 620 */	NdrFcShort( 0x0 ),	/* 0 */
/* 622 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter pVal */

/* 624 */	NdrFcShort( 0x2150 ),	/* Flags:  out, base type, simple ref, srv alloc size=8 */
/* 626 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 628 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Return value */

/* 630 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 632 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 634 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure put_SessionID */

/* 636 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 638 */	NdrFcLong( 0x0 ),	/* 0 */
/* 642 */	NdrFcShort( 0x8 ),	/* 8 */
/* 644 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 646 */	NdrFcShort( 0x8 ),	/* 8 */
/* 648 */	NdrFcShort( 0x8 ),	/* 8 */
/* 650 */	0x44,		/* Oi2 Flags:  has return, has ext, */
			0x2,		/* 2 */
/* 652 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 654 */	NdrFcShort( 0x0 ),	/* 0 */
/* 656 */	NdrFcShort( 0x0 ),	/* 0 */
/* 658 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter pVal */

/* 660 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 662 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 664 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Return value */

/* 666 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 668 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 670 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure put_SessionState */

/* 672 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 674 */	NdrFcLong( 0x0 ),	/* 0 */
/* 678 */	NdrFcShort( 0xa ),	/* 10 */
/* 680 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 682 */	NdrFcShort( 0x8 ),	/* 8 */
/* 684 */	NdrFcShort( 0x8 ),	/* 8 */
/* 686 */	0x44,		/* Oi2 Flags:  has return, has ext, */
			0x2,		/* 2 */
/* 688 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 690 */	NdrFcShort( 0x0 ),	/* 0 */
/* 692 */	NdrFcShort( 0x0 ),	/* 0 */
/* 694 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter pVal */

/* 696 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 698 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 700 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Return value */

/* 702 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 704 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 706 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure get_DomainName */

/* 708 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 710 */	NdrFcLong( 0x0 ),	/* 0 */
/* 714 */	NdrFcShort( 0xb ),	/* 11 */
/* 716 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 718 */	NdrFcShort( 0x0 ),	/* 0 */
/* 720 */	NdrFcShort( 0x8 ),	/* 8 */
/* 722 */	0x45,		/* Oi2 Flags:  srv must size, has return, has ext, */
			0x2,		/* 2 */
/* 724 */	0x8,		/* 8 */
			0x3,		/* Ext Flags:  new corr desc, clt corr check, */
/* 726 */	NdrFcShort( 0x6 ),	/* 6 */
/* 728 */	NdrFcShort( 0x0 ),	/* 0 */
/* 730 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter pVal */

/* 732 */	NdrFcShort( 0x2113 ),	/* Flags:  must size, must free, out, simple ref, srv alloc size=8 */
/* 734 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 736 */	NdrFcShort( 0x428 ),	/* Type Offset=1064 */

	/* Return value */

/* 738 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 740 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 742 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure put_DomainName */

/* 744 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 746 */	NdrFcLong( 0x0 ),	/* 0 */
/* 750 */	NdrFcShort( 0xc ),	/* 12 */
/* 752 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 754 */	NdrFcShort( 0x0 ),	/* 0 */
/* 756 */	NdrFcShort( 0x8 ),	/* 8 */
/* 758 */	0x46,		/* Oi2 Flags:  clt must size, has return, has ext, */
			0x2,		/* 2 */
/* 760 */	0x8,		/* 8 */
			0x5,		/* Ext Flags:  new corr desc, srv corr check, */
/* 762 */	NdrFcShort( 0x0 ),	/* 0 */
/* 764 */	NdrFcShort( 0x1 ),	/* 1 */
/* 766 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter pVal */

/* 768 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 770 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 772 */	NdrFcShort( 0x41a ),	/* Type Offset=1050 */

	/* Return value */

/* 774 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 776 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 778 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure get_UserName */

/* 780 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 782 */	NdrFcLong( 0x0 ),	/* 0 */
/* 786 */	NdrFcShort( 0xd ),	/* 13 */
/* 788 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 790 */	NdrFcShort( 0x0 ),	/* 0 */
/* 792 */	NdrFcShort( 0x8 ),	/* 8 */
/* 794 */	0x45,		/* Oi2 Flags:  srv must size, has return, has ext, */
			0x2,		/* 2 */
/* 796 */	0x8,		/* 8 */
			0x3,		/* Ext Flags:  new corr desc, clt corr check, */
/* 798 */	NdrFcShort( 0x1 ),	/* 1 */
/* 800 */	NdrFcShort( 0x0 ),	/* 0 */
/* 802 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter pVal */

/* 804 */	NdrFcShort( 0x2113 ),	/* Flags:  must size, must free, out, simple ref, srv alloc size=8 */
/* 806 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 808 */	NdrFcShort( 0x428 ),	/* Type Offset=1064 */

	/* Return value */

/* 810 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 812 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 814 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure put_UserName */

/* 816 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 818 */	NdrFcLong( 0x0 ),	/* 0 */
/* 822 */	NdrFcShort( 0xe ),	/* 14 */
/* 824 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 826 */	NdrFcShort( 0x0 ),	/* 0 */
/* 828 */	NdrFcShort( 0x8 ),	/* 8 */
/* 830 */	0x46,		/* Oi2 Flags:  clt must size, has return, has ext, */
			0x2,		/* 2 */
/* 832 */	0x8,		/* 8 */
			0x5,		/* Ext Flags:  new corr desc, srv corr check, */
/* 834 */	NdrFcShort( 0x0 ),	/* 0 */
/* 836 */	NdrFcShort( 0x1 ),	/* 1 */
/* 838 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter pVal */

/* 840 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 842 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 844 */	NdrFcShort( 0x41a ),	/* Type Offset=1050 */

	/* Return value */

/* 846 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 848 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 850 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure GetNoviceUserInfo */

/* 852 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 854 */	NdrFcLong( 0x0 ),	/* 0 */
/* 858 */	NdrFcShort( 0x7 ),	/* 7 */
/* 860 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 862 */	NdrFcShort( 0x0 ),	/* 0 */
/* 864 */	NdrFcShort( 0x8 ),	/* 8 */
/* 866 */	0x47,		/* Oi2 Flags:  srv must size, clt must size, has return, has ext, */
			0x2,		/* 2 */
/* 868 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 870 */	NdrFcShort( 0x0 ),	/* 0 */
/* 872 */	NdrFcShort( 0x0 ),	/* 0 */
/* 874 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter szName */

/* 876 */	NdrFcShort( 0x201b ),	/* Flags:  must size, must free, in, out, srv alloc size=8 */
/* 878 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 880 */	NdrFcShort( 0x448 ),	/* Type Offset=1096 */

	/* Return value */

/* 882 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 884 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 886 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure GetSessionInfo */

/* 888 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 890 */	NdrFcLong( 0x0 ),	/* 0 */
/* 894 */	NdrFcShort( 0x8 ),	/* 8 */
/* 896 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 898 */	NdrFcShort( 0x1c ),	/* 28 */
/* 900 */	NdrFcShort( 0x24 ),	/* 36 */
/* 902 */	0x47,		/* Oi2 Flags:  srv must size, clt must size, has return, has ext, */
			0x3,		/* 3 */
/* 904 */	0x8,		/* 8 */
			0x7,		/* Ext Flags:  new corr desc, clt corr check, srv corr check, */
/* 906 */	NdrFcShort( 0x2c ),	/* 44 */
/* 908 */	NdrFcShort( 0x2c ),	/* 44 */
/* 910 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter UserNames */

/* 912 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 914 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 916 */	NdrFcShort( 0x454 ),	/* Type Offset=1108 */

	/* Parameter Count */

/* 918 */	NdrFcShort( 0x158 ),	/* Flags:  in, out, base type, simple ref, */
/* 920 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 922 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Return value */

/* 924 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 926 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 928 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

			0x0
        }
    };

static const msra_MIDL_TYPE_FORMAT_STRING msra__MIDL_TypeFormatString =
    {
        0,
        {
			NdrFcShort( 0x0 ),	/* 0 */
/*  2 */	
			0x11, 0x10,	/* FC_RP [pointer_deref] */
/*  4 */	NdrFcShort( 0x2 ),	/* Offset= 2 (6) */
/*  6 */	
			0x2f,		/* FC_IP */
			0x5a,		/* FC_CONSTANT_IID */
/*  8 */	NdrFcLong( 0x0 ),	/* 0 */
/* 12 */	NdrFcShort( 0x0 ),	/* 0 */
/* 14 */	NdrFcShort( 0x0 ),	/* 0 */
/* 16 */	0xc0,		/* 192 */
			0x0,		/* 0 */
/* 18 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 20 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 22 */	0x0,		/* 0 */
			0x46,		/* 70 */
/* 24 */	
			0x11, 0x4,	/* FC_RP [alloced_on_stack] */
/* 26 */	NdrFcShort( 0x3ee ),	/* Offset= 1006 (1032) */
/* 28 */	
			0x13, 0x0,	/* FC_OP */
/* 30 */	NdrFcShort( 0x3d6 ),	/* Offset= 982 (1012) */
/* 32 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x9,		/* FC_ULONG */
/* 34 */	0x7,		/* Corr desc: FC_USHORT */
			0x0,		/*  */
/* 36 */	NdrFcShort( 0xfff8 ),	/* -8 */
/* 38 */	NdrFcShort( 0x1 ),	/* Corr flags:  early, */
/* 40 */	NdrFcShort( 0x2 ),	/* Offset= 2 (42) */
/* 42 */	NdrFcShort( 0x10 ),	/* 16 */
/* 44 */	NdrFcShort( 0x2f ),	/* 47 */
/* 46 */	NdrFcLong( 0x14 ),	/* 20 */
/* 50 */	NdrFcShort( 0x800b ),	/* Simple arm type: FC_HYPER */
/* 52 */	NdrFcLong( 0x3 ),	/* 3 */
/* 56 */	NdrFcShort( 0x8008 ),	/* Simple arm type: FC_LONG */
/* 58 */	NdrFcLong( 0x11 ),	/* 17 */
/* 62 */	NdrFcShort( 0x8001 ),	/* Simple arm type: FC_BYTE */
/* 64 */	NdrFcLong( 0x2 ),	/* 2 */
/* 68 */	NdrFcShort( 0x8006 ),	/* Simple arm type: FC_SHORT */
/* 70 */	NdrFcLong( 0x4 ),	/* 4 */
/* 74 */	NdrFcShort( 0x800a ),	/* Simple arm type: FC_FLOAT */
/* 76 */	NdrFcLong( 0x5 ),	/* 5 */
/* 80 */	NdrFcShort( 0x800c ),	/* Simple arm type: FC_DOUBLE */
/* 82 */	NdrFcLong( 0xb ),	/* 11 */
/* 86 */	NdrFcShort( 0x8006 ),	/* Simple arm type: FC_SHORT */
/* 88 */	NdrFcLong( 0xa ),	/* 10 */
/* 92 */	NdrFcShort( 0x8008 ),	/* Simple arm type: FC_LONG */
/* 94 */	NdrFcLong( 0x6 ),	/* 6 */
/* 98 */	NdrFcShort( 0xe8 ),	/* Offset= 232 (330) */
/* 100 */	NdrFcLong( 0x7 ),	/* 7 */
/* 104 */	NdrFcShort( 0x800c ),	/* Simple arm type: FC_DOUBLE */
/* 106 */	NdrFcLong( 0x8 ),	/* 8 */
/* 110 */	NdrFcShort( 0xe2 ),	/* Offset= 226 (336) */
/* 112 */	NdrFcLong( 0xd ),	/* 13 */
/* 116 */	NdrFcShort( 0xff92 ),	/* Offset= -110 (6) */
/* 118 */	NdrFcLong( 0x9 ),	/* 9 */
/* 122 */	NdrFcShort( 0xf0 ),	/* Offset= 240 (362) */
/* 124 */	NdrFcLong( 0x2000 ),	/* 8192 */
/* 128 */	NdrFcShort( 0xfc ),	/* Offset= 252 (380) */
/* 130 */	NdrFcLong( 0x24 ),	/* 36 */
/* 134 */	NdrFcShort( 0x324 ),	/* Offset= 804 (938) */
/* 136 */	NdrFcLong( 0x4024 ),	/* 16420 */
/* 140 */	NdrFcShort( 0x31e ),	/* Offset= 798 (938) */
/* 142 */	NdrFcLong( 0x4011 ),	/* 16401 */
/* 146 */	NdrFcShort( 0x31c ),	/* Offset= 796 (942) */
/* 148 */	NdrFcLong( 0x4002 ),	/* 16386 */
/* 152 */	NdrFcShort( 0x31a ),	/* Offset= 794 (946) */
/* 154 */	NdrFcLong( 0x4003 ),	/* 16387 */
/* 158 */	NdrFcShort( 0x318 ),	/* Offset= 792 (950) */
/* 160 */	NdrFcLong( 0x4014 ),	/* 16404 */
/* 164 */	NdrFcShort( 0x316 ),	/* Offset= 790 (954) */
/* 166 */	NdrFcLong( 0x4004 ),	/* 16388 */
/* 170 */	NdrFcShort( 0x314 ),	/* Offset= 788 (958) */
/* 172 */	NdrFcLong( 0x4005 ),	/* 16389 */
/* 176 */	NdrFcShort( 0x312 ),	/* Offset= 786 (962) */
/* 178 */	NdrFcLong( 0x400b ),	/* 16395 */
/* 182 */	NdrFcShort( 0x2fc ),	/* Offset= 764 (946) */
/* 184 */	NdrFcLong( 0x400a ),	/* 16394 */
/* 188 */	NdrFcShort( 0x2fa ),	/* Offset= 762 (950) */
/* 190 */	NdrFcLong( 0x4006 ),	/* 16390 */
/* 194 */	NdrFcShort( 0x304 ),	/* Offset= 772 (966) */
/* 196 */	NdrFcLong( 0x4007 ),	/* 16391 */
/* 200 */	NdrFcShort( 0x2fa ),	/* Offset= 762 (962) */
/* 202 */	NdrFcLong( 0x4008 ),	/* 16392 */
/* 206 */	NdrFcShort( 0x2fc ),	/* Offset= 764 (970) */
/* 208 */	NdrFcLong( 0x400d ),	/* 16397 */
/* 212 */	NdrFcShort( 0x2fa ),	/* Offset= 762 (974) */
/* 214 */	NdrFcLong( 0x4009 ),	/* 16393 */
/* 218 */	NdrFcShort( 0x2f8 ),	/* Offset= 760 (978) */
/* 220 */	NdrFcLong( 0x6000 ),	/* 24576 */
/* 224 */	NdrFcShort( 0x2f6 ),	/* Offset= 758 (982) */
/* 226 */	NdrFcLong( 0x400c ),	/* 16396 */
/* 230 */	NdrFcShort( 0x2f4 ),	/* Offset= 756 (986) */
/* 232 */	NdrFcLong( 0x10 ),	/* 16 */
/* 236 */	NdrFcShort( 0x8002 ),	/* Simple arm type: FC_CHAR */
/* 238 */	NdrFcLong( 0x12 ),	/* 18 */
/* 242 */	NdrFcShort( 0x8006 ),	/* Simple arm type: FC_SHORT */
/* 244 */	NdrFcLong( 0x13 ),	/* 19 */
/* 248 */	NdrFcShort( 0x8008 ),	/* Simple arm type: FC_LONG */
/* 250 */	NdrFcLong( 0x15 ),	/* 21 */
/* 254 */	NdrFcShort( 0x800b ),	/* Simple arm type: FC_HYPER */
/* 256 */	NdrFcLong( 0x16 ),	/* 22 */
/* 260 */	NdrFcShort( 0x8008 ),	/* Simple arm type: FC_LONG */
/* 262 */	NdrFcLong( 0x17 ),	/* 23 */
/* 266 */	NdrFcShort( 0x8008 ),	/* Simple arm type: FC_LONG */
/* 268 */	NdrFcLong( 0xe ),	/* 14 */
/* 272 */	NdrFcShort( 0x2d2 ),	/* Offset= 722 (994) */
/* 274 */	NdrFcLong( 0x400e ),	/* 16398 */
/* 278 */	NdrFcShort( 0x2d6 ),	/* Offset= 726 (1004) */
/* 280 */	NdrFcLong( 0x4010 ),	/* 16400 */
/* 284 */	NdrFcShort( 0x2d4 ),	/* Offset= 724 (1008) */
/* 286 */	NdrFcLong( 0x4012 ),	/* 16402 */
/* 290 */	NdrFcShort( 0x290 ),	/* Offset= 656 (946) */
/* 292 */	NdrFcLong( 0x4013 ),	/* 16403 */
/* 296 */	NdrFcShort( 0x28e ),	/* Offset= 654 (950) */
/* 298 */	NdrFcLong( 0x4015 ),	/* 16405 */
/* 302 */	NdrFcShort( 0x28c ),	/* Offset= 652 (954) */
/* 304 */	NdrFcLong( 0x4016 ),	/* 16406 */
/* 308 */	NdrFcShort( 0x282 ),	/* Offset= 642 (950) */
/* 310 */	NdrFcLong( 0x4017 ),	/* 16407 */
/* 314 */	NdrFcShort( 0x27c ),	/* Offset= 636 (950) */
/* 316 */	NdrFcLong( 0x0 ),	/* 0 */
/* 320 */	NdrFcShort( 0x0 ),	/* Offset= 0 (320) */
/* 322 */	NdrFcLong( 0x1 ),	/* 1 */
/* 326 */	NdrFcShort( 0x0 ),	/* Offset= 0 (326) */
/* 328 */	NdrFcShort( 0xffff ),	/* Offset= -1 (327) */
/* 330 */	
			0x15,		/* FC_STRUCT */
			0x7,		/* 7 */
/* 332 */	NdrFcShort( 0x8 ),	/* 8 */
/* 334 */	0xb,		/* FC_HYPER */
			0x5b,		/* FC_END */
/* 336 */	
			0x13, 0x0,	/* FC_OP */
/* 338 */	NdrFcShort( 0xe ),	/* Offset= 14 (352) */
/* 340 */	
			0x1b,		/* FC_CARRAY */
			0x1,		/* 1 */
/* 342 */	NdrFcShort( 0x2 ),	/* 2 */
/* 344 */	0x9,		/* Corr desc: FC_ULONG */
			0x0,		/*  */
/* 346 */	NdrFcShort( 0xfffc ),	/* -4 */
/* 348 */	NdrFcShort( 0x1 ),	/* Corr flags:  early, */
/* 350 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 352 */	
			0x17,		/* FC_CSTRUCT */
			0x3,		/* 3 */
/* 354 */	NdrFcShort( 0x8 ),	/* 8 */
/* 356 */	NdrFcShort( 0xfff0 ),	/* Offset= -16 (340) */
/* 358 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 360 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 362 */	
			0x2f,		/* FC_IP */
			0x5a,		/* FC_CONSTANT_IID */
/* 364 */	NdrFcLong( 0x20400 ),	/* 132096 */
/* 368 */	NdrFcShort( 0x0 ),	/* 0 */
/* 370 */	NdrFcShort( 0x0 ),	/* 0 */
/* 372 */	0xc0,		/* 192 */
			0x0,		/* 0 */
/* 374 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 376 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 378 */	0x0,		/* 0 */
			0x46,		/* 70 */
/* 380 */	
			0x13, 0x10,	/* FC_OP [pointer_deref] */
/* 382 */	NdrFcShort( 0x2 ),	/* Offset= 2 (384) */
/* 384 */	
			0x13, 0x0,	/* FC_OP */
/* 386 */	NdrFcShort( 0x216 ),	/* Offset= 534 (920) */
/* 388 */	
			0x2a,		/* FC_ENCAPSULATED_UNION */
			0x49,		/* 73 */
/* 390 */	NdrFcShort( 0x18 ),	/* 24 */
/* 392 */	NdrFcShort( 0xa ),	/* 10 */
/* 394 */	NdrFcLong( 0x8 ),	/* 8 */
/* 398 */	NdrFcShort( 0x5a ),	/* Offset= 90 (488) */
/* 400 */	NdrFcLong( 0xd ),	/* 13 */
/* 404 */	NdrFcShort( 0x7e ),	/* Offset= 126 (530) */
/* 406 */	NdrFcLong( 0x9 ),	/* 9 */
/* 410 */	NdrFcShort( 0x9e ),	/* Offset= 158 (568) */
/* 412 */	NdrFcLong( 0xc ),	/* 12 */
/* 416 */	NdrFcShort( 0xc8 ),	/* Offset= 200 (616) */
/* 418 */	NdrFcLong( 0x24 ),	/* 36 */
/* 422 */	NdrFcShort( 0x124 ),	/* Offset= 292 (714) */
/* 424 */	NdrFcLong( 0x800d ),	/* 32781 */
/* 428 */	NdrFcShort( 0x140 ),	/* Offset= 320 (748) */
/* 430 */	NdrFcLong( 0x10 ),	/* 16 */
/* 434 */	NdrFcShort( 0x15a ),	/* Offset= 346 (780) */
/* 436 */	NdrFcLong( 0x2 ),	/* 2 */
/* 440 */	NdrFcShort( 0x174 ),	/* Offset= 372 (812) */
/* 442 */	NdrFcLong( 0x3 ),	/* 3 */
/* 446 */	NdrFcShort( 0x18e ),	/* Offset= 398 (844) */
/* 448 */	NdrFcLong( 0x14 ),	/* 20 */
/* 452 */	NdrFcShort( 0x1a8 ),	/* Offset= 424 (876) */
/* 454 */	NdrFcShort( 0xffff ),	/* Offset= -1 (453) */
/* 456 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 458 */	NdrFcShort( 0x4 ),	/* 4 */
/* 460 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 462 */	NdrFcShort( 0x0 ),	/* 0 */
/* 464 */	NdrFcShort( 0x1 ),	/* Corr flags:  early, */
/* 466 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 468 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 470 */	NdrFcShort( 0x4 ),	/* 4 */
/* 472 */	NdrFcShort( 0x0 ),	/* 0 */
/* 474 */	NdrFcShort( 0x1 ),	/* 1 */
/* 476 */	NdrFcShort( 0x0 ),	/* 0 */
/* 478 */	NdrFcShort( 0x0 ),	/* 0 */
/* 480 */	0x13, 0x0,	/* FC_OP */
/* 482 */	NdrFcShort( 0xff7e ),	/* Offset= -130 (352) */
/* 484 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 486 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 488 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 490 */	NdrFcShort( 0x8 ),	/* 8 */
/* 492 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 494 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 496 */	NdrFcShort( 0x4 ),	/* 4 */
/* 498 */	NdrFcShort( 0x4 ),	/* 4 */
/* 500 */	0x11, 0x0,	/* FC_RP */
/* 502 */	NdrFcShort( 0xffd2 ),	/* Offset= -46 (456) */
/* 504 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 506 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 508 */	
			0x21,		/* FC_BOGUS_ARRAY */
			0x3,		/* 3 */
/* 510 */	NdrFcShort( 0x0 ),	/* 0 */
/* 512 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 514 */	NdrFcShort( 0x0 ),	/* 0 */
/* 516 */	NdrFcShort( 0x1 ),	/* Corr flags:  early, */
/* 518 */	NdrFcLong( 0xffffffff ),	/* -1 */
/* 522 */	NdrFcShort( 0x0 ),	/* Corr flags:  */
/* 524 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 526 */	NdrFcShort( 0xfdf8 ),	/* Offset= -520 (6) */
/* 528 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 530 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 532 */	NdrFcShort( 0x8 ),	/* 8 */
/* 534 */	NdrFcShort( 0x0 ),	/* 0 */
/* 536 */	NdrFcShort( 0x6 ),	/* Offset= 6 (542) */
/* 538 */	0x8,		/* FC_LONG */
			0x36,		/* FC_POINTER */
/* 540 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 542 */	
			0x11, 0x0,	/* FC_RP */
/* 544 */	NdrFcShort( 0xffdc ),	/* Offset= -36 (508) */
/* 546 */	
			0x21,		/* FC_BOGUS_ARRAY */
			0x3,		/* 3 */
/* 548 */	NdrFcShort( 0x0 ),	/* 0 */
/* 550 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 552 */	NdrFcShort( 0x0 ),	/* 0 */
/* 554 */	NdrFcShort( 0x1 ),	/* Corr flags:  early, */
/* 556 */	NdrFcLong( 0xffffffff ),	/* -1 */
/* 560 */	NdrFcShort( 0x0 ),	/* Corr flags:  */
/* 562 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 564 */	NdrFcShort( 0xff36 ),	/* Offset= -202 (362) */
/* 566 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 568 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 570 */	NdrFcShort( 0x8 ),	/* 8 */
/* 572 */	NdrFcShort( 0x0 ),	/* 0 */
/* 574 */	NdrFcShort( 0x6 ),	/* Offset= 6 (580) */
/* 576 */	0x8,		/* FC_LONG */
			0x36,		/* FC_POINTER */
/* 578 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 580 */	
			0x11, 0x0,	/* FC_RP */
/* 582 */	NdrFcShort( 0xffdc ),	/* Offset= -36 (546) */
/* 584 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 586 */	NdrFcShort( 0x4 ),	/* 4 */
/* 588 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 590 */	NdrFcShort( 0x0 ),	/* 0 */
/* 592 */	NdrFcShort( 0x1 ),	/* Corr flags:  early, */
/* 594 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 596 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 598 */	NdrFcShort( 0x4 ),	/* 4 */
/* 600 */	NdrFcShort( 0x0 ),	/* 0 */
/* 602 */	NdrFcShort( 0x1 ),	/* 1 */
/* 604 */	NdrFcShort( 0x0 ),	/* 0 */
/* 606 */	NdrFcShort( 0x0 ),	/* 0 */
/* 608 */	0x13, 0x0,	/* FC_OP */
/* 610 */	NdrFcShort( 0x192 ),	/* Offset= 402 (1012) */
/* 612 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 614 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 616 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 618 */	NdrFcShort( 0x8 ),	/* 8 */
/* 620 */	NdrFcShort( 0x0 ),	/* 0 */
/* 622 */	NdrFcShort( 0x6 ),	/* Offset= 6 (628) */
/* 624 */	0x8,		/* FC_LONG */
			0x36,		/* FC_POINTER */
/* 626 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 628 */	
			0x11, 0x0,	/* FC_RP */
/* 630 */	NdrFcShort( 0xffd2 ),	/* Offset= -46 (584) */
/* 632 */	
			0x2f,		/* FC_IP */
			0x5a,		/* FC_CONSTANT_IID */
/* 634 */	NdrFcLong( 0x2f ),	/* 47 */
/* 638 */	NdrFcShort( 0x0 ),	/* 0 */
/* 640 */	NdrFcShort( 0x0 ),	/* 0 */
/* 642 */	0xc0,		/* 192 */
			0x0,		/* 0 */
/* 644 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 646 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 648 */	0x0,		/* 0 */
			0x46,		/* 70 */
/* 650 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 652 */	NdrFcShort( 0x1 ),	/* 1 */
/* 654 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 656 */	NdrFcShort( 0x4 ),	/* 4 */
/* 658 */	NdrFcShort( 0x1 ),	/* Corr flags:  early, */
/* 660 */	0x1,		/* FC_BYTE */
			0x5b,		/* FC_END */
/* 662 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 664 */	NdrFcShort( 0x10 ),	/* 16 */
/* 666 */	NdrFcShort( 0x0 ),	/* 0 */
/* 668 */	NdrFcShort( 0xa ),	/* Offset= 10 (678) */
/* 670 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 672 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 674 */	NdrFcShort( 0xffd6 ),	/* Offset= -42 (632) */
/* 676 */	0x36,		/* FC_POINTER */
			0x5b,		/* FC_END */
/* 678 */	
			0x13, 0x0,	/* FC_OP */
/* 680 */	NdrFcShort( 0xffe2 ),	/* Offset= -30 (650) */
/* 682 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 684 */	NdrFcShort( 0x4 ),	/* 4 */
/* 686 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 688 */	NdrFcShort( 0x0 ),	/* 0 */
/* 690 */	NdrFcShort( 0x1 ),	/* Corr flags:  early, */
/* 692 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 694 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 696 */	NdrFcShort( 0x4 ),	/* 4 */
/* 698 */	NdrFcShort( 0x0 ),	/* 0 */
/* 700 */	NdrFcShort( 0x1 ),	/* 1 */
/* 702 */	NdrFcShort( 0x0 ),	/* 0 */
/* 704 */	NdrFcShort( 0x0 ),	/* 0 */
/* 706 */	0x13, 0x0,	/* FC_OP */
/* 708 */	NdrFcShort( 0xffd2 ),	/* Offset= -46 (662) */
/* 710 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 712 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 714 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 716 */	NdrFcShort( 0x8 ),	/* 8 */
/* 718 */	NdrFcShort( 0x0 ),	/* 0 */
/* 720 */	NdrFcShort( 0x6 ),	/* Offset= 6 (726) */
/* 722 */	0x8,		/* FC_LONG */
			0x36,		/* FC_POINTER */
/* 724 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 726 */	
			0x11, 0x0,	/* FC_RP */
/* 728 */	NdrFcShort( 0xffd2 ),	/* Offset= -46 (682) */
/* 730 */	
			0x1d,		/* FC_SMFARRAY */
			0x0,		/* 0 */
/* 732 */	NdrFcShort( 0x8 ),	/* 8 */
/* 734 */	0x1,		/* FC_BYTE */
			0x5b,		/* FC_END */
/* 736 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 738 */	NdrFcShort( 0x10 ),	/* 16 */
/* 740 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 742 */	0x6,		/* FC_SHORT */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 744 */	0x0,		/* 0 */
			NdrFcShort( 0xfff1 ),	/* Offset= -15 (730) */
			0x5b,		/* FC_END */
/* 748 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 750 */	NdrFcShort( 0x18 ),	/* 24 */
/* 752 */	NdrFcShort( 0x0 ),	/* 0 */
/* 754 */	NdrFcShort( 0xa ),	/* Offset= 10 (764) */
/* 756 */	0x8,		/* FC_LONG */
			0x36,		/* FC_POINTER */
/* 758 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 760 */	NdrFcShort( 0xffe8 ),	/* Offset= -24 (736) */
/* 762 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 764 */	
			0x11, 0x0,	/* FC_RP */
/* 766 */	NdrFcShort( 0xfefe ),	/* Offset= -258 (508) */
/* 768 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 770 */	NdrFcShort( 0x1 ),	/* 1 */
/* 772 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 774 */	NdrFcShort( 0x0 ),	/* 0 */
/* 776 */	NdrFcShort( 0x1 ),	/* Corr flags:  early, */
/* 778 */	0x1,		/* FC_BYTE */
			0x5b,		/* FC_END */
/* 780 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 782 */	NdrFcShort( 0x8 ),	/* 8 */
/* 784 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 786 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 788 */	NdrFcShort( 0x4 ),	/* 4 */
/* 790 */	NdrFcShort( 0x4 ),	/* 4 */
/* 792 */	0x13, 0x0,	/* FC_OP */
/* 794 */	NdrFcShort( 0xffe6 ),	/* Offset= -26 (768) */
/* 796 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 798 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 800 */	
			0x1b,		/* FC_CARRAY */
			0x1,		/* 1 */
/* 802 */	NdrFcShort( 0x2 ),	/* 2 */
/* 804 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 806 */	NdrFcShort( 0x0 ),	/* 0 */
/* 808 */	NdrFcShort( 0x1 ),	/* Corr flags:  early, */
/* 810 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 812 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 814 */	NdrFcShort( 0x8 ),	/* 8 */
/* 816 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 818 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 820 */	NdrFcShort( 0x4 ),	/* 4 */
/* 822 */	NdrFcShort( 0x4 ),	/* 4 */
/* 824 */	0x13, 0x0,	/* FC_OP */
/* 826 */	NdrFcShort( 0xffe6 ),	/* Offset= -26 (800) */
/* 828 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 830 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 832 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 834 */	NdrFcShort( 0x4 ),	/* 4 */
/* 836 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 838 */	NdrFcShort( 0x0 ),	/* 0 */
/* 840 */	NdrFcShort( 0x1 ),	/* Corr flags:  early, */
/* 842 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 844 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 846 */	NdrFcShort( 0x8 ),	/* 8 */
/* 848 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 850 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 852 */	NdrFcShort( 0x4 ),	/* 4 */
/* 854 */	NdrFcShort( 0x4 ),	/* 4 */
/* 856 */	0x13, 0x0,	/* FC_OP */
/* 858 */	NdrFcShort( 0xffe6 ),	/* Offset= -26 (832) */
/* 860 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 862 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 864 */	
			0x1b,		/* FC_CARRAY */
			0x7,		/* 7 */
/* 866 */	NdrFcShort( 0x8 ),	/* 8 */
/* 868 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 870 */	NdrFcShort( 0x0 ),	/* 0 */
/* 872 */	NdrFcShort( 0x1 ),	/* Corr flags:  early, */
/* 874 */	0xb,		/* FC_HYPER */
			0x5b,		/* FC_END */
/* 876 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 878 */	NdrFcShort( 0x8 ),	/* 8 */
/* 880 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 882 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 884 */	NdrFcShort( 0x4 ),	/* 4 */
/* 886 */	NdrFcShort( 0x4 ),	/* 4 */
/* 888 */	0x13, 0x0,	/* FC_OP */
/* 890 */	NdrFcShort( 0xffe6 ),	/* Offset= -26 (864) */
/* 892 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 894 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 896 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 898 */	NdrFcShort( 0x8 ),	/* 8 */
/* 900 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 902 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 904 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 906 */	NdrFcShort( 0x8 ),	/* 8 */
/* 908 */	0x7,		/* Corr desc: FC_USHORT */
			0x0,		/*  */
/* 910 */	NdrFcShort( 0xffd8 ),	/* -40 */
/* 912 */	NdrFcShort( 0x1 ),	/* Corr flags:  early, */
/* 914 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 916 */	NdrFcShort( 0xffec ),	/* Offset= -20 (896) */
/* 918 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 920 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 922 */	NdrFcShort( 0x28 ),	/* 40 */
/* 924 */	NdrFcShort( 0xffec ),	/* Offset= -20 (904) */
/* 926 */	NdrFcShort( 0x0 ),	/* Offset= 0 (926) */
/* 928 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 930 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 932 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 934 */	NdrFcShort( 0xfdde ),	/* Offset= -546 (388) */
/* 936 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 938 */	
			0x13, 0x0,	/* FC_OP */
/* 940 */	NdrFcShort( 0xfeea ),	/* Offset= -278 (662) */
/* 942 */	
			0x13, 0x8,	/* FC_OP [simple_pointer] */
/* 944 */	0x1,		/* FC_BYTE */
			0x5c,		/* FC_PAD */
/* 946 */	
			0x13, 0x8,	/* FC_OP [simple_pointer] */
/* 948 */	0x6,		/* FC_SHORT */
			0x5c,		/* FC_PAD */
/* 950 */	
			0x13, 0x8,	/* FC_OP [simple_pointer] */
/* 952 */	0x8,		/* FC_LONG */
			0x5c,		/* FC_PAD */
/* 954 */	
			0x13, 0x8,	/* FC_OP [simple_pointer] */
/* 956 */	0xb,		/* FC_HYPER */
			0x5c,		/* FC_PAD */
/* 958 */	
			0x13, 0x8,	/* FC_OP [simple_pointer] */
/* 960 */	0xa,		/* FC_FLOAT */
			0x5c,		/* FC_PAD */
/* 962 */	
			0x13, 0x8,	/* FC_OP [simple_pointer] */
/* 964 */	0xc,		/* FC_DOUBLE */
			0x5c,		/* FC_PAD */
/* 966 */	
			0x13, 0x0,	/* FC_OP */
/* 968 */	NdrFcShort( 0xfd82 ),	/* Offset= -638 (330) */
/* 970 */	
			0x13, 0x10,	/* FC_OP [pointer_deref] */
/* 972 */	NdrFcShort( 0xfd84 ),	/* Offset= -636 (336) */
/* 974 */	
			0x13, 0x10,	/* FC_OP [pointer_deref] */
/* 976 */	NdrFcShort( 0xfc36 ),	/* Offset= -970 (6) */
/* 978 */	
			0x13, 0x10,	/* FC_OP [pointer_deref] */
/* 980 */	NdrFcShort( 0xfd96 ),	/* Offset= -618 (362) */
/* 982 */	
			0x13, 0x10,	/* FC_OP [pointer_deref] */
/* 984 */	NdrFcShort( 0xfda4 ),	/* Offset= -604 (380) */
/* 986 */	
			0x13, 0x10,	/* FC_OP [pointer_deref] */
/* 988 */	NdrFcShort( 0x2 ),	/* Offset= 2 (990) */
/* 990 */	
			0x13, 0x0,	/* FC_OP */
/* 992 */	NdrFcShort( 0x14 ),	/* Offset= 20 (1012) */
/* 994 */	
			0x15,		/* FC_STRUCT */
			0x7,		/* 7 */
/* 996 */	NdrFcShort( 0x10 ),	/* 16 */
/* 998 */	0x6,		/* FC_SHORT */
			0x1,		/* FC_BYTE */
/* 1000 */	0x1,		/* FC_BYTE */
			0x8,		/* FC_LONG */
/* 1002 */	0xb,		/* FC_HYPER */
			0x5b,		/* FC_END */
/* 1004 */	
			0x13, 0x0,	/* FC_OP */
/* 1006 */	NdrFcShort( 0xfff4 ),	/* Offset= -12 (994) */
/* 1008 */	
			0x13, 0x8,	/* FC_OP [simple_pointer] */
/* 1010 */	0x2,		/* FC_CHAR */
			0x5c,		/* FC_PAD */
/* 1012 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x7,		/* 7 */
/* 1014 */	NdrFcShort( 0x20 ),	/* 32 */
/* 1016 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1018 */	NdrFcShort( 0x0 ),	/* Offset= 0 (1018) */
/* 1020 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1022 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 1024 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 1026 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 1028 */	NdrFcShort( 0xfc1c ),	/* Offset= -996 (32) */
/* 1030 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 1032 */	0xb4,		/* FC_USER_MARSHAL */
			0x83,		/* 131 */
/* 1034 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1036 */	NdrFcShort( 0x10 ),	/* 16 */
/* 1038 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1040 */	NdrFcShort( 0xfc0c ),	/* Offset= -1012 (28) */
/* 1042 */	
			0x11, 0xc,	/* FC_RP [alloced_on_stack] [simple_pointer] */
/* 1044 */	0x8,		/* FC_LONG */
			0x5c,		/* FC_PAD */
/* 1046 */	
			0x12, 0x0,	/* FC_UP */
/* 1048 */	NdrFcShort( 0xfd48 ),	/* Offset= -696 (352) */
/* 1050 */	0xb4,		/* FC_USER_MARSHAL */
			0x83,		/* 131 */
/* 1052 */	NdrFcShort( 0x1 ),	/* 1 */
/* 1054 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1056 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1058 */	NdrFcShort( 0xfff4 ),	/* Offset= -12 (1046) */
/* 1060 */	
			0x11, 0x4,	/* FC_RP [alloced_on_stack] */
/* 1062 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1064) */
/* 1064 */	0xb4,		/* FC_USER_MARSHAL */
			0x83,		/* 131 */
/* 1066 */	NdrFcShort( 0x1 ),	/* 1 */
/* 1068 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1070 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1072 */	NdrFcShort( 0xfd20 ),	/* Offset= -736 (336) */
/* 1074 */	
			0x11, 0x10,	/* FC_RP [pointer_deref] */
/* 1076 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1078) */
/* 1078 */	
			0x2f,		/* FC_IP */
			0x5a,		/* FC_CONSTANT_IID */
/* 1080 */	NdrFcLong( 0x833e4100 ),	/* -2093072128 */
/* 1084 */	NdrFcShort( 0xaff7 ),	/* -20489 */
/* 1086 */	NdrFcShort( 0x4ac3 ),	/* 19139 */
/* 1088 */	0xaa,		/* 170 */
			0xc2,		/* 194 */
/* 1090 */	0x9f,		/* 159 */
			0x24,		/* 36 */
/* 1092 */	0xc1,		/* 193 */
			0x45,		/* 69 */
/* 1094 */	0x7b,		/* 123 */
			0xce,		/* 206 */
/* 1096 */	
			0x11, 0x14,	/* FC_RP [alloced_on_stack] [pointer_deref] */
/* 1098 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1100) */
/* 1100 */	
			0x13, 0x8,	/* FC_OP [simple_pointer] */
/* 1102 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1104 */	
			0x11, 0x0,	/* FC_RP */
/* 1106 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1108) */
/* 1108 */	0xb4,		/* FC_USER_MARSHAL */
			0x83,		/* 131 */
/* 1110 */	NdrFcShort( 0x2 ),	/* 2 */
/* 1112 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1114 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1116 */	NdrFcShort( 0xfd20 ),	/* Offset= -736 (380) */
/* 1118 */	
			0x11, 0x8,	/* FC_RP [simple_pointer] */
/* 1120 */	0x8,		/* FC_LONG */
			0x5c,		/* FC_PAD */

			0x0
        }
    };

static const USER_MARSHAL_ROUTINE_QUADRUPLE UserMarshalRoutines[ WIRE_MARSHAL_TABLE_SIZE ] = 
        {
            
            {
            VARIANT_UserSize
            ,VARIANT_UserMarshal
            ,VARIANT_UserUnmarshal
            ,VARIANT_UserFree
            },
            {
            BSTR_UserSize
            ,BSTR_UserMarshal
            ,BSTR_UserUnmarshal
            ,BSTR_UserFree
            },
            {
            LPSAFEARRAY_UserSize
            ,LPSAFEARRAY_UserMarshal
            ,LPSAFEARRAY_UserUnmarshal
            ,LPSAFEARRAY_UserFree
            }

        };



/* Standard interface: __MIDL_itf_msra_0000_0000, ver. 0.0,
   GUID={0x00000000,0x0000,0x0000,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}} */


/* Object interface: IUnknown, ver. 0.0,
   GUID={0x00000000,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}} */


/* Object interface: IDispatch, ver. 0.0,
   GUID={0x00020400,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}} */


/* Object interface: IPCHCollection, ver. 0.0,
   GUID={0x833E4100,0xAFF7,0x4AC3,{0xAA,0xC2,0x9F,0x24,0xC1,0x45,0x7B,0xCE}} */

#pragma code_seg(".orpc")
static const unsigned short IPCHCollection_FormatStringOffsetTable[] =
    {
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    0,
    36,
    78
    };

static const MIDL_STUBLESS_PROXY_INFO IPCHCollection_ProxyInfo =
    {
    &Object_StubDesc,
    msra__MIDL_ProcFormatString.Format,
    &IPCHCollection_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO IPCHCollection_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    msra__MIDL_ProcFormatString.Format,
    &IPCHCollection_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(10) _IPCHCollectionProxyVtbl = 
{
    &IPCHCollection_ProxyInfo,
    &IID_IPCHCollection,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    0 /* (void *) (INT_PTR) -1 /* IDispatch::GetTypeInfoCount */ ,
    0 /* (void *) (INT_PTR) -1 /* IDispatch::GetTypeInfo */ ,
    0 /* (void *) (INT_PTR) -1 /* IDispatch::GetIDsOfNames */ ,
    0 /* IDispatch_Invoke_Proxy */ ,
    (void *) (INT_PTR) -1 /* IPCHCollection::get__NewEnum */ ,
    (void *) (INT_PTR) -1 /* IPCHCollection::get_Item */ ,
    (void *) (INT_PTR) -1 /* IPCHCollection::get_Count */
};


static const PRPC_STUB_FUNCTION IPCHCollection_table[] =
{
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2
};

CInterfaceStubVtbl _IPCHCollectionStubVtbl =
{
    &IID_IPCHCollection,
    &IPCHCollection_ServerInfo,
    10,
    &IPCHCollection_table[-3],
    CStdStubBuffer_DELEGATING_METHODS
};


/* Object interface: IPCHService, ver. 0.0,
   GUID={0x833E4200,0xAFF7,0x4AC3,{0xAA,0xC2,0x9F,0x24,0xC1,0x45,0x7B,0xCE}} */

#pragma code_seg(".orpc")
static const unsigned short IPCHService_FormatStringOffsetTable[] =
    {
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    114,
    144,
    174,
    204,
    234,
    264,
    294,
    324,
    354,
    384,
    414,
    444,
    474,
    534,
    570
    };

static const MIDL_STUBLESS_PROXY_INFO IPCHService_ProxyInfo =
    {
    &Object_StubDesc,
    msra__MIDL_ProcFormatString.Format,
    &IPCHService_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO IPCHService_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    msra__MIDL_ProcFormatString.Format,
    &IPCHService_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(22) _IPCHServiceProxyVtbl = 
{
    &IPCHService_ProxyInfo,
    &IID_IPCHService,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    0 /* (void *) (INT_PTR) -1 /* IDispatch::GetTypeInfoCount */ ,
    0 /* (void *) (INT_PTR) -1 /* IDispatch::GetTypeInfo */ ,
    0 /* (void *) (INT_PTR) -1 /* IDispatch::GetIDsOfNames */ ,
    0 /* IDispatch_Invoke_Proxy */ ,
    (void *) (INT_PTR) -1 /* IPCHService::Opnum7NotUsedByProtocol */ ,
    (void *) (INT_PTR) -1 /* IPCHService::Opnum8NotUsedByProtocol */ ,
    (void *) (INT_PTR) -1 /* IPCHService::Opnum9NotUsedByProtocol */ ,
    (void *) (INT_PTR) -1 /* IPCHService::Opnum10NotUsedByProtocol */ ,
    (void *) (INT_PTR) -1 /* IPCHService::Opnum11NotUsedByProtocol */ ,
    (void *) (INT_PTR) -1 /* IPCHService::Opnum12NotUsedByProtocol */ ,
    (void *) (INT_PTR) -1 /* IPCHService::Opnum13NotUsedByProtocol */ ,
    (void *) (INT_PTR) -1 /* IPCHService::Opnum14NotUsedByProtocol */ ,
    (void *) (INT_PTR) -1 /* IPCHService::Opnum15NotUsedByProtocol */ ,
    (void *) (INT_PTR) -1 /* IPCHService::Opnum16NotUsedByProtocol */ ,
    (void *) (INT_PTR) -1 /* IPCHService::Opnum17NotUsedByProtocol */ ,
    (void *) (INT_PTR) -1 /* IPCHService::Opnum18NotUsedByProtocol */ ,
    (void *) (INT_PTR) -1 /* IPCHService::RemoteConnectionParms */ ,
    (void *) (INT_PTR) -1 /* IPCHService::RemoteUserSessionInfo */ ,
    (void *) (INT_PTR) -1 /* IPCHService::Opnum21NotUsedByProtocol */
};


static const PRPC_STUB_FUNCTION IPCHService_table[] =
{
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2
};

CInterfaceStubVtbl _IPCHServiceStubVtbl =
{
    &IID_IPCHService,
    &IPCHService_ServerInfo,
    22,
    &IPCHService_table[-3],
    CStdStubBuffer_DELEGATING_METHODS
};


/* Object interface: ISAFSession, ver. 0.0,
   GUID={0x833E41AA,0xAFF7,0x4AC3,{0xAA,0xC2,0x9F,0x24,0xC1,0x45,0x7B,0xCE}} */

#pragma code_seg(".orpc")
static const unsigned short ISAFSession_FormatStringOffsetTable[] =
    {
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    600,
    636,
    78,
    672,
    708,
    744,
    780,
    816
    };

static const MIDL_STUBLESS_PROXY_INFO ISAFSession_ProxyInfo =
    {
    &Object_StubDesc,
    msra__MIDL_ProcFormatString.Format,
    &ISAFSession_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ISAFSession_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    msra__MIDL_ProcFormatString.Format,
    &ISAFSession_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(15) _ISAFSessionProxyVtbl = 
{
    &ISAFSession_ProxyInfo,
    &IID_ISAFSession,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    0 /* (void *) (INT_PTR) -1 /* IDispatch::GetTypeInfoCount */ ,
    0 /* (void *) (INT_PTR) -1 /* IDispatch::GetTypeInfo */ ,
    0 /* (void *) (INT_PTR) -1 /* IDispatch::GetIDsOfNames */ ,
    0 /* IDispatch_Invoke_Proxy */ ,
    (void *) (INT_PTR) -1 /* ISAFSession::get_SessionID */ ,
    (void *) (INT_PTR) -1 /* ISAFSession::put_SessionID */ ,
    (void *) (INT_PTR) -1 /* ISAFSession::get_SessionState */ ,
    (void *) (INT_PTR) -1 /* ISAFSession::put_SessionState */ ,
    (void *) (INT_PTR) -1 /* ISAFSession::get_DomainName */ ,
    (void *) (INT_PTR) -1 /* ISAFSession::put_DomainName */ ,
    (void *) (INT_PTR) -1 /* ISAFSession::get_UserName */ ,
    (void *) (INT_PTR) -1 /* ISAFSession::put_UserName */
};


static const PRPC_STUB_FUNCTION ISAFSession_table[] =
{
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2
};

CInterfaceStubVtbl _ISAFSessionStubVtbl =
{
    &IID_ISAFSession,
    &ISAFSession_ServerInfo,
    15,
    &ISAFSession_table[-3],
    CStdStubBuffer_DELEGATING_METHODS
};


/* Object interface: IRASrv, ver. 0.0,
   GUID={0xF120A684,0xB926,0x447F,{0x9D,0xF4,0xC9,0x66,0xCB,0x78,0x56,0x48}} */

#pragma code_seg(".orpc")
static const unsigned short IRASrv_FormatStringOffsetTable[] =
    {
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    852,
    888
    };

static const MIDL_STUBLESS_PROXY_INFO IRASrv_ProxyInfo =
    {
    &Object_StubDesc,
    msra__MIDL_ProcFormatString.Format,
    &IRASrv_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO IRASrv_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    msra__MIDL_ProcFormatString.Format,
    &IRASrv_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(9) _IRASrvProxyVtbl = 
{
    &IRASrv_ProxyInfo,
    &IID_IRASrv,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    0 /* (void *) (INT_PTR) -1 /* IDispatch::GetTypeInfoCount */ ,
    0 /* (void *) (INT_PTR) -1 /* IDispatch::GetTypeInfo */ ,
    0 /* (void *) (INT_PTR) -1 /* IDispatch::GetIDsOfNames */ ,
    0 /* IDispatch_Invoke_Proxy */ ,
    (void *) (INT_PTR) -1 /* IRASrv::GetNoviceUserInfo */ ,
    (void *) (INT_PTR) -1 /* IRASrv::GetSessionInfo */
};


static const PRPC_STUB_FUNCTION IRASrv_table[] =
{
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    NdrStubCall2,
    NdrStubCall2
};

CInterfaceStubVtbl _IRASrvStubVtbl =
{
    &IID_IRASrv,
    &IRASrv_ServerInfo,
    9,
    &IRASrv_table[-3],
    CStdStubBuffer_DELEGATING_METHODS
};

static const MIDL_STUB_DESC Object_StubDesc = 
    {
    0,
    NdrOleAllocate,
    NdrOleFree,
    0,
    0,
    0,
    0,
    0,
    msra__MIDL_TypeFormatString.Format,
    0, /* -error bounds_check flag */
    0x50002, /* Ndr library version */
    0,
    0x70001f4, /* MIDL Version 7.0.500 */
    0,
    UserMarshalRoutines,
    0,  /* notify & notify_flag routine table */
    0x1, /* MIDL flag */
    0, /* cs routines */
    0,   /* proxy/server info */
    0
    };

const CInterfaceProxyVtbl * _msra_ProxyVtblList[] = 
{
    ( CInterfaceProxyVtbl *) &_IPCHCollectionProxyVtbl,
    ( CInterfaceProxyVtbl *) &_IPCHServiceProxyVtbl,
    ( CInterfaceProxyVtbl *) &_IRASrvProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ISAFSessionProxyVtbl,
    0
};

const CInterfaceStubVtbl * _msra_StubVtblList[] = 
{
    ( CInterfaceStubVtbl *) &_IPCHCollectionStubVtbl,
    ( CInterfaceStubVtbl *) &_IPCHServiceStubVtbl,
    ( CInterfaceStubVtbl *) &_IRASrvStubVtbl,
    ( CInterfaceStubVtbl *) &_ISAFSessionStubVtbl,
    0
};

PCInterfaceName const _msra_InterfaceNamesList[] = 
{
    "IPCHCollection",
    "IPCHService",
    "IRASrv",
    "ISAFSession",
    0
};

const IID *  _msra_BaseIIDList[] = 
{
    &IID_IDispatch,
    &IID_IDispatch,
    &IID_IDispatch,
    &IID_IDispatch,
    0
};


#define _msra_CHECK_IID(n)	IID_GENERIC_CHECK_IID( _msra, pIID, n)

int __stdcall _msra_IID_Lookup( const IID * pIID, int * pIndex )
{
    IID_BS_LOOKUP_SETUP

    IID_BS_LOOKUP_INITIAL_TEST( _msra, 4, 2 )
    IID_BS_LOOKUP_NEXT_TEST( _msra, 1 )
    IID_BS_LOOKUP_RETURN_RESULT( _msra, 4, *pIndex )
    
}

const ExtendedProxyFileInfo msra_ProxyFileInfo = 
{
    (PCInterfaceProxyVtblList *) & _msra_ProxyVtblList,
    (PCInterfaceStubVtblList *) & _msra_StubVtblList,
    (const PCInterfaceName * ) & _msra_InterfaceNamesList,
    (const IID ** ) & _msra_BaseIIDList,
    & _msra_IID_Lookup, 
    4,
    2,
    0, /* table of [async_uuid] interfaces */
    0, /* Filler1 */
    0, /* Filler2 */
    0  /* Filler3 */
};
#pragma optimize("", on )
#if _MSC_VER >= 1200
#pragma warning(pop)
#endif


#endif /* !defined(_M_IA64) && !defined(_M_AMD64)*/

