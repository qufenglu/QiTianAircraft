/*
 * ESC.h
 *
 *  Created on: 2021年12月20日
 *      Author: Qufenglu
 */

#ifndef MAIN_ESC_ESC_H_
#define MAIN_ESC_ESC_H_

#include "driver/ledc.h"

class Esc
{
private:
	ledc_channel_config_t m_ChannelConfig;

public:
	Esc(const ledc_channel_config_t &config);
	virtual ~Esc();

	int32_t Init();
	int32_t SetDuty(uint32_t duty);

	static void InitEscTimer(const ledc_timer_config_t &config);

private:
	//生成pwm的timer,所有pwm共用一个
	static ledc_timer_config_t m_TimerConfig;
};

#endif /* MAIN_ESC_ESC_H_ */
