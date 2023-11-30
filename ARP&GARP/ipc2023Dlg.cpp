
// ipc2023Dlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "ipc2023.h"
#include "ipc2023Dlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
public:
 //afx_msg void OnBnClickedButtonAddfile();
};

CAboutDlg::CAboutDlg() : CDialogEx()
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// Cipc2023Dlg 대화 상자



Cipc2023Dlg::Cipc2023Dlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_IPC2023_DIALOG, pParent)
	, CBaseLayer("ChatDlg")
	, m_bSendReady(FALSE)

	, m_unSrcAddr("")
	, m_unDstAddr("")
	, m_stMessage(_T(""))
	, m_Filepath(_T(""))
{
	//대화상자 멤버 변수 초기화
	//  m_unDstAddr = 0;
	//  unSrcAddr = 0;
	//  m_stMessage = _T("");
	//대화 상자 멤버 초기화 완료

	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	//Protocol Layer Setting
	m_LayerMgr.AddLayer(new CARPLayer("ARP"));
	m_LayerMgr.AddLayer(new CIPLayer("IP"));
	m_LayerMgr.AddLayer(new CChatAppLayer("ChatApp"));
	m_LayerMgr.AddLayer(new CEthernetLayer("Ethernet"));
	m_LayerMgr.AddLayer(new CFileAppLayer("FileApp"));
	m_LayerMgr.AddLayer(new CNILayer("Link"));
	m_LayerMgr.AddLayer(this);

	// 레이어를 연결한다. (레이어 생성)
	// 수업 PPT p.30 참고.
	m_LayerMgr.ConnectLayers("Link ( *Ethernet ( *IP ( *ChatDlg -ARP ) *ARP ) )");

	m_ChatApp = (CChatAppLayer*)m_LayerMgr.GetLayer("ChatApp");
	m_Link = (CNILayer*)m_LayerMgr.GetLayer("Link");
	m_Ethernet = (CEthernetLayer*)m_LayerMgr.GetLayer("Ethernet");
	m_File = (CFileAppLayer*)m_LayerMgr.GetLayer("FileApp");
	m_ARP = (CARPLayer*)m_LayerMgr.GetLayer("ARP");
	m_IP = (CIPLayer*)m_LayerMgr.GetLayer("IP");
	//Protocol Layer Setting
}

void Cipc2023Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_ARP_CACHE, m_ListCtrl);
	DDX_Text(pDX, IDC_EDIT_SRC, m_unSrcAddr);
	DDX_Control(pDX, IDC_COMBO1, deviceComboBox);
	DDX_Control(pDX, IDC_LIST_PROXY_ARP_CACHE, m_ListCtrl2);
	DDX_Control(pDX, IDC_IPADDRESS3, m_SrcAddr);
	DDX_Control(pDX, IDC_BUTTON_SELECT, m_select);
	DDX_Control(pDX, IDC_IPADDRESS, m_DestAddr);
	DDX_Control(pDX, IDC_BUTTON_SEND_IPADDRESS, m_SendArp);
	DDX_Control(pDX, IDC_BUTTON_ARP_ID, m_Delete);
	DDX_Control(pDX, IDC_BUTTON_ARP_AD, m_allDelete);
}

// 레지스트리에 등록하기 위한 변수
UINT nRegSendMsg;
UINT nRegAckMsg;
// 레지스트리에 등록하기 위한 변수

BEGIN_MESSAGE_MAP(Cipc2023Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_CBN_SELCHANGE(IDC_COMBO1, &Cipc2023Dlg::OnCbnSelchangeCombo1)
	ON_LBN_SELCHANGE(IDC_LIST_CHAT_ARP_BOX, &Cipc2023Dlg::OnLbnSelchangeListChatArpBox)
	ON_LBN_SELCHANGE(IDC_LIST_CHAT_PROXY_BOX, &Cipc2023Dlg::OnLbnSelchangeListChatProxyBox)
	ON_BN_CLICKED(IDC_BUTTON_PROXY_ADD, &Cipc2023Dlg::OnBnClickedButtonProxyAdd)
	ON_BN_CLICKED(IDC_BUTTON_PROXY_DELETE, &Cipc2023Dlg::OnBnClickedButtonProxyDelete)
	ON_NOTIFY(IPN_FIELDCHANGED, IDC_IPADDRESS3, &Cipc2023Dlg::OnIpnFieldchangedIpaddress3)
	ON_BN_CLICKED(IDC_BUTTON_HW_SEND, &Cipc2023Dlg::OnBnClickedButtonHwSend)
	ON_EN_CHANGE(IDC_EDIT_HW, &Cipc2023Dlg::OnEnChangeEditHw)
	ON_BN_CLICKED(IDC_BUTTON_SELECT, &Cipc2023Dlg::OnBnClickedButtonSelect)
	ON_BN_CLICKED(IDC_BUTTON_SEND_IPADDRESS, &Cipc2023Dlg::OnBnClickedButtonSendIpaddress)
	ON_NOTIFY(IPN_FIELDCHANGED, IDC_IPADDRESS, &Cipc2023Dlg::OnIpnFieldchangedIpaddress)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_PROXY_ARP_CACHE, &Cipc2023Dlg::OnLvnItemchangedListProxyArpCache)
	ON_BN_CLICKED(IDC_BUTTON_ARP_ID, &Cipc2023Dlg::OnBnClickedButtonArpId)
	ON_BN_CLICKED(IDC_BUTTON_ARP_AD, &Cipc2023Dlg::OnBnClickedButtonArpAd)
	ON_NOTIFY(NM_CLICK, IDC_LIST_ARP_CACHE, &Cipc2023Dlg::OnNMClickListArpCache)
END_MESSAGE_MAP()

BOOL Cipc2023Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.


	m_LayerMgr.GetLayer("Link");

	CNILayer* linkLayer = (CNILayer*)m_LayerMgr.GetLayer("Link");
	auto vector = linkLayer->GetDevicesList();
	for (size_t i = 0; i < vector->size(); i++)
	{
		auto& device = (*vector)[i];

		deviceComboBox.InsertString(i, device.description);
		deviceComboBox.SetItemDataPtr(i, device.name);
	}
	InitializeListCtrl();
	InitializeListCtrl2();

	return TRUE;
	// Add your initialization code here
}

void Cipc2023Dlg::OnNMClickListArpCache(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	// 선택된 항목의 인덱스를 가져옵니다.
	int nSelected = pNMItemActivate->iItem;

	// 선택된 항목을 처리합니다. 예를 들어, 멤버 변수에 저장할 수 있습니다.
	m_nSelectedItemIndex = nSelected;

	*pResult = 0;
}

void Cipc2023Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void Cipc2023Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.



HCURSOR Cipc2023Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void Cipc2023Dlg::OnCbnSelchangeCombo1()
{
	// TODO: Add your control notification handler code here
	int selectedIndex = deviceComboBox.GetCurSel();
	if (selectedIndex == 0xffffffff)
	{
		return;
	}
	char* deviceName = (char*)deviceComboBox.GetItemDataPtr(selectedIndex);
	CNILayer* linkLayer = (CNILayer*)m_LayerMgr.GetLayer("Link");
	CNILayer::PhysicalAddress address{};
	if (linkLayer->GetMacAddress(deviceName, &address))
	{
		CString format;
		format.Format(_T("%02x:%02x:%02x:%02x:%02x:%02x"), (int)address.a, (int)address.b, (int)address.c, (int)address.d, (int)address.e, (int)address.f);
		m_unSrcAddr = format;
		UpdateData(FALSE);
	}

	CString ipAddress;
	if (linkLayer->GetIpAddress(deviceName, ipAddress))
	{
		// CString 형식의 IP 주소를 IP 컨트롤에 설정
		DWORD ip1, ip2, ip3, ip4;
		_stscanf_s(ipAddress, _T("%d.%d.%d.%d"), &ip1, &ip2, &ip3, &ip4);
		m_SrcAddr.SetAddress(ip1, ip2, ip3, ip4);
		UpdateData(FALSE);
	}
	else
	{
		AfxMessageBox(_T("IP 주소를 가져오는 데 실패했습니다."));
	}
	if (linkLayer->GetMacAddress(deviceName, &address))
	{
		// Ethernet Layer에 소스 MAC 주소 설정
		m_Ethernet->SetSourceAddress(reinterpret_cast<unsigned char*>(&address));
	}

	if (linkLayer->GetIpAddress(deviceName, ipAddress))
	{
		// IP Layer에 소스 IP 주소 설정
		BYTE ipAddrBytes[4];
		sscanf_s(CStringA(ipAddress), "%hhu.%hhu.%hhu.%hhu", &ipAddrBytes[0], &ipAddrBytes[1], &ipAddrBytes[2], &ipAddrBytes[3]);
		m_IP->SetSrcIPAddress(ipAddrBytes);
	}
	}

BOOL Cipc2023Dlg::PreTranslateMessage(MSG* pMsg)
{
	// Add your message filter code here, if needed
	return CDialogEx::PreTranslateMessage(pMsg);
}

void Cipc2023Dlg::OnTimer(UINT_PTR nIDEvent) {
    if (nIDEvent == 1) { // Assuming 1 is the timer ID for ARP entries
        CTime currentTime = CTime::GetCurrentTime();
        auto& arpTable = m_ARP->GetArpTable();

        for (auto iter = arpTable.begin(); iter != arpTable.end(); ) {
            CTimeSpan elapsedTime = currentTime - iter->lifeTime;

            if (iter->status == TRUE && elapsedTime.GetTotalMinutes() > 20) {
                // 완료된 엔트리가 20분을 초과했을 경우 삭제
                iter = arpTable.erase(iter);
            }
            else if (iter->status == FALSE && elapsedTime.GetTotalMinutes() > 3) {
                // 미완성 엔트리가 3분을 초과했을 경우 삭제
                iter = arpTable.erase(iter);
            }
            else {
                ++iter;
            }
        }

        UpdateArpListCtrl(); // GUI 업데이트
    }

    CDialogEx::OnTimer(nIDEvent);
}



void Cipc2023Dlg::OnLbnSelchangeListChat2()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}


void Cipc2023Dlg::OnBnClickedButton2()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}


void Cipc2023Dlg::OnLbnSelchangeListChat3()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}


void Cipc2023Dlg::OnLbnSelchangeListChatArpBox()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}


void Cipc2023Dlg::OnLbnSelchangeListChatProxyBox()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}


void Cipc2023Dlg::OnBnClickedButtonProxyAdd()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}


void Cipc2023Dlg::OnBnClickedButtonProxyDelete()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}


void Cipc2023Dlg::OnEnChangeEdit4()
{
	// TODO:  RICHEDIT 컨트롤인 경우, 이 컨트롤은
	// __super::OnInitDialog() 함수를 재지정 
	//하고 마스크에 OR 연산하여 설정된 ENM_CHANGE 플래그를 지정하여 CRichEditCtrl().SetEventMask()를 호출하지 않으면
	// 이 알림 메시지를 보내지 않습니다.

	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
}


void Cipc2023Dlg::OnIpnFieldchangedIpaddress3(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMIPADDRESS pIPAddr = reinterpret_cast<LPNMIPADDRESS>(pNMHDR);
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	*pResult = 0;
}


void Cipc2023Dlg::OnBnClickedButtonHwSend()
{
	// 변경된 NIC에 대한 맥 주소 문자열을 바이트 배열로 변환
	CString strMacAddress;
	GetDlgItemText(IDC_EDIT_HW, strMacAddress);
	m_ARP->setDlgMac(strMacAddress);

	BYTE src_ip[4] = { 0,0,0,0 };	//BYTE는 unsgined char의 typedef

	m_SrcAddr.GetAddress(src_ip[0], src_ip[1], src_ip[2], src_ip[3]);

	m_IP->SetSrcIPAddress(src_ip);
	m_IP->SetDestIPAddress(src_ip);

	if (!m_IP->Send((unsigned char*)"request", 8))
	{
		AfxMessageBox(_T("Failed to send ARP request."));
	}


}


void Cipc2023Dlg::OnEnChangeEditHw()
{
	// TODO:  RICHEDIT 컨트롤인 경우, 이 컨트롤은
	// __super::OnInitDialog() 함수를 재지정 
	//하고 마스크에 OR 연산하여 설정된 ENM_CHANGE 플래그를 지정하여 CRichEditCtrl().SetEventMask()를 호출하지 않으면
	// 이 알림 메시지를 보내지 않습니다.

	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
}


void Cipc2023Dlg::OnBnClickedButtonSelect()
{
	// IP 주소 컨트롤 비활성화
	m_SrcAddr.EnableWindow(FALSE);

	// 맥 주소 입력 컨트롤 비활성화
	GetDlgItem(IDC_EDIT_SRC)->EnableWindow(FALSE);

	// 맥 주소 문자열을 바이트 배열로 변환
	CString strMacAddress;
	GetDlgItemText(IDC_EDIT_SRC, strMacAddress);
	m_ARP->setDlgMac(strMacAddress);

	// IP 주소를 가져와서 바이트 배열로 변환
	BYTE ip1, ip2, ip3, ip4;
	m_SrcAddr.GetAddress(ip1, ip2, ip3, ip4);
	CString strIpAddress;
	strIpAddress.Format(_T("%d.%d.%d.%d"), ip1, ip2, ip3, ip4);
	m_ARP->setDlgIp(strIpAddress);

	// NILayer에 어댑터 설정
	int selectedIndex = deviceComboBox.GetCurSel();
	char* adapterName = (char*)deviceComboBox.GetItemDataPtr(selectedIndex);
	m_Link->StartReceive(adapterName);
	SetTimer(1, 1000, NULL);
	m_Link->SetThreadloop();
	AfxBeginThread(m_Link->ReadingThread, m_Link);
}


void Cipc2023Dlg::OnBnClickedButtonSendIpaddress()
{
	BYTE src_ip[4] = {0,0,0,0};	//BYTE는 unsgined char의 typedef
	m_SrcAddr.GetAddress(src_ip[0], src_ip[1], src_ip[2], src_ip[3]);

	BYTE dst_ip[4] = {0,0,0,0};
	m_DestAddr.GetAddress(dst_ip[0], dst_ip[1], dst_ip[2], dst_ip[3]);

	// Pass the IP addresses to the IP layer
	m_IP->SetSrcIPAddress(src_ip);
	m_IP->SetDestIPAddress(dst_ip);

	BYTE temp[4] = { 0,0,0,0 };
	//CARPLayer::LARP_NODE& arp_node = m_ARP->CreateArpNode(dst_ip, temp);
	// Send the ARP request
	if (!m_IP->Send((unsigned char*)"request", 8))
	{
		AfxMessageBox(_T("Failed to send ARP request."));
	}
	else {
		// Convert dst_ip to CString for arp entry key
		CString strDstIp;
		//m_ARP->byte2Str(dst_ip, strDstIp, ARP_IP_TYPE);

		// Add or update ARP entry
		//m_ARP->addOrUpdateARPEntry(strDstIp, arp_node);

		// GUI 업데이트
		//UpdateArpListCtrl();
	}
}
void Cipc2023Dlg::UpdateArpListCtrl() {
	m_ListCtrl.DeleteAllItems(); // 테이블 초기화

	for (const auto& node : m_ARP->GetArpTable()) {
		// 각 노드를 위한 항목 추가
		CString strIP, strMAC, strStatus;
		m_ARP->byte2Str(node.prot_addr, strIP, ARP_IP_TYPE);
		m_ARP->byte2Str(node.hard_addr, strMAC, ARP_ENET_TYPE);
		strStatus = node.status ? _T("complete") : _T("incomplete");

		int nIndex = m_ListCtrl.InsertItem(0, strIP);
		m_ListCtrl.SetItemText(nIndex, 1, strMAC);
		m_ListCtrl.SetItemText(nIndex, 2, strStatus);
	}
}

void Cipc2023Dlg::OnIpnFieldchangedIpaddress(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMIPADDRESS pIPAddr = reinterpret_cast<LPNMIPADDRESS>(pNMHDR);
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	*pResult = 0;
}

void Cipc2023Dlg::OnLvnItemchangedListProxyArpCache(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	*pResult = 0;
}

void Cipc2023Dlg::InitializeListCtrl()
{
	m_ListCtrl.InsertColumn(0, _T("IP Address"), LVCFMT_CENTER, 120);
	m_ListCtrl.InsertColumn(1, _T("Ethernet Address"), LVCFMT_CENTER, 150);
	m_ListCtrl.InsertColumn(2, _T("Status"), LVCFMT_CENTER, 100);
}
void Cipc2023Dlg::InitializeListCtrl2()
{
	m_ListCtrl2.InsertColumn(0, _T("Device"), LVCFMT_CENTER, 100);
	m_ListCtrl2.InsertColumn(1, _T("IP Address"), LVCFMT_CENTER, 150);
	m_ListCtrl2.InsertColumn(2, _T("Ethernet Address"), LVCFMT_CENTER, 100);

}



void Cipc2023Dlg::OnBnClickedButtonArpId()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (m_nSelectedItemIndex != -1) // 항목이 선택되었는지 확인
	{
		CString strIp = m_ListCtrl.GetItemText(m_nSelectedItemIndex, 0); // 선택된 항목의 IP 주소

		m_ARP->deleteARPEntry(strIp);

		m_ListCtrl.DeleteItem(m_nSelectedItemIndex); // 리스트 컨트롤에서 해당 항목을 삭제

		// 선택 인덱스 초기화
		m_nSelectedItemIndex = -1;
	}
}


void Cipc2023Dlg::OnBnClickedButtonArpAd()
{

	m_ARP->deleteAllARPEntries();
	m_ListCtrl.DeleteAllItems();
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}
