

```diff
-// Semana 3 — botão com pull-up interno + debounce por software (polling)
+// Semana 3 — botão com pull-down externo + debounce por software (polling)

 #include "freertos/FreeRTOS.h"
 #include "freertos/task.h"
 #include "driver/gpio.h"
 #include "esp_timer.h"
 #include <stdio.h>

 #define LED   GPIO_NUM_2
-#define BTN   GPIO_NUM_0
+#define BTN   GPIO_NUM_4
 #define DEBOUNCE_MS 20
+#define DOUBLE_CLICK_MS 400

 void app_main(void)
 {
     gpio_reset_pin(LED);
     gpio_set_direction(LED, GPIO_MODE_OUTPUT);

     gpio_reset_pin(BTN);
     gpio_set_direction(BTN, GPIO_MODE_INPUT);
-    gpio_pullup_en(BTN);              // repouso = 1; pressionado = 0
+    gpio_pullup_dis(BTN);
+    gpio_pulldown_dis(BTN);
+                                      // repouso = 0; pressionado = 1

     int led = 0, eventos = 0;
-    int nivel_ant = 1;
-    int64_t t_ok = 0;                 // instante a partir do qual aceitamos nova borda
+    int nivel_ant = 0;
+    int64_t t_ok = 0;
+    int primeiro_pendente = 0;
+    int64_t limite_duplo = 0;

     while (1) {
         int nivel = gpio_get_level(BTN);
         int64_t agora = esp_timer_get_time() / 1000;

-        if (nivel_ant == 1 && nivel == 0 && agora >= t_ok) {
-            led = !led;
-            gpio_set_level(LED, led);
-            printf("evento #%d\n", ++eventos);
+        if (primeiro_pendente && agora >= limite_duplo) {
+            led = !led;
+            gpio_set_level(LED, led);
+            printf("clique simples #%d\n", ++eventos);
+            primeiro_pendente = 0;
+        }
+
+        if (nivel_ant == 0 && nivel == 1 && agora >= t_ok) {
+            if (primeiro_pendente && agora < limite_duplo) {
+                printf("clique duplo\n");
+                primeiro_pendente = 0;
+
+                for (int piscada = 0; piscada < 3; piscada++) {
+                    gpio_set_level(LED, 1);
+                    vTaskDelay(pdMS_TO_TICKS(100));
+                    gpio_set_level(LED, 0);
+                    vTaskDelay(pdMS_TO_TICKS(100));
+                }
+
+                gpio_set_level(LED, led);
+            } else {
+                primeiro_pendente = 1;
+                limite_duplo = agora + DOUBLE_CLICK_MS;
+                printf("primeiro clique; aguardando segundo\n");
+            }
+
             t_ok = agora + DEBOUNCE_MS;
         }

         nivel_ant = nivel;
         vTaskDelay(pdMS_TO_TICKS(10));
```