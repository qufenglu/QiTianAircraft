/*
 * QuadDriver.cpp
 *
 *  Created on: 2021Äê12ÔÂ19ÈÕ
 *      Author: Qufenglu
 */

#include <math.h>
#include "esp_timer.h"
#include "QuadDriver.h"
#include "Log.h"

QuadDriver::QuadDriver()
{
	m_EscArray[0] = nullptr;
	m_EscArray[1] = nullptr;
	m_EscArray[2] = nullptr;
	m_EscArray[3] = nullptr;

	m_pMPU6050 = nullptr;
	m_bLocked = true;
	m_nIdleSpeed = 1000;
	m_nPrevAngleCtrTime = 0;
	m_nPrevOmageCtrTime = 0;

	PostionPIDCtr::PID pid;
	//pid ={ 0.0f, .0f, .0f };
	pid ={ 4.0f, 0.0f, 0.00000};
	m_AnglePitchPIDCtr.Init(pid, -125.0f, 125.0f);
	pid ={ 3.5f, 0.0f, 0.00000};
	//pid ={ 0.0f,.0f, .0f };
	m_AngleRollPIDCtr.Init(pid, -125.0f, 125.0f);

	pid = { 0.0f, 0.0f, 0.0f };
	m_OmagePitchPIDCtr.Init(pid, -125.0f, 125.0f);
	pid = { 5.0f, 0.0f, 0.0f };
	m_OmageRollPIDCtr.Init(pid, -125.0f, 125.0f);
	pid = { 0.0f, .0f, .0f };
	m_OmageYawPIDCtr.Init(pid, -50.0f, 50.0f);

	m_OmagePitchOut = 0.f;
	m_OmageRollOut = .0f;
	m_PwmPitchOut = 0.f;
	m_PwmRollOut = .0f;
	m_PwmYawOut = .0f;
}

QuadDriver::~QuadDriver()
{
	ReleaseAll();
}

int QuadDriver::Init()
{
	Info("[%p][QuadDriver::Init] Enter Init()\n", this);

	ReleaseAll();

	ledc_timer_config_t ledc_timer =
	{
		.speed_mode = LEDC_HIGH_SPEED_MODE,
		.duty_resolution = LEDC_TIMER_12_BIT,
		.timer_num = LEDC_TIMER_0,
		.freq_hz = 400,
		.clk_cfg = LEDC_AUTO_CLK,
	};

	ledc_channel_config_t ledc_channel =
	{
		.gpio_num = GPIO_NUM_0,
		.speed_mode = LEDC_HIGH_SPEED_MODE,
		.channel = LEDC_CHANNEL_0,
		.timer_sel = LEDC_TIMER_0,
		.duty = 1000 * 4096 / 2500,
		.hpoint = 2000 * 4096 / 2500,
	};

	Esc::InitEscTimer(ledc_timer);

	int table[4][2] =
	{
	{ GPIO_NUM_0, LEDC_CHANNEL_0 },
	{ GPIO_NUM_2, LEDC_CHANNEL_1 },
	{ GPIO_NUM_4, LEDC_CHANNEL_2 },
	{ GPIO_NUM_5, LEDC_CHANNEL_3 } };

	bool bSuccess = true;
	do
	{
		for (int i = 0; i < 4; i++)
		{
			ledc_channel.gpio_num = table[i][0];
			ledc_channel.channel = (ledc_channel_t) table[i][1];

			Esc *esc = new (std::nothrow) Esc(ledc_channel);
			if (esc == nullptr)
			{
				Error("[%p][QuadDriver::Init] New esc fail\n", this);
				bSuccess = false;
				break;
			}

			m_EscArray[i] = esc;

			int32_t ret = esc->Init();
			if (ret != 0)
			{
				Error("[%p][QuadDriver::Init] Init esc fail,return:%d\n", this, ret);
				bSuccess = false;
				break;
			}
		}
	} while (0);

	if (!bSuccess)
	{
		ReleaseAll();
		return -1;
	}

	return 0;
}

int32_t QuadDriver::ReleaseAll()
{
	Info("[%p][QuadDriver::ReleaseAll] Enter ReleaseAll()\n", this);

	m_pMPU6050 = nullptr;
	for (Esc *esc : m_EscArray)
	{
		if (esc != nullptr)
		{
			delete esc;
		}
	}

	m_EscArray[0] = nullptr;
	m_EscArray[1] = nullptr;
	m_EscArray[2] = nullptr;
	m_EscArray[3] = nullptr;

	m_bLocked = true;
	m_nIdleSpeed = 1000;
	m_nPrevAngleCtrTime = 0;
	m_nPrevOmageCtrTime = 0;

	m_AnglePitchPIDCtr.Reset();
	m_AngleRollPIDCtr.Reset();
	m_OmagePitchPIDCtr.Reset();
	m_OmageRollPIDCtr.Reset();
	m_OmageYawPIDCtr.Reset();

	m_OmagePitchOut = 0.f;
	m_OmageRollOut = .0f;
	m_PwmPitchOut = 0.f;
	m_PwmRollOut = .0f;
	m_PwmYawOut = .0f;

	return 0;
}

int32_t QuadDriver::SetDutys(
	const uint16_t &duty1,
	const uint16_t &duty2,
	const uint16_t &duty3,
	const uint16_t &duty4)
{
	if(m_bLocked)
	{
		m_EscArray[0]->SetDuty(1000 * 4096 / 2500);
		m_EscArray[1]->SetDuty(1000 * 4096 / 2500);
		m_EscArray[2]->SetDuty(1000 * 4096 / 2500);
		m_EscArray[3]->SetDuty(1000 * 4096 / 2500);

		Info("[%p][QuadDriver::SetDutys] Motor has been locked\n", this);
	}

	if (m_EscArray[0] != nullptr)
	{
		m_EscArray[0]->SetDuty(duty1 * 4096 / 2500);
	}

	if (m_EscArray[1] != nullptr)
	{
		m_EscArray[1]->SetDuty(duty2 * 4096 / 2500);
	}

	if (m_EscArray[2] != nullptr)
	{
		m_EscArray[2]->SetDuty(duty3 * 4096 / 2500);
	}

	if (m_EscArray[3] != nullptr)
	{
		m_EscArray[3]->SetDuty(duty4 * 4096 / 2500);
	}

	return 0;
}

void QuadDriver::SteGyro(std::shared_ptr<MPU6050> &pMPU6050)
{
	m_pMPU6050 = pMPU6050;
}

uint8_t count = 0;
int QuadDriver::Move2Posture(
	const uint16_t &throttle,
	const uint16_t &yaw,
	const float &pitch,
	const float &roll)
{
	if(m_pMPU6050 == nullptr)
	{
		Error("[%p][QuadDriver::Move2Posture] Gyro is null\n", this);
		return -1;
	}

	int64_t time = esp_timer_get_time();
	{
		//count++;
		//if(count % 2 == 0)
		{
			//count = 0;
			float fAngleX, fAngleY, fAngleZ;
			m_pMPU6050->GetAngle(fAngleX, fAngleY, fAngleZ);
			fAngleX = fabs(fAngleX) < 0.001 ? 0 : fAngleX;
			fAngleY = fabs(fAngleY) < 0.001 ? 0 : fAngleY;
			fAngleZ = fabs(fAngleZ) < 0.001 ? 0 : fAngleZ;

			int32_t dt = (m_nPrevAngleCtrTime > 0) ? (time - m_nPrevAngleCtrTime) : 2500;
			m_nPrevAngleCtrTime = time;

			m_OmagePitchOut = m_AnglePitchPIDCtr.CalPIDOut(pitch, fAngleY, dt, 300.0f);
			m_OmageRollOut = m_AngleRollPIDCtr.CalPIDOut(roll, fAngleX, dt, 300.0f);

			m_PwmPitchOut = m_OmagePitchOut;
			m_PwmRollOut = m_OmageRollOut;
			m_PwmYawOut = .0f;
			//
			printf("1: X:%.4f Y:%.4f Z:%.4f P:%.4f R:%.4f OP:%.4f OR:%.4f\n",
			fAngleX, fAngleY, fAngleZ, pitch, roll, m_OmagePitchOut, m_OmageRollOut);
		}
	}

	/*{
		float fOmageX, fOmageY, fOmageZ;
		m_pMPU6050->GetOmega(fOmageX, fOmageY, fOmageZ);
		fOmageX = fabs(fOmageX) < 0.001 ? 0 : fOmageX;
		fOmageY = fabs(fOmageY) < 0.001 ? 0 : fOmageY;
		fOmageZ = fabs(fOmageZ) < 0.001 ? 0 : fOmageZ;

		int32_t dt = (m_nPrevOmageCtrTime > 0) ? (time - m_nPrevOmageCtrTime) : 2500;
		m_nPrevOmageCtrTime = time;

		m_PwmPitchOut = m_OmagePitchPIDCtr.CalPIDOut(m_OmagePitchOut, fOmageY, dt, 300);
		m_PwmRollOut = m_OmageRollPIDCtr.CalPIDOut(m_OmageRollOut, fOmageX, dt, 300);
		m_PwmYawOut = m_OmageYawPIDCtr.CalPIDOut(yaw, fOmageZ, dt, 100);

		{
			//printf("2: X:%.4f Y:%.4f Z:%.4f OP:%.4f OR:%.4f P:%.4f R:%.4f Y:%.4f\n",
			//fOmageX,fOmageY,fOmageZ, m_OmagePitchOut, m_OmageRollOut,m_PwmPitchOut,m_PwmRollOut,m_PwmYawOut);
		}
	}*/

	int16_t pwm1 = m_PwmYawOut + m_PwmPitchOut + m_PwmRollOut + throttle;
	int16_t pwm2 = -m_PwmYawOut + m_PwmPitchOut - m_PwmRollOut + throttle;
	int16_t pwm3 = m_PwmYawOut - m_PwmPitchOut - m_PwmRollOut + throttle;
	int16_t pwm4 = -m_PwmYawOut - m_PwmPitchOut + m_PwmRollOut + throttle;

	pwm1 = pwm1 < 1100 ? 1100 : pwm1;
	pwm2 = pwm2 < 1100 ? 1100 : pwm2;
	pwm3 = pwm3 < 1100 ? 1100 : pwm3;

	pwm4 = pwm4 < 1100 ? 1100 : pwm4;

	pwm1 = pwm1 > 2000 ? 2000 : pwm1;
	pwm2 = pwm2 > 2000 ? 2000 : pwm2;
	pwm3 = pwm3 > 2000 ? 2000 : pwm3;
	pwm4 = pwm4 > 2000 ? 2000 : pwm4;

	//printf("4:%d %d %d %d\n", pwm1, pwm2, pwm3, pwm4);
	SetDutys(pwm1,pwm2,pwm3,pwm4);

	return 0;
}


int QuadDriver::LockMotor()
{
	Info("[%p][QuadDriver::LockMotor] Enter LockMotor()\n", this);
	m_bLocked = true;

	m_AnglePitchPIDCtr.Reset();
	m_AngleRollPIDCtr.Reset();
	m_OmagePitchPIDCtr.Reset();
	m_OmageRollPIDCtr.Reset();
	m_OmageYawPIDCtr.Reset();

	m_OmagePitchOut = 0.f;
	m_OmageRollOut = .0f;
	m_PwmPitchOut = 0.f;
	m_PwmRollOut = .0f;
	m_PwmYawOut = .0f;

	m_nPrevAngleCtrTime = 0;
	m_nPrevOmageCtrTime = 0;

	return SetDutys(1000, 1000, 1000, 1000);
}

int QuadDriver::UnlockMotor()
{
	Info("[%p][QuadDriver::UnlockMotor] Enter UnlockMotor()\n", this);
	m_bLocked = false;

	m_AnglePitchPIDCtr.Reset();
	m_AngleRollPIDCtr.Reset();
	m_OmagePitchPIDCtr.Reset();
	m_OmageRollPIDCtr.Reset();
	m_OmageYawPIDCtr.Reset();

	m_OmagePitchOut = 0.f;
	m_OmageRollOut = .0f;
	m_PwmPitchOut = 0.f;
	m_PwmRollOut = .0f;
	m_PwmYawOut = .0f;

	m_nPrevAngleCtrTime = 0;
	m_nPrevOmageCtrTime = 0;

	uint16_t speed = m_nIdleSpeed > 1000 ? m_nIdleSpeed : 1000;
	return SetDutys(speed, speed, speed, speed);
}

int QuadDriver::SetMotorIdleSpeed(const uint16_t &speed)
{
	Info("[%p][QuadDriver::SetMotorIdleSpeed] Enter SetMotorIdleSpeed(speed:%d)\n", this, speed);

	if (speed < 1000 || speed > 1200)
	{
		Error("[%p][QuadDriver::SetMotorIdleSpeed] Input speed:%d err\n", this, speed);
		return -1;
	}

	m_nIdleSpeed = speed;

	if (!m_bLocked)
	{
		//SetDutys(m_nEsc1Pwm,m_nEsc2Pwm,m_nEsc3Pwm,m_nEsc4Pwm);
	}

	return 0;
}

void QuadDriver::Adjust()
{
	m_bLocked = false;
	SetDutys(2000, 2000, 2000, 2000);
	vTaskDelay(10000 / portTICK_PERIOD_MS);
	SetDutys(1000, 1000, 1000, 1000);
	vTaskDelay(10000 / portTICK_PERIOD_MS);
	for (int i = 1000; i < 1050; i += 1)
	{
		vTaskDelay(20 / portTICK_PERIOD_MS);
		SetDutys(i, i, i, i);
	}
	for (int i = 1050; i > 1050; i -= 1)
	{
		vTaskDelay(20 / portTICK_PERIOD_MS);
		SetDutys(i, i, i, i);
	}
}
