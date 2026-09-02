#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/gpio_reg.h"      // define GPIO_OUT_W1TS_REG etc. (evita números mágicos)
#include "driver/gpio.h"

#define BIT_LED (1u << 2)      // GPIO 2 → bit 2 dos registradores de GPIO 0–31

void app_main(void)
{
    gpio_reset_pin(GPIO_NUM_2);
    
    // Substitui a direção via driver pelo acesso direto ao registrador ENABLE
    *(volatile uint32_t *)GPIO_ENABLE_REG |= BIT_LED;

    volatile uint32_t *w1ts = (volatile uint32_t *)GPIO_OUT_W1TS_REG; // "write 1 to SET"
    volatile uint32_t *w1tc = (volatile uint32_t *)GPIO_OUT_W1TC_REG; // "write 1 to CLEAR"

    while (1) {
        *w1ts = BIT_LED;                    // liga: escreve 1 SÓ no bit 2 (atômico!)
        vTaskDelay(pdMS_TO_TICKS(250));
        *w1tc = BIT_LED;                    // desliga: idem
        vTaskDelay(pdMS_TO_TICKS(250));
    }
}