/*
 * Aircraft.h
 *
 *  Created on: 2021Äê12ÔÂ19ÈÕ
 *      Author: Qufenglu
 */

#ifndef MAIN_AIRCRAFT_AIRCRAFT_H_
#define MAIN_AIRCRAFT_AIRCRAFT_H_

#include "Receiver.h"
#include "QuadDriver.h"

#define MAX_YAW_ANGULAR_VELOCITY (20.0f)
#define MAX_ROLL_ANGLE (45.0f)
#define MAX_PITCH_ANGLE (45.0f)
#define MAX_YAW_OMAGE (45.0f)
#define CONTROL_TICK (8000)
#define UNLOCK_TIME (4000)
#define LOCK_TIME (2000)

class Aircraft: public QuadDriver
{
public:
	Aircraft();
	virtual ~Aircraft();

	int32_t Init();
	int32_t StartFlight();
	int32_t StopFlight();

private:
	int32_t ReleaseAll();
	int32_t FlightControlThread();
	static void ControlCycleCallback(void *arg);

private:
	std::shared_ptr<Receiver> m_pReceiver;
	std::shared_ptr<MPU6050> m_pMPU6050;

	bool m_bLocked;
	bool m_bStopFlight;
	std::thread *m_pControlThread;
	esp_timer_handle_t m_pControlCycleTimer;
	bool m_bFirstUnlock;

	int32_t m_nLockTime;
	int32_t m_nUnlockTime;
	ControlMsg m_ControlMsg;
};

#endif /* MAIN_AIRCRAFT_AIRCRAFT_H_ */
