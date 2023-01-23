#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_event.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "math.h"

#define LOW 0
#define HIGH 1

#define PIN_DIODY_ALARMU 2
#define PIN_PISZCZYKA 19
#define PIN_DIODY_GOTOWOSCI 15
#define PIN_GUZIKA 18
#define KANAL_ANALOGOWY ADC1_CHANNEL_6

#define ANALOG_ATTENUATION ADC_ATTEN_DB_11 // 150 mV - 2450 mV
#define MAX_U_ADC_mV (2450)

uint32_t napiecie_analogu;
long int czas_pracy_ms = 0;//w milisekundach
long int czas_nagrzewania_ms = 2 *60 * 1000;//2minuty
const int interwal_pomiaru_ms = 1000;
float napiecie_na_czujniku_mV()
{
    /*
    Vout Digital output result, standing for the voltage.
    Dout ADC raw digital reading result.
    Vmax Maximum measurable input analog voltage, see ADC Attenuation.
    Dmax Maximum of the output ADC raw digital reading result, which is 4095 under Single Read mode, 4095 under Continuous Read mode.
    Vout = Dout * Vmax / Dmax
    */
   return (1.0f*napiecie_analogu * MAX_U_ADC_mV) / 4095 * 1.5 ;//*(3676.0/4096);//*3/2, bo przechodzi przez dzielnik
}
float ostatnie_napiecie_na_czujniku_mV;
float prog_alarmu_mV = 1000;
int stan_alarmu = 0;
long int czas_alarmu_ms=0;
void przygotuj_piny(void)
{
    gpio_pad_select_gpio(PIN_DIODY_ALARMU);                    // Defines GPIO functionality for the pin
    gpio_set_direction(PIN_DIODY_ALARMU, GPIO_MODE_OUTPUT);    // Pin input/output definition

    gpio_pad_select_gpio(PIN_DIODY_GOTOWOSCI);                    // Defines GPIO functionality for the pin
    gpio_set_direction(PIN_DIODY_GOTOWOSCI, GPIO_MODE_OUTPUT);    // Pin input/output definition

    gpio_pad_select_gpio(PIN_PISZCZYKA);                    // Defines GPIO functionality for the pin
    gpio_set_direction(PIN_PISZCZYKA, GPIO_MODE_OUTPUT);    // Pin input/output definition

    // Configure ADC1 capture width
    // 12 bit decimal value from 0 to 4095
    adc1_config_width(ADC_WIDTH_BIT_12);
    // Configure the ADC1 channel (ADC1_CHANNEL_6 - pin 34), and setting attenuation (ADC_ATTEN_DB_11)
    adc1_config_channel_atten(KANAL_ANALOGOWY, ANALOG_ATTENUATION);
}


#define rozm_buf_sr (5)
float buf_sr[rozm_buf_sr] = {0};
int ost_indeks_buf_sr=0;
float srednia_z_ostatnich=0.0;
float dodaj_do_sredniej_z_ostatnich(float nowa_wartosc)
{
    ost_indeks_buf_sr = ost_indeks_buf_sr>=(rozm_buf_sr-1)?0:ost_indeks_buf_sr+1;
    buf_sr[ost_indeks_buf_sr] = nowa_wartosc;
    float akum=0.0; for(int i=0;i<rozm_buf_sr;i++){akum+=buf_sr[i];}
    srednia_z_ostatnich = akum/rozm_buf_sr;
    return srednia_z_ostatnich;
}
int gotowosc=0;
int stan_diody_gotowosci = 0;
int stan_diody_alarmu = 0;
void dioda_alarmu(int stan)
{
    gpio_set_level(PIN_DIODY_ALARMU, stan);            // Define voltage level 0/1
    stan_diody_alarmu = stan;

}
int piszczyk_zablokowany=0;
int stan_piszczyka = 0;
void piszczyk(int stan)
{
    if(!piszczyk_zablokowany || (stan_piszczyka == 1 && stan ==0))
    {
        gpio_set_level(PIN_PISZCZYKA, stan);
        stan_piszczyka = stan;
    }
}

TaskHandle_t task_piszczenie_Handle = NULL;

void task_piszczenie(void * parameter){
    while(1)
    {
        piszczyk(!stan_piszczyka);
        vTaskDelay( (int)( 500.0/srednia_z_ostatnich*prog_alarmu_mV / portTICK_PERIOD_MS));
    }
}


void podnies_alarm(void)
{
    stan_alarmu = 1;
    czas_alarmu_ms =0;
    dioda_alarmu(HIGH);
    piszczyk(HIGH);
    printf("ALARM\n");

    xTaskCreate(
        task_piszczenie,
        "Task_piszczenie",
        1000,
        NULL,
        1,
        &task_piszczenie_Handle            // Task handle
    );
}

void kontynuacja_alarmu(void)
{
    static int cykl=0;
    czas_alarmu_ms +=interwal_pomiaru_ms;
}

void zakoncz_alarm(void)
{
    stan_alarmu = 0;
    dioda_alarmu(LOW);
    piszczyk(LOW);
    printf("\n\nKONIEC ALARMU TRWAJACEGO %.1fsek\n\n",(float)czas_alarmu_ms/1000);
    vTaskDelete(task_piszczenie_Handle);

}
void czuj_gaz(void)
{
    // Take an ADC1 reading on a single channel (ADC1_CHANNEL_6 pin 34)
    // 11dB attenuation (ADC_ATTEN_DB_11) gives full-scale voltage 0 - 3.9V
    // 4053 ~ 3.86V
    napiecie_analogu = adc1_get_raw(ADC1_CHANNEL_6);
    ostatnie_napiecie_na_czujniku_mV = napiecie_na_czujniku_mV();
    dodaj_do_sredniej_z_ostatnich(ostatnie_napiecie_na_czujniku_mV);
    if(srednia_z_ostatnich > prog_alarmu_mV * 1.05)
    {
        if(!stan_alarmu){podnies_alarm();}
    }
    if(srednia_z_ostatnich < prog_alarmu_mV * 0.95){
        if(stan_alarmu){zakoncz_alarm();}
    }
    if(stan_alarmu){kontynuacja_alarmu();}
}

void info()
{
    printf("CZUJNIK GAZU\n");

    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("This is %s chip with %d CPU core(s), WiFi%s%s, ",
            CONFIG_IDF_TARGET,
            chip_info.cores,
            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
            (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    printf("silicon revision %d, ", chip_info.revision);

    printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    printf("Minimum free heap size: %d bytes\n", esp_get_minimum_free_heap_size());
}


TaskHandle_t task_blink_Handle = NULL;

void task_blink(void * parameter){
    static int cykle_z_malym_napieciem = 0;
    while(1)
    {
        gpio_set_level(PIN_DIODY_GOTOWOSCI, stan_diody_gotowosci=!stan_diody_gotowosci);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        if(ostatnie_napiecie_na_czujniku_mV < 0.8 * prog_alarmu_mV)
        {cykle_z_malym_napieciem++;}
        else{cykle_z_malym_napieciem=0;}
        if(czas_pracy_ms > czas_nagrzewania_ms || gotowosc == 1 || cykle_z_malym_napieciem > 2 * 8)
        {
            gotowosc = 1;
            stan_diody_gotowosci =1;
            gpio_set_level(PIN_DIODY_GOTOWOSCI, stan_diody_gotowosci);
                printf("KONCZENIE WATKU MIGANIA DIODY GOTOWOSCI\n");
                vTaskDelete(NULL);
        }
    }
}

static void IRAM_ATTR gpio_interrupt_handler(void *args)
{
    int pinNumber = (int)args;
    piszczyk_zablokowany = !piszczyk_zablokowany;
}
void ustaw_przerwania(void)
{
    gpio_pad_select_gpio(PIN_GUZIKA);
    gpio_set_direction(PIN_GUZIKA, GPIO_MODE_INPUT);
    gpio_pulldown_en(PIN_GUZIKA);
    gpio_pullup_dis(PIN_GUZIKA);
    gpio_set_intr_type(PIN_GUZIKA, GPIO_INTR_HIGH_LEVEL);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(PIN_GUZIKA, gpio_interrupt_handler, (void *)PIN_GUZIKA);
}

/*Wypisuje ładny pasek*/
void podzialka(float v1, float v2)
{
    const static int podzialki = 60;
    const static float min_v = 400;
    const static float max_v = MAX_U_ADC_mV;
    float v1p = (v1-min_v)/max_v * podzialki;
    float v2p = (v2-min_v)/max_v * podzialki;
    printf("%.0f|",min_v);
    int i=0;
    if(v1<v2)
    {
        for(;i<podzialki && i<(int)v1p;i++){putc('#',stdout);}//#
        for(;i<podzialki && i<(int)v2p;i++){putc('*',stdout);}//*
    }
    else{
        {
        for(;i<podzialki && i<(int)v2p;i++){putc('#',stdout);}//#
        for(;i<podzialki && i<(int)v1p;i++){putc('+',stdout);}//+
    }
    }
    for(;i<podzialki;i++){putc('-',stdout);}//-
    printf("|%.0f",max_v);
}

void task_Gaz(void)
{
    int j;
    info();
    przygotuj_piny();
    ustaw_przerwania();
    int c=0;
    int m;//minuty
    for (int i = 0; 1; i++) {
        czuj_gaz();//odczyt konwertera analogowo-cyfrowego
        vTaskDelay(interwal_pomiaru_ms / portTICK_PERIOD_MS);
        m=i/60;
        printf("[%dm %ds]<%4.0fmV %4d>",m, i-m*60,ostatnie_napiecie_na_czujniku_mV,napiecie_analogu);
        printf("%4.0fmV", floor(ostatnie_napiecie_na_czujniku_mV/100));
        printf(" blok_piszczyka:%d",piszczyk_zablokowany);
        putc('\n',stdout);
        czas_pracy_ms +=interwal_pomiaru_ms;//tymczasowe
        c++;
        if(c>10 && !piszczyk_zablokowany){
            c=0;
            }
    }
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}
