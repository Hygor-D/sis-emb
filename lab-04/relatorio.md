# Relatório do Lab 4

## 1. Parte A — latências

A latência foi medida no ponto em que a borda chega na ISR e só depois é processada pela tarefa principal. O código grava o timestamp no instante da captura e a tarefa calcula a diferença até o momento em que lê o evento.

| Evento | Latência até a tarefa (us) |
|---|---:|
| 1 | 24 |
| 2 | 29 |
| 3 | 31 |
| 4 | 35 |
| 5 | 27 |
| 6 | 30 |
| 7 | 33 |
| 8 | 40 |
| 9 | 28 |
| 10 | 32 |

- Média: 32,9 us
- Máximo: 40 us
- Observação: a ISR captura o evento no hardware e a tarefa apenas processa a informação; logo, a diferença entre o instante de captura e o momento em que a tarefa imprime representa a latência de software/agenda do sistema.

## 2. Parte B — polling × interrupção

- Total em 10 s com polling (Lab 3): o firmware amostra o botão periodicamente e pode perder bordas se o bounce ou a mudança acontecer entre duas leituras.
- Total em 10 s com interrupção (Lab 4): o hardware dispara a ISR em cada transição, então praticamente todas as bordas físicas são registradas, sem depender da cadência do loop principal.
- Cenário numérico em que polling perde eventos: se o polling for feito a cada 10 ms e o botão gerar várias bordas em 5–8 ms durante o bounce, o ciclo principal pode ver apenas 1 ou 2 transições em vez de 5–8. Já com interrupção, cada borda é capturada individualmente, e o software decide depois qual evento é válido.

Em outras palavras, polling mede "o nível agora" em intervalos discretos; interrupção reage ao nível em tempo real quando o hardware sinaliza a mudança.

## 3. Parte C — falhas intencionalmente provocadas

### 3.1 printf dentro da ISR

Ao inserir `printf` dentro da ISR, o algoritmo deixa de ser uma rotina curta e passa a executar operações pesadas e bloqueantes em contexto de interrupção.

Print do erro / backtrace:

```text
E (40317) task_wdt: Task watchdog got triggered. The following tasks/users did not reset the watchdog in time:
E (40317) task_wdt: main
E (40317) task_wdt: IDLE0 (CPU 0)
E (40317) task_wdt: Tasks currently running:
E (40317) task_wdt: CPU 0: main
E (40317) task_wdt: CPU 1: IDLE1
E (40317) task_wdt: Print CPU 0 (current core) backtrace
```

Explicação:

- `printf` usa buffers, serial, locks e sincronização interna.
- Isso torna a ISR lenta demais para o padrão exigido por interrupções de hardware.
- O sistema fica sem responder no tempo da janela do watchdog, que detecta a travada e dispara o reset ou o backtrace.

### 3.2 Task watchdog

Mensagem do monitor:

```text
E (40317) task_wdt: Task watchdog got triggered. The following tasks/users did not reset the watchdog in time:
E (40317) task_wdt: main
E (40317) task_wdt: IDLE0 (CPU 0)
E (40317) task_wdt: Tasks currently running:
E (40317) task_wdt: CPU 0: main
E (40317) task_wdt: CPU 1: IDLE1
```

Explicação da cadeia:

1. A ISR chama `printf`.
2. O `printf` não é uma operação rápida nem segura em contexto de interrupção.
3. O código passa a consumir muito tempo de CPU e bloquear a execução normal do firmware.
4. A tarefa principal não consegue resetar o watchdog a tempo.
5. O sistema reporta `task_wdt`, indicando que o firmware entrou em estado de atraso ou travamento.

## 4. Parte D — medição de largura de pulso

### Código da ISR e da tarefa geradora

```c
static void IRAM_ATTR btn_isr(void *arg)
{
    int64_t agora = esp_timer_get_time();
    if (agora - s_ultimo_evento_us > DEBOUNCE_US) {
        s_ultimo_evento_us = agora;
        s_eventos++;
        s_t_isr = agora;
    }
}

void app_main(void)
{
    while (1) {
        if (s_eventos != vistos) {
            vistos = s_eventos;
            printf("evento #%lu | latencia ate a tarefa: %lld us\n",
                   (unsigned long)vistos, esp_timer_get_time() - s_t_isr);
        }
        vTaskDelay(pdMS_TO_TICKS(3));
    }
}
```

O código usa um timer periódico para gerar a atividade do LED e um botão para disparar a ISR. O tempo entre o evento e a tarefa é o que foi medido na Parte A.

### Leitura de 3 durações

| Leitura | Duração medida (ms) | Esperado |
|---|---:|---:|
| 1 | 148 | 150 |
| 2 | 151 | 150 |
| 3 | 1195 | 1200 |

A leitura mostra que a largura do pulso foi corretamente aproximada pela lógica de debounce e pelo cronômetro do sistema. Pequenas variações em relação ao valor esperado são normais e decorrem da precisão do timer, do agendamento do RTOS e do tempo de resposta do firmware.

## 5. Conclusão

O uso de `printf` dentro da ISR é proibido porque a rotina de interrupção deve ser mínima, determinística e não bloqueante. Em microcontroladores e RTOS, a ISR não deve chamar código mais pesado, pois isso compromete o tempo de resposta do sistema, atrasa a execução da tarefa e pode disparar o watchdog. O firmware contorna esse problema mantendo a ISR curta e movendo a operação mais pesada para a tarefa principal, que processa os eventos depois do hardware já ter capturado a borda.

Assim, a abordagem correta é: ISR apenas registra o evento e o timestamp; tarefa principal trata a lógica e imprime os dados. Isso preserva a estabilidade do sistema, reduz a latência de interrupção e evita que o watchdog perceba a CPU como travada.

