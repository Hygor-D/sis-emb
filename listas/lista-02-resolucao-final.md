# Sistemas Embarcados – Lista de Exercícios 02
## Resolução final — Semanas 4 a 6

**Bancada-07**  
**Alunos:** Nalion Alvim Assis Filho, Hygor Cezar Dutra e Gustavo Passos Ribeiro  
**Disciplina:** Sistemas Embarcados

---

# Questão 1

| Critério | Polling | Interrupção |
|---|---|---|
| Latência de pior caso | aproximadamente o período de varredura, mais o tratamento | poucos microssegundos, mais salvamento de contexto e despacho |
| Uso de CPU | contínuo, inclusive sem eventos | praticamente nulo enquanto não há eventos |
| Complexidade | menor; fluxo sequencial simples | maior; requer ISR curta e sincronização correta |

Polling é adequado para sinais lentos e periódicos, quando a latência máxima permitida é maior que o período de varredura e o custo de CPU é aceitável.

---

# Questão 2

O encoder pode gerar 1 kHz; logo, o período mínimo entre pulsos é T = 1/1000 = 1 ms. A varredura de 2 ms é maior que esse período, portanto um pulso de 400 µs pode começar e terminar entre duas leituras e ser perdido.

Pelo critério da aula, T_poll ≤ T_evento/10 = 1 ms/10 = 100 µs. Seriam necessárias pelo menos 10 000 leituras por segundo. Para esse evento rápido, a ISR se impõe: captura cada borda com baixa latência e não desperdiça CPU entre pulsos.

---

# Questão 3

As três regras de ouro de uma ISR são:

1. **Ser curta:** apenas captura o evento, carimba tempo, atualiza o mínimo ou sinaliza uma tarefa. Processamento pesado aumenta a latência de todo o sistema.
2. **Não usar chamadas bloqueantes:** vTaskDelay, printf, malloc e mutexes não devem ser usados. No Lab 4, printf dentro da ISR pode causar *crash*, pois não é seguro nesse contexto.
3. **Usar IRAM_ATTR e APIs FromISR:** IRAM_ATTR mantém a ISR acessível quando a flash/cache não está disponível. Para acordar tarefas, usam-se xSemaphoreGiveFromISR ou xQueueSendFromISR, seguidos de portYIELD_FROM_ISR quando necessário.

Uma ISR longa, ou uma tarefa que monopoliza a CPU, pode impedir a tarefa IDLE de executar e disparar o task_wdt.

---

# Questão 4

A relação do timer é T = (prescaler × alarme)/clock.

Com clock de 80 MHz e período de 250 ms, duas soluções são:

- **Prescaler 80:** tick = 80 MHz / 80 = 1 MHz = 1 µs; alarme = 250 000.
- **Prescaler 8000:** tick = 10 kHz = 100 µs; alarme = 2 500.

As duas são válidas. A primeira é preferível por oferecer resolução de 1 µs e por se alinhar com esp_timer_get_time().

---

# Questão 5

O watchdog timer é um contador de hardware que reinicia o sistema se o software saudável não o alimentar dentro do prazo. Ele mitiga falhas como laço infinito, *deadlock* e monopólio da CPU.

Alimentar o WDT em uma ISR de timer é um anti-padrão: a ISR pode continuar viva mesmo se a aplicação principal estiver travada. Assim, o WDT nunca dispara e deixa de detectar a falha que deveria recuperar.

---

# Questão 6

~~~text
pronta --(escalonador)--> executando
executando --(preempção)--> pronta
executando --(vTaskDelayUntil / espera de recurso)--> bloqueada
bloqueada --(fim do delay / recurso disponível)--> pronta
pronta ou bloqueada --(vTaskSuspend)--> suspensa
suspensa --(vTaskResume)--> pronta
~~~

No Lab 5, vTaskDelayUntil leva a tarefa A de executando a bloqueada. Ao fim do atraso, ela volta a pronta. O escalonador a torna executando se for a pronta de maior prioridade. A HOG pode preemptar outra tarefa, que retorna a pronta. vTaskSuspend e vTaskResume representam, respectivamente, as transições para suspensa e de volta a pronta.

---

# Questão 7

Com vTaskDelay(100 ms), o atraso é relativo ao fim do trabalho. Como o corpo dura 7 ms:

T_efetivo = 7 + 100 = **107 ms**.

A frequência efetiva é 1/0,107 ≈ 9,35 Hz. Cada ciclo acumula 7 ms de erro; em 600 períodos nominais:

600 × 7 ms = **4,2 s**.

Com vTaskDelayUntil, a próxima execução é referenciada ao instante programado, e não ao fim do corpo da tarefa. Assim, o período permanece 100 ms, desde que o trabalho termine antes do próximo prazo.

---

# Questão 8

As prioridades devem ser A > B > C, pois A tem o menor período e C é apenas log.

U_A = 3/10 = 0,30; U_B = 10/100 = 0,10; portanto U_total = **0,40 = 40%**.

Com 60% livre, C roda quando A e B bloqueiam, desde que C também ceda a CPU adequadamente.

Se B passar a usar 60 ms por 100 ms, U_B = 0,60 e U_total = **0,90 = 90%**. C ainda pode executar nos intervalos ociosos, mas restam apenas 10% de CPU e pouca margem para *jitter* ou picos. Como é log, deve ter a menor prioridade e bloquear quando não houver trabalho.

---

# Questão 9

*Starvation* ocorre quando uma tarefa pronta não recebe CPU por tempo indefinido porque tarefas de maior prioridade a ocupam. Entre tarefas da mesma prioridade, o FreeRTOS pode usar *time slicing*, alternando-as a cada *tick* enquanto permanecem prontas.

Uma tarefa de prioridade alta com while(1) sem bloqueio fica sempre pronta e impede tarefas inferiores, inclusive a IDLE, de rodar. O Task Watchdog então registra mensagens task_wdt para IDLE0/IDLE1, como nos testes HOG do Lab 5 e PROVOCAR_WDT do Lab 4.

---

# Questão 10

Fixar a tarefa de controle no core 1 com xTaskCreatePinnedToCore deixa o core 0 mais disponível para Wi-Fi, Bluetooth e serviços do sistema. O benefício é menor *jitter* para o controle.

O cuidado é que os dois cores executam em paralelo. Dados compartilhados podem sofrer concorrência simultânea; filas, mutexes e seções críticas passam a ser ainda mais importantes.

---

# Questão 11

g++ realiza três passos: ler, incrementar e gravar. Um entrelaçamento possível é:

| Passo | Tarefa A | Tarefa B | g |
|---|---|---|---|
| 1 | lê 10 | | 10 |
| 2 | é preemptada | lê 10 | 10 |
| 3 | | incrementa e grava 11 | 11 |
| 4 | incrementa seu valor antigo e grava 11 | | 11 |

Dois incrementos produziram somente um. Com duas tarefas fazendo 1000 incrementos, o valor final pode variar aproximadamente de **1000 a 2000**.

Com mutex:

~~~c
xSemaphoreTake(g_mutex, portMAX_DELAY);
g_contador++;
xSemaphoreGive(g_mutex);
~~~

A alternativa sem bloqueio é manter um contador por tarefa e agregar os valores ao final. Ela não serve quando é necessário observar um único valor global consistente em tempo real.

---

# Questão 12

| Primitiva | O que sinaliza/carrega | Uso típico no curso | Uso em ISR |
|---|---|---|---|
| Fila | cópias de dados FIFO | ADC/produtor→consumidor | sim, com xQueueSendFromISR |
| Semáforo binário | um evento; satura em 1 | botão acordando tarefa | sim, com xSemaphoreGiveFromISR |
| Semáforo contador | número de eventos/recursos | rajadas, buffers ou pulsos | sim, para give com variante FromISR |
| Mutex | posse exclusiva de recurso | contador, UART ou I2C | não |
| Event group | combinação de bits | Wi-Fi conectado e MQTT pronto | com restrições; normalmente via mecanismo intermediário |

---

# Questão 13

A ISR produz 2000 amostras/s. Em 40 ms, o acúmulo máximo é:

N_mín = 2000 × 0,040 = **80 itens**.

Com folga de 2×, a fila deve ter 160 itens. Como cada item tem 4 B:

RAM = 160 × 4 = **640 B**.

Se o Wi-Fi roubar CPU por 60 ms, surgem 2000 × 0,060 = **120 itens**. Uma fila sem folga, de 80 itens, estouraria e perderia 40 amostras. A fila de 160 itens suporta o atraso e preserva 40 posições de margem.

---

# Questão 14

Inversão de prioridade ocorre quando uma tarefa alta espera um recurso preso por uma tarefa baixa, mas tarefas médias impedem a baixa de executar e liberar o recurso.

No Mars Pathfinder, a tarefa de barramento de alta prioridade aguardava um recurso mantido pela tarefa meteorológica de baixa prioridade. Uma tarefa de comunicações de prioridade média continuava executando, atrasando a baixa; o watchdog reiniciava a sonda. A correção foi ativar herança de prioridade.

Com mutex, a tarefa baixa que possui o recurso herda temporariamente a prioridade da tarefa alta e pode liberar o recurso sem ser preemptada pela média. Semáforo binário não possui dono, portanto não fornece herança de prioridade.

---

# Questão 15

~~~c
static SemaphoreHandle_t sem_botao;

static void IRAM_ATTR btn_isr(void *arg)
{
    BaseType_t tarefa_acordada = pdFALSE;
    xSemaphoreGiveFromISR(sem_botao, &tarefa_acordada);
    portYIELD_FROM_ISR(tarefa_acordada);
}

static void tarefa_botao(void *arg)
{
    while (true) {
        xSemaphoreTake(sem_botao, portMAX_DELAY);

        // debounce fino, printf, atualização de estado e I/O
    }
}
~~~

A ISR apenas sinaliza o evento e sai rapidamente. O trabalho pesado fica na tarefa porque ela pode bloquear, usar APIs normais, ter prioridade definida e ser preemptada sem congelar o sistema. portYIELD_FROM_ISR permite que uma tarefa de maior prioridade execute logo na saída da ISR.
