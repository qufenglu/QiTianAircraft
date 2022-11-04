/*
 * SwitchChannel.h
 *
 *  Created on: 2021Äê12ÔÂ5ÈÕ
 *      Author: Qufenglu
 */

#ifndef MAIN_SWITCHCHANNEL_H_
#define MAIN_SWITCHCHANNEL_H_

#include "Channel.h"

class SwitchChannel: public Channel
{
private:
	ControlChannelType m_eChannelType;
	bool m_bReverseChanne;
	uint32_t m_nMinValue;
	uint32_t m_nMaxValue;
	gpio_num_t m_gpio1;
	gpio_num_t m_gpio2;
	uint32_t m_nRange;

public:
	SwitchChannel();
	virtual ~SwitchChannel();

	int32_t Init(const gpio_num_t &gpio1, const gpio_num_t &gpio2);
	inline virtual ControlChannelType GetChannelType()
	{
		return m_eChannelType;
	}
	;
	virtual int32_t GetChannelValue();
	virtual int32_t ReverseChannel(const bool &reverse);
	virtual int32_t SetChannelRange(const int32_t &min, const int32_t &max);
};

#endif /* MAIN_SWITCHCHANNEL_H_ */
