/*
 * Gyro.h
 *
 *  Created on: 2021Äê12ÔÂ19ÈÕ
 *      Author: Qufenglu
 */

#ifndef MAIN_GYRO_GYRO_H_
#define MAIN_GYRO_GYRO_H_

#include <thread>
#include <mutex>
#include "driver/gpio.h"

class Gyro
{
public:
	Gyro();
	virtual ~Gyro();

	int32_t Init();
	int32_t StartReadData();
	int32_t StopReadData();

	int32_t GetAcceleration(float &fAccX, float &fAccY, float &fAccZ);
	int32_t GetAngle(float &fAngleX, float &fAngleY, float &fAngleZ);
	int32_t GetOmega(float &fOmegaX, float &fOmegaY, float &fOmegaZ);

	int32_t ZeroAdjust();

private:
	int32_t ReleaseAll();
	int32_t ReadData();

private:
	uint8_t* m_pRxBuff;
	uint8_t  m_nDataCount;
	uint8_t  m_pDataBuff[11];
	bool m_bStopReadGyro;
	std::thread* m_pReadGyroThread;

	std::mutex m_AccelerationLock;
	float m_Acceleration[3];
	std::mutex m_AngleLock;
	float m_Angle[3];
	float m_AngleOffeset[3];
	std::mutex m_OmegaLock;
	float m_Angle_Prev[3];
	float m_Omega[3];
};


#endif /* MAIN_GYRO_GYRO_H_ */
