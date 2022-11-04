#include <cmath>
#include "mpu6050.hpp"

#define ORIGINAL_OUTPUT			 (0)
#define ACC_FULLSCALE        	 (4)
#define GYRO_FULLSCALE			 (2000)
#define READ_TICK				 (2000)

#if ORIGINAL_OUTPUT == 0
	#if  ACC_FULLSCALE  == 2
		#define AccAxis_Sensitive (float)(16384.0)
	#elif ACC_FULLSCALE == 4
		#define AccAxis_Sensitive (float)(8192.0)
	#elif ACC_FULLSCALE == 8
		#define AccAxis_Sensitive (float)(4096.0)
	#elif ACC_FULLSCALE == 16
		#define AccAxis_Sensitive (float)(2048.0)
	#endif 
		
	#if   GYRO_FULLSCALE == 250
		#define GyroAxis_Sensitive (float)(131.0)
	#elif GYRO_FULLSCALE == 500
		#define GyroAxis_Sensitive (float)(65.5)
	#elif GYRO_FULLSCALE == 1000
		#define GyroAxis_Sensitive (float)(32.8)
	#elif GYRO_FULLSCALE == 2000
		#define GyroAxis_Sensitive (float)(16.4)
	#endif
		
#else
	#define AccAxis_Sensitive  (1)
	#define GyroAxis_Sensitive (1)
#endif

MPU6050::MPU6050(gpio_num_t scl, gpio_num_t sda, i2c_port_t port) :
m_Pitchfilter(0.001f * 0.001f * READ_TICK),
m_Rollfilter(0.001f * 0.001f * READ_TICK)
{
    i2c = new I2C(scl, sda, port);
    m_fAccX = .0f;
    m_fAccY = .0f;
    m_fAccZ = .0f;
    m_fOmeagX = .0f;
    m_fOmeagY = .0f;
    m_fOmeagZ = .0f;
    m_fAngleX = .0f;
    m_fAngleY = .0f;
    m_fAngleZ = .0f;

    m_pReadCycleTimer = nullptr;
}

MPU6050::~MPU6050()
{
	StopReadData();
    delete(i2c);
}

bool MPU6050::Init()
{
	if (!i2c->slave_write(MPU6050_ADDR, PWR_MGMT_1, 0x80))
		return false;
	vTaskDelay(50 / portTICK_PERIOD_MS);
	if (!i2c->slave_write(MPU6050_ADDR, SMPLRT_DIV, 0x00))
		return false;
	if (!i2c->slave_write(MPU6050_ADDR, PWR_MGMT_1, 0x03))
		return false;
	if (!i2c->slave_write(MPU6050_ADDR, CONFIG, 0x03))
		return false;
	if (!i2c->slave_write(MPU6050_ADDR, GYRO_CONFIG, 0x18))
		return false;
	if (!i2c->slave_write(MPU6050_ADDR, ACCEL_CONFIG, 0x08))
		return false;

	return true;
}

float MPU6050::getAccX()
{
    uint8_t r[2];
    i2c -> slave_read(MPU6050_ADDR, ACCEL_XOUT_H, r, 2);
    short accx = r[0] << 8 | r[1];
    accx += m_fAccelOffsetX;
    return (float)accx / AccAxis_Sensitive;
}

float MPU6050::getAccY()
{
    uint8_t r[2];
    i2c -> slave_read(MPU6050_ADDR, ACCEL_YOUT_H, r, 2);
    short accy = r[0] << 8 | r[1];
    accy += m_fAccelOffsetY;
    return (float)accy / AccAxis_Sensitive;
}

float MPU6050::getAccZ()
{
    uint8_t r[2];
    i2c -> slave_read(MPU6050_ADDR, ACCEL_ZOUT_H, r, 2);
    short accz = r[0] << 8 | r[1];
    accz += m_fAccelOffsetZ;
    return (float)accz / AccAxis_Sensitive;
}

float MPU6050::getGyroX()
{
    uint8_t r[2];
    i2c -> slave_read(MPU6050_ADDR, GYRO_XOUT_H, r, 2);
    short gyrox = r[0] << 8 | r[1];
    gyrox += m_fGyroOffsetX;
    return (float)gyrox / GyroAxis_Sensitive;
}

float MPU6050::getGyroY()
{
    uint8_t r[2];
    i2c -> slave_read(MPU6050_ADDR, GYRO_YOUT_H, r, 2);
    short gyroy = r[0] << 8 | r[1];
    gyroy += m_fGyroOffsetY;
    return (float)gyroy / GyroAxis_Sensitive;
}

float MPU6050::getGyroZ()
{
    uint8_t r[2];
    i2c -> slave_read(MPU6050_ADDR, GYRO_ZOUT_H, r, 2);
    short gyroz = r[0] << 8 | r[1];
    gyroz += m_fGyroOffsetZ;
    return (float)gyroz / GyroAxis_Sensitive;
}

short MPU6050::getTemp()
{
    uint8_t r[2];
    i2c -> slave_read(MPU6050_ADDR, TEMP_OUT_H, r, 2);
    return r[0] << 8 | r[1];
}

int32_t MPU6050::StartReadData()
{
	if(m_pReadCycleTimer != nullptr)
	{
		//ERROR(" MPU6050::StartReadData MPU6050 already started\n");
		return -1;
	}

	esp_timer_create_args_t timerArgs;
	timerArgs.callback = &MPU6050::ReadCycleCallback;
	timerArgs.name = "MPU6050Timer";
	timerArgs.arg = this;
	esp_timer_create(&timerArgs, &m_pReadCycleTimer);
	esp_timer_start_periodic(m_pReadCycleTimer, READ_TICK);

	return 0;
}

int32_t MPU6050::StopReadData()
{
	if (m_pReadCycleTimer != nullptr)
	{
		esp_timer_stop(m_pReadCycleTimer);
		esp_timer_delete(m_pReadCycleTimer);
		m_pReadCycleTimer = nullptr;
	}
	return 0;
}

int32_t MPU6050::GetAcceleration(float &fAccX, float &fAccY, float &fAccZ)
{
	std::lock_guard<std::mutex> lock(m_AccLock);
	fAccX = m_fAccX;
	fAccY = m_fAccY;
	fAccZ = m_fAccZ;
	return 0;
}

int32_t MPU6050::GetAngle(float &fAngleX, float &fAngleY, float &fAngleZ)
{
	std::lock_guard<std::mutex> lock(m_AngleLock);
	fAngleX = m_fAngleX;
	fAngleY = m_fAngleY;
	fAngleZ = m_fAngleZ;
	return 0;
}

int32_t MPU6050::GetOmega(float &fOmegaX, float &fOmegaY, float &fOmegaZ)
{
	std::lock_guard<std::mutex> lock(m_OmeagLock);
	fOmegaX = m_fOmeagX;
	fOmegaY = m_fOmeagY;
	fOmegaZ = m_fOmeagZ;
	return 0;
}

void MPU6050::ReadCycleCallback(void *arg)
{
	MPU6050 *pMPU6050 = (MPU6050*) arg;
	float ax, ay, az, gx, gy, gz;
	float pitch, roll;
	float fpitch, froll;

	ax = -pMPU6050->getAccX();
	ay = -pMPU6050->getAccY();
	az = -pMPU6050->getAccZ();
	gx = pMPU6050->getGyroX();
	gy = -pMPU6050->getGyroY();
	gz = -pMPU6050->getGyroZ();

	pitch = atan(ax / az) * 57.2958f;
	roll = atan(ay / az) * 57.2958f;
	fpitch = pMPU6050->m_Pitchfilter.filter(pitch, gy);
	froll = pMPU6050->m_Rollfilter.filter(roll, gx);

	{
		std::lock_guard<std::mutex> lock(pMPU6050->m_AccLock);
		pMPU6050->m_fAccX = ax;
		pMPU6050->m_fAccY = ay;
		pMPU6050->m_fAccZ = az;

		//printf("AX:%f AY:%f AZ:%f\n",pMPU6050->m_fAccX,pMPU6050->m_fAccY,pMPU6050->m_fAccZ);
	}

	{
		std::lock_guard<std::mutex> lock(pMPU6050->m_AngleLock);
		pMPU6050->m_fAngleX = froll;
		pMPU6050->m_fAngleY = fpitch;
		//printf("GX:%f GY:%f GZ:%f\n",pMPU6050->m_fAngleX,pMPU6050->m_fAngleY,pMPU6050->m_fAngleZ);
	}

	{
		std::lock_guard<std::mutex> lock(pMPU6050->m_OmeagLock);
		pMPU6050->m_fOmeagX = gx;
		pMPU6050->m_fOmeagY = gy;
		pMPU6050->m_fOmeagZ = gz;
		//printf("OX:%f OY:%f OZ:%f\n",pMPU6050->m_fOmeagX,pMPU6050->m_fOmeagY,pMPU6050->m_fOmeagZ);
	}
}
