#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "pch.h"
#include "BaseLayer.h"
#include <unordered_map>
#include <string>

class CARPLayer
	: public CBaseLayer
{
private:
	inline void		ResetHeader();
	inline void		ResetHeader2();


public:
	CARPLayer(char* pName);
	virtual ~CARPLayer();

	typedef struct _ARP_HEADER {
		unsigned short  arp_hardType;	//반드시 1
		unsigned short  arp_protocolType;	// ip한테 받아서 0x0800
		unsigned char   arp_hardLength;	//mac
		unsigned char   arp_protocolLength;	//ip
		unsigned short  arp_option; //stdafx.h
		unsigned char   arp_HardSrcAddr[ENET_ADDR_SIZE];
		unsigned char   arp_ProtcolSrcAddr[IP_ADDR_SIZE];
		unsigned char   arp_HardDstaddr[ENET_ADDR_SIZE];
		unsigned char   arp_PorotocolDstAddr[IP_ADDR_SIZE];
	} ARP_HEADER, * PARP_HEADER;

	typedef struct _LARP_NODE {
		unsigned char prot_addr[IP_ADDR_SIZE];
		unsigned char hard_addr[ENET_ADDR_SIZE];
		BOOL status;
		CTime lifeTime;
		struct _LARP_NODE();
		struct _LARP_NODE(unsigned char* ipaddr, unsigned char* enetaddr, BOOL incomplete);
	} LARP_NODE, * PLARP_NODE;

	typedef struct _IP_HEADER {	//ARP Layer에 payload casting위해 IP Header가 필요.
		unsigned char   ip_VersionHeaderLegnth;
		unsigned char   ip_Tos;
		unsigned short  ip_Tlength;

		unsigned short  ip_Id;
		unsigned char   ip_Offset;

		unsigned char   ip_Ttl;
		unsigned char   ip_Ptype;
		unsigned short  ip_Checksum;

		unsigned char   ip_SrcAddr[IP_ADDR_SIZE];
		unsigned char   ip_DstAddr[IP_ADDR_SIZE];
		unsigned char   ip_Data[1480];
	}IP_HEADER, * PIP_HEADER;

	void SetSrcAddress(const unsigned char macAddr[], const unsigned char ipAddr[]);
	void SetDstAddress(const unsigned char macAddr[], const unsigned char ipAddr[]);
	void SetOption(unsigned int arp_option);
	BOOL checkAddressWithMyIp(unsigned char dstip[]); //인자로 들어온 dstip와 현재 Host의 id가 다르면 False 반환

	BOOL FindArpEntry(const CString& ip_key, LARP_NODE& outNode);
	void addOrUpdateARPEntry(const CString& ip_key, const LARP_NODE& node);
	void deleteARPEntry(CString& target);
	void deleteAllARPEntries();
	void deleteARPEntryByNodeAddr(const LARP_NODE& targetNode);
	void setDlgIp(CString ip);
	void setDlgMac(CString enet);

	BOOL Send(unsigned char* ppayload, int nlength);
	BOOL Receive(unsigned char* ppayload);
	static std::vector<LARP_NODE> m_arpTable;

	void  byte2Str(const unsigned char* src, CString& dst, unsigned short t);
	void  str2Byte(unsigned char* dst, CString& src, unsigned short t);
	std::vector<LARP_NODE>& GetArpTable();
	LARP_NODE& CreateArpNode(unsigned char* ipaddr, unsigned char* enetaddr, BOOL incomplete = TRUE);
protected:
	unsigned char m_myIp[IP_ADDR_SIZE];
	unsigned char m_myMac[ENET_ADDR_SIZE];
	_ARP_HEADER m_sHeader;
	IP_HEADER Header;
};
