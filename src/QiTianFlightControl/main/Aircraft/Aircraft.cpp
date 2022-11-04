/*
 * Aircraft.cpp
 *
 *  Created on: 2021年12月19日
 *      Author: Qufenglu
 */

#include <thread>
#include "esp_timer.h"
#include "Aircraft.h"
#include "Log.h"

Aircraft::Aircraft()
{
	m_pReceiver = nullptr;
	m_pMPU6050 = nullptr;
	m_bLocked = true;
	m_bStopFlight = true;
	m_pControlThread = nullptr;
	m_bFirstUnlock = true;
	m_pControlCycleTimer = nullptr;
	m_nLockTime = 0;
	m_nUnlockTime = 0;
}

Aircraft::~Aircraft()
{
	ReleaseAll();
}

int32_t Aircraft::ReleaseAll()
{
	Info("[%p][Aircraft::ReleaseAll] Enter ReleaseAll()\n", this);

	StopFlight();

	m_pReceiver = nullptr;
	m_pMPU6050 = nullptr;
	m_bLocked = true;

	return 0;
}

int32_t Aircraft::Init()
{
	Info("[%p][Aircraft::Init] Enter Init()\n", this);

	bool bSuccess = true;
	int32_t ret = 0;

	do
	{
		ret = QuadDriver::Init();
		QuadDriver::Adjust();
		if (ret < 0)
		{
			Error("[%p][Aircraft::Init] Init QuadDriver fail,return:%d\n", this, ret);
			bSuccess = false;
			break;
		}
		QuadDriver::LockMotor();
		QuadDriver::SetMotorIdleSpeed(1000);
		std::this_thread::sleep_for(std::chrono::milliseconds(200));

		if (m_pReceiver == nullptr)
		{
			MsgTransmitter::InitWifiAsSoftAP("QFL123", "421930188");
			while (!MsgTransmitter::IsGotRemoteIp())
			{
				vTaskDelay(50 / portTICK_PERIOD_MS);
			}

			m_pReceiver = std::make_shared<Receiver>();
			if (m_pReceiver == nullptr)
			{
				Error("[%p][Aircraft::Init] New Receiver fail\n", this);
				bSuccess = false;
				break;
			}
			ret = m_pReceiver->StartTransmitMsg(1, 7777);
			if (ret < 0)
			{
				Error("[%p][Aircraft::Init] StartTransmitMsg fail,return:%d\n", this, ret);
				bSuccess = false;
				break;
			}
		}

		if (m_pMPU6050 == nullptr)
		{
			m_pMPU6050 = std::make_shared<MPU6050>(GPIO_NUM_18, GPIO_NUM_17, I2C_NUM_0);
			if (m_pMPU6050 == nullptr)
			{
				Error("[%p][Aircraft::Init] New gyro fail\n", this);
				bSuccess = false;
				break;
			}

			ret = m_pMPU6050->Init();
			if (ret <= 0)
			{
				Error("[%p][Aircraft::Init] Init gyro fail,return:%d\n", this, ret);
				bSuccess = false;
				break;
			}

			ret = m_pMPU6050->StartReadData();
			if (ret < 0)
			{
				Error("[%p][Aircraft::Init] Gyro StartReadData fail,return:%d\n", this, ret);
				bSuccess = false;
				break;
			}
		}

		QuadDriver::SteGyro(m_pMPU6050);
	} while (0);

	if (!bSuccess)
	{
		Error("[%p][Aircraft::Init] init fail\n", this);
		return -1;
	}

	return 0;
}

int32_t Aircraft::StartFlight()
{
	Info("[%p][Aircraft::StartFlight] Enter StartFlight()\n", this);

	StopFlight();

	/*m_pControlThread = new (std::nothrow) std::thread(&Aircraft::FlightControlThread, this);
	 if (m_pControlThread == nullptr)
	 {
	 Error("[%p][Aircraft::StartFlight] New FlightControlThread fail\n", this);
	 return -1;
	 }*/
	m_nLockTime = 0;
	m_nUnlockTime = 0;
	esp_timer_create_args_t timerArgs;
	timerArgs.callback = &Aircraft::ControlCycleCallback;
	timerArgs.name = "FlightControlTimer";
	timerArgs.arg = this;
	esp_timer_create(&timerArgs, &m_pControlCycleTimer);
	esp_timer_start_periodic(m_pControlCycleTimer, CONTROL_TICK);

	return 0;
}

int32_t Aircraft::StopFlight()
{
	Info("[%p][Aircraft::StopFlight] Enter StopFlight()\n", this);

	m_bStopFlight = true;
	if (m_pControlThread != nullptr)
	{
		if(m_pControlThread->joinable())
		{
			m_pControlThread->join();
		}
		delete m_pControlThread;
		m_pControlThread = nullptr;
	}

	if(m_pControlCycleTimer != nullptr)
	{
		esp_timer_stop(m_pControlCycleTimer);
		esp_timer_delete(m_pControlCycleTimer);
		m_pControlCycleTimer = nullptr;
	}

	m_nLockTime = 0;
	m_nUnlockTime = 0;

	return 0;
}

int32_t Aircraft::FlightControlThread()
{
	Info("[%p][Aircraft::FlightControlThread] Enter FlightControlThread()\n", this);

	m_bStopFlight = false;
	if (m_pReceiver == nullptr || m_pMPU6050 == nullptr)
	{
		Error("[%p][Aircraft::FlightControlThread] QuadDriver or Receiver or Gyro is null\n", this);
		return -1;
	}

	ControlMsg msg;
	float pitch = 0;
	float roll = 0;
	uint16_t count = 0;

	while (!m_bStopFlight)
	{
		{
			while (m_bLocked)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(100));

				m_pReceiver->GetControlMsg(msg);
				if (msg.chValues[0] < 1050 && msg.chValues[1] > 1950 &&
					msg.chValues[2] < 1200 && msg.chValues[3] < 1050)
				{
					count++;
				}
				else
				{
					count = 0;
				}

				if (count > 30)
				{
					m_bLocked = false;
					/***********************
					 * 第一次解锁时将陀螺仪调零,
					 * 解锁起飞前需要将无人机放平
					 * ********************/
					if(m_bFirstUnlock)
					{
						//m_pGyro->ZeroAdjust();
						m_bFirstUnlock = false;
					}
					QuadDriver::UnlockMotor();
				}
			}
		}

		count = 0;
		{
			while(!m_bLocked)
			{
				m_pReceiver->GetControlMsg(msg);
				if (msg.chValues[0] < 1050)
				{
					count++;
				}
				else
				{
					count = 0;
				}
				if (count > 1000)
				{
					m_bLocked = true;
					QuadDriver::LockMotor();
					continue;
				}


				pitch = (2.0 * MAX_PITCH_ANGLE / 1000) * (1500 - (int32_t) msg.chValues[2]);
				roll = (2.0 * MAX_ROLL_ANGLE / 1000) * ((int32_t) msg.chValues[3] - 1500);

				//QuadDriver::Move2Posture(msg.chValues[0], 1500, 0, 0);
				QuadDriver::Move2Posture(msg.chValues[0], msg.chValues[1], pitch, roll);

				std::this_thread::sleep_for(std::chrono::milliseconds(4));
			}
		}
	}

	return 0;
}

int64_t time_since_boot_old = 0;
void Aircraft::ControlCycleCallback(void *arg)
{
	Aircraft *pAircraft = (Aircraft*) arg;
	pAircraft->m_pReceiver->GetControlMsg(pAircraft->m_ControlMsg);

	/*{
		float fAngleX, fAngleY, fAngleZ;
		pAircraft->m_pGyro->GetAngle(fAngleX, fAngleY, fAngleZ);
		int64_t time_since_boot = esp_timer_get_time();
		printf("%lld %f %f %f\n", time_since_boot - time_since_boot_old, fAngleX, fAngleY, fAngleZ);
		time_since_boot_old = time_since_boot;
	}*/
	if (pAircraft->m_bLocked)
	{
		if (pAircraft->m_ControlMsg.chValues[0] < 1050 && pAircraft->m_ControlMsg.chValues[1] > 1950 && pAircraft->m_ControlMsg.chValues[2] < 1200
				&& pAircraft->m_ControlMsg.chValues[3] < 1050)
		{
			pAircraft->m_nUnlockTime += CONTROL_TICK;
		}
		else
		{
			pAircraft->m_nUnlockTime = 0;
		}

		if (pAircraft->m_nUnlockTime >= UNLOCK_TIME * 1000)
		{
			pAircraft->m_bLocked = false;
			/***********************
			 * 第一次解锁时将陀螺仪调零,
			 * 解锁起飞前需要将无人机放平
			 * ********************/
			if (pAircraft->m_bFirstUnlock)
			{
				//pAircraft->m_pGyro->ZeroAdjust();
				pAircraft->m_bFirstUnlock = false;
			}
			pAircraft->UnlockMotor();

			pAircraft->m_nLockTime = 0;
			pAircraft->m_nUnlockTime = 0;
		}
	}
	else
	{
		if (pAircraft->m_ControlMsg.chValues[0] < 1050)
		{
			pAircraft->m_nLockTime += CONTROL_TICK;
		}
		else
		{
			pAircraft->m_nLockTime = 0;
		}

		if (pAircraft->m_nLockTime > LOCK_TIME * 1000)
		{
			pAircraft->m_bLocked = true;
			pAircraft->LockMotor();
			pAircraft->m_nLockTime = 0;
			pAircraft->m_nUnlockTime = 0;
			return;
		}

		float yaw = (2.0 * MAX_YAW_OMAGE / 1000) * ((int32_t) pAircraft->m_ControlMsg.chValues[1] - 1500);
		float pitch = (2.0 * MAX_PITCH_ANGLE / 1000) * (1500 - (int32_t) pAircraft->m_ControlMsg.chValues[2]);
		float roll = (2.0 * MAX_ROLL_ANGLE / 1000) * ((int32_t) pAircraft->m_ControlMsg.chValues[3] - 1500);
		float throttle = pAircraft->m_ControlMsg.chValues[0];
		yaw = (yaw > -1.0f && yaw < 1.0f) ? .0f : yaw;
		pitch = (pitch > -1.0f && pitch < 1.0f) ? .0f : pitch;
		roll = (roll > -1.0f && roll < 1.0f) ? .0f : roll;
		if(throttle < 1050)
		{
			yaw = .0f;
			pitch = .0f;
			roll = .0f;
		}

		yaw = .0f;
		pitch = 3.0f;
		roll = .0f;

		pAircraft->Move2Posture(throttle, yaw, pitch, roll);
	}
}
