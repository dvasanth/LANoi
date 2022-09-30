#include "StdAfx.h"
#include "LOITCPIPv6Socket.h"

CLOITCPIPv6Socket::CLOITCPIPv6Socket(PROVIDER *Provider ):CLSPOverlappedSocket(Provider) 
{
	Init();
}

CLOITCPIPv6Socket::~CLOITCPIPv6Socket(void)
{
	Init();
}
CLOITCPIPv6Socket::CLOITCPIPv6Socket (PROVIDER *Provider ,SOCKET ProviderSocket):CLSPOverlappedSocket(Provider,ProviderSocket) 
{
	Init();
}

void CLOITCPIPv6Socket::Init()
{
}
void CLOITCPIPv6Socket::Inherit(CLSPSocket *pAssignee)
{
	//copy all existsing parameters to this object happens for accepted socket
	CLOITCPIPv6Socket		*pIPv6TcpSocket=dynamic_cast<CLOITCPIPv6Socket*>(pAssignee);

	if(pIPv6TcpSocket)
	{
		//do memeber intialising
	}
	CLSPOverlappedSocket::Inherit(pAssignee);
}

int WSPAPI CLOITCPIPv6Socket::WSPBind(
								const struct sockaddr FAR * name,
								int                   namelen,
								LPINT                 lpErrno
								)
{
	int   iRet=SOCKET_ERROR;

	iRet=CLSPOverlappedSocket::WSPBind(name,namelen,lpErrno);
	if(iRet == SOCKET_ERROR &&
		*lpErrno==WSAEADDRINUSE )
	{
		//may be the ipv6 addr & port is used by us
		//for ex media encoder listens in ipv4:port ipv6:port if we binded ipv4:port to ipv6:port
		//when he performs ipv6:port bind call will fails
		//just increamneted the port value
		USHORT usPort=	((sockaddr_in*)name)->sin_port ;
		USHORT usHostOrderPort;
		usHostOrderPort=ntohs(usPort);
		usHostOrderPort++;
		((sockaddr_in*)name)->sin_port =htons(usHostOrderPort);
		//try again with new port value
		return CLSPOverlappedSocket::WSPBind(name,namelen,lpErrno);
	}

	return 	iRet;
}