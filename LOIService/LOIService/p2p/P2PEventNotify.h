#include <winsock2.h>
#include <Ws2tcpip.h>

class IP2PEventNotify{
public:
	virtual	BOOL			IsAuthenticatedPeer(in_addr6 IPv6Address,USHORT usPort=0);
	virtual void			OnReverseProxyStop();
};