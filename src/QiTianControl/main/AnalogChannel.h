/*
 * AnalogChannel.h
 *
 *  Created on: 2021Äê12ÔÂ1ÈÕ
 *      Author: Qufenglu
 */

#ifndef MAIN_ANALOGCHANNEL_H_
#define MAIN_ANALOGCHANNEL_H_

#include "Channel.h"

class AnalogChannel: public Channel
{
private:
	ControlChannelType m_eChannelType;
	bool m_bReverseChanne;
	uint32_t m_nMinValue;
	uint32_t m_nMaxValue;
	uint32_t m_nRange;
	int m_nChannel;
	int m_nAdcNo;

public:
	AnalogChannel();
	virtual ~AnalogChannel();

	int32_t Init(const gpio_num_t &gpio);
	inline virtual ControlChannelType GetChannelType(){return m_eChannelType;};
	virtual int32_t GetChannelValue();
	virtual int32_t ReverseChannel(const bool &reverse);
	virtual int32_t SetChannelRange(const int32_t &min, const int32_t &max);
};

#endif /* MAIN_ANALOGCHANNEL_H_ */
