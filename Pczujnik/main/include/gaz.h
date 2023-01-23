#ifndef MAIN_BLE
#define MAIN_BLE

#ifdef __cplusplus
extern "C" {
#endif

//
extern float ostatnie_napiecie_na_czujniku_mV;
extern float prog_alarmu_mV;
extern long int czas_alarmu_ms;
extern long int czas_pracy_ms;//w milisekundach

//

float napiecie_na_czujniku_mV();

void przygotuj_piny(void);

float dodaj_do_sredniej_z_ostatnich(float nowa_wartosc);

void dioda_alarmu(int stan);

void piszczyk(int stan);

void task_piszczenie(void * parameter);

void podnies_alarm(void);

void kontynuacja_alarmu(void);

void zakoncz_alarm(void);

void czuj_gaz(void);

void info();

void task_blink(void * parameter);

void ustaw_przerwania(void);

void podzialka(float v1, float v2);

void task_Gaz(void);

#ifdef __cplusplus
}
#endif

#endif /* MAIN_BLE */
