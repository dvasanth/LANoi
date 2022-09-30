#pragma once

#include <winsock2.h>
#include <WS2TCPIP.H >

//Type field of Query and Answer
#define T_A 1 /* host address */
#define T_NS 2 /* authoritative server */
#define T_CNAME 5 /* canonical name */
#define T_SOA 6 /* start of authority zone */
#define T_PTR 12 /* domain name pointer */
#define T_MX 15 /* mail routing information */

//Function Prototypes
void			ngethostbyname (unsigned char*);
void			ChangetoDnsNameFormat (unsigned char*,unsigned char*);
unsigned char*	ReadName (unsigned char*,unsigned char*,int*);
void			RetrieveDnsServersFromRegistry(void);
unsigned char*  PrepareDnsQueryPacket (unsigned char*);

//DNS header structure
struct DNS_HEADER
{
	unsigned short id; // identification number

	unsigned char rd :1; // recursion desired
	unsigned char tc :1; // truncated message
	unsigned char aa :1; // authoritive answer
	unsigned char opcode :4; // purpose of message
	unsigned char qr :1; // query/response flag

	unsigned char rcode :4; // response code
	unsigned char cd :1; // checking disabled
	unsigned char ad :1; // authenticated data
	unsigned char z :1; // its z! reserved
	unsigned char ra :1; // recursion available

	unsigned short q_count; // number of question entries
	unsigned short ans_count; // number of answer entries
	unsigned short auth_count; // number of authority entries
	unsigned short add_count; // number of resource entries
};

//Constant sized fields of query structure
struct QUESTION
{
	unsigned short qtype;
	unsigned short qclass;
};

//Constant sized fields of the resource record structure
#pragma pack(push, 1)
struct R_DATA
{
	unsigned short type;
	unsigned short _class;
	unsigned int ttl;
	unsigned short data_len;
};
#pragma pack(pop)

//Pointers to resource record contents
struct RES_RECORD
{
	unsigned char *name;
	struct R_DATA *resource;
	unsigned char *rdata;
};

//Structure of a Query
typedef struct
{
	unsigned char *name;
	struct QUESTION *ques;
} QUERY;


class CIPv6DNSResolver
{
public:
	CIPv6DNSResolver(void);
	~CIPv6DNSResolver(void);
	BOOL			IsResolvable(LPCTSTR lpszDNSName);
	BOOL			ManualResolve(LPCTSTR lpszDNSName,in_addr6 &IPv6Addr);
private:
	static const CHAR			GOOGLE_DNS_SERVER[];
	BOOL						ProcessReply(BYTE *byReply,DWORD dwReplySize,DWORD dwRequestSize,in_addr6 &IPv6Addr);
	DWORD						FillDNSRequest(LPCTSTR lpszDNSName,BYTE *byPacket,DWORD dwLen);
	void						ChangetoDnsNameFormat(char* dns, char* host);
};
