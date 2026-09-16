# Relatório do Lab 4

## 1. Parte A — latências

| Evento | Latência até a tarefa (us) |
|---|---:|
| 1 | |
| 2 | |
| 3 | |
| 4 | |
| 5 | |
| 6 | |
| 7 | |
| 8 | |
| 9 | |
| 10 | |

- Média: 
- Máximo: 
- Observação: capturar (ISR) vs processar (tarefa)

## 2. Parte B — polling × interrupção

- Total em 10 s com polling (Lab 3):
- Total em 10 s com interrupção (Lab 4):
- Cenário numérico em que polling perde eventos:

## 3. Parte C — falhas intencionalmente provocadas

### 3.1 printf dentro da ISR

Print do erro / backtrace:

```

```

Explicação:

### 3.2 Task watchdog

Mensagem do monitor:

```

```

Explicação da cadeia:

## 4. Parte D — medição de largura de pulso

### Código da ISR e da tarefa geradora

```c

```

### Leitura de 3 durações

| Leitura | Duração medida (ms) | Esperado |
|---|---:|---:|
| 1 | | 150 |
| 2 | | 150 |
| 3 | | 1200 |

## 5. Conclusão

Por que `printf` dentro da ISR é proibido e como o firmware contorna o problema:

