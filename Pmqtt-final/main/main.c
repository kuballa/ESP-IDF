#include "esp_log.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "include/ble-read.h"
#include "include/mqtt-pub.h"
#include "include/mqtt-sub.h"

void task_BLERead(void);
void task_MQTTPub();
void task_MQTTSub();

void app_main(void)
{
	task_BLERead();
	task_MQTTPub();
//	task_MQTTSub();
}
