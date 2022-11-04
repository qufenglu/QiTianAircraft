/*
 * MsgTransmitter.h
 *
 *  Created on: 2021��12��5��
 *      Author: Qufenglu
 */

#ifndef MAIN_MSGTRANSMITTER_H_
#define MAIN_MSGTRANSMITTER_H_

#include <inttypes.h>
#include <thread>
#include <mutex>
#include <functional>
#include <set>
#include "esp_wifi.h"
#include "lwip/sockets.h"

//10��ͨ��
#define CHANNEL_NUM 10

//typedef std::function<void()> RemoteConnectedCallback;
//typedef std::function<void()> RemoteDisConnectedCallback;

/*************************
 * ��Ϣ��װ����
 * Version+MsgType+MsgLength+MsgContent+CheckSum
 * Version:�汾(2bit��Ŀǰ������00)
 * MsgType:��Ϣ����(6bit)
 * MsgLength:��Ϣ����(16bit,��ѡ����MsgTypeѡ��,����
 * ������Ϣ����)
 * MsgContent:��Ϣ����
 * CheckSum:У���(16bit)
 *************************/

enum MsgType
{
	MSG_Control = 0,
	MSG_Hello,
	MSG_UNKNOW
};

typedef struct ControlMsg
{
	uint16_t chValues[CHANNEL_NUM];
} ControlMsg;

class MsgTransmitter
{
private:
	//uint8_t *m_pControlMsgBuff;	//��ֹ���������ڴ棬��ʼ��ʱ������
	uint16_t m_nControlMsgbuffLen;
	bool m_bIsInitialized;
	int m_nSocket;		//����socket
	uint8_t m_nLocalType;		//�����ն�����
	uint8_t m_nRemoteType;		//Զ���ն�����
	struct sockaddr_in m_RemoteAddr;	//Զ�˵�ַ
	int8_t m_nWorkMode;		//����ģʽ,0-�ͻ��� 1-�����

	ControlMsg m_ControlMsg;
	std::mutex m_ControlMsgLock;

	bool m_bStopWork;
	std::thread *m_pClientThread;
	std::thread *m_pServerThread;

private:
	static wifi_mode_t m_eWifiMode;
	static bool m_bWifiConnected;
	static bool m_bGotRemoteIp;
	static ip4_addr_t m_LocalIp;
	static ip4_addr_t m_RemoteIp;

private:
	int32_t UnpackControlMsg(uint8_t* buff,const uint8_t len);
	int32_t MsgProcess();

	void ClientThread(uint16_t port);
	int32_t BindSocket(const uint16_t &port);
	int32_t ListenClient();

	void ServerThread(uint16_t port);
	int32_t MakeClientSocket(const uint16_t &port);
	int32_t Connect2Server();

	static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
	static int32_t UpdataRemoteIp();

public:
	MsgTransmitter();
	~MsgTransmitter();

	int32_t Init();
	int32_t ReleaseAll();
	int32_t WorkAsClient(const uint16_t &port);
	int32_t WorkAsServer(const uint16_t &port);
	int32_t CloseClient();
	int32_t CloseServer();
	int32_t SendHelloMsg();

	int32_t GetControlMsg(ControlMsg &controlMsg);

	inline void SetTerminalType(const uint8_t &type){m_nLocalType = type;};
	inline uint8_t GetTerminalType(){return m_nLocalType;};
	inline int8_t GetWorkMode(){return m_nWorkMode;};


	static int32_t InitWifiAsSoftAP(const char *ssid, const char *password);
	static int32_t InitWifiAsSTA();
	static int32_t ConnectToAP(const char *ssid, const char *password);
	static int32_t CloseWifi();
	static inline wifi_mode_t GetWifiMode(){return m_eWifiMode;};
	static inline bool IsWifiConnected(){return m_bWifiConnected;};
	static inline bool IsGotRemoteIp(){return m_bGotRemoteIp;};
	static inline ip4_addr_t GetLocalIp(){return m_LocalIp;};
	static inline ip4_addr_t GetRemoteIp(){return m_RemoteIp;};
};

#endif /* MAIN_MSGTRANSMITTER_H_ */
