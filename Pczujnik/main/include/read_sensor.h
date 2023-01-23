
#ifndef MAIN_READ_SENSOR
#define MAIN_READ_SENSOR

#ifdef __cplusplus
extern "C" {
#endif

extern float temperature;
extern float pressure;


//static const char *TAG = "example_usage";

void task_readI2CData();

#ifdef __cplusplus
}
#endif

#endif /* MAIN_READ_SENSOR */
