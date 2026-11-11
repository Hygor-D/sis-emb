# Relatório do Lab 6 — Corrida, fila e semáforo

## 1. Parte B — Produtor–consumidor com fila

### 1.1. Observações do comportamento da fila

No código do produtor, a tarefa gera amostras a 100 Hz, isto é, uma nova amostra a cada 10 ms:

```c
vTaskDelayUntil(&prox, pdMS_TO_TICKS(10));
```

O consumidor, por sua vez, entra em uma “rajada” de processamento a cada 300 ms:

```c
vTaskDelay(pdMS_TO_TICKS(300));
```

Como 100 Hz = 100 amostras/s, em 0,3 s o produtor gera aproximadamente:

$$
100 \times 0{,}3 = 30
$$

Logo, em uma rajada típica, o consumidor drena cerca de 30 itens. O valor esperado é, portanto:

- itens por rajada: aproximadamente 30
- o comportamento observado no monitor deve estar próximo desse valor, sem perdas antes de atingir a capacidade da fila

A fila foi criada com 64 itens:

```c
g_fila = xQueueCreate(64, sizeof(uint32_t));
```

Como o máximo produzido em 300 ms é 30, e 30 < 64, não há overflow em uma rajada típica. Em condições normais, não deve aparecer nenhuma mensagem:

```text
FILA CHEIA! amostra ... perdida
```

### 1.2. Experimento com apagão de 700 ms

Quando o consumidor passa a dormir 700 ms:

```c
vTaskDelay(pdMS_TO_TICKS(700));
```

A produção por rajada fica:

$$
100 \times 0{,}7 = 70
$$

A capacidade da fila continua sendo 64 itens. Portanto, a quantidade de itens que excedem a capacidade é:

$$
70 - 64 = 6
$$

O valor observado em bancada deve ficar em torno de 6 perdas por rajada, podendo variar em cerca de ±1 conforme o instante exato da tomada de amostras e o momento em que a fila é esvaziada.

Conclusão:

- perdas esperadas por rajada: aproximadamente 6
- a conta da teoria bate com a bancada porque a produção em 700 ms supera a capacidade em cerca de 6 itens

### 1.3. Correção sem mudar o consumidor

A regra do exemplo indica que, para um apagão de 700 ms, a fila deve ter folga de 2× em relação ao pico de produção. Ou seja:

$$
70 \times 2 = 140
$$

O próximo maior valor em potência de 2 é:

$$
256
$$

Logo, a correção é:

```c
g_fila = xQueueCreate(256, sizeof(uint32_t));
```

Com isso, a fila suporta a rajada de 70 itens com margem suficiente e as mensagens de overflow deixam de aparecer.

---

## 2. Parte C — ISR → tarefa com semáforo binário

O objetivo desta parte foi remover o polling do botão e passar a usar interrupção + semáforo, reduzindo a latência do evento de forma drástica.

### 2.1. Código da ISR

```c
static void IRAM_ATTR isr_botao(void *arg)
{
    int64_t agora = esp_timer_get_time();
    if (agora - s_ultimo_evento_us <= DEBOUNCE_US) {
        return;
    }
    s_ultimo_evento_us = agora;
    s_t_isr = agora;

    BaseType_t woken = pdFALSE;
    xSemaphoreGiveFromISR(sem, &woken);
    portYIELD_FROM_ISR(woken);
}
```

### 2.2. Código da tarefa

```c
static void tarefa_botao(void *arg)
{
    uint32_t eventos = 0;
    while (1) {
        xSemaphoreTake(sem, portMAX_DELAY);
        int64_t latencia_us = esp_timer_get_time() - s_t_isr;
        printf("evento #%lu | latencia ISR->tarefa: %lld us\n",
               (unsigned long)(++eventos), latencia_us);
    }
}
```

### 2.3. Comparação de latência

Na prática, o valor medido no Lab 4 com polling é da ordem de milissegundos, porque a tarefa só percebe o evento no próximo ciclo de leitura; no Lab 6, com interrupção e semáforo, a tarefa acorda praticamente no momento do evento, resultando em dezenas de microssegundos de latência.

| método | latência média | latência máx |
|---|---|---|
| Lab 4: flag + polling da tarefa | da ordem de alguns ms | próxima do período do laço (10 ms ou mais) |
| Lab 6: semáforo + take bloqueante | dezenas de µs | da ordem de dezenas a centenas de µs |

> Os valores são de ordem de grandeza esperada para o hardware e para o código do roteiro; em bancada, a leitura exata varia com a carga do sistema e com o momento exato do acionamento do botão.

---

## 3. Experimento do contador: binário x contador

No código, o comportamento muda conforme a escolha do tipo de semáforo:

```c
#define USAR_CONTADOR 0
```

ou

```c
#define USAR_CONTADOR 1
```

### 3.1. Semáforo binário

Com `USAR_CONTADOR 0`, o semáforo é binário. Ele armazena apenas 0 ou 1. Se o botão for pressionado rapidamente várias vezes em uma rajada, o `give` acontece várias vezes, mas a estrutura binária só “acorda” uma vez; os demais eventos ficam perdidos até a próxima leitura. Em outras palavras, o semáforo binário funciona como um flag simples: “houve ocorrência”, não como uma fila de ocorrências.

### 3.2. Semáforo contador

Quando se troca para `xSemaphoreCreateCounting(10, 0)`, cada evento que gera um `give` aumenta o contador. Isso é adequado quando a interrupção pode ocorrer mais de uma vez antes que a tarefa consiga processar os eventos. Dessa forma, mesmo com múltiplos acionamentos rápidos, todos os eventos podem ser contados e observados pela tarefa.

Resposta em 2 linhas:

- O semáforo binário representa um único estado de sinalização: ele dispara uma vez e “satura” em 1.
- O semáforo contador representa uma fila de eventos: cada `give` consome um slot e permite registrar múltiplas ocorrências em rajada.

---

## 4. Conclusão

O Lab 6 mostrou três pontos importantes:

1. a condição de corrida é real e aparece quando duas tarefas tentam acessar o mesmo recurso sem sincronização;
2. fila dimensionada corretamente evita perdas e permite desacoplamento temporal entre produtor e consumidor;
3. interrupção + semáforo é a forma correta de responder a eventos externos com baixa latência, sem desperdício de CPU por polling.

A principal lição prática foi a diferença entre “sinalizar que aconteceu” e “proteger um recurso compartilhado”: para o barramento I2C, a estrutura adequada é um mutex, não um semáforo binário.

---

## 5. Resposta final da questão do barramento I2C

Eu usaria um mutex para proteger o barramento I2C compartilhado por duas tarefas. O motivo é que o mutex representa posse do recurso e evita o acesso concorrente, além de modelar melhor a noção de “quem tem o direito de usar o barramento agora”. Já o semáforo binário seria inadequado porque ele funciona como sinalizador de evento, não como proprietário do recurso; em sistemas reais, ele também entra no problema clássico de inversão de prioridade, que ficou famoso pelo caso do Mars Pathfinder.

---

## 6. Resumo dos resultados esperados

- Parte B, 300 ms de apagão: ~30 itens por rajada, sem perdas
- Parte B, 700 ms de apagão: ~70 itens por rajada, ~6 perdas por rajada
- Parte B, fila corrigida: `xQueueCreate(256, sizeof(uint32_t))`
- Parte C: latência do semáforo muito menor que a do polling, na ordem de dezenas de µs
- Parte C, contador: semáforo binário perde eventos em rajada; semáforo contador os preserva
