/*
 * Receiver.h
 *
 *  Created on: 2021年12月19日
 *      Author: Qufenglu
 */

#ifndef MAIN_RECEIVER_RECEIVER_H_
#define MAIN_RECEIVER_RECEIVER_H_

#include "MsgTransmitter.h"

class Receiver
{
private:
	MsgTransmitter *m_pMsgTransmitter;

private:
	int32_t ReleaseAll();

public:
	Receiver();
	~Receiver();

	int32_t Init();

	//mode: 工作模式,0-客户端 1-服务端
	int32_t StartTransmitMsg(const int8_t &mode, const uint16_t &port);
	int32_t StopTransmitMsg();

	int32_t GetControlMsg(ControlMsg &controlMsg);
};

#endif /* MAIN_RECEIVER_RECEIVER_H_ */
