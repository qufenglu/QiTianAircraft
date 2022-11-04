/*
 * SwitchChannel.cpp
 *
 *  Created on: 2021Äê12ÔÂ5ÈÕ
 *      Author: Qufenglu
 */

#include "SwitchChannel.h"
#include "Log.h"

SwitchChannel::SwitchChannel()
{
	m_eChannelType = CH_SWITCH;
	m_bReverseChanne = false;
	m_nMinValue = CHANNEL_MIN_VALUE;
	m_nMaxValue = CHANNEL_MAX_VALUE;
	m_gpio1 = GPIO_NUM_NC;
	m_gpio2 = GPIO_NUM_NC;
	m_nRange = m_nMaxValue - m_nMinValue;
}

SwitchChannel::~SwitchChannel()
{

}

int32_t SwitchChannel::Init(const gpio_num_t &gpio1, const gpio_num_t &gpio2)
{
	Info("[%p][SwitchChannel::Init] Enter Init(gpio1:%d gpio2:%d)\n", this, gpio1, gpio2);

	gpio_config_t config;
	config.intr_type = GPIO_INTR_DISABLE;
	config.mode = GPIO_MODE_INPUT;
	config.pin_bit_mask = 1ULL << gpio1;
	config.pull_down_en = GPIO_PULLDOWN_ENABLE;
	config.pull_up_en = GPIO_PULLUP_DISABLE;

	esp_err_t ret = gpio_config(&config);
	{
		if (ret != ESP_OK)
		{
			Error("[%p][SwitchChannel::Init] Config gpio1:%d fail,return:%d\n", this, gpio1, ret);
			return -1;
		}
	}

	config.pin_bit_mask = 1ULL << gpio2;
	ret = gpio_config(&config);
	{
		if (ret != ESP_OK)
		{
			Error("[%p][SwitchChannel::Init] Config gpio2:%d fail,return:%d\n", this, gpio2, ret);
			return -2;
		}
	}

	m_gpio1 = gpio1;
	m_gpio2 = gpio2;

	return 0;
}

int32_t SwitchChannel::GetChannelValue()
{
	int level1 = gpio_get_level(m_gpio1);
	int level2 = gpio_get_level(m_gpio2);

	int32_t value = 0;	//0:low 1:medium 2:high
	if (level1 == 0 && level2 == 0)
	{
		value = 1;
	}
	else if (level1 == 1 && level2 == 0)
	{
		value = 0;
	}
	else if (level1 == 0 && level2 == 1)
	{
		value = 2;
	}
	else
	{
		Error("[%p][SwitchChannel::GetChannelValue] Unknow IO state\n", this);
		return -1;
	}

	if (m_bReverseChanne && value == 0)
	{
		value = 2;
	}
	else if (m_bReverseChanne && value == 2)
	{
		value = 0;
	}

	switch (value)
	{
	case 0:
		return m_nMinValue;
	case 1:
		return (m_nMinValue + m_nMaxValue) / 2;
	case 2:
		return m_nMaxValue;
	}

	Error("[%p][SwitchChannel::GetChannelValue] Unknow value state\n", this);
	return -2;
}

int32_t SwitchChannel::ReverseChannel(const bool &reverse)
{
	Info("[%p][SwitchChannel::ReverseChannel] Enter ReverseChannel(reverse:%d)\n", this, reverse);

	m_bReverseChanne = reverse;
	return 0;
}

int32_t SwitchChannel::SetChannelRange(const int32_t &min, const int32_t &max)
{
	Info("[%p][SwitchChannel::SetChannelRange] Enter SetChannelRange(min:%d max:%d)\n", this, min, max);

	m_nMinValue = min;
	m_nMaxValue = max;
	m_nRange = m_nMaxValue - m_nMinValue;
	return 0;
}
