

/* this ALWAYS GENERATED file contains the IIDs and CLSIDs */

/* link this file in with the server and any clients */


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

#pragma warning( disable: 4049 )  /* more than 64k source lines */


#ifdef __cplusplus
extern "C"{
#endif 


#include <rpc.h>
#include <rpcndr.h>

#ifdef _MIDL_USE_GUIDDEF_

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)

#else // !_MIDL_USE_GUIDDEF_

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

#endif !_MIDL_USE_GUIDDEF_

MIDL_DEFINE_GUID(IID, IID_IPCHCollection,0x833E4100,0xAFF7,0x4AC3,0xAA,0xC2,0x9F,0x24,0xC1,0x45,0x7B,0xCE);


MIDL_DEFINE_GUID(IID, IID_IPCHService,0x833E4200,0xAFF7,0x4AC3,0xAA,0xC2,0x9F,0x24,0xC1,0x45,0x7B,0xCE);


MIDL_DEFINE_GUID(IID, IID_ISAFSession,0x833E41AA,0xAFF7,0x4AC3,0xAA,0xC2,0x9F,0x24,0xC1,0x45,0x7B,0xCE);


MIDL_DEFINE_GUID(IID, IID_IRASrv,0xF120A684,0xB926,0x447F,0x9D,0xF4,0xC9,0x66,0xCB,0x78,0x56,0x48);


MIDL_DEFINE_GUID(CLSID, CLSID_PCHService,0x833E4010,0xAFF7,0x4AC3,0xAA,0xC2,0x9F,0x24,0xC1,0x45,0x7B,0xCE);


MIDL_DEFINE_GUID(CLSID, CLSID_RASrv,0x3C3A70A7,0xA468,0x49B9,0x8A,0xDA,0x28,0xE1,0x1F,0xCC,0xAD,0x5D);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



