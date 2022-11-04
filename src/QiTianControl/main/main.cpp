#include <new>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_pm.h"
#include "Log.h"
#include "AnalogChannel.h"
#include "SwitchChannel.h"
#include "MsgTransmitter.h"
#include "QiTianControl.h"

int init()
{
	esp_pm_config_esp32_t config;
	config.max_freq_mhz = 240;
	config.min_freq_mhz = 240;
	config.light_sleep_enable = false;
	esp_err_t ret = esp_pm_configure(&config);
	if (ret != ESP_OK)
	{
		Error("config esp pm fail,return:%d\n", ret);
		return -1;
	}
	return 0;
}

extern "C" void app_main()
{
	if (init() < 0)
	{
		return;
	}
	Info("QiTianControl app start\n");

	/*MsgTransmitter::InitWifiAsSoftAP("QFL123", "421930188");
	while (!MsgTransmitter::IsGotRemoteIp())
	{
		vTaskDelay(50 / portTICK_PERIOD_MS);
	}*/

	MsgTransmitter::InitWifiAsSTA();
	while (!MsgTransmitter::IsGotRemoteIp())
	{
		MsgTransmitter::ConnectToAP("QFL123", "421930188");
		vTaskDelay(3000 / portTICK_PERIOD_MS);
	}

	QiTianControl *control = new QiTianControl();
	control->Init();
	control->StartSample();
	control->StartTransmitMsg(0, 7777);

	while (true)
	{
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}
