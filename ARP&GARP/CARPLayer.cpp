#include "pch.h"

#include "pch.h"
#include "stdafx.h"
#include "CARPLayer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

CARPLayer::CARPLayer(char* pName)
    : CBaseLayer(pName)
{
    ResetHeader();
    ResetHeader2();
    memset(m_myIp, 0, IP_ADDR_SIZE);
    memset(m_myMac, 0, ENET_ADDR_SIZE);
}

CARPLayer::~CARPLayer() {}

std::vector<CARPLayer::LARP_NODE> CARPLayer::m_arpTable;

void CARPLayer::ResetHeader() {
    m_sHeader.arp_hardType = 0x0100;    //Ethernet 0x0001 based on layer structure
    m_sHeader.arp_protocolType = ARP_IP_TYPE;    //IP 0x0800 base on layer structrue
    m_sHeader.arp_hardLength = ENET_ADDR_SIZE;
    m_sHeader.arp_protocolLength = IP_ADDR_SIZE;
    m_sHeader.arp_option = 0x0000;  //Request 1,Reply 2
    memset(m_sHeader.arp_HardSrcAddr, 0, ENET_ADDR_SIZE);
    memset(m_sHeader.arp_ProtcolSrcAddr, 0, IP_ADDR_SIZE);
    memset(m_sHeader.arp_HardDstaddr, 0, ENET_ADDR_SIZE);
    memset(m_sHeader.arp_PorotocolDstAddr, 0, IP_ADDR_SIZE);
}
void CARPLayer::ResetHeader2() {
    memset(Header.ip_SrcAddr, 0, 4);
    memset(Header.ip_DstAddr, 0, 4);
    Header.ip_VersionHeaderLegnth = 0x00;
    Header.ip_Tos = 0x00;
    Header.ip_Tlength = 0x0000;
    Header.ip_Id = 0x0000;
    Header.ip_Offset = 0x0000;
    Header.ip_Ttl = 0x00;
    Header.ip_Ptype = 0x00;
    Header.ip_Checksum = 0x00;
    memset(Header.ip_Data, 0, 1480);
}
CARPLayer::_LARP_NODE::_LARP_NODE()
    : status(0), lifeTime(CTime::GetCurrentTime()) {
    memset(prot_addr, 0, IP_ADDR_SIZE);
    memset(hard_addr, 0, ENET_ADDR_SIZE);
}
CARPLayer::_LARP_NODE::_LARP_NODE(unsigned char* ipaddr, unsigned char* enetaddr, BOOL incomplete = FALSE) {
    memcpy(prot_addr, ipaddr, IP_ADDR_SIZE);
    memcpy(hard_addr, enetaddr, ENET_ADDR_SIZE);
    status = incomplete; 
    lifeTime = CTime::GetCurrentTime();
}


    BOOL CARPLayer::Send(unsigned char* ppayload, int nlength) {
        PIP_HEADER ip_header = (PIP_HEADER)ppayload;  // C-style cast
        CString dst_ip_str;
        byte2Str(ip_header->ip_DstAddr, dst_ip_str, ARP_IP_TYPE); // Using ARP_IP_TYPE for clarity 굳이굳이긴.

        LARP_NODE node;
        BOOL found = FindArpEntry(dst_ip_str, node);    //arp basic process

        if (found && node.status == FALSE) { //incomplete
            AfxMessageBox(_T("plz wait"));
            return TRUE;
        }
        else if(!found) {
            unsigned char broadcastAddr[ENET_ADDR_SIZE] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
            LARP_NODE newNode(ip_header->ip_DstAddr, broadcastAddr, FALSE);
            addOrUpdateARPEntry(dst_ip_str, newNode);

            SetSrcAddress(m_myMac, ip_header->ip_SrcAddr);
            SetDstAddress(broadcastAddr, ip_header->ip_DstAddr);  // Broadcast address
            SetOption(ARP_OPCODE_REQUEST);

            return this->GetUnderLayer()->Send((unsigned char*)&m_sHeader, sizeof(ARP_HEADER), ETHER_ARP_TYPE);
        }
    }


BOOL CARPLayer::Receive(unsigned char* ppayload) {
    PARP_HEADER arp_header = (PARP_HEADER)ppayload; // C-style cast for the ARP header

    // Process ARP request
    if (arp_header->arp_option == ARP_OPCODE_REQUEST) {
        // Check if the destination IP is ours
        if (checkAddressWithMyIp(arp_header->arp_ProtcolSrcAddr)) { //Perhaps In Dlg
            // Respond to the ARP request swap!
            unsigned char   temp_HardDestAddr[ENET_ADDR_SIZE], temp_ProtocolDestAddr[IP_ADDR_SIZE];
             memcpy(temp_HardDestAddr, arp_header->arp_HardDstaddr, ENET_ADDR_SIZE);
             memcpy(temp_ProtocolDestAddr, arp_header->arp_PorotocolDstAddr, IP_ADDR_SIZE);
            SetDstAddress(arp_header->arp_HardSrcAddr, arp_header->arp_ProtcolSrcAddr);
            SetSrcAddress(temp_HardDestAddr, temp_ProtocolDestAddr); // Perhaps In Dlg
            SetOption(ARP_OPCODE_REPLY);
        }
        return mp_UnderLayer->Send((unsigned char*)arp_header, sizeof(ARP_HEADER), ETHER_ARP_TYPE);
    }
    // Process ARP reply
    else if (arp_header->arp_option == ARP_OPCODE_REPLY) {
        CString src_ip_str;
        byte2Str(arp_header->arp_ProtcolSrcAddr, src_ip_str, ARP_IP_TYPE);

        LARP_NODE node;
        BOOL found = FindArpEntry(src_ip_str, node);
        if (found) {
            memcpy(node.hard_addr, arp_header->arp_HardSrcAddr, ENET_ADDR_SIZE);
            node.status = TRUE;
            node.lifeTime = CTime::GetCurrentTime();
            addOrUpdateARPEntry(src_ip_str, node);
        }
    }

    return TRUE;
}
void CARPLayer::SetSrcAddress(const unsigned char enetAddr[], const unsigned char ipAddr[]) {
    memcpy(m_sHeader.arp_HardSrcAddr, enetAddr, ENET_ADDR_SIZE);
    memcpy(m_sHeader.arp_ProtcolSrcAddr, ipAddr, IP_ADDR_SIZE);
}

void CARPLayer::SetDstAddress(const unsigned char enetAddr[], const unsigned char ipAddr[]) {
    memcpy(m_sHeader.arp_HardDstaddr, enetAddr, ENET_ADDR_SIZE);
    memcpy(m_sHeader.arp_PorotocolDstAddr, ipAddr, IP_ADDR_SIZE);
}

BOOL CARPLayer::checkAddressWithMyIp(unsigned char dstip[]) {
    return memcmp(dstip, m_myIp, IP_ADDR_SIZE) == 0;
}

BOOL CARPLayer::FindArpEntry(const CString& ip_key, LARP_NODE& outNode) {
    for (auto& node : m_arpTable) {
        CString currentIp;
        byte2Str(node.prot_addr, currentIp, ARP_IP_TYPE);
        if (currentIp == ip_key) {
            outNode = node;
            return TRUE;
        }
    }
    return FALSE;
}

void CARPLayer::addOrUpdateARPEntry(const CString& ip_key, const LARP_NODE& node) {
    // ARP 엔트리 추가 또는 갱신
    for (auto& item : m_arpTable) {
        CString currentIp;
        byte2Str(item.prot_addr, currentIp, ARP_IP_TYPE);
        if (currentIp == ip_key) {
            memcpy(item.hard_addr, node.hard_addr, ENET_ADDR_SIZE);
            item.lifeTime = node.lifeTime;
            item.status = node.status;
            return;
        }
    }
    m_arpTable.push_back(node); // 새 엔트리 추가 case : 존재하지 않는 경우
}

void CARPLayer::deleteARPEntry(CString& target) {
    for (auto iter = m_arpTable.begin(); iter != m_arpTable.end(); ) {
        CString currentIp;
        byte2Str(iter->prot_addr, currentIp, ARP_IP_TYPE);

        if (currentIp == target) {
            iter = m_arpTable.erase(iter);  // 요소를 제거하고 다음 요소의 반복자를 반환
        }
        else {
            ++iter;  // 요소를 제거하지 않고 다음 요소로 이동
        }
    }
}
void CARPLayer::deleteARPEntryByNodeAddr(const LARP_NODE& targetNode) {
    for (auto iter = m_arpTable.begin(); iter != m_arpTable.end(); ) {
        if (memcmp(iter->prot_addr, targetNode.prot_addr, IP_ADDR_SIZE) == 0) {
            iter = m_arpTable.erase(iter);  
        }
        else {
            ++iter;  
        }
    }
}
void CARPLayer::deleteAllARPEntries() {
    m_arpTable.clear(); // 전체 ARP 엔트리 삭제
}

void CARPLayer::byte2Str(const unsigned char* src, CString& dst, unsigned short type) {
    if (type == ARP_IP_TYPE) { // IP Address
        dst.Format(_T("%d.%d.%d.%d"), src[0], src[1], src[2], src[3]);
    }
    else if (type == ARP_ENET_TYPE) { // ENET Address
        dst.Format(_T("%02x:%02x:%02x:%02x:%02x:%02x"), src[0], src[1], src[2], src[3], src[4], src[5]);
    }
    else {
        dst.Empty(); // Unknown type
    }
}

void CARPLayer::str2Byte(unsigned char* dst, CString& src, unsigned short type) {
    int values[6]; // Array to hold the bytes
    if (type == ARP_IP_TYPE) { // IP Address
        if (sscanf_s((LPCTSTR)src, "%d.%d.%d.%d", &values[0], &values[1], &values[2], &values[3]) == 4) {
            for (int i = 0; i < 4; ++i) {
                dst[i] = (unsigned char)values[i];
            }
        }
    }
    else if (type == ARP_ENET_TYPE) { // ENET Address
        if (sscanf_s((LPCTSTR)src, "%02x:%02x:%02x:%02x:%02x:%02x", &values[0], &values[1], &values[2], &values[3], &values[4], &values[5]) == 6) {
            for (int i = 0; i < 6; ++i) {
                dst[i] = (unsigned char)values[i];
            }
        }
    }
}

void CARPLayer::SetOption(unsigned int arp_option) {
    m_sHeader.arp_option = (unsigned short)(arp_option);
}

void CARPLayer::setDlgIp(CString ip) {
    str2Byte(m_myIp, ip, ARP_IP_TYPE);
}

void CARPLayer::setDlgMac(CString enet) {
    str2Byte(m_myMac, enet, ARP_ENET_TYPE);
}

 std::vector<CARPLayer::LARP_NODE>& CARPLayer::GetArpTable() {
    return m_arpTable;
}

 CARPLayer::LARP_NODE& CARPLayer::CreateArpNode(unsigned char* ipaddr, unsigned char* enetaddr, BOOL incomplete) {
     m_arpTable.emplace_back(ipaddr, enetaddr, incomplete);
     return m_arpTable.back(); // 마지막에 추가된 요소의 참조 반환
 }

 BOOL CARPLayer::GSend(unsigned char* ppayload, int nlength) {
     unsigned char broadcastAddr[ENET_ADDR_SIZE] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
     SetSrcAddress(m_myMac, m_myIp);
     SetDstAddress(broadcastAddr, m_myIp); 
     SetOption(ARP_OPCODE_REQUEST);

     return this->GetUnderLayer()->Send((unsigned char*)&m_sHeader, sizeof(ARP_HEADER), ETHER_ARP_TYPE);
 }

 BOOL CARPLayer::GReceive(unsigned char* ppayload, int nlength) {
     PARP_HEADER arp_header = (PARP_HEADER)ppayload;
     LARP_NODE node;
     CString src_ip_str;
     byte2Str(arp_header->arp_ProtcolSrcAddr, src_ip_str, ARP_IP_TYPE);

     memcpy(node.hard_addr, arp_header->arp_HardSrcAddr,ENET_ADDR_SIZE);
     node.status = TRUE;
     node.lifeTime = CTime::GetCurrentTime();
     addOrUpdateARPEntry(src_ip_str, node);

     return TRUE;
 }