/*
 * Driver.h
 *
 *  Created on: 2021Äê12ÔÂ19ÈÕ
 *      Author: Qufenglu
 */

#ifndef MAIN_DRIVER_DRIVER_H_
#define MAIN_DRIVER_DRIVER_H_

class Driver
{
public:
	virtual ~Driver(){};
	virtual int Init() = 0;
	virtual int Move2Posture(const uint16_t &throttle,const uint16_t &yaw,const float &pitch,const float &roll) = 0;

	virtual int LockMotor() = 0;
	virtual int UnlockMotor() = 0;
	virtual int SetMotorIdleSpeed(const uint16_t &speed) = 0;
};

#endif /* MAIN_DRIVER_DRIVER_H_ */
