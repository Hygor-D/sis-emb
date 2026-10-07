// Semana 5 — Parte D, item 15: pilha com buffer grande (provocação)
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include <stdio.h>
#include <string.h>

static void tarefa(void *arg)
{
    const char *nome = (const char *)arg;
    TickType_t proximo = xTaskGetTickCount();
    int64_t t_ant = esp_timer_get_time();
    while (1) {
        vTaskDelayUntil(&proximo, pdMS_TO_TICKS(500));
        int64_t t = esp_timer_get_time();

        if (strcmp(nome, "A") == 0) {
            // item 15 — buffer grande para provocar uso de pilha
            char buf[1500];
            snprintf(buf, sizeof buf, "x");
            printf("%s", buf);

            printf("[A] pilha livre: %u palavras (%u bytes)\n",
                   (unsigned)uxTaskGetStackHighWaterMark(NULL),
                   (unsigned)uxTaskGetStackHighWaterMark(NULL) * 4);
        }

        printf("[%s] core=%d  periodo=%.1f ms\n",
               nome, xPortGetCoreID(), (t - t_ant) / 1000.0);
        t_ant = t;
    }
}

void app_main(void)
{
    xTaskCreate(tarefa, "A", 2048, "A", 5, NULL);
    xTaskCreate(tarefa, "B", 2048, "B", 3, NULL);
    xTaskCreate(tarefa, "C", 2048, "C", 1, NULL);
}
