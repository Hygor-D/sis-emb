// Semana 6A — condição de corrida e correção com mutex
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_task_wdt.h"
#include "esp_random.h"
#include <stdio.h>

#define USAR_MUTEX 0
#define N 1000000

static volatile uint32_t g_contador = 0;
static SemaphoreHandle_t g_mutex;

// Em hardware real (240 MHz), a janela de risco entre o LOAD e o STORE de "g_contador++"
// dura poucos nanossegundos — curta demais para o escalonador "acertar" com frequência
// em 1 milhão de tentativas. Esta função alarga essa janela de propósito (só para tornar
// a corrida observável neste experimento), sem mudar a natureza do bug: continua sendo
// leitura-modificação-escrita não atômica, só que agora com tempo de sobra para colidir.
//
// O tamanho da pausa é sorteado a cada chamada com esp_random() (gerador de números
// aleatórios por hardware do ESP32). Sem isso, a pausa fixa colide sempre no MESMO ponto
// relativo entre T1 e T2 a cada reset — sem Wi-Fi/Bluetooth/botão rodando, não há nenhum
// evento assíncrono do mundo real para desalinhar o encontro, e o resultado final sai
// idêntico execução após execução (um determinismo "escondido" dentro do próprio
// hardware, não só no simulador). Sorteando a pausa, o ponto exato da colisão muda a cada
// execução de verdade.
static inline void amplia_janela_de_risco(void)
{
    int n = 10 + (esp_random() % 40);     // entre 10 e 49 iterações, sorteado agora
    for (volatile int k = 0; k < n; k++) { }
}

static void incrementador(void *arg)
{
    for (int i = 0; i < N; i++) {
#if USAR_MUTEX
        xSemaphoreTake(g_mutex, portMAX_DELAY);
        uint32_t tmp = g_contador;      // LOAD
        amplia_janela_de_risco();
        g_contador = tmp + 1;           // STORE
        xSemaphoreGive(g_mutex);
#else
        uint32_t tmp = g_contador;      // LOAD
        amplia_janela_de_risco();       // leitura-modificação-escrita NÃO atômica
        g_contador = tmp + 1;           // STORE
#endif
    }
    printf("tarefa %s terminou; contador=%lu\n",
           (char *)arg, (unsigned long)g_contador);
    vTaskDelete(NULL);               // a tarefa termina sozinha ao sair do for
}

void app_main(void)
{
    // T1 e T2 ficam no mesmo núcleo, mesma prioridade, e (com USAR_MUTEX 1) fazem
    // 2 milhões de Take/Give SEM nunca ceder a CPU — a IDLE1 daquele núcleo fica sem
    // rodar tempo suficiente para alimentar o Task Watchdog (timeout padrão 5 s), e o
    // sistema reiniciaria antes de você ler o resultado. Provocar o watchdog não é o
    // assunto desta parte (isso já foi coberto nos Labs 4 e 5) — desligamos aqui de
    // propósito, só para esta medição.
    esp_task_wdt_deinit();

    g_mutex = xSemaphoreCreateMutex();
    // mesmo núcleo p/ maximizar preempções visíveis
    xTaskCreatePinnedToCore(incrementador, "T1", 2048, "T1", 3, NULL, 1);
    xTaskCreatePinnedToCore(incrementador, "T2", 2048, "T2", 3, NULL, 1);
}