#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_pm.h"
#include "Log.h"
#include "Aircraft.h"

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
	Info("QiTianFlightControl start\n");

	/*QuadDriver* driver = new QuadDriver();
	driver->Init();
	driver->Adjust();*/


	Aircraft *aircraft = new (std::nothrow) Aircraft();
	if (aircraft == nullptr)
	{
		Error("New Aircraft fail\n");
		return;
	}

	int32_t ret = aircraft->Init();
	if (ret < 0)
	{
		Error("Init Aircraft fail\n");
		return;
	}

	aircraft->StartFlight();

	while (true)
	{
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}
