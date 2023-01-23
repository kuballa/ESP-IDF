#ifndef MAIN_BLE
#define MAIN_BLE

#ifdef __cplusplus
extern "C" {
#endif

extern float battery_percentage;
extern float battery_percentage2;
extern float battery_percentage3;

void task_BLERead(void);

#ifdef __cplusplus
}
#endif

#endif /* MAIN_BLE */
