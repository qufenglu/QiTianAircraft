#pragma once

#include <mutex>
#include "i2c.hpp"
#include "kalmanfilter.hpp"

#define	SMPLRT_DIV		0x19	//陀螺仪采样率，典型值：0x07(125Hz)
#define	CONFIG			0x1A	//低通滤波频率，典型值：0x06(5Hz)
#define	GYRO_CONFIG		0x1B	//陀螺仪自检及测量范围，典型值：0x18(不自检，2000deg/s)
#define	ACCEL_CONFIG	0x1C	//加速计自检、测量范围及高通滤波频率，典型值：0x01(不自检，2G，5Hz)
#define	ACCEL_XOUT_H	0x3B
#define	ACCEL_XOUT_L	0x3C
#define	ACCEL_YOUT_H	0x3D
#define	ACCEL_YOUT_L	0x3E
#define	ACCEL_ZOUT_H	0x3F
#define	ACCEL_ZOUT_L	0x40
#define	TEMP_OUT_H		0x41
#define	TEMP_OUT_L		0x42
#define	GYRO_XOUT_H		0x43
#define	GYRO_XOUT_L		0x44
#define	GYRO_YOUT_H		0x45
#define	GYRO_YOUT_L		0x46
#define	GYRO_ZOUT_H		0x47
#define	GYRO_ZOUT_L		0x48
#define	PWR_MGMT_1		0x6B	//电源管理，典型值：0x00(正常启用)
#define	WHO_AM_I		0x75	//IIC地址寄存器(默认数值0x68，只读)
#define MPU6050_ADDR	0x68    //IIC写入时的地址字节数据，+1为读取

class MPU6050 {
private:
    I2C *i2c;
    float m_fAccelOffsetX = -943.0f;
    float m_fAccelOffsetY = 86.0f;
    float m_fAccelOffsetZ = 510.0f;
    float m_fGyroOffsetX = -5.0f;
    float m_fGyroOffsetY = -4.0f;
    float m_fGyroOffsetZ = -33.0f;

    std::mutex m_AccLock;
    float m_fAccX,m_fAccY,m_fAccZ;
    std::mutex m_OmeagLock;
    float m_fOmeagX,m_fOmeagY,m_fOmeagZ;
    std::mutex m_AngleLock;
    float m_fAngleX,m_fAngleY,m_fAngleZ;

    esp_timer_handle_t m_pReadCycleTimer;

    KALMAN m_Pitchfilter;
    KALMAN m_Rollfilter;

private:
	float getAccX();
	float getAccY();
	float getAccZ();

	float getGyroX();
	float getGyroY();
	float getGyroZ();

	short getTemp();

	static void ReadCycleCallback(void *arg);

public:
    MPU6050(gpio_num_t scl, gpio_num_t sda, i2c_port_t port);
    ~MPU6050();
    bool Init();

    int32_t StartReadData();
    int32_t StopReadData();

    int32_t GetAcceleration(float &fAccX, float &fAccY, float &fAccZ);
    int32_t GetAngle(float &fAngleX, float &fAngleY, float &fAngleZ);
    int32_t GetOmega(float &fOmegaX, float &fOmegaY, float &fOmegaZ);
};
