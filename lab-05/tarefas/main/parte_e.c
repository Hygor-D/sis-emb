// Semana 5 — Parte E, item 16: dual-core (HOG isolado no core 1)
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

// item 16 — tarefa gulosa, agora isolada no core 1: não compete mais com A/B/C (core 0)
static void cpu_bound(void *arg)
{
    volatile uint32_t x = 0;
    int64_t proximo_print = esp_timer_get_time();
    while (1) {
        x++;
        // HOG "confessa" o núcleo sem afogar o monitor:
        if (esp_timer_get_time() >= proximo_print) {
            printf("[HOG] core=%d\n", xPortGetCoreID());
            proximo_print = esp_timer_get_time() + 1000000;   // a cada ~1 s
        }
    }
}

void app_main(void)
{
    xTaskCreate(tarefa, "A", 2048, "A", 5, NULL);
    xTaskCreate(tarefa, "B", 2048, "B", 3, NULL);
    xTaskCreate(tarefa, "C", 2048, "C", 1, NULL);

    xTaskCreatePinnedToCore(cpu_bound, "HOG", 2048, NULL, 6, NULL, 1);   // item 16
}
