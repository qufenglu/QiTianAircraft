/*
 * Gyro.cpp
 *
 *  Created on: 2021Äê12ÔÂ19ÈÕ
 *      Author: Qufenglu
 */

#include "Gyro.h"
#include "driver/uart.h"
#include "freertos/task.h"
#include "Log.h"
#include "esp_log.h"

#define GYRO_TXD  (GPIO_NUM_26)
#define GYRO_RXD  (GPIO_NUM_27)

#define BUF_SIZE (129)
#define RX_BUFF_SIZE (32)
#define UARTNUM (UART_NUM_1)

Gyro::Gyro()
{
	m_pRxBuff = nullptr;
	m_nDataCount = 0;
	m_bStopReadGyro = true;
	m_pReadGyroThread = nullptr;
	for (int i = 0; i < 3; i++)
	{
		m_Acceleration[i] = .0f;
		m_Angle[i] = .0f;
		m_AngleOffeset[i] = .0f;
		m_Angle_Prev[i] = .0f;
		m_Omega[i] = .0f;
	}
}

Gyro::~Gyro()
{
	ReleaseAll();
}

int32_t Gyro::ReleaseAll()
{
	Info("[%p][Gyro::ReleaseAll] Enter ReleaseAll()\n", this);

	StopReadData();

	if (m_pRxBuff != nullptr)
	{
		free (m_pRxBuff);
		m_pRxBuff = nullptr;
	}

	uart_driver_delete(UARTNUM);

	return 0;
}

int32_t Gyro::Init()
{
	Info("[%p][Gyro::Init] Enter Init()\n", this);

	ReleaseAll();

	uart_config_t uart_config =
	{
		.baud_rate = 921600,
		.data_bits = UART_DATA_8_BITS,
		.parity = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_1,
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE
	};

	m_pRxBuff = (uint8_t*) malloc(RX_BUFF_SIZE);
	if (m_pRxBuff == nullptr)
	{
		Error("[%p][Gyro::Init] Malloc rxbuff fail\n", this);
		return -1;
	}

	esp_err_t err = uart_param_config(UARTNUM, &uart_config);
	if (err != ESP_OK)
	{
		Error("[%p][Gyro::Init] Set uart param fail,return:%d\n", this, err);
		ReleaseAll();
		return -2;
	}
	err = uart_set_pin(UARTNUM, GYRO_TXD, GYRO_RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
	if (err != ESP_OK)
	{
		Error("[%p][Gyro::Init] Set uart pin fail,return:%d\n", this, err);
		ReleaseAll();
		return -3;
	}
	err = uart_driver_install(UARTNUM, BUF_SIZE, 0, 0, NULL, 0);
	if (err != ESP_OK)
	{
		Error("[%p][Gyro::Init] Set uart pin fail,return:%d\n", this, err);
		ReleaseAll();
		return -4;
	}
	err = uart_set_rx_timeout(UARTNUM, 2);
	if (err != ESP_OK)
	{
		Error("[%p][Gyro::Init] Set uart timeout fail,return:%d\n", this, err);
		ReleaseAll();
		return -5;
	}

	return 0;
}

int32_t Gyro::ZeroAdjust()
{
	Info("[%p][Gyro::ZeroAdjust] Enter ZeroAdjust()\n", this);

	std::lock_guard<std::mutex> lock1(m_AngleLock);

	m_AngleOffeset[0] = .0f - m_Angle[0];
	m_AngleOffeset[1] = .0f - m_Angle[1];
	m_AngleOffeset[2] = .0f - m_Angle[2];

	return 0;
}

int32_t Gyro::ReadData()
{
	if (m_pRxBuff == nullptr)
	{
		Error("[%p][Gyro::ReadData] RxBuff is null\n", this);
		return -1;
	}

	m_bStopReadGyro = false;

	while (!m_bStopReadGyro)
	{
		int len = uart_read_bytes(UART_NUM_1, m_pRxBuff, RX_BUFF_SIZE, 1);
		if (len < 0)
		{
			Error("[%p][Gyro::ReadData] read uart fail\n", this);
			return -2;
		}

		int read = 0;
		if (m_nDataCount == 0)
		{
			while (read < len && m_pRxBuff[read] != 0x55)
			{
				read++;
			}
		}

		while (read < len)
		{
			m_pDataBuff[m_nDataCount] = m_pRxBuff[read];
			m_nDataCount++;
			if (m_nDataCount == 11)
			{
				switch (m_pDataBuff[1])
				{
				case 0x51:
				{
					std::lock_guard < std::mutex > lock(m_AccelerationLock);
					m_Acceleration[0] = (short(m_pDataBuff[3] << 8 | m_pDataBuff[2])) / 32768.0f * 16.0f;
					m_Acceleration[1] = (short(m_pDataBuff[5] << 8 | m_pDataBuff[4])) / 32768.0f * 16.0f;
					m_Acceleration[2] = (short(m_pDataBuff[7] << 8 | m_pDataBuff[6])) / 32768.0f * 16.0f;
					break;
				}
					/*case 0x52:
					 {
					 std::lock_guard<std::mutex> lock(m_OmegaLock);
					 m_Omega[0] = (short(m_pDataBuff[3] << 8 | m_pDataBuff[2])) * 2000.0f / 32768.0f;
					 m_Omega[1] = (short(m_pDataBuff[5] << 8 | m_pDataBuff[4])) * 2000.0f / 32768.0f;
					 m_Omega[2] = (short(m_pDataBuff[7] << 8 | m_pDataBuff[6])) * 2000.0f / 32768.0f;
					 break;
					 }*/
				case 0x53:
				{
					std::lock_guard < std::mutex > lock(m_AngleLock);
					m_Angle[0] = (short(m_pDataBuff[3] << 8 | m_pDataBuff[2])) / 32768.0f * 180.0f;
					m_Angle[0] += m_AngleOffeset[0];
					m_Angle[1] = (short(m_pDataBuff[5] << 8 | m_pDataBuff[4])) / 32768.0f * 180.0f;
					m_Angle[1] += m_AngleOffeset[1];
					m_Angle[2] = (short(m_pDataBuff[7] << 8 | m_pDataBuff[6])) / 32768.0f * 180.0f;
					m_Angle[2] += m_AngleOffeset[2];

					std::lock_guard < std::mutex > lock1(m_OmegaLock);
					m_Omega[0] = (m_Angle[0] - m_Angle_Prev[0]) * 200.0f;
					m_Omega[1] = (m_Angle[1] - m_Angle_Prev[1]) * 200.0f;
					m_Omega[2] = (m_Angle[2] - m_Angle_Prev[2]) * 200.0f;
					m_Angle_Prev[0] = m_Angle[0];
					m_Angle_Prev[1] = m_Angle[1];
					m_Angle_Prev[2] = m_Angle[2];

					break;
				}
				default:
					break;
				}

				m_nDataCount = 0;
			}
			read++;
		}
	}

	return 0;
}

int32_t Gyro::StartReadData()
{
	Info("[%p][Gyro::StartReadData] Enter StartReadData()\n", this);

	m_pReadGyroThread = new (std::nothrow) std::thread(&Gyro::ReadData,this);
	if(m_pReadGyroThread == nullptr)
	{
		Error("[%p][Gyro::StartReadData] New ReadGyroThread fail\n", this);
		return -1;
	}

	return 0;
}

int32_t Gyro::StopReadData()
{
	Info("[%p][Gyro::StopReadData] Enter StopReadData()\n", this);

	m_bStopReadGyro = true;
	if (m_pReadGyroThread != nullptr)
	{
		if (m_pReadGyroThread->joinable())
		{
			m_pReadGyroThread->join();
		}
		delete m_pReadGyroThread;
		m_pReadGyroThread = nullptr;
	}

	return 0;
}

int32_t Gyro::GetAcceleration(float &fAccX, float &fAccY, float &fAccZ)
{
	{
		std::lock_guard<std::mutex> lock(m_AccelerationLock);
		fAccX = m_Acceleration[0];
		fAccY = m_Acceleration[1];
		fAccZ = m_Acceleration[2];
	}
	return 0;
}

int32_t Gyro::GetAngle(float &fAngleX, float &fAngleY, float &fAngleZ)
{
	{
		std::lock_guard<std::mutex> lock(m_AngleLock);
		fAngleX = m_Angle[0];
		fAngleY = m_Angle[1];
		fAngleZ = m_Angle[2];
	}
	return 0;
}

int32_t Gyro::GetOmega(float &fOmegaX, float &fOmegaY, float &fOmegaZ)
{
	{
		std::lock_guard<std::mutex> lock(m_OmegaLock);
		fOmegaX = m_Omega[0];
		fOmegaY = m_Omega[1];
		fOmegaZ = m_Omega[2];
	}
	return 0;
}
