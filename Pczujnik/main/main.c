
#include "esp_log.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


#include "include/gaz.h"
#include "include/read_sensor.h"
#include "include/ble.h"
#include "include/mqtt-pub.h"


void task_connectBluetooth(void);
void task_MQTTPub();

void task_updateDataGaz(void *param)
{
	while(1)
	{
		task_Gaz();
	}
	vTaskDelete(NULL);
}

void task_updateDataI2C(void *param)
{
	while(1)
	{
		task_readI2CData();
	}
	vTaskDelete(NULL);
}

void app_main(void)
{
	task_connectBluetooth();
	task_MQTTPub();
//	vTaskDelay(10000 / portTICK_PERIOD_MS);
	xTaskCreate(task_updateDataGaz, "taskGaz", 1024*2, NULL, 3, NULL);
	xTaskCreate(task_updateDataI2C, "taskI2C", 1024*2, NULL, 4, NULL);
//	task_MQTTPub();
}

