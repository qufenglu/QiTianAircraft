/*
 * QiTianControl.cpp
 *
 *  Created on: 2021Äê12ÔÂ15ÈÕ
 *      Author: Qufenglu
 */

#include "QiTianControl.h"
#include "Log.h"

QiTianControl::QiTianControl()
{
	m_pMsgTransmitter = nullptr;;
	m_bStopSample = true;
	m_pSampleThread = nullptr;
}

QiTianControl::~QiTianControl()
{
	ReleaseAll();
}

int32_t QiTianControl::Init()
{
	Info("[%p][QiTianControl::Init] Enter Init()\n", this);

	ReleaseAll();

	AnalogChannel *ch1 = new (std::nothrow) AnalogChannel();
	if (ch1 == nullptr)
	{
		Error("[%p][QiTianControl::Init] new AnalogChannel ch1 fail\n", this);
		return -1;
	}
	int32_t ret = ch1->Init(GPIO_NUM_34);
	if (ret < 0)
	{
		Error("[%p][QiTianControl::Init] Init AnalogChannel ch1 fail,return:%d\n", this, ret);
		return -2;
	}
	m_ChannelList.push_back(ch1);

	AnalogChannel *ch2 = new (std::nothrow) AnalogChannel();
	if (ch2 == nullptr)
	{
		ReleaseAll();
		Error("[%p][QiTianControl::Init] new AnalogChannel ch2 fail\n", this);
		return -3;
	}
	ret = ch2->Init(GPIO_NUM_35);
	if (ret < 0)
	{
		delete ch2;
		ReleaseAll();
		Error("[%p][QiTianControl::Init] Init AnalogChannel ch2 fail,return:%d\n", this, ret);
		return -4;
	}
	m_ChannelList.push_back(ch2);

	AnalogChannel *ch3 = new (std::nothrow) AnalogChannel();
	if (ch3 == nullptr)
	{
		ReleaseAll();
		Error("[%p][QiTianControl::Init] new AnalogChannel ch3 fail\n", this);
		return -5;
	}
	ret = ch3->Init(GPIO_NUM_32);
	if (ret < 0)
	{
		delete ch3;
		ReleaseAll();
		Error("[%p][QiTianControl::Init] Init AnalogChannel ch3 fail,return:%d\n", this, ret);
		return -6;
	}
	m_ChannelList.push_back(ch3);

	AnalogChannel *ch4 = new (std::nothrow) AnalogChannel();
	if (ch4 == nullptr)
	{
		ReleaseAll();
		Error("[%p][QiTianControl::Init] new AnalogChannel ch4 fail\n", this);
		return -7;
	}
	ret = ch4->Init(GPIO_NUM_33);
	if (ret < 0)
	{
		delete ch4;
		ReleaseAll();
		Error("[%p][QiTianControl::Init] Init AnalogChannel ch4 fail,return:%d\n", this, ret);
		return -8;
	}
	m_ChannelList.push_back(ch4);

	SwitchChannel *ch5 = new (std::nothrow) SwitchChannel();
	if (ch5 == nullptr)
	{
		ReleaseAll();
		Error("[%p][QiTianControl::Init] new SwitchChannel ch5 fail\n", this);
		return -9;
	}
	ret = ch5->Init(GPIO_NUM_18, GPIO_NUM_19);
	if (ret < 0)
	{
		delete ch5;
		ReleaseAll();
		Error("[%p][QiTianControl::Init] Init SwitchChannel ch4 fail,return:%d\n", this, ret);
		return -10;
	}
	m_ChannelList.push_back(ch5);

	return 0;
}

int32_t QiTianControl::ReleaseAll()
{
	Info("[%p][QiTianControl::ReleaseAll] Enter ReleaseAll()\n", this);

	StopSample();
	StopTransmitMsg();

	if (m_pMsgTransmitter != nullptr)
	{
		delete m_pMsgTransmitter;
		m_pMsgTransmitter = nullptr;
	}

	for (Channel *channel : m_ChannelList)
	{
		delete channel;
	}
	m_ChannelList.clear();

	return 0;
}

void QiTianControl::SampleThread()
{
	Info("[%p][QiTianControl::SampleThread] Enter SampleThread()\n", this);

	m_bStopSample = false;
	ControlMsg msg;
	uint8_t chNo = 0;

	while (!m_bStopSample)
	{
		chNo = 0;
		for (Channel *channel : m_ChannelList)
		{
			msg.chValues[chNo++] = channel->GetChannelValue();
		}

		while (chNo < CHANNEL_NUM)
		{
			msg.chValues[chNo++] = CHANNEL_MIN_VALUE;
		}

		if (m_pMsgTransmitter != nullptr)
		{
			m_pMsgTransmitter->UpdateControlMsg(msg);
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(20));
	}
}

int32_t QiTianControl::StartSample()
{
	Info("[%p][QiTianControl::StartSample] Enter StartSample()\n", this);

	StopSample();

	m_pSampleThread = new (std::nothrow) std::thread(&QiTianControl::SampleThread, this);
	if (m_pSampleThread == nullptr)
	{
		Error("[%p][QiTianControl::Init] new SampleThread fail\n", this);
		return -1;
	}

	return 0;
}

int32_t QiTianControl::StopSample()
{
	Info("[%p][QiTianControl::StopSample] Enter StopSample()\n", this);

	m_bStopSample = true;
	if (m_pSampleThread != nullptr)
	{
		if (m_pSampleThread->joinable())
		{
			m_pSampleThread->join();
		}

		delete m_pSampleThread;
		m_pSampleThread = nullptr;
	}

	return 0;
}

int32_t QiTianControl::StartTransmitMsg(const int8_t &mode, const uint16_t &port)
{
	Info("[%p][QiTianControl::StartTransmitMsg] Enter StartTransmitMsg(mode:%d port:%d)\n", this, mode, port);

	if (m_pMsgTransmitter == nullptr)
	{
		m_pMsgTransmitter = new (std::nothrow) MsgTransmitter();
		if (m_pMsgTransmitter == nullptr)
		{
			Error("[%p][QiTianControl::StartTransmitMsg] New MsgTransmitter fail\n", this);
			return -1;
		}

		int32_t ret = m_pMsgTransmitter->Init();
		if (ret < 0)
		{
			Error("[%p][QiTianControl::StartTransmitMsg] Init m_pMsgTransmitter fail,return %d\n", this, ret);
			delete m_pMsgTransmitter;
			m_pMsgTransmitter = nullptr;
			return -2;
		}
	}

	if (mode == 0)
	{
		int32_t ret = m_pMsgTransmitter->WorkAsClient(port);
		if (ret < 0)
		{
			Error("[%p][QiTianControl::StartTransmitMsg] Work as client fail,return %d\n", this, ret);
			return -2;
		}
	}
	else if (mode == 1)
	{
		int32_t ret = m_pMsgTransmitter->WorkAsServer(port);
		if (ret < 0)
		{
			Error("[%p][QiTianControl::StartTransmitMsg] Work as server fail,return %d\n", this, ret);
			return -3;
		}
	}
	else
	{
		Error("[%p][QiTianControl::StartTransmitMsg] Unknow mode %d\n", this, mode);
		return -4;
	}

	return 0;
}

int32_t QiTianControl::StopTransmitMsg()
{
	Info("[%p][QiTianControl::StopTransmitMsg] Enter StopTransmitMsg\n", this);

	if(m_pMsgTransmitter != nullptr)
	{
		delete m_pMsgTransmitter;
		m_pMsgTransmitter = nullptr;
	}
	return 0;
}
