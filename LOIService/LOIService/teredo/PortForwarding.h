#pragma once
#include <Natupnp.h>
class CPortForwarding
{
public:
	CPortForwarding(void);
	~CPortForwarding(void);
	BOOL				Init(USHORT  usExternalPort);
	void				Close();
private:
	static const CString			 PROTOCOL_NAME;
	static const CString			 MAP_DESCRIPTION;
	IUPnPNAT						*m_pUPnPNAT; 
	IStaticPortMappingCollection	*m_PortMapCollection; 
	IStaticPortMapping				*m_pPortForwardedMap; 
	USHORT							 m_usForwardingPort;
	HRESULT							 ForwardForInternalAddress(IStaticPortMappingCollection *pPortMapCollection,USHORT usForwardingPort,IStaticPortMapping	**pPortForwardedMap);

};
