#include "pch.h"
#include "stdafx.h"
#include "CIPLayer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

CIPLayer::CIPLayer(char* pName)
	: CBaseLayer(pName)
{
	ResetHeader();
}

CIPLayer::~CIPLayer()
{
}

void CIPLayer::ResetHeader()
{
	memset(m_sHeader.ip_src, 0, 4);
	memset(m_sHeader.ip_dest, 0, 4);
	m_sHeader.ip_ver = 0x00;
	m_sHeader.ip_tos = 0x00;
	m_sHeader.ip_len = 0x0000;
	m_sHeader.ip_id = 0x0000;
	m_sHeader.ip_fragoffset = 0x0000;
	m_sHeader.ip_ttl = 0x00;
	m_sHeader.ip_proto = 0x00;
	m_sHeader.ip_checksum = 0x00;
	memset(m_sHeader.ip_data, 0, 1480);
}

void CIPLayer::SetSrcIPAddress(unsigned char* src_ip)
{
	memcpy(m_sHeader.ip_src, src_ip, 4);
}

void CIPLayer::SetDestIPAddress(unsigned char* dst_ip)
{
	memcpy(m_sHeader.ip_dest, dst_ip, 4);
}

BOOL CIPLayer::Send(unsigned char* ppayload, int nlength)
{
	memcpy(m_sHeader.ip_data, ppayload, nlength);

	BOOL bSuccess = FALSE;
	bSuccess = this->GetUnderLayer()->Send((unsigned char*)&m_sHeader, nlength + IP_HEADER_SIZE);

	return bSuccess;
}

BOOL CIPLayer::Receive(unsigned char* ppayload)
{
	pIPLayer_HEADER Frame = (pIPLayer_HEADER)ppayload;

	BOOL bSuccess = FALSE;

	if (memcmp((char*)Frame->ip_dest, (char*)m_sHeader.ip_src, 4) == 0)
	{
		bSuccess = mp_aUpperLayer[0]->Receive((unsigned char*)Frame->ip_data);	
	}
	return bSuccess;
}
