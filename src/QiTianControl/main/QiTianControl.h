/*
 * QiTianControl.h
 *
 *  Created on: 2021年12月15日
 *      Author: Qufenglu
 */

#ifndef MAIN_QITIANCONTROL_H_
#define MAIN_QITIANCONTROL_H_

#include <list>
#include "MsgTransmitter.h"
#include "AnalogChannel.h"
#include "SwitchChannel.h"

class QiTianControl
{
private:
	MsgTransmitter* m_pMsgTransmitter;
	std::list<Channel*> m_ChannelList;

	bool m_bStopSample;
	std::thread* m_pSampleThread;
	void SampleThread();

	//std::function<void()>* m_pOnConnectFunction;
	//std::function<void()>* m_pOnDisConnectFunction;
	void OnConnect();
	void nDisConnect();

private:
	int32_t ReleaseAll();

public:
	QiTianControl();
	~QiTianControl();

	int32_t Init();

	int32_t StartSample();
	int32_t StopSample();

	//mode: 工作模式,0-客户端 1-服务端
	int32_t StartTransmitMsg(const int8_t &mode, const uint16_t &port);
	int32_t StopTransmitMsg();
};


#endif /* MAIN_QITIANCONTROL_H_ */
