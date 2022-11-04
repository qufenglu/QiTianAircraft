/*
 * Channel.h
 *
 *  Created on: 2021��12��1��
 *      Author: Qufenglu
 */

#ifndef MAIN_CHANNEL_H_
#define MAIN_CHANNEL_H_

#include "driver/gpio.h"

#define CHANNEL_MIN_VALUE 1000
#define CHANNEL_MAX_VALUE 2000

enum ControlChannelType
{
	CH_UNKNOWN = 0,
	CH_SWITCH,
	CH_ANALOG
};

//ͨ���������Ⱪ¶�Ľӿ�
class Channel
{
public:
	virtual ~Channel(){};
	virtual ControlChannelType GetChannelType() = 0;
	virtual int32_t GetChannelValue() = 0;
	virtual int32_t ReverseChannel(const bool &reverse) = 0;
	virtual int32_t SetChannelRange(const int32_t &min, const int32_t &max) = 0;
};

#endif /* MAIN_CHANNEL_H_ */
