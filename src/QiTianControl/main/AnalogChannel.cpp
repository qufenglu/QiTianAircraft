/*
 * AnalogChannel.cpp
 *
 *  Created on: 2021年12月2日
 *      Author: Qufenglu
 */
#include "Log.h"
#include "AnalogChannel.h"
#include "driver/adc.h"

//设置ADC采样位数只用设置1此
bool g_bHasConfigAdc1 = false;
bool g_bHasConfigAdc2 = false;

bool GetAdcNo(const gpio_num_t &gpio, int &adcNo, int &channel)
{

	adcNo = gpio == GPIO_NUM_36 ? 1 : gpio == GPIO_NUM_37 ? 1 : gpio == GPIO_NUM_38 ? 1 : gpio == GPIO_NUM_39 ? 1 : gpio == GPIO_NUM_32 ? 1 :
			gpio == GPIO_NUM_33 ? 1 : gpio == GPIO_NUM_34 ? 1 : gpio == GPIO_NUM_35 ? 1 : -1;

	if (adcNo != 1)
	{
		adcNo = gpio == GPIO_NUM_4 ? 2 : gpio == GPIO_NUM_0 ? 2 : gpio == GPIO_NUM_2 ? 2 : gpio == GPIO_NUM_15 ? 2 : gpio == GPIO_NUM_13 ? 2 :
				gpio == GPIO_NUM_12 ? 2 : gpio == GPIO_NUM_14 ? 2 : gpio == GPIO_NUM_27 ? 2 : gpio == GPIO_NUM_25 ? 2 : gpio == GPIO_NUM_26 ? 2 : -1;
	}

	if (adcNo == 1)
	{
		channel = gpio == GPIO_NUM_36 ? ADC1_CHANNEL_0 : gpio == GPIO_NUM_37 ? ADC1_CHANNEL_1 : gpio == GPIO_NUM_38 ? ADC1_CHANNEL_2 :
					gpio == GPIO_NUM_39 ? ADC1_CHANNEL_3 : gpio == GPIO_NUM_32 ? ADC1_CHANNEL_4 : gpio == GPIO_NUM_33 ? ADC1_CHANNEL_5 :
					gpio == GPIO_NUM_34 ? ADC1_CHANNEL_6 : gpio == GPIO_NUM_35 ? ADC1_CHANNEL_7 : ADC1_CHANNEL_MAX;
	}
	else if (adcNo == 2)
	{
		channel = gpio == GPIO_NUM_4 ? ADC2_CHANNEL_0 : gpio == GPIO_NUM_0 ? ADC2_CHANNEL_1 : gpio == GPIO_NUM_2 ? ADC2_CHANNEL_2 :
					gpio == GPIO_NUM_15 ? ADC2_CHANNEL_3 : gpio == GPIO_NUM_13 ? ADC2_CHANNEL_4 : gpio == GPIO_NUM_12 ? ADC2_CHANNEL_5 :
					gpio == GPIO_NUM_14 ? ADC2_CHANNEL_6 : gpio == GPIO_NUM_27 ? ADC2_CHANNEL_7 : gpio == GPIO_NUM_25 ? ADC2_CHANNEL_8 :
					gpio == GPIO_NUM_26 ? ADC2_CHANNEL_9 : ADC2_CHANNEL_MAX;
	}

	if (adcNo == -1)
	{
		return false;
	}
	else
	{
		return true;
	}
}

AnalogChannel::AnalogChannel()
{
	m_eChannelType = CH_ANALOG;
	m_bReverseChanne = false;
	m_nMinValue = CHANNEL_MIN_VALUE;
	m_nMaxValue = CHANNEL_MAX_VALUE;
	m_nRange = m_nMaxValue - m_nMinValue;
	m_nChannel = -1;
	m_nAdcNo = -1;
}

AnalogChannel::~AnalogChannel()
{

}

int32_t AnalogChannel::Init(const gpio_num_t &gpio)
{
	Info("[%p][AnalogChannel::Init] Enter Init(gpio:%d)\n", this, gpio);

	int adcNo = 0;
	int channel = 0;
	if (!GetAdcNo(gpio, adcNo, channel))
	{
		Error("[%p][AnalogChannel::Init] Can not find ADC by gpio:%d\n", this, gpio);
		return -1;
	}

	if (adcNo == 1 && !g_bHasConfigAdc1)
	{
		adc1_config_width(ADC_WIDTH_BIT_12);
		g_bHasConfigAdc1 = true;
	}
	else if (adcNo == 2 && !g_bHasConfigAdc2)
	{
		g_bHasConfigAdc2 = true;
	}

	m_nAdcNo = adcNo;
	m_nChannel = channel;

	if (adcNo == 1)
	{
		if (ESP_OK != adc1_config_channel_atten((adc1_channel_t) channel, ADC_ATTEN_DB_11))
		{
			Error("[%p][AnalogChannel::Init] Config channel atten fail,adcNo:%d channel:%d\n", this, adcNo, channel);
			return -2;
		}
	}

	return 0;
}

int32_t AnalogChannel::GetChannelValue()
{
	int value = 0;
	if (m_nAdcNo == 1)
	{
		value = adc1_get_raw((adc1_channel_t) m_nChannel);
	}
	else
	{
		//return adc2_get_raw(m_nChannel);
		return 0;
	}

	value = value * m_nRange / 4096;

	if (m_bReverseChanne)
	{
		value = m_nMaxValue - value;
	}
	else
	{
		value += m_nMinValue;
	}

	value += ( (value - 1000) * 0.13636364);
	if(value > 2000)
	{
		value = 2000;
	}

	return value;
}

int32_t AnalogChannel::ReverseChannel(const bool &reverse)
{
	Info("[%p][AnalogChannel::ReverseChannel] Enter ReverseChannel(reverse:%d)\n", this, reverse);

	m_bReverseChanne = reverse;
	return 0;
}

int32_t AnalogChannel::SetChannelRange(const int32_t &min, const int32_t &max)
{
	Info("[%p][AnalogChannel::SetChannelRange] Enter SetChannelRange(min:%d max:%d)\n", this, min, max);

	if (max <= min)
	{
		Error("[%p][AnalogChannel::SetChannelRange] max:%d <= min:%d\n", this, max, min);
		return -1;
	}

	m_nMinValue = min;
	m_nMaxValue = max;
	m_nRange = m_nMaxValue - m_nMinValue;

	return 0;
}
