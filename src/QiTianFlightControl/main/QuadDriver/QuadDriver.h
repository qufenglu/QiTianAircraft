/*
 * QuadDriver.h
 *
 *  Created on: 2021Äê12ÔÂ19ÈÕ
 *      Author: Qufenglu
 */

#ifndef MAIN_QUADDRIVER_QUADDRIVER_H_
#define MAIN_QUADDRIVER_QUADDRIVER_H_

#include <array>
#include <memory>
#include "Driver.h"
#include "Esc.h"
#include "mpu6050.hpp"
#include "PostionPIDCtr.h"

class QuadDriver: public Driver
{
public:
	QuadDriver();
	virtual ~QuadDriver();
	virtual int Init();

	virtual int Move2Posture(const uint16_t &throttle, const uint16_t &yaw, const float &pitch, const float &roll);

	virtual int LockMotor();
	virtual int UnlockMotor();
	virtual int SetMotorIdleSpeed(const uint16_t &speed);

	void SteGyro(std::shared_ptr<MPU6050> &pMPU6050);
	void Adjust();

private:
	int32_t ReleaseAll();
	int32_t SetDutys(const uint16_t &duty1, const uint16_t &duty2, const uint16_t &duty3, const uint16_t &duty4);

private:
	std::array<Esc*, 4> m_EscArray;
	std::shared_ptr<MPU6050> m_pMPU6050;
	uint16_t m_nIdleSpeed;
	bool m_bLocked;

	PostionPIDCtr m_AnglePitchPIDCtr;
	PostionPIDCtr m_AngleRollPIDCtr;
	PostionPIDCtr m_OmagePitchPIDCtr;
	PostionPIDCtr m_OmageRollPIDCtr;
	PostionPIDCtr m_OmageYawPIDCtr;

	float m_OmagePitchOut;
	float m_OmageRollOut;
	float m_PwmPitchOut;
	float m_PwmRollOut;
	float m_PwmYawOut;

	int64_t m_nPrevAngleCtrTime;
	int64_t m_nPrevOmageCtrTime;
};

#endif /* MAIN_QUADDRIVER_QUADDRIVER_H_ */
