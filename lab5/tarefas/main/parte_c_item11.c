// Semana 5 — Parte C, item 11: deriva de período (versão com vTaskDelay)
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include <stdio.h>

static void tarefa(void *arg)
{
    const char *nome = (const char *)arg;
    TickType_t proximo = xTaskGetTickCount();
    int64_t t_ant = esp_timer_get_time();
    while (1) {
        vTaskDelayUntil(&proximo, pdMS_TO_TICKS(500));
        int64_t t = esp_timer_get_time();
        printf("[%s] core=%d  periodo=%.1f ms\n",
               nome, xPortGetCoreID(), (t - t_ant) / 1000.0);
        t_ant = t;
    }
}

// item 11 — corpo "lento" (50 ms de trabalho ocupado) + vTaskDelay (sono RELATIVO
// ao instante em que é chamado) — a receita da deriva.
static void tarefa_d(void *arg)
{
    int64_t t_ant = esp_timer_get_time();
    while (1) {
        int64_t fim = esp_timer_get_time() + 50000;      // "trabalho" de 50 ms
        while (esp_timer_get_time() < fim) { }            // (ocupado de propósito)
        vTaskDelay(pdMS_TO_TICKS(200));                    // dorme 200 ms A PARTIR DE AGORA
        int64_t t = esp_timer_get_time();
        printf("[D] periodo=%.1f ms\n", (t - t_ant) / 1000.0);
        t_ant = t;
    }
}

void app_main(void)
{
    xTaskCreate(tarefa, "A", 2048, "A", 5, NULL);
    xTaskCreate(tarefa, "B", 2048, "B", 3, NULL);
    xTaskCreate(tarefa, "C", 2048, "C", 1, NULL);

    xTaskCreate(tarefa_d, "D", 2048, NULL, 4, NULL);
}
