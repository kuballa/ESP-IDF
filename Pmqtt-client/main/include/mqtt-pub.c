#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "rom/ets_sys.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"
#include "ble-read.h"

static const char *TAG = "MQTT_EXAMPLE";

//#define EXAMPLE_ESP_WIFI_SSID "G6_2708"
//#define EXAMPLE_ESP_WIFI_PASS "12345678"
#define EXAMPLE_ESP_WIFI_SSID "NETIASPOT-hhHc9Am-2.4G"
#define EXAMPLE_ESP_WIFI_PASS "meVwzW74vuTYqMTexV"
#define MAX_RETRY 10
static int retry_cnt = 0;

#define MQTT_PUB_TEMP_DHT "mq5/temperature/44"
#define MQTT_PUB_PRE_DHT "mq5/pressure/44"
#define MQTT_PUB_GAS_DHT "mq5/gas/44"
#define MQTT_PUB_ALARM_DUR_DHT "mq5/alarm_duration/44"
#define MQTT_SUB_ALARM "mq5/alarm_level/44"

uint32_t MQTT_CONNECTED = 0;

static void mqtt_app_start(void);

static esp_err_t wifi_event_handler(void *arg, esp_event_base_t event_base,
                                    int32_t event_id, void *event_data)
{
    switch (event_id)
    {
    case WIFI_EVENT_STA_START:
        esp_wifi_connect();
        ESP_LOGI(TAG, "Trying to connect with Wi-Fi\n");
        break;

    case WIFI_EVENT_STA_CONNECTED:
        ESP_LOGI(TAG, "Wi-Fi connected\n");
        break;

    case IP_EVENT_STA_GOT_IP:
        ESP_LOGI(TAG, "got ip: startibg MQTT Client\n");
        mqtt_app_start();
        break;

    case WIFI_EVENT_STA_DISCONNECTED:
        ESP_LOGI(TAG, "disconnected: Retrying Wi-Fi\n");
        if (retry_cnt++ < MAX_RETRY)
        {
            esp_wifi_connect();
        }
        else
            ESP_LOGI(TAG, "Max Retry Failed: Wi-Fi Connection\n");
        break;

    default:
        break;
    }
    return ESP_OK;
}

void wifi_init(void)
{
    esp_event_loop_create_default();
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL);

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    esp_netif_init();
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
    esp_wifi_start();
}

/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    esp_mqtt_event_handle_t event = event_data;

    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        MQTT_CONNECTED = 1;
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        MQTT_CONNECTED = 0;
        break;

    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

esp_mqtt_client_handle_t client = NULL;
static void mqtt_app_start(void)
{
    ESP_LOGI(TAG, "STARTING MQTT");
    esp_mqtt_client_config_t mqttConfig = {
        .uri = "mqtt://34.88.143.118:1883"};

    client = esp_mqtt_client_init(&mqttConfig);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);
}

void DHT_Publisher_task(void *pvParameter)
{

	while(1) {

		char gas[12];
		char temperature[12];
		char pressure[12];
		char alarm_ESP32[12] = "12345";

		sprintf(pressure, "%.2f", battery_percentage);
		sprintf(temperature, "%.2f", battery_percentage2);
		sprintf(gas, "%.2f", battery_percentage3);

		printf("Humidity %s %%\n", pressure);
		printf("Temperature %s degC\n\n", temperature);
		printf("Gas %s degC\n\n", gas);

		esp_mqtt_client_publish(client, MQTT_PUB_ALARM_DUR_DHT, alarm_ESP32, 0, 0, 0);
		esp_mqtt_client_publish(client, MQTT_PUB_PRE_DHT, pressure, 0, 0, 0);
		esp_mqtt_client_publish(client, MQTT_PUB_TEMP_DHT, temperature, 0, 0, 0);
		esp_mqtt_client_publish(client, MQTT_PUB_GAS_DHT, gas, 0, 0, 0);

		vTaskDelay(2000 / portTICK_RATE_MS);

	}

}

void task_MQTTPub()
{
	nvs_flash_init();
    wifi_init();
	xTaskCreate(&DHT_Publisher_task, "DHT_Publisher_task", 2048, NULL, 5, NULL );
}
