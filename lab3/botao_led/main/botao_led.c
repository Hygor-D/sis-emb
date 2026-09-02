// Semana 3 — botão com pull-down externo + debounce por software (polling)
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include <stdio.h>

#define LED   GPIO_NUM_2
#define BTN   GPIO_NUM_4    // Mudança 1: GPIO0 é o pino de boot — não use pull-down nele
#define DEBOUNCE_MS 20
#define DOUBLE_CLICK_MS 400

void app_main(void)
{
    gpio_reset_pin(LED);
    gpio_set_direction(LED, GPIO_MODE_OUTPUT);

    gpio_reset_pin(BTN);
    gpio_set_direction(BTN, GPIO_MODE_INPUT);
    gpio_pullup_dis(BTN);             // Mudança 2: desabilita o pull-up interno
    gpio_pulldown_dis(BTN);           // Mudança 3: desabilita o pull-down interno também
                                       // (queremos só o resistor de 10 kΩ externo)
                                       // repouso = 0; pressionado = 1

    int led = 0, eventos = 0;
    int nivel_ant = 0;                // repouso agora é 0 (antes era 1)
    int64_t t_ok = 0;                 // instante a partir do qual aceitamos nova borda
    int primeiro_pendente = 0;
    int64_t limite_duplo = 0;

    while (1) {
        int nivel = gpio_get_level(BTN);
        int64_t agora = esp_timer_get_time() / 1000;      // ms
        if (primeiro_pendente && agora >= limite_duplo) {
            led = !led;
            gpio_set_level(LED, led);
            printf("clique simples #%d\n", ++eventos);
            primeiro_pendente = 0;
        }

        if (nivel_ant == 0 && nivel == 1 && agora >= t_ok) {  // borda de subida válida
            if (primeiro_pendente && agora < limite_duplo) {
                printf("clique duplo\n");
                primeiro_pendente = 0;

                for (int piscada = 0; piscada < 3; piscada++) {
                    gpio_set_level(LED, 1);
                    vTaskDelay(pdMS_TO_TICKS(100));
                    gpio_set_level(LED, 0);
                    vTaskDelay(pdMS_TO_TICKS(100));
                }
                gpio_set_level(LED, led);
            } else {
                primeiro_pendente = 1;
                limite_duplo = agora + DOUBLE_CLICK_MS;
                printf("primeiro clique; aguardando segundo\n");
            }

            t_ok = agora + DEBOUNCE_MS;
        }
        nivel_ant = nivel;
        // Garantir pelo menos 1 tick no padrão de 100Hz do ESP-IDF
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}