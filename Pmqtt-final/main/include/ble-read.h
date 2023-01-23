#ifndef MAIN_BLE
#define MAIN_BLE

#ifdef __cplusplus
extern "C" {
#endif

extern float odczytBLE;
extern float temperatureBLE;
extern float pressureBLE;

void task_BLERead(void);

#ifdef __cplusplus
}
#endif

#endif /* MAIN_BLE */
