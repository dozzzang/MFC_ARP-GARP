#pragma once
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "pch.h"
#include "BaseLayer.h"

class CIPLayer
	: public CBaseLayer
{
private:
	inline void		ResetHeader();

public:
	CIPLayer(char* pName);
	virtual ~CIPLayer();
	void SetSrcIPAddress(unsigned char* src_ip);
	void SetDestIPAddress(unsigned char* dest_ip);

	BOOL Send(unsigned char* ppayload, int nlength);
	BOOL Receive(unsigned char* ppayload);

	typedef struct _IPLayer_HEADER {
		unsigned char ip_ver;	// 1byte
		unsigned char ip_tos;		// 1byte
		unsigned short ip_len;		// 2byte
		unsigned short ip_id;		// 2byte
		unsigned char ip_fragoffset;	// 2byte
		unsigned char ip_ttl;		// 1byte
		unsigned char ip_proto;		// 1byte
		unsigned short ip_checksum;	// 2byte
		unsigned char ip_src[4];	// 4byte
		unsigned char ip_dest[4];	// 4byte
		unsigned char ip_data[IP_MAX_DATA_SIZE];

	} IPLayer_HEADER, * pIPLayer_HEADER;

protected:
	IPLayer_HEADER	m_sHeader;
};




