#include "pch.h"
#include "CNILayer.h"
#include <pcap.h>
#include <Packet32.h>
#include <winsock2.h>
#pragma comment(lib, "packet.lib")
#define BPF_MAJOR_VERSION

CNILayer::CNILayer(char* pName)
    : CBaseLayer(pName), devicesList{}
{
    LoadNpcapDlls();
    PopulateDeviceList();
    _adapter = nullptr;
}

CNILayer::~CNILayer()
{
    pcap_freealldevs(pointerToDeviceList);
}

BOOL CNILayer::LoadNpcapDlls()
{
    TCHAR npcap_dir[512];
    UINT len;
    len = GetSystemDirectory(npcap_dir, 480);
    if (!len) {
        fprintf(stderr, "Error in GetSystemDirectory: %x", GetLastError());
        return FALSE;
    }
    strcat_s(npcap_dir, 512, TEXT("\\Npcap"));
    if (SetDllDirectory(npcap_dir) == 0) {
        fprintf(stderr, "Error in SetDllDirectory: %x", GetLastError());
        return FALSE;
    }
    return TRUE;
}

void CNILayer::PopulateDeviceList()
{
    char error[PCAP_ERRBUF_SIZE];
    pcap_if_t* allDevices;
    if (!pcap_findalldevs(&allDevices, error))
    {
        this->pointerToDeviceList = allDevices;
        for (pcap_if_t* currentDevice = allDevices; currentDevice != nullptr; currentDevice = currentDevice->next)
        {
            NetworkDevice device{};
            device.name = currentDevice->name;
            device.description = currentDevice->description;
            devicesList.push_back(device);
        }
    }
}

std::vector<CNILayer::NetworkDevice>* CNILayer::GetDevicesList()
{
    return &this->devicesList;
}

BOOL CNILayer::GetMacAddress(char* deviceName, CNILayer::PhysicalAddress* outAddress)
{
    PPACKET_OID_DATA oidData = (PPACKET_OID_DATA)malloc(sizeof(PACKET_OID_DATA) + 6);
    if (oidData == nullptr)
    {

        return false;
    }

    oidData->Oid = OID_802_3_PERMANENT_ADDRESS;
    oidData->Length = 6;
    ZeroMemory(oidData->Data, 6);
    if (deviceName == nullptr)
    {
        return false;
    }
    LPADAPTER adapter = PacketOpenAdapter(deviceName);
    if (adapter == nullptr)
    {
        return false;
    }

    if (!PacketRequest(adapter, false, oidData))
    {
        return false;
    }

    for (int i = 0; i < 6; i++)
    {
        ((char*)outAddress)[i] = oidData->Data[i];
    }

    PacketCloseAdapter(adapter);
    return true;
}
#include <Iphlpapi.h>
#pragma comment(lib, "iphlpapi.lib")

BOOL CNILayer::GetIpAddress(const char* deviceName, CString& outIpAddress) {
    ULONG outBufLen = 0;
    DWORD dwRetVal = GetAdaptersAddresses(AF_INET, GAA_FLAG_INCLUDE_PREFIX, NULL, NULL, &outBufLen);
    if (dwRetVal == ERROR_BUFFER_OVERFLOW) {
        PIP_ADAPTER_ADDRESSES pAddresses = static_cast<PIP_ADAPTER_ADDRESSES>(malloc(outBufLen));
        if (pAddresses == NULL) {
            AfxMessageBox(_T("Error allocating memory needed to call GetAdaptersAddresses"));
            return FALSE;
        }

        dwRetVal = GetAdaptersAddresses(AF_INET, GAA_FLAG_INCLUDE_PREFIX, NULL, pAddresses, &outBufLen);
        if (dwRetVal == NO_ERROR) {
            while (pAddresses) {
                // deviceName에서 '\\Device\\NPF_' 제거
                CString strDeviceName(deviceName);
                strDeviceName.Replace("\\Device\\NPF_", "");
                // AdapterName과 비교
                if (strDeviceName.CompareNoCase(CString(pAddresses->AdapterName)) == 0) {
                    // We've found the adapter
                    PIP_ADAPTER_UNICAST_ADDRESS pUnicast = pAddresses->FirstUnicastAddress;
                    if (pUnicast != NULL) {
                        // Assuming there is at least one IP address
                        SOCKADDR_IN* sockAddr = reinterpret_cast<SOCKADDR_IN*>(pUnicast->Address.lpSockaddr);
                        char ipstr[INET_ADDRSTRLEN];
                        inet_ntop(AF_INET, &(sockAddr->sin_addr), ipstr, sizeof(ipstr));
                        outIpAddress = CString(ipstr);
                        return TRUE;
                    }
                }
                pAddresses = pAddresses->Next;
            }
        }
        else {
            CString err;
            err.Format(_T("GetAdaptersAddresses failed with error: %d"), dwRetVal);
            AfxMessageBox(err);
        }

        if (pAddresses) {
            free(pAddresses);
        }
    }
    else {
        CString err;
        err.Format(_T("GetAdaptersAddresses failed with error: %d"), dwRetVal);
        AfxMessageBox(err);
    }

    return FALSE; // IP Address not found for this adapter
}




BOOL CNILayer::Send(unsigned char* payload, int length)
{
    if (pcap_sendpacket(_adapter, payload, length))
    {
        AfxMessageBox(_T("패킷 전송 실패"));
        return FALSE;
    }
    return TRUE;
}

BOOL CNILayer::StartReceive(int index)
{
    if (index < 0 || index >= devicesList.size()) {
        AfxMessageBox(_T("Invalid adapter index"));
        return FALSE;
    }

    const char* adapterName = devicesList[index].name;

    if (_adapter != nullptr) {
        pcap_close(_adapter);
        _adapter = nullptr;
    }

    char errbuf[PCAP_ERRBUF_SIZE];
    _adapter = pcap_open_live(adapterName, 65536, 1, 1000, errbuf);
    if (_adapter == nullptr) {
        CString errorMsg;
        errorMsg.Format(_T("Failed to open device: %s"), CString(errbuf));
        AfxMessageBox(errorMsg);
        return FALSE;
    }

    return TRUE;
}


BOOL CNILayer::Receive(unsigned char* packet)
{
    return this->GetUpperLayer(0)->Receive(packet);
}


UINT CNILayer::ReadingThread(LPVOID pParam)
{
    struct pcap_pkthdr* packetHeader;
    const u_char* packet;
    CNILayer* NIlayer = (CNILayer*)pParam;

    int code = 1;
    while (NIlayer->m_isWorkingThread)
    {
        code = pcap_next_ex(NIlayer->_adapter, &packetHeader, &packet);
        switch (code)
        {
        case 1:
            break;
        case PCAP_ERROR:
            AfxMessageBox(pcap_geterr(NIlayer->_adapter));
#ifdef DEBUG
            DebugBreak();
#endif
        case 0:
        default:
            return false;
        }
        memcpy(NIlayer->data, packet, ETHER_MAX_SIZE);
        NIlayer->Receive(NIlayer->data);

        //Do something...
    }
    return 0;
}

void CNILayer::SetThreadloop() {
    m_isWorkingThread = true;
}
