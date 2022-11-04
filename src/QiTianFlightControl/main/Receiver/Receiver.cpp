/*
 * Receiver.cpp
 *
 *  Created on: 2021Äê12ÔÂ19ÈÕ
 *      Author: Qufenglu
 */

#include "Receiver.h"
#include "Log.h"

Receiver::Receiver()
{
	m_pMsgTransmitter = nullptr;
}

Receiver::~Receiver()
{
	ReleaseAll();
}

int32_t Receiver::Init()
{
	return 0;
}

int32_t Receiver::ReleaseAll()
{
	if (m_pMsgTransmitter != nullptr)
	{
		delete m_pMsgTransmitter;
		m_pMsgTransmitter = nullptr;
	}

	return 0;
}

int32_t Receiver::StartTransmitMsg(const int8_t &mode, const uint16_t &port)
{
	if (m_pMsgTransmitter == nullptr)
	{
		m_pMsgTransmitter = new (std::nothrow) MsgTransmitter();
		if (m_pMsgTransmitter == nullptr)
		{
			Error("[%p][Receiver::StartTransmitMsg] New MsgTransmitter fail\n", this);
			return -1;
		}

		int32_t ret = m_pMsgTransmitter->Init();
		if (ret < 0)
		{
			Error("[%p][Receiver::StartTransmitMsg] Init m_pMsgTransmitter fail,return %d\n", this, ret);
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
			Error("[%p][Receiver::StartTransmitMsg] Work as client fail,return %d\n", this, ret);
			return -2;
		}
	}
	else if (mode == 1)
	{
		int32_t ret = m_pMsgTransmitter->WorkAsServer(port);
		if (ret < 0)
		{
			Error("[%p][Receiver::StartTransmitMsg] Work as server fail,return %d\n", this, ret);
			return -3;
		}
	}
	else
	{
		Error("[%p][Receiver::StartTransmitMsg] Unknow mode %d\n", this, mode);
		return -4;
	}

	return 0;
}

int32_t Receiver::StopTransmitMsg()
{
	if (m_pMsgTransmitter != nullptr)
	{
		delete m_pMsgTransmitter;
		m_pMsgTransmitter = nullptr;
	}
	return 0;
}

int32_t Receiver::GetControlMsg(ControlMsg &controlMsg)
{
	if (m_pMsgTransmitter == nullptr)
	{
		Error("[%p][Receiver::GetControlMsg] MsgTransmitter is null,maybe not StartTransmitMsg\n", this);
		return -1;
	}

	return m_pMsgTransmitter->GetControlMsg(controlMsg);
}
