// Semana 5 — Parte B, item 10: HOG com prioridade 1 (time slicing com C)
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

// item 10 — tarefa gulosa com prioridade rebaixada para 1
static void cpu_bound(void *arg)
{
    volatile uint32_t x = 0;
    while (1) { x++; }        // sem delay: monopoliza o núcleo
}

void app_main(void)
{
    xTaskCreate(tarefa, "A", 2048, "A", 5, NULL);
    xTaskCreate(tarefa, "B", 2048, "B", 3, NULL);
    xTaskCreate(tarefa, "C", 2048, "C", 1, NULL);

    xTaskCreatePinnedToCore(cpu_bound, "HOG", 2048, NULL, 1, NULL, 0);   // item 10
}
