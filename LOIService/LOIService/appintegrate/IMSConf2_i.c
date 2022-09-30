

/* this ALWAYS GENERATED file contains the IIDs and CLSIDs */

/* link this file in with the server and any clients */


 /* File created by MIDL compiler version 7.00.0500 */
/* at Sun Jan 25 04:16:57 2009
 */
/* Compiler settings for .\imsconf2.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
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

MIDL_DEFINE_GUID(IID, IID_INmManager,0x068B0701,0x718C,0x11d0,0x8B,0x1A,0x00,0xA0,0xC9,0x1B,0xC9,0x0E);


MIDL_DEFINE_GUID(IID, IID_INmManagerNotify,0x068B0702,0x718C,0x11d0,0x8B,0x1A,0x00,0xA0,0xC9,0x1B,0xC9,0x0E);


MIDL_DEFINE_GUID(IID, IID_INmSysInfo,0x068B0703,0x718C,0x11d0,0x8B,0x1A,0x00,0xA0,0xC9,0x1B,0xC9,0x0E);


MIDL_DEFINE_GUID(IID, IID_INmCall,0x068B0704,0x718C,0x11d0,0x8B,0x1A,0x00,0xA0,0xC9,0x1B,0xC9,0x0E);


MIDL_DEFINE_GUID(IID, IID_INmCallNotify,0x068B0705,0x718C,0x11d0,0x8B,0x1A,0x00,0xA0,0xC9,0x1B,0xC9,0x0E);


MIDL_DEFINE_GUID(IID, IID_INmConference,0x068B0710,0x718C,0x11d0,0x8B,0x1A,0x00,0xA0,0xC9,0x1B,0xC9,0x0E);


MIDL_DEFINE_GUID(IID, IID_INmConferenceNotify,0x068B0711,0x718C,0x11d0,0x8B,0x1A,0x00,0xA0,0xC9,0x1B,0xC9,0x0E);


MIDL_DEFINE_GUID(IID, IID_INmMember,0x068B0712,0x718C,0x11d0,0x8B,0x1A,0x00,0xA0,0xC9,0x1B,0xC9,0x0E);


MIDL_DEFINE_GUID(IID, IID_INmChannel,0x068B0720,0x718C,0x11d0,0x8B,0x1A,0x00,0xA0,0xC9,0x1B,0xC9,0x0E);


MIDL_DEFINE_GUID(IID, IID_INmChannelNotify,0x068B0721,0x718C,0x11d0,0x8B,0x1A,0x00,0xA0,0xC9,0x1B,0xC9,0x0E);


MIDL_DEFINE_GUID(IID, IID_INmChannelData,0x068B0722,0x718C,0x11d0,0x8B,0x1A,0x00,0xA0,0xC9,0x1B,0xC9,0x0E);


MIDL_DEFINE_GUID(IID, IID_INmChannelDataNotify,0x068B0723,0x718C,0x11d0,0x8B,0x1A,0x00,0xA0,0xC9,0x1B,0xC9,0x0E);


MIDL_DEFINE_GUID(IID, IID_INmChannelAudio,0x068B0724,0x718C,0x11d0,0x8B,0x1A,0x00,0xA0,0xC9,0x1B,0xC9,0x0E);


MIDL_DEFINE_GUID(IID, IID_INmChannelAudioNotify,0x068B0725,0x718C,0x11d0,0x8B,0x1A,0x00,0xA0,0xC9,0x1B,0xC9,0x0E);


MIDL_DEFINE_GUID(IID, IID_INmChannelVideo,0x068B0726,0x718C,0x11d0,0x8B,0x1A,0x00,0xA0,0xC9,0x1B,0xC9,0x0E);


MIDL_DEFINE_GUID(IID, IID_INmChannelVideoNotify,0x068B0727,0x718C,0x11d0,0x8B,0x1A,0x00,0xA0,0xC9,0x1B,0xC9,0x0E);


MIDL_DEFINE_GUID(IID, IID_INmChannelFt,0x068B0728,0x718C,0x11d0,0x8B,0x1A,0x00,0xA0,0xC9,0x1B,0xC9,0x0E);


MIDL_DEFINE_GUID(IID, IID_INmChannelFtNotify,0x068B0729,0x718C,0x11d0,0x8B,0x1A,0x00,0xA0,0xC9,0x1B,0xC9,0x0E);


MIDL_DEFINE_GUID(IID, IID_INmFt,0x068B0732,0x718C,0x11d0,0x8B,0x1A,0x00,0xA0,0xC9,0x1B,0xC9,0x0E);


MIDL_DEFINE_GUID(IID, IID_INmChannelAppShare,0x068B072A,0x718C,0x11d0,0x8B,0x1A,0x00,0xA0,0xC9,0x1B,0xC9,0x0E);


MIDL_DEFINE_GUID(IID, IID_INmChannelAppShareNotify,0x068B072B,0x718C,0x11d0,0x8B,0x1A,0x00,0xA0,0xC9,0x1B,0xC9,0x0E);


MIDL_DEFINE_GUID(IID, IID_INmSharableApp,0x068B0734,0x718C,0x11d0,0x8B,0x1A,0x00,0xA0,0xC9,0x1B,0xC9,0x0E);


MIDL_DEFINE_GUID(IID, IID_IEnumNmConference,0x068B0741,0x718C,0x11d0,0x8B,0x1A,0x00,0xA0,0xC9,0x1B,0xC9,0x0E);


MIDL_DEFINE_GUID(IID, IID_IEnumNmMember,0x068B0742,0x718C,0x11d0,0x8B,0x1A,0x00,0xA0,0xC9,0x1B,0xC9,0x0E);


MIDL_DEFINE_GUID(IID, IID_IEnumNmChannel,0x068B0743,0x718C,0x11d0,0x8B,0x1A,0x00,0xA0,0xC9,0x1B,0xC9,0x0E);


MIDL_DEFINE_GUID(IID, IID_IEnumNmCall,0x068B0744,0x718C,0x11d0,0x8B,0x1A,0x00,0xA0,0xC9,0x1B,0xC9,0x0E);


MIDL_DEFINE_GUID(IID, IID_IEnumNmSharableApp,0x068B0745,0x718C,0x11d0,0x8B,0x1A,0x00,0xA0,0xC9,0x1B,0xC9,0x0E);


MIDL_DEFINE_GUID(IID, IID_INmObject,0x068B0780,0x718C,0x11d0,0x8B,0x1A,0x00,0xA0,0xC9,0x1B,0xC9,0x0E);


MIDL_DEFINE_GUID(IID, LIBID_NmManager,0x068B07FF,0x718C,0x11d0,0x8B,0x1A,0x00,0xA0,0xC9,0x1B,0xC9,0x0E);


MIDL_DEFINE_GUID(CLSID, CLSID_NmManager,0x068B0700,0x718C,0x11d0,0x8B,0x1A,0x00,0xA0,0xC9,0x1B,0xC9,0x0E);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



