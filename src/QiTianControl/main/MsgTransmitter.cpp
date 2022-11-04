/*
 * MsgTransmitter.cpp
 *
 *  Created on: 2021年12月5日
 *      Author: Qufenglu
 */

#include <cstdlib>
#include <cstring>
#include "esp_event.h"
#include "nvs_flash.h"
#include <lwip/netdb.h>
#include "MsgTransmitter.h"
#include "Log.h"

wifi_mode_t MsgTransmitter::m_eWifiMode = WIFI_MODE_NULL;
bool MsgTransmitter::m_bWifiConnected = false;
bool MsgTransmitter::m_bGotRemoteIp = false;
ip4_addr_t MsgTransmitter::m_LocalIp = {0};
ip4_addr_t MsgTransmitter::m_RemoteIp = {0};

MsgTransmitter::MsgTransmitter()
{
	m_pControlMsgBuff = nullptr;
	m_bIsInitialized = false;
	m_nMsgBuffLen = 0;
	m_nSocket = -1;
	m_nLocalType = 1;
	m_nRemoteType = 2;
	m_RemoteAddr = {0};
	m_nWorkMode = -1;
	m_ControlMsg = {0};

	m_bStopWork = true;
	m_pClientThread = nullptr;
	m_pServerThread = nullptr;
}

MsgTransmitter::~MsgTransmitter()
{
	ReleaseAll();
}

int32_t MsgTransmitter::Init()
{
	Info("[%p][MsgTransmitter::Init] Enter Init()\n", this);

	if (m_bIsInitialized)
	{
		Warn("[%p][MsgTransmitter::Init] MsgTransmitter has initialized,release old\n", this);
		ReleaseAll();
	}

	//计算控制消息buff长度
	uint16_t ByteNum = 1 + CHANNEL_NUM * 2 + 2;
	m_pControlMsgBuff = (uint8_t*) malloc(ByteNum);
	if (m_pControlMsgBuff == nullptr)
	{
		Error("[%p][MsgTransmitter::Init] Malloc control msg buff fail\n", this);
		return -1;
	}
	memset(m_pControlMsgBuff, 0, ByteNum);
	m_nMsgBuffLen = ByteNum;

	m_bIsInitialized = true;

	return 0;
}

int32_t MsgTransmitter::ReleaseAll()
{
	Info("[%p][MsgTransmitter::ReleaseAll] Enter ReleaseAll()\n", this);

	if (m_pControlMsgBuff != nullptr)
	{
		free(m_pControlMsgBuff);
		m_pControlMsgBuff = nullptr;
	}
	m_nMsgBuffLen = 0;

	if (m_nSocket >= 0)
	{
		close(m_nSocket);
		m_nSocket = -1;
	}

	CloseClient();
	CloseServer();

	m_nRemoteType = 2;
	m_RemoteAddr = {0};
	m_nWorkMode = -1;
	m_bIsInitialized = false;

	return 0;
}

int32_t MsgTransmitter::WorkAsClient(const uint16_t &port)
{
	Info("[%p][MsgTransmitter::WorkAsClient] Enter WorkAsClient(port:%d)\n", this, port);

	if (m_nWorkMode == 0 || m_nWorkMode == 1)
	{
		Error("[%p][MsgTransmitter::WorkAsClient] MsgTransmitter has work as:%d\n", this, m_nWorkMode);
		return -1;
	}

	m_pClientThread = new (std::nothrow) std::thread(&MsgTransmitter::ClientThread, this, port);
	if (m_pClientThread == nullptr)
	{
		Error("[%p][MsgTransmitter::WorkAsClient] new ClientThread fail\n", this);
		return -2;
	}

	m_nWorkMode = 0;

	return 0;
}

int32_t MsgTransmitter::WorkAsServer(const uint16_t &port)
{
	Info("[%p][MsgTransmitter::WorkAsServer] Enter WorkAsServer(port:%d)\n", this, port);

	if (m_nWorkMode == 0 || m_nWorkMode == 1)
	{
		Error("[%p][MsgTransmitter::WorkAsServer] MsgTransmitter has work as:%d\n", this, m_nWorkMode);
		return -1;
	}

	m_pServerThread = new (std::nothrow) std::thread(&MsgTransmitter::ServerThread, this, port);
	if (m_pServerThread == nullptr)
	{
		Error("[%p][MsgTransmitter::WorkAsServer] new ServerThread fail\n", this);
		return -2;
	}

	m_nWorkMode = 1;

	return 0;
}

int32_t MsgTransmitter::CloseClient()
{
	Info("[%p][MsgTransmitter::CloseClient] Enter CloseClient()\n", this);

	m_bStopWork = true;

	if (m_pClientThread != nullptr)
	{
		if (m_pClientThread->joinable())
		{
			m_pClientThread->join();
		}
		delete m_pClientThread;
		m_pClientThread = nullptr;
	}

	if (m_nWorkMode == 0)
	{
		m_nWorkMode = -1;
	}

	return 0;
}

int32_t MsgTransmitter::CloseServer()
{
	Info("[%p][MsgTransmitter::CloseServer] Enter CloseServer()\n", this);

	m_bStopWork = true;

	if (m_pServerThread != nullptr)
	{
		if (m_pServerThread->joinable())
		{
			m_pServerThread->join();
		}
		delete m_pServerThread;
		m_pServerThread = nullptr;
	}

	if (m_nLocalType == 1)
	{
		m_nLocalType = 1;
	}

	return 0;
}

int32_t MsgTransmitter::MsgProcess()
{
	uint8_t buffer[64];
	int len = 0;
	while (!m_bStopWork)
	{
		{
			SendControlMsg();
			std::this_thread::sleep_for(std::chrono::milliseconds(20));
		}

		len = recv(m_nSocket, buffer, sizeof(buffer), 0);
		if (len < 0)
		{
			if (errno != EAGAIN)
			{
				Error("[%p][MsgTransmitter::MsgProcess] recv err:%d\n", this, errno);
				close(m_nSocket);
				m_nSocket = -1;
				return -1;
			}
		}
		else if (len > 0 && (buffer[0] & 0xc0) == 0)
		{
			uint8_t msgType = (buffer[0] & 0x3f);
			switch (msgType)
			{
			case MSG_Hello:
				SendHelloMsg();
				break;
			default:
				break;

			}
		}

		/*if (len < 0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}*/
	}

	return 0;
}

void MsgTransmitter::ClientThread(uint16_t port)
{
	m_bStopWork = false;
	Info("[%p][MsgTransmitter::ClientThread] Start client thread\n", this);

	while (!m_bStopWork)
	{
		if (!IsGotRemoteIp())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(4000));
			if(m_eWifiMode == WIFI_MODE_STA)
			{
				esp_wifi_connect();
			}
			continue;
		}

		int32_t ret = MakeClientSocket(port);
		if (ret < 0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
			continue;
		}

		int flags = fcntl(m_nSocket, F_GETFL, 0);
		fcntl(m_nSocket, F_SETFL, flags | O_NONBLOCK);

		bool bIsConnect2Server = false;
		while (!m_bStopWork)
		{
			ret = Connect2Server();
			if (ret < 0)
			{
				break;
			}
			else if (ret == 0)
			{
				bIsConnect2Server = true;
				break;
			}
			else
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(200));
			}
		}
		if (!bIsConnect2Server)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
			continue;
		}

		MsgProcess();
	}
}

int32_t MsgTransmitter::MakeClientSocket(const uint16_t &port)
{
	Info("[%p][MsgTransmitter::MakeClientSocket] Enter MakeClientSocket(port:%d)\n", this, port);

	if (m_nSocket >= 0)
	{
		Warn("[%p][MsgTransmitter::MakeClientSocket] Close the old socket\n", this);
		close(m_nSocket);
	}

	m_nSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (m_nSocket < 0)
	{
		Error("[%p][MsgTransmitter::MakeClientSocket] Unable to create socket: errno %d\n", this, errno);
		return -1;
	}

	m_RemoteAddr.sin_family = AF_INET;
	m_RemoteAddr.sin_port = htons(port);
	m_RemoteAddr.sin_addr.s_addr = m_RemoteIp.addr;

	return 0;
}

int32_t MsgTransmitter::Connect2Server()
{
	Info("[%p][MsgTransmitter::CloseServer] Enter Connect2Server()\n", this);

	int32_t ret = SendHelloMsg();
	if (ret < 0)
	{
		Error("[%p][MsgTransmitter::Connect2Server] Send hello msg fail,return %d\n", this, ret);
		close(m_nSocket);
		m_nSocket = -1;
		return -1;
	}

	{
		uint8_t rx_buffer[3];
		socklen_t socklen = sizeof(m_RemoteAddr);
		int len = recvfrom(m_nSocket, rx_buffer, sizeof(rx_buffer), 0, (struct sockaddr*) &m_RemoteAddr, &socklen);
		if (len < 0)
		{
			if (errno == EAGAIN)
			{
				return 1;
			}

			Error("[%p][MsgTransmitter::Connect2Server] recv err:%d\n", this, errno);
			close(m_nSocket);
			m_nSocket = -1;
			return -2;
		}
		Info("[%p][MsgTransmitter::Connect2Server] server ip:%s\n", this, ip4addr_ntoa((ip4_addr*) &m_RemoteAddr.sin_addr));

		if (len != 2 || rx_buffer[0] != MSG_Hello)
		{
			Error("[%p][MsgTransmitter::Connect2Server] Not recv hello msg\n", this);
			close(m_nSocket);
			m_nSocket = -1;
			return -3;
		}

		m_nRemoteType = rx_buffer[1];
		Info("[%p][MsgTransmitter::Connect2Server] Connect to remote type:%d\n", this, m_nRemoteType);
	}

	return 0;
}

void MsgTransmitter::ServerThread(uint16_t port)
{
	m_bStopWork = false;
	Info("[%p][MsgTransmitter::ServerThread] Start server thread\n", this);

	while (!m_bStopWork)
	{
		if (!IsGotRemoteIp())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(4000));
			if(m_eWifiMode == WIFI_MODE_STA)
			{
				esp_wifi_connect();
			}
			continue;
		}

		int32_t ret = BindSocket(port);
		if (ret < 0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
			continue;
		}

		int flags = fcntl(m_nSocket, F_GETFL, 0);
		fcntl(m_nSocket, F_SETFL, flags | O_NONBLOCK);

		bool bIsConnect2Client = false;
		while (!m_bStopWork)
		{
			ret = ListenClient();
			if (ret < 0)
			{
				break;
			}
			else if (ret == 0)
			{
				bIsConnect2Client = true;
				break;
			}
			else
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(200));
			}
		}
		if (!bIsConnect2Client)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
			continue;
		}

		ret = SendHelloMsg();
		if (ret < 0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
			continue;
		}

		MsgProcess();
	}
}

int32_t MsgTransmitter::BindSocket(const uint16_t &port)
{
	Info("[%p][MsgTransmitter::BindSocket] Enter BindSocket(port:%d)\n", this, port);

	if (m_nSocket >= 0)
	{
		Warn("[%p][MsgTransmitter::BindSocket] Close the old socket\n", this);
		close(m_nSocket);
	}

	m_nSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (m_nSocket < 0)
	{
		Error("[%p][MsgTransmitter::BindSocket] Create socket err:%d\n", this, errno);
		return -1;
	}

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	int err = bind(m_nSocket, (struct sockaddr*) &addr, sizeof(addr));
	if (err < 0)
	{
		Error("[%p][MsgTransmitter::BindSocket] Bind socket err:%d\n", this, errno);
		close(m_nSocket);
		m_nSocket = -1;
		return -2;
	}

	return 0;
}

int32_t MsgTransmitter::ListenClient()
{
	Info("[%p][MsgTransmitter::ListenClient] Enter ListenClient\n", this);

	uint8_t rx_buffer[3];
	socklen_t socklen = sizeof(m_RemoteAddr);
	int len = recvfrom(m_nSocket, rx_buffer, sizeof(rx_buffer), 0, (struct sockaddr*) &m_RemoteAddr, &socklen);
	if (len < 0)
	{
		if (errno == EAGAIN)
		{
			return 1;
		}

		Error("[%p][MsgTransmitter::ListenClient] recv err:%d\n", this, errno);
		close(m_nSocket);
		m_nSocket = -1;
		return -1;
	}

	Info("[%p][MsgTransmitter::ListenClient] Client ip:%s\n", this, ip4addr_ntoa((ip4_addr*) &m_RemoteAddr.sin_addr));

	if (len != 2 || rx_buffer[0] != MSG_Hello)
	{
		Error("[%p][MsgTransmitter::ListenClient] Not recv hello msg\n", this);
		close(m_nSocket);
		m_nSocket = -1;
		return -2;
	}

	m_nRemoteType = rx_buffer[1];
	Info("[%p][MsgTransmitter::ListenClient] Connect to remote type:%d\n", this, m_nRemoteType);

	return 0;
}

/****************************************
 * 遥控器只需要将最新的控制消息尽快发送出去
 * 因此这里的处理逻辑为读取到通道的值后更新
 * 消息，MsgProcess周期的将最新消息发送出去
 ****************************************/
void MsgTransmitter::UpdateControlMsg(const ControlMsg &msg)
{
	{
		std::lock_guard<std::mutex> lock(m_ControlMsgLock);
		memcpy(&m_ControlMsg,&msg,sizeof(ControlMsg));
	}
}

int32_t MsgTransmitter::PackControlMsg()
{
	if (m_pControlMsgBuff == nullptr)
	{
		Error("[%p][MsgTransmitter::SendControlMsg] Control msg buff is null", this);
		return -1;
	}

	//Version
		uint8_t value = 0x00;
		m_pControlMsgBuff[0] |= (value & 0xc0);

	//MsgType
	value = MSG_Control;
	m_pControlMsgBuff[0] |= (value & 0x3f);

	//MsgContent
	{
		std::lock_guard<std::mutex> lock(m_ControlMsgLock);
		for (int i = 0; i < CHANNEL_NUM; i++)
		{
			memcpy(&m_pControlMsgBuff[1 + 2 * i], &m_ControlMsg.chValues[i], sizeof(uint16_t));
		}
	}

	//CheckSum
	uint16_t checksum = 0;
	for (int i = m_nMsgBuffLen - 2; i >= 0; i--)
	{
		checksum += m_pControlMsgBuff[i];
	}
	memcpy(&m_pControlMsgBuff[1 + 2 * CHANNEL_NUM], &checksum, sizeof(uint16_t));

	return 0;
}

int32_t MsgTransmitter::SendControlMsg()
{
	int32_t ret = PackControlMsg();
	if (ret < 0)
	{
		Error("[%p][MsgTransmitter::SendControlMsg] Pack control msg fail,return:%d\n", this, ret);
		return -1;
	}

	int err = sendto(m_nSocket, m_pControlMsgBuff, m_nMsgBuffLen, 0, (struct sockaddr*) &m_RemoteAddr, sizeof(m_RemoteAddr));
	if (err < 0)
	{
		Error("[%p][MsgTransmitter::SendHelloMsg] sendto err:%d\n", this, errno);
		close(m_nSocket);
		m_nSocket = -1;
		return -2;
	}

	return 0;
}

int32_t MsgTransmitter::SendHelloMsg()
{
	if (m_nSocket < 0)
	{
		Error("[%p][MsgTransmitter::SendHelloMsg] socket is invalid\n", this);
		return -1;
	}

	{
		uint8_t tx_buffer[2];
		tx_buffer[0] = MSG_Hello;
		tx_buffer[1] = m_nLocalType;
		int err = sendto(m_nSocket, tx_buffer, sizeof(tx_buffer), 0, (struct sockaddr*) &m_RemoteAddr, sizeof(m_RemoteAddr));
		if (err < 0)
		{
			Error("[%p][MsgTransmitter::SendHelloMsg] sendto err:%d\n", this, errno);
			close(m_nSocket);
			m_nSocket = -1;
			return -2;
		}
	}

	return 0;
}

void MsgTransmitter::wifi_event_handler(
		void *arg,
		esp_event_base_t event_base,
		int32_t event_id,
		void *event_data)
{
	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STACONNECTED)
	{
		wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t*) event_data;
		Info("[MsgTransmitter::WifiEvent] Station "MACSTR" join, AID=%d\n", MAC2STR(event->mac), event->aid);
		tcpip_adapter_ip_info_t info;
		tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_AP, &info);
		m_LocalIp = info.ip;
		Info("[MsgTransmitter::WifiEvent] Local ip:%s\n", ip4addr_ntoa(&m_LocalIp));
		m_bWifiConnected = true;
	}
	else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STADISCONNECTED)
	{
		wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t*) event_data;
		Info("[MsgTransmitter::WifiEvent] Station "MACSTR" leave, AID=%d\n", MAC2STR(event->mac), event->aid);
		m_RemoteIp = {0};
		m_bWifiConnected = false;
		m_bGotRemoteIp = false;
	}
	else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED)
	{
		Info("[MsgTransmitter::WifiEvent] STA connected\n");
		/*tcpip_adapter_ip_info_t info;
		tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_AP, &info);
		m_LocalIp = info.ip;
		Info("[MsgTransmitter::WifiEvent] 222 Local ip:%s\n", ip4addr_ntoa(&m_LocalIp));*/
		m_bWifiConnected = true;
	}
	else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
	{
		if (m_bWifiConnected)
		{
			Info("[MsgTransmitter::WifiEvent] STA disconnected\n");
		}
		m_RemoteIp = {0};
		m_LocalIp = {0};
		m_bWifiConnected = false;
		m_bGotRemoteIp = false;
	}
	else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
	{
		ip_event_got_ip_t *event = (ip_event_got_ip_t*) event_data;
		m_LocalIp = event->ip_info.ip;
		m_RemoteIp = event->ip_info.gw;
		m_bGotRemoteIp = true;
		Info("[MsgTransmitter::WifiEvent] Got local ip:%s\n", ip4addr_ntoa(&m_LocalIp));
		Info("[MsgTransmitter::WifiEvent] Got Remote ip:%s\n", ip4addr_ntoa(&m_RemoteIp));
	}
	else if (event_base == IP_EVENT && event_id == IP_EVENT_AP_STAIPASSIGNED)
	{
		UpdataRemoteIp();
	}
}

int32_t MsgTransmitter::InitWifiAsSoftAP(const char *ssid, const char *password)
{
	Info("[MsgTransmitter::InitWifiAsSoftAP] Enter InitWifiAsSoftAP(ssid:%s pass:%s)\n", ssid, password);

	if (ssid == nullptr || password == nullptr)
	{
		Error("[MsgTransmitter::InitWifiAsSoftAP] ssid:%p or password:%p is null\n", ssid, password);
		return -1;
	}

	if (m_eWifiMode != WIFI_MODE_NULL)
	{
		Error("[MsgTransmitter::InitWifiAsSoftAP] Wifi has init as:%d\n", m_eWifiMode);
		return -2;
	}

	esp_err_t err = nvs_flash_init();
	if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		nvs_flash_erase();
		err = nvs_flash_init();
	}
	if (err != ESP_OK)
	{
		Error("[MsgTransmitter::InitWifiAsSoftAP] Flash init fali,retunn:%d\n", err);
		return -3;
	}

	tcpip_adapter_init();
	err = esp_event_loop_create_default();
	if (err != ESP_OK)
	{
		Error("[MsgTransmitter::InitWifiAsSoftAP] Create default even loop fail,retunn:%d\n", err);
		return -4;
	}

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	err = esp_wifi_init(&cfg);
	if (ESP_OK != err)
	{
		Error("[MsgTransmitter::InitWifiAsSoftAP] Init wifi fail,retunn:%d\n", err);
		return -5;
	}

	err = esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &MsgTransmitter::wifi_event_handler, NULL);
	if (ESP_OK != err)
	{
		Error("[MsgTransmitter::InitWifiAsSoftAP] Reg wifi event handler fail,retunn:%d\n", err);
		return -6;
	}
	err = esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &MsgTransmitter::wifi_event_handler, NULL);
	if (ESP_OK != err)
	{
		Error("[MsgTransmitter::InitWifiAsSoftAP] Reg ip event handler fail,retunn:%d\n", err);
		return -7;
	}

	wifi_config_t config;
	strcpy((char*) config.ap.ssid, ssid);
	config.ap.ssid_len = strlen(ssid);
	config.ap.channel = 1;
	strcpy((char*) config.ap.password, password);
	config.ap.max_connection = 1;
	if (strlen(password) != 0)
	{
		config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;
	}
	else
	{
		config.ap.authmode = WIFI_AUTH_OPEN;
	}

	err = esp_wifi_set_mode(WIFI_MODE_AP);
	if (ESP_OK != err)
	{
		Error("[MsgTransmitter::InitWifiAsSoftAP] Set AP mode fail,retunn:%d\n", err);
		return -8;
	}
	err = esp_wifi_set_config(WIFI_IF_AP, &config);
	if (ESP_OK != err)
	{
		Error("[MsgTransmitter::InitWifiAsSoftAP] Set config fail,retunn:%d\n", err);
		return -9;
	}
	err = esp_wifi_start();
	if (ESP_OK != err)
	{
		Error("[MsgTransmitter::InitWifiAsSoftAP] Start wifi fail,retunn:%d\n", err);
		return -10;
	}

	m_eWifiMode = WIFI_MODE_AP;

	return 0;
}

int32_t MsgTransmitter::InitWifiAsSTA()
{
	Info("[MsgTransmitter::InitWifiAsSTA] Enter InitWifiAsSTA()\n");

	if (m_eWifiMode != WIFI_MODE_NULL)
	{
		Error("[MsgTransmitter::InitWifiAsSTA] Wifi has init as:%d\n", m_eWifiMode);
		return -1;
	}

	esp_err_t err = nvs_flash_init();
	if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		nvs_flash_erase();
		err = nvs_flash_init();
	}
	if (err != ESP_OK)
	{
		Error("[MsgTransmitter::InitWifiAsSTA] Flash init fali,retunn:%d\n", err);
		return -2;
	}

	tcpip_adapter_init();
	err = esp_event_loop_create_default();
	if (err != ESP_OK)
	{
		Error("[MsgTransmitter::InitWifiAsSTA] Create default even loop fail,retunn:%d\n", err);
		return -3;
	}

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	err = esp_wifi_init(&cfg);
	if (ESP_OK != err)
	{
		Error("[MsgTransmitter::InitWifiAsSTA] Init wifi fail,retunn:%d\n", err);
		return -4;
	}

	err = esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &MsgTransmitter::wifi_event_handler, NULL);
	if (ESP_OK != err)
	{
		Error("[MsgTransmitter::InitWifiAsSTA] Reg wifi event handler fail,retunn:%d\n", err);
		return -5;
	}

	err = esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &MsgTransmitter::wifi_event_handler, NULL);
	if (ESP_OK != err)
	{
		Error("[MsgTransmitter::InitWifiAsSTA] Reg ip event handler fail,retunn:%d\n", err);
		return -6;
	}

	err = esp_wifi_set_mode(WIFI_MODE_STA);
	if (ESP_OK != err)
	{
		Error("[MsgTransmitter::InitWifiAsSoftAP] Set STA mode fail,retunn:%d\n", err);
		return -7;
	}

	err = esp_wifi_start();
	if (ESP_OK != err)
	{
		Error("[MsgTransmitter::InitWifiAsSoftAP] Start wifi fail,retunn:%d\n", err);
		return -8;
	}

	m_eWifiMode = WIFI_MODE_STA;

	return 0;
}

int32_t MsgTransmitter::ConnectToAP(const char *ssid, const char *password)
{
	Info("[MsgTransmitter::ConnectToAP] Enter ConnectToAP(ssid:%s pass:%s)\n", ssid, password);

	if (ssid == nullptr || password == nullptr)
	{
		Error("[MsgTransmitter::ConnectToAP] ssid:%p or password:%p is null\n", ssid, password);
		return -1;
	}

	wifi_config_t config = {0};
	strcpy((char*) config.sta.ssid, ssid);
	strcpy((char*) config.sta.password, password);
	config.sta.pmf_cfg.capable = true;
	config.sta.pmf_cfg.required = false;

	esp_err_t err = esp_wifi_set_config(WIFI_IF_STA, &config);
	if (err != ESP_OK)
	{
		Error("[MsgTransmitter::ConnectToAP] Set config fail,return:%d\n", err);
		return -2;
	}

	err = esp_wifi_connect();
	if (ESP_OK != err)
	{
		Error("[MsgTransmitter::InitWifiAsSoftAP] Connect wifi fail,retunn:%d\n", err);
		return -3;
	}

	return 0;
}

int32_t MsgTransmitter::CloseWifi()
{
	Info("[MsgTransmitter::CloseWifi] Enter CloseWifi()\n");

	esp_wifi_stop();
	esp_wifi_deinit();
	m_LocalIp = {0};
	m_RemoteIp = {0};
	m_bWifiConnected = false;
	m_bGotRemoteIp = false;
	m_eWifiMode = WIFI_MODE_NULL;

	return 0;
}

int32_t MsgTransmitter::UpdataRemoteIp()
{
	Info("[MsgTransmitter::UpdataRemoteIp] Enter UpdataRemoteIp()\n");

	wifi_sta_list_t wifiStaList;
	esp_err_t err = esp_wifi_ap_get_sta_list(&wifiStaList);
	if (err != ESP_OK)
	{
		Error("[MsgTransmitter::UpDataRemoteIp] Wifi get sta list fail,return:%d\n", err);
		return -1;
	}

	tcpip_adapter_sta_list_t tcpipStaList;
	err = tcpip_adapter_get_sta_list(&wifiStaList, &tcpipStaList);
	if (err != ESP_OK)
	{
		Error("[MsgTransmitter::UpDataRemoteIp] Adapter get sta fail,return:%d\n", err);
		return -2;
	}

	if (tcpipStaList.num > 0 && tcpipStaList.sta[0].ip.addr != 0)
	{
		m_RemoteIp = tcpipStaList.sta[0].ip;
		m_bGotRemoteIp = true;
		Info("[MsgTransmitter::UpDataRemoteIp] Updata remote ip:"IPSTR"\n", IP2STR(&m_RemoteIp));
	}
	else
	{
		Info("[MsgTransmitter::UpDataRemoteIp] No sta connected\n");
	}

	return 0;
}
