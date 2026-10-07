// Semana 5 — Parte C, item 13: correção da deriva com vTaskDelayUntil
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

// item 13 — mesma tarefa D, agora com vTaskDelayUntil: o corpo de 50 ms fica
// "absorvido" dentro do período de 200 ms, em vez de somado a ele.
static void tarefa_d(void *arg)
{
    TickType_t proximo = xTaskGetTickCount();
    int64_t t_ant = esp_timer_get_time();
    while (1) {
        int64_t fim = esp_timer_get_time() + 50000;      // "trabalho" de 50 ms
        while (esp_timer_get_time() < fim) { }            // (ocupado de propósito)
        vTaskDelayUntil(&proximo, pdMS_TO_TICKS(200));     // dorme até o PRÓXIMO alvo absoluto
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
