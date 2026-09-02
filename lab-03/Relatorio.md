# Relatório: Lab 3 — GPIO de entrada e saída + o experimento do bouncing

**Equipe:** Nalion Alvim Assis Filho, Hygor Cezar Dutra, Gustavo Ribeiro Passos.

## 1. Comportamento de borda vs. nível (Parte A.4)

**Pergunta:** Qual condição do código impede a repetição com o botão seguro?

**Resposta:** A condição que impede a repetição é `nivel_ant == 1 && nivel == 0`. Essa expressão lógica caracteriza a detecção de uma **borda de descida** (transição). Quando o botão é mantido pressionado, a variável `nivel` permanece em `0` e, no ciclo seguinte, `nivel_ant` também é atualizado para `0`. Como a condição exige que o estado anterior fosse obrigatoriamente `1`, o bloco `if` não é mais executado, diferenciando um "evento" (o ato de pressionar) de um "estado" (estar pressionado).

## 2. O experimento do bouncing (Parte C.11)

Abaixo estão os dados coletados ao pressionar o botão 10 vezes para diferentes janelas de debounce:

| DEBOUNCE_MS | eventos por 10 pressionadas | eventos "fantasma" |
| :--- | :--- | :--- |
| **0** | 31 | 21 |
| **5** | 29 | 19 |
| **20** | 21 | 11 |
| **50** | 21 | 11 |

**Conclusão sobre a janela ideal:** 
Os dados mostram que sem debounce (0 ms) ou com uma janela muito curta (5 ms), o ruído mecânico dos contatos do botão gera uma quantidade excessiva de leituras falsas (até 21 eventos fantasmas). Ao aumentar a janela para 20 ms, os eventos fantasmas caem quase pela metade, estabilizando nesse mesmo patamar para a janela de 50 ms. Como não houve melhoria de 20 ms para 50 ms, e tempos muito longos podem prejudicar a responsividade do sistema, a janela de **20 ms** se apresenta como o filtro mais equilibrado para o nosso hardware, embora o botão utilizado continue apresentando algum nível de duplo acionamento acidental (11 fantasmas remanescentes).

## 3. Lógica Invertida: Pull-down Externo (Parte D)

Abaixo estão as três mudanças fundamentais (*diff*) no código para a adaptação do circuito de pull-up interno para pull-down externo:

```diff
// 1. Mudança do pino (evitando o pino de boot GPIO0)
-#define BTN   GPIO_NUM_0
+#define BTN   GPIO_NUM_4

// 2. Desabilitando resistores internos e invertendo o repouso inicial
-    gpio_pullup_en(BTN);              // repouso = 1; pressionado = 0
+    gpio_pullup_dis(BTN);
+    gpio_pulldown_dis(BTN);           // repouso = 0; pressionado = 1
...
-    int nivel_ant = 1;
+    int nivel_ant = 0;

// 3. Invertendo a detecção de borda (agora borda de subida: 0 -> 1)
-        if (nivel_ant == 1 && nivel == 0 && agora >= t_ok) { 
+        if (nivel_ant == 0 && nivel == 1 && agora >= t_ok) {