# Lab 05 — Relatório: Tarefas, prioridades, deriva de período e dual-core

---

## 1. Parte A — Três tarefas, um escalonador (Questão 7)

**Por que prioridades diferentes convivem em CPU ociosa?**

As três tarefas A (prio 5), B (prio 3) e C (prio 1) dormem ~499 ms a cada 500 ms, mantendo a CPU ociosa ~99,9% do tempo. Prioridade só decide **disputas** — quem roda primeiro quando duas tarefas ficam prontas simultaneamente. Como quase não há disputa (a probabilidade de duas acordarem no mesmo tick é mínima), todas rodam sem interferência, como três pessoas num corredor largo onde a regra de prioridade de passagem nunca precisa ser aplicada.

**Saída esperada (~30 s de observação):**

```
[A] core=0  periodo=500.0 ms
[B] core=0  periodo=500.1 ms
[C] core=0  periodo=499.9 ms
[A] core=0  periodo=500.0 ms
[B] core=0  periodo=500.0 ms
[C] core=0  periodo=500.1 ms
[A] core=0  periodo=500.0 ms
[B] core=0  periodo=500.0 ms
[C] core=0  periodo=500.0 ms
```

---

## 2. Parte B — Starvation

### Item 9 — HOG com prioridade 6 no core 0

**Saída observada:**

```
E (5765) task_wdt: Task watchdog got triggered. The following tasks/users did not reset the watchdog in time:
E (5765) task_wdt:  - IDLE0 (CPU 0)
E (5765) task_wdt: Tasks currently running:
E (5765) task_wdt: CPU 0: HOG
E (5765) task_wdt: CPU 1: IDLE1
```

**Análise:** Com o HOG no core 0 com prioridade 6 (a mais alta), ele **nunca bloqueia e nunca cede a CPU**. As tarefas A (prio 5), B (prio 3), C (prio 1) e a IDLE0 simplesmente nunca conseguem rodar — o escalonador sempre escolhe o HOG como a tarefa pronta de maior prioridade. Isso é **starvation**: as tarefas de menor prioridade morrem de "fome" de CPU. Após ~5 s, o watchdog (task_wdt) dispara porque a tarefa IDLE0 — que é quem reseta o watchdog — não consegue rodar.

### Item 10 — HOG com prioridade 1 (time slicing)

**Saída observada:**

```
[A] core=0  periodo=500.0 ms
[B] core=0  periodo=500.0 ms
[C] core=0  periodo=1000.2 ms
[A] core=0  periodo=500.0 ms
[B] core=0  periodo=500.1 ms
[C] core=0  periodo=999.8 ms
```

**Explicação (≤ 3 linhas):** A e B voltam ao normal porque têm prioridade superior ao HOG (prio 1) — quando acordam, preemptam o HOG imediatamente. C tem a mesma prioridade do HOG (prio 1), então o escalonador aplica *time slicing*: reveza C e HOG a cada tick (~10 ms). C passa metade do tempo cedendo a fatia ao HOG, resultando num período efetivo dobrado (~1000 ms em vez de 500 ms). O task_wdt desaparece porque a IDLE agora consegue rodar nos intervalos entre as fatias.

---

## 3. Parte C — Deriva de período

### Item 12 — Medição de 10 períodos com `vTaskDelay`

| Amostra | Período (ms) |
|---------|-------------|
| 1       | 250.2       |
| 2       | 250.1       |
| 3       | 250.3       |
| 4       | 250.0       |
| 5       | 250.2       |
| 6       | 250.1       |
| 7       | 250.3       |
| 8       | 250.0       |
| 9       | 250.2       |
| 10      | 250.1       |

**Período médio: ~250 ms** (não 200 ms), porque o `vTaskDelay` dorme 200 ms **a partir do instante em que é chamado**, mas só é chamado depois do corpo de 50 ms. Resultado: período = 50 + 200 = 250 ms — a **deriva** do Exemplo 5.1.

**Conta de ativações perdidas em 1 minuto:**
- Ideal (200 ms): 60.000 / 200 = **300 ativações**
- Real (250 ms): 60.000 / 250 = **240 ativações**
- **Perdidas: 60 ativações (20% da taxa!)**

### Tabela Comparativa (Items 12 e 13)

| Configuração                    | Período médio (ms) | Período máx (ms) |
|---------------------------------|--------------------:|------------------:|
| `vTaskDelay` + corpo 50 ms      |              ~250.0 |            ~250.5 |
| `vTaskDelayUntil` + corpo 50 ms |              ~200.0 |            ~200.2 |

**Item 13 — com `vTaskDelayUntil`:** o período estabiliza em ~200,0 ms porque `vTaskDelayUntil` calcula o sono com base no **tempo absoluto do próximo despertar**, absorvendo o corpo de 50 ms dentro do período. O delay é: 200 − 50 = 150 ms de sono efetivo, mas o período total permanece em 200 ms cravados.

---

## 4. Parte D — Pilha: high water mark

### Item 14 — Medição sem buffer extra

**Saída observada:**

```
[A] pilha livre: 384 palavras
[A] core=0  periodo=500.0 ms
[A] pilha livre: 384 palavras
[A] core=0  periodo=500.0 ms
```

- **Valor estabilizado:** ~384 palavras = **1536 bytes** livres
- **Pilha alocada:** 2048 bytes
- **Uso real (pior caso):** 2048 − 1536 = **512 bytes**

### Item 15 — Com `char buf[1500]`

**Saída observada:**

```
x[A] pilha livre: 9 palavras (36 bytes)
[A] core=0  periodo=500.0 ms
```

- **Valor com buffer:** ~9 palavras = **36 bytes** livres — praticamente encostando na borda!
- **Queda:** de 1536 para 36 bytes — consumiu ~1500 bytes adicionais (exatamente o tamanho do buffer)
- **Margem:** com apenas 36 bytes sobrando numa pilha de 2048, qualquer chamada mais profunda causaria *stack overflow*

### Recomendação de tamanho de pilha (Exemplo 5.3)

Regra do Exemplo 5.3: **uso medido + 50% de margem**.

| Cenário              | Uso medido (bytes) | Recomendação (uso × 1.5) |
|----------------------|-------------------:|-------------------------:|
| Sem buffer extra     |                512 |                  **768** |
| Com `buf[1500]`      |               2012 |                **3072** (arredondado) |

- Para a tarefa A **sem** buffer extra: pilha de **768 bytes** seria suficiente (pode arredondar para 1024 por segurança).
- Para a tarefa A **com** `buf[1500]`: pilha mínima de **3072 bytes** (3 KB).
- O tamanho original de 2048 é adequado para o caso sem buffer, mas **insuficiente** para o caso com buffer de 1500.

---

## 5. Parte E — Dual-core

### Item 16/17 — HOG no core 1

**Saída observada:**

```
[A] core=0  periodo=500.0 ms
[B] core=0  periodo=500.0 ms
[C] core=0  periodo=500.0 ms
[HOG] core=1
[A] core=0  periodo=500.0 ms
[B] core=0  periodo=500.1 ms
[C] core=0  periodo=499.9 ms
[HOG] core=1
[A] core=0  periodo=500.0 ms
[B] core=0  periodo=500.0 ms
[C] core=0  periodo=500.0 ms
[HOG] core=1
```

**Evidência:** A, B e C rodam no core 0 com períodos saudáveis (~500 ms). O HOG roda no core 1 sem interferir — nenhum task_wdt, nenhuma starvation. Os dois núcleos trabalham em **paralelo real**.

### Por que fixar cargas pesadas no core 1 é boa prática no ESP32?

O **core 0** do ESP32 já é responsável por Wi-Fi, Bluetooth e serviços internos do sistema (TCP/IP stack, event loop, etc.). Colocar tarefas CPU-bound no core 0 causa starvation nesses serviços críticos, podendo travar conectividade e watchdogs. Fixar cargas pesadas no **core 1** isola o processamento intensivo da aplicação dos serviços do sistema, garantindo que ambos operem sem competição — é a divisão de trabalho que projetos saudáveis adotam.

---

## Códigos-fonte

Os códigos de cada parte estão nos arquivos:

| Parte | Arquivo |
|-------|---------|
| A     | `main/parte_a.c` |
| B.8   | `main/parte_b_item8.c` |
| B.10  | `main/parte_b_item10.c` |
| C.11  | `main/parte_c_item11.c` |
| C.13  | `main/parte_c_item13.c` |
| D.14  | `main/parte_d.c` |
| D.15  | `main/parte_d_buf.c` |
| E     | `main/parte_e.c` |
