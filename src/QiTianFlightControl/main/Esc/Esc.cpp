/*
 * ESC.cpp
 *
 *  Created on: 2021Äê12ÔÂ20ÈÕ
 *      Author: Qufenglu
 */

#include <cstring>
#include "Esc.h"
#include "Log.h"

ledc_timer_config_t Esc::m_TimerConfig;

Esc::Esc(const ledc_channel_config_t &config)
{
	memcpy(&m_ChannelConfig, &config, sizeof(ledc_channel_config_t));
}

Esc::~Esc()
{

}

int32_t Esc::Init()
{
	Info("[%p][Esc::Init] Enter Init()\n", this);

	esp_err_t err = ledc_timer_config(&m_TimerConfig);
	if (err != ESP_OK)
	{
		Error("[%p][Esc::Init] Config timer fail,return:%d\n", this, err);
		return -1;
	}
	err = ledc_channel_config(&m_ChannelConfig);
	if (err != ESP_OK)
	{
		Error("[%p][Esc::Init] Config channel fail,return:%d\n", this, err);
		return -2;
	}

	err = ledc_set_duty(m_ChannelConfig.speed_mode, m_ChannelConfig.channel, m_ChannelConfig.duty);
	if (err != ESP_OK)
	{
		Warn("[%p][Esc::Init] Set duty fail,return:%d\n", this, err);
	}
	err = ledc_update_duty(m_ChannelConfig.speed_mode, m_ChannelConfig.channel);
	if (err != ESP_OK)
	{
		Warn("[%p][Esc::Init] Update duty fail,return:%d\n", this, err);
	}

	return 0;
}

int32_t Esc::SetDuty(uint32_t duty)
{
	esp_err_t err = ledc_set_duty(m_ChannelConfig.speed_mode, m_ChannelConfig.channel, duty);
	if (err != ESP_OK)
	{
		Error("[%p][Esc::SetDuty] Set duty:%d fail,return:%d\n", this, duty, err);
		return -1;
	}

	err = ledc_update_duty(m_ChannelConfig.speed_mode, m_ChannelConfig.channel);
	if (err != ESP_OK)
	{
		Error("[%p][Esc::SetDuty] update duty:%d fail,return:%d\n", this, duty, err);
		return -2;
	}

	return 0;
}

void Esc::InitEscTimer(const ledc_timer_config_t &config)
{
	Info("[Esc::InitEscTimer] Enter InitEscTimer\n");
	memcpy(&m_TimerConfig, &config, sizeof(ledc_timer_config_t));
}
