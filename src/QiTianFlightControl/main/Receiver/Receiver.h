/*
 * Receiver.h
 *
 *  Created on: 2021��12��19��
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

	//mode: ����ģʽ,0-�ͻ��� 1-�����
	int32_t StartTransmitMsg(const int8_t &mode, const uint16_t &port);
	int32_t StopTransmitMsg();

	int32_t GetControlMsg(ControlMsg &controlMsg);
};

#endif /* MAIN_RECEIVER_RECEIVER_H_ */
