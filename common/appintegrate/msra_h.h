

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


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


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __msra_h_h__
#define __msra_h_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IPCHCollection_FWD_DEFINED__
#define __IPCHCollection_FWD_DEFINED__
typedef interface IPCHCollection IPCHCollection;
#endif 	/* __IPCHCollection_FWD_DEFINED__ */


#ifndef __IPCHService_FWD_DEFINED__
#define __IPCHService_FWD_DEFINED__
typedef interface IPCHService IPCHService;
#endif 	/* __IPCHService_FWD_DEFINED__ */


#ifndef __ISAFSession_FWD_DEFINED__
#define __ISAFSession_FWD_DEFINED__
typedef interface ISAFSession ISAFSession;
#endif 	/* __ISAFSession_FWD_DEFINED__ */


#ifndef __IRASrv_FWD_DEFINED__
#define __IRASrv_FWD_DEFINED__
typedef interface IRASrv IRASrv;
#endif 	/* __IRASrv_FWD_DEFINED__ */


#ifndef __PCHService_FWD_DEFINED__
#define __PCHService_FWD_DEFINED__

#ifdef __cplusplus
typedef class PCHService PCHService;
#else
typedef struct PCHService PCHService;
#endif /* __cplusplus */

#endif 	/* __PCHService_FWD_DEFINED__ */


#ifndef __RASrv_FWD_DEFINED__
#define __RASrv_FWD_DEFINED__

#ifdef __cplusplus
typedef class RASrv RASrv;
#else
typedef struct RASrv RASrv;
#endif /* __cplusplus */

#endif 	/* __RASrv_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


/* interface __MIDL_itf_msra_0000_0000 */
/* [local] */ 

typedef /* [public] */ 
enum __MIDL___MIDL_itf_msra_0000_0000_0001
    {	pchActive	= 0,
	pchConnected	= 1,
	pchConnectQuery	= 2,
	pchShadow	= 3,
	pchDisconnected	= 4,
	pchIdle	= 5,
	pchListen	= 6,
	pchReset	= 7,
	pchDown	= 8,
	pchInit	= 9,
	pchStateInvalid	= 10
    } 	SessionStateEnum;



extern RPC_IF_HANDLE __MIDL_itf_msra_0000_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_msra_0000_0000_v0_0_s_ifspec;

#ifndef __IPCHCollection_INTERFACE_DEFINED__
#define __IPCHCollection_INTERFACE_DEFINED__

/* interface IPCHCollection */
/* [unique][helpstring][oleautomation][dual][uuid][object] */ 


EXTERN_C const IID IID_IPCHCollection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("833E4100-AFF7-4AC3-AAC2-9F24C1457BCE")
    IPCHCollection : public IDispatch
    {
    public:
        virtual /* [id][propget] */ HRESULT STDMETHODCALLTYPE get__NewEnum( 
            /* [retval][out] */ IUnknown **pVal) = 0;
        
        virtual /* [id][propget] */ HRESULT STDMETHODCALLTYPE get_Item( 
            /* [in] */ long vIndex,
            /* [retval][out] */ VARIANT *ppEntry) = 0;
        
        virtual /* [id][propget] */ HRESULT STDMETHODCALLTYPE get_Count( 
            /* [retval][out] */ long *pVal) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IPCHCollectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPCHCollection * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPCHCollection * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPCHCollection * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IPCHCollection * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IPCHCollection * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IPCHCollection * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IPCHCollection * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [id][propget] */ HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IPCHCollection * This,
            /* [retval][out] */ IUnknown **pVal);
        
        /* [id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IPCHCollection * This,
            /* [in] */ long vIndex,
            /* [retval][out] */ VARIANT *ppEntry);
        
        /* [id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IPCHCollection * This,
            /* [retval][out] */ long *pVal);
        
        END_INTERFACE
    } IPCHCollectionVtbl;

    interface IPCHCollection
    {
        CONST_VTBL struct IPCHCollectionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPCHCollection_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IPCHCollection_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IPCHCollection_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IPCHCollection_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IPCHCollection_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IPCHCollection_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IPCHCollection_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IPCHCollection_get__NewEnum(This,pVal)	\
    ( (This)->lpVtbl -> get__NewEnum(This,pVal) ) 

#define IPCHCollection_get_Item(This,vIndex,ppEntry)	\
    ( (This)->lpVtbl -> get_Item(This,vIndex,ppEntry) ) 

#define IPCHCollection_get_Count(This,pVal)	\
    ( (This)->lpVtbl -> get_Count(This,pVal) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IPCHCollection_INTERFACE_DEFINED__ */


#ifndef __IPCHService_INTERFACE_DEFINED__
#define __IPCHService_INTERFACE_DEFINED__

/* interface IPCHService */
/* [unique][helpstring][oleautomation][dual][uuid][object] */ 


EXTERN_C const IID IID_IPCHService;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("833E4200-AFF7-4AC3-AAC2-9F24C1457BCE")
    IPCHService : public IDispatch
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Opnum7NotUsedByProtocol( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Opnum8NotUsedByProtocol( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Opnum9NotUsedByProtocol( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Opnum10NotUsedByProtocol( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Opnum11NotUsedByProtocol( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Opnum12NotUsedByProtocol( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Opnum13NotUsedByProtocol( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Opnum14NotUsedByProtocol( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Opnum15NotUsedByProtocol( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Opnum16NotUsedByProtocol( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Opnum17NotUsedByProtocol( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Opnum18NotUsedByProtocol( void) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE RemoteConnectionParms( 
            /* [in] */ BSTR bstrUserName,
            /* [in] */ BSTR bstrDomainName,
            /* [in] */ long lSessionID,
            /* [in] */ BSTR bstrUserHelpBlob,
            /* [retval][out] */ BSTR *pbstrConnectionString) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE RemoteUserSessionInfo( 
            /* [retval][out] */ IPCHCollection **pVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Opnum21NotUsedByProtocol( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IPCHServiceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPCHService * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPCHService * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPCHService * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IPCHService * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IPCHService * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IPCHService * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IPCHService * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        HRESULT ( STDMETHODCALLTYPE *Opnum7NotUsedByProtocol )( 
            IPCHService * This);
        
        HRESULT ( STDMETHODCALLTYPE *Opnum8NotUsedByProtocol )( 
            IPCHService * This);
        
        HRESULT ( STDMETHODCALLTYPE *Opnum9NotUsedByProtocol )( 
            IPCHService * This);
        
        HRESULT ( STDMETHODCALLTYPE *Opnum10NotUsedByProtocol )( 
            IPCHService * This);
        
        HRESULT ( STDMETHODCALLTYPE *Opnum11NotUsedByProtocol )( 
            IPCHService * This);
        
        HRESULT ( STDMETHODCALLTYPE *Opnum12NotUsedByProtocol )( 
            IPCHService * This);
        
        HRESULT ( STDMETHODCALLTYPE *Opnum13NotUsedByProtocol )( 
            IPCHService * This);
        
        HRESULT ( STDMETHODCALLTYPE *Opnum14NotUsedByProtocol )( 
            IPCHService * This);
        
        HRESULT ( STDMETHODCALLTYPE *Opnum15NotUsedByProtocol )( 
            IPCHService * This);
        
        HRESULT ( STDMETHODCALLTYPE *Opnum16NotUsedByProtocol )( 
            IPCHService * This);
        
        HRESULT ( STDMETHODCALLTYPE *Opnum17NotUsedByProtocol )( 
            IPCHService * This);
        
        HRESULT ( STDMETHODCALLTYPE *Opnum18NotUsedByProtocol )( 
            IPCHService * This);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *RemoteConnectionParms )( 
            IPCHService * This,
            /* [in] */ BSTR bstrUserName,
            /* [in] */ BSTR bstrDomainName,
            /* [in] */ long lSessionID,
            /* [in] */ BSTR bstrUserHelpBlob,
            /* [retval][out] */ BSTR *pbstrConnectionString);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *RemoteUserSessionInfo )( 
            IPCHService * This,
            /* [retval][out] */ IPCHCollection **pVal);
        
        HRESULT ( STDMETHODCALLTYPE *Opnum21NotUsedByProtocol )( 
            IPCHService * This);
        
        END_INTERFACE
    } IPCHServiceVtbl;

    interface IPCHService
    {
        CONST_VTBL struct IPCHServiceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPCHService_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IPCHService_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IPCHService_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IPCHService_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IPCHService_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IPCHService_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IPCHService_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IPCHService_Opnum7NotUsedByProtocol(This)	\
    ( (This)->lpVtbl -> Opnum7NotUsedByProtocol(This) ) 

#define IPCHService_Opnum8NotUsedByProtocol(This)	\
    ( (This)->lpVtbl -> Opnum8NotUsedByProtocol(This) ) 

#define IPCHService_Opnum9NotUsedByProtocol(This)	\
    ( (This)->lpVtbl -> Opnum9NotUsedByProtocol(This) ) 

#define IPCHService_Opnum10NotUsedByProtocol(This)	\
    ( (This)->lpVtbl -> Opnum10NotUsedByProtocol(This) ) 

#define IPCHService_Opnum11NotUsedByProtocol(This)	\
    ( (This)->lpVtbl -> Opnum11NotUsedByProtocol(This) ) 

#define IPCHService_Opnum12NotUsedByProtocol(This)	\
    ( (This)->lpVtbl -> Opnum12NotUsedByProtocol(This) ) 

#define IPCHService_Opnum13NotUsedByProtocol(This)	\
    ( (This)->lpVtbl -> Opnum13NotUsedByProtocol(This) ) 

#define IPCHService_Opnum14NotUsedByProtocol(This)	\
    ( (This)->lpVtbl -> Opnum14NotUsedByProtocol(This) ) 

#define IPCHService_Opnum15NotUsedByProtocol(This)	\
    ( (This)->lpVtbl -> Opnum15NotUsedByProtocol(This) ) 

#define IPCHService_Opnum16NotUsedByProtocol(This)	\
    ( (This)->lpVtbl -> Opnum16NotUsedByProtocol(This) ) 

#define IPCHService_Opnum17NotUsedByProtocol(This)	\
    ( (This)->lpVtbl -> Opnum17NotUsedByProtocol(This) ) 

#define IPCHService_Opnum18NotUsedByProtocol(This)	\
    ( (This)->lpVtbl -> Opnum18NotUsedByProtocol(This) ) 

#define IPCHService_RemoteConnectionParms(This,bstrUserName,bstrDomainName,lSessionID,bstrUserHelpBlob,pbstrConnectionString)	\
    ( (This)->lpVtbl -> RemoteConnectionParms(This,bstrUserName,bstrDomainName,lSessionID,bstrUserHelpBlob,pbstrConnectionString) ) 

#define IPCHService_RemoteUserSessionInfo(This,pVal)	\
    ( (This)->lpVtbl -> RemoteUserSessionInfo(This,pVal) ) 

#define IPCHService_Opnum21NotUsedByProtocol(This)	\
    ( (This)->lpVtbl -> Opnum21NotUsedByProtocol(This) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IPCHService_INTERFACE_DEFINED__ */


#ifndef __ISAFSession_INTERFACE_DEFINED__
#define __ISAFSession_INTERFACE_DEFINED__

/* interface ISAFSession */
/* [unique][helpstring][oleautomation][dual][uuid][object] */ 


EXTERN_C const IID IID_ISAFSession;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("833E41AA-AFF7-4AC3-AAC2-9F24C1457BCE")
    ISAFSession : public IDispatch
    {
    public:
        virtual /* [id][propget] */ HRESULT STDMETHODCALLTYPE get_SessionID( 
            /* [retval][out] */ DWORD *pVal) = 0;
        
        virtual /* [id][propput] */ HRESULT STDMETHODCALLTYPE put_SessionID( 
            /* [in] */ DWORD pVal) = 0;
        
        virtual /* [id][propget] */ HRESULT STDMETHODCALLTYPE get_SessionState( 
            /* [retval][out] */ int *pVal) = 0;
        
        virtual /* [id][propput] */ HRESULT STDMETHODCALLTYPE put_SessionState( 
            /* [in] */ int pVal) = 0;
        
        virtual /* [id][propget] */ HRESULT STDMETHODCALLTYPE get_DomainName( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [id][propput] */ HRESULT STDMETHODCALLTYPE put_DomainName( 
            /* [in] */ BSTR pVal) = 0;
        
        virtual /* [id][propget] */ HRESULT STDMETHODCALLTYPE get_UserName( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [id][propput] */ HRESULT STDMETHODCALLTYPE put_UserName( 
            /* [in] */ BSTR pVal) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISAFSessionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISAFSession * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISAFSession * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISAFSession * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISAFSession * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISAFSession * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISAFSession * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISAFSession * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_SessionID )( 
            ISAFSession * This,
            /* [retval][out] */ DWORD *pVal);
        
        /* [id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_SessionID )( 
            ISAFSession * This,
            /* [in] */ DWORD pVal);
        
        /* [id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_SessionState )( 
            ISAFSession * This,
            /* [retval][out] */ int *pVal);
        
        /* [id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_SessionState )( 
            ISAFSession * This,
            /* [in] */ int pVal);
        
        /* [id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_DomainName )( 
            ISAFSession * This,
            /* [retval][out] */ BSTR *pVal);
        
        /* [id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_DomainName )( 
            ISAFSession * This,
            /* [in] */ BSTR pVal);
        
        /* [id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_UserName )( 
            ISAFSession * This,
            /* [retval][out] */ BSTR *pVal);
        
        /* [id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_UserName )( 
            ISAFSession * This,
            /* [in] */ BSTR pVal);
        
        END_INTERFACE
    } ISAFSessionVtbl;

    interface ISAFSession
    {
        CONST_VTBL struct ISAFSessionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISAFSession_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ISAFSession_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ISAFSession_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ISAFSession_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define ISAFSession_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define ISAFSession_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define ISAFSession_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define ISAFSession_get_SessionID(This,pVal)	\
    ( (This)->lpVtbl -> get_SessionID(This,pVal) ) 

#define ISAFSession_put_SessionID(This,pVal)	\
    ( (This)->lpVtbl -> put_SessionID(This,pVal) ) 

#define ISAFSession_get_SessionState(This,pVal)	\
    ( (This)->lpVtbl -> get_SessionState(This,pVal) ) 

#define ISAFSession_put_SessionState(This,pVal)	\
    ( (This)->lpVtbl -> put_SessionState(This,pVal) ) 

#define ISAFSession_get_DomainName(This,pVal)	\
    ( (This)->lpVtbl -> get_DomainName(This,pVal) ) 

#define ISAFSession_put_DomainName(This,pVal)	\
    ( (This)->lpVtbl -> put_DomainName(This,pVal) ) 

#define ISAFSession_get_UserName(This,pVal)	\
    ( (This)->lpVtbl -> get_UserName(This,pVal) ) 

#define ISAFSession_put_UserName(This,pVal)	\
    ( (This)->lpVtbl -> put_UserName(This,pVal) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ISAFSession_INTERFACE_DEFINED__ */


#ifndef __IRASrv_INTERFACE_DEFINED__
#define __IRASrv_INTERFACE_DEFINED__

/* interface IRASrv */
/* [unique][helpstring][nonextensible][dual][uuid][object] */ 


EXTERN_C const IID IID_IRASrv;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F120A684-B926-447F-9DF4-C966CB785648")
    IRASrv : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetNoviceUserInfo( 
            /* [out][in] */ LPWSTR *szName) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetSessionInfo( 
            /* [out][in] */ SAFEARRAY * *UserNames,
            /* [out][in] */ INT *Count) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRASrvVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRASrv * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRASrv * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRASrv * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IRASrv * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IRASrv * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IRASrv * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IRASrv * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetNoviceUserInfo )( 
            IRASrv * This,
            /* [out][in] */ LPWSTR *szName);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetSessionInfo )( 
            IRASrv * This,
            /* [out][in] */ SAFEARRAY * *UserNames,
            /* [out][in] */ INT *Count);
        
        END_INTERFACE
    } IRASrvVtbl;

    interface IRASrv
    {
        CONST_VTBL struct IRASrvVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRASrv_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IRASrv_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IRASrv_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IRASrv_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IRASrv_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IRASrv_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IRASrv_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IRASrv_GetNoviceUserInfo(This,szName)	\
    ( (This)->lpVtbl -> GetNoviceUserInfo(This,szName) ) 

#define IRASrv_GetSessionInfo(This,UserNames,Count)	\
    ( (This)->lpVtbl -> GetSessionInfo(This,UserNames,Count) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IRASrv_INTERFACE_DEFINED__ */


/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

unsigned long             __RPC_USER  LPSAFEARRAY_UserSize(     unsigned long *, unsigned long            , LPSAFEARRAY * ); 
unsigned char * __RPC_USER  LPSAFEARRAY_UserMarshal(  unsigned long *, unsigned char *, LPSAFEARRAY * ); 
unsigned char * __RPC_USER  LPSAFEARRAY_UserUnmarshal(unsigned long *, unsigned char *, LPSAFEARRAY * ); 
void                      __RPC_USER  LPSAFEARRAY_UserFree(     unsigned long *, LPSAFEARRAY * ); 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long *, unsigned long            , VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserMarshal(  unsigned long *, unsigned char *, VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserUnmarshal(unsigned long *, unsigned char *, VARIANT * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long *, VARIANT * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


