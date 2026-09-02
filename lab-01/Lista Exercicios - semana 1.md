# Sistemas Embarcados – Lista de Exercícios 01
## Resolução — Semanas 1 a 3

**Bancada-07**  
**Alunos:** Nalion Alvim Assis Filho, Hygor Cezar Dutra e Gustavo Passos Ribeiro  
**Disciplina:** Sistemas Embarcados

---

# Questão 1

Um sistema embarcado é um sistema computacional dedicado, integrado a um produto maior, para executar uma função específica. Ele combina hardware e firmware e é projetado sob restrições de engenharia.

- **Tempo real:** a resposta deve ocorrer antes de um prazo. Exemplo: disparo do airbag; perder o prazo é uma falha.
- **Energia:** o consumo define a autonomia. Exemplo: sensor de umidade alimentado por bateria.
- **Custo:** o preço por unidade domina em produção em massa. Exemplo: forno de micro-ondas.
- **Memória:** código e dados precisam caber nos recursos disponíveis. Exemplo: controlador simples de máquina de lavar.
- **Confiabilidade:** não pode falhar durante a operação. Exemplo: marca-passo cardíaco.

---

# Questão 2

### (a) Controle de airbag — MCU

O microcontrolador é adequado porque o requisito dominante é tempo real rígido, com alta confiabilidade e resposta previsível em poucos milissegundos.

### (b) Roteador Wi-Fi doméstico — MPU/SoC

Um MPU/SoC suporta sistema operacional, pilha de rede, Wi-Fi, múltiplos processos e maior memória. Aqui dominam capacidade de processamento e conectividade.

### (c) Protótipo de codec de vídeo proprietário — FPGA

O FPGA permite alterar a arquitetura durante o desenvolvimento e explorar processamento paralelo, ambos importantes em codificação de vídeo.

### (d) Sensor de umidade a bateria por dois anos — MCU

O MCU pode permanecer em *deep sleep* e acordar apenas para medir e transmitir. A restrição dominante é energia.

---

# Questão 3

Dados: transmissão de 120 mA por 3 s; sono de 40 µA = 0,04 mA; período de 600 s; bateria de 1200 mAh.

\[
Q_{ativo}=120\times3=360\ \text{mA·s}
\]

\[
Q_{sono}=0,04\times597=23,88\ \text{mA·s}
\]

\[
I_{média}=\frac{360+23,88}{600}=0,6398\ \text{mA}\approx0,64\ \text{mA}
\]

\[
t=\frac{1200\ \text{mAh}}{0,6398\ \text{mA}}\approx1875,6\ \text{h}\approx78,2\ \text{dias}
\]

A transmissão domina o consumo: ela representa 360 dos 383,88 mA·s de cada ciclo. Aumentar o intervalo de transmissão reduz a parcela ativa. Para dobrar **exatamente** a autonomia, a corrente média deve cair para 0,3199 mA; mantendo os mesmos 3 s de transmissão, o período deve ser aproximadamente 1286 s, ou **21,4 min**. Usar 20 min já quase dobra a autonomia: a média cai para aproximadamente 0,340 mA e a autonomia sobe para cerca de 147 dias.

---

# Questão 4

Em tempo real **rígido** (*hard real-time*), perder o prazo significa falha do sistema; o airbag é um exemplo automotivo. Em tempo real **brando** (*soft real-time*), atrasos degradam a qualidade, sem causar falha catastrófica; a atualização da central multimídia ou do GPS é um exemplo.

O projeto de malha de luminosidade é de tempo real brando: um atraso prejudica a resposta visual, mas não cria risco de segurança.

---

# Questão 5

No fluxo requisitos → particionamento HW/SW → protótipo → validação, o Wokwi acelera o protótipo e a validação inicial: permite testar firmware, lógica de conexão e comportamento básico antes da montagem física.

Duas limitações importantes são:

1. Não representa fielmente ruído elétrico, tolerâncias de componentes, quedas de tensão e interferências.
2. Não reproduz com precisão consumo real, temporização física e fenômenos mecânicos como o *bouncing* de botões.

Por isso, a simulação reduz erros iniciais, mas não substitui a validação no hardware real.

---

# Questão 6

Na arquitetura **Von Neumann**, dados e instruções compartilham uma memória e um barramento; a CPU disputa o mesmo caminho para buscar instruções e acessar dados, criando o gargalo de Von Neumann.

Na arquitetura **Harvard**, dados e instruções usam memórias e barramentos separados. Assim, a busca de instruções pode ocorrer em paralelo ao acesso a dados, com maior vazão e previsibilidade.

MCUs adotam predominantemente Harvard ou Harvard modificada porque normalmente usam flash para código e SRAM para dados. O ESP32 usa **Harvard modificada**: os espaços são separados, mas há mecanismos para, por exemplo, ler constantes da flash como dados e executar rotinas da RAM.

---

# Questão 7

São necessárias:

\[
12\,000\ \text{instruções/iteração}\times1\,000\ \text{iterações/s}=12\times10^6\ \text{instruções/s}
\]

- ESP32 a 240 MHz, assumindo 1 instrução/ciclo:

\[
\frac{12\times10^6}{240\times10^6}=0,05=5\%\ \text{de um núcleo}
\]

- AVR a 16 MHz:

\[
\frac{12\times10^6}{16\times10^6}=0,75=75\%\ \text{da CPU}
\]

O ESP32 deixa ampla margem para RTOS, comunicação e processamento adicional. O AVR ficaria muito próximo da saturação; para PDS, o ESP32 é a escolha mais adequada.

---

# Questão 8

- **Flash:** não volátil e relativamente lenta; armazena firmware, constantes e valor inicial de dados.
- **SRAM:** volátil e rápida; armazena variáveis em execução, pilha, *heap* e seção `.data` após a inicialização.
- **Memória RTC:** pequena, de baixo consumo e capaz de sobreviver ao *deep sleep*; guarda estado que não pode ser perdido nesse modo.

Uma variável global inicializada possui seu valor inicial gravado na flash. No *boot*, o sistema copia esse valor para a SRAM, onde a variável é lida e alterada rapidamente durante a execução. Ao desligar a alimentação, a SRAM perde o conteúdo; no próximo *boot*, a cópia é refeita.

---

# Questão 9

```c
*(volatile uint32_t *)0x3FF44008 = (1u << 2);
```

O endereço é o `GPIO_OUT_W1TS_REG`. O *cast* cria um ponteiro para um registrador de 32 bits volátil; a escrita de `1u << 2` acende o GPIO2.

Os registradores `W1TS` (*write-1-to-set*) e `W1TC` (*write-1-to-clear*) alteram somente os bits escritos como 1. Portanto, não é necessário ler o registrador, modificar um bit e escrevê-lo de volta. Isso evita a condição de corrida da sequência lê-modifica-escreve.

---

# Questão 10

Cada amostra ocupa:

\[
3\ \text{eixos}\times16\ \text{bits}=48\ \text{bits}=6\ \text{bytes}
\]

Para 5 s a 1 kHz:

\[
5\times1000\times6=30\,000\ \text{bytes}\approx29,3\ \text{KiB}
\]

Cabe com folga na SRAM de aproximadamente 520 KiB do ESP32.

Para 60 s:

\[
60\times1000\times6=360\,000\ \text{bytes}\approx351,6\ \text{KiB}
\]

Embora caiba matematicamente, consome grande parte da SRAM e deixa pouca margem para pilhas do FreeRTOS, *heap*, Wi-Fi e outros dados. Uma solução é processar ou transmitir as amostras em blocos, usando *streaming*, *double buffering* ou armazenamento externo.

---

# Questão 11

Em uma ISR, uma variável pode ser alterada fora do fluxo normal do programa:

```c
volatile uint32_t pronto = 0;

void espera(void) {
    while (!pronto) { }
}
```

Sem `volatile`, o compilador pode supor que `pronto` não muda dentro do laço, carregá-la uma única vez em um registrador e transformar a espera em laço infinito. `volatile` obriga a releitura da memória a cada iteração.

Contudo, `volatile` não garante atomicidade nem exclusão mútua. Ele torna mudanças visíveis ao compilador; mutex, seção crítica ou operações atômicas protegem atualizações concorrentes, como `contador++`.

---

# Questão 12

Para um registrador de 8 bits, preservando os demais bits:

```c
reg &= ~(0b11u << 2);   // zera os bits 3–2
reg |=  (0b01u << 2);   // escreve 01 nos bits 3–2
reg ^=  (1u << 7);      // inverte o bit 7
```

As duas primeiras operações podem ser reunidas no padrão *clear-then-set*:

```c
reg = (reg & ~(0b11u << 2)) | (0b01u << 2);
```

---

# Questão 13

Para 3,3 V, LED azul com \(V_F=3,0\) V e 4 mA:

\[
R=\frac{3,3-3,0}{0,004}=75\ \Omega
\]

O resultado tem margem pequena: somente 0,3 V ficam no resistor. Se \(V_F\) subir para 3,2 V, a corrente com 75 \(\Omega\) cai para:

\[
I=\frac{3,3-3,2}{75}=1,33\ \text{mA}
\]

Logo, pequenas variações no LED alteram muito a corrente.

Para uma fonte de 5 V:

\[
R=\frac{5,0-3,0}{0,004}=500\ \Omega
\]

O LED pode ser alimentado por uma fonte de 5 V com resistor apropriado, mas **5 V nunca deve ser aplicado diretamente ao GPIO de 3,3 V** do ESP32 ou RPi. Para comandar cargas em 5 V, usa-se circuito de acionamento, como transistor/MOSFET, com terra comum quando necessário.

---

# Questão 14

*Pull-up* conecta a entrada ao VCC por um resistor; *pull-down* conecta a entrada ao GND por um resistor. Ambos evitam entrada flutuante.

Com *pull-up* interno e botão ligado ao GND:

```text
VCC ──[pull-up interno]── GPIO ── botão ── GND
```

- Botão solto: o resistor puxa o GPIO para nível alto, logo a leitura é **1**.
- Botão pressionado: o botão cria caminho de baixa resistência para o GND, logo a leitura é **0**.

O botão “vence” o resistor porque o resistor limita a corrente e oferece resistência muito maior que o caminho do botão fechado.

---

# Questão 15

Como o *bouncing* dura até 6 ms, uma solução é aceitar a mudança de estado e confirmar sua estabilidade após, por exemplo, **10 ms**:

1. Ao detectar a borda, registra-se o instante.
2. Ignoram-se novas bordas pelos 10 ms seguintes.
3. Após a janela, confirma-se o nível lido antes de considerar o evento válido.

Uma janela de 10 ms é maior que os 6 ms previstos e oferece margem para ruído. O compromisso é acrescentar até cerca de 10 ms de latência, normalmente imperceptível para interação humana, em troca de evitar múltiplos acionamentos falsos.

---

# Questão 16

O *pipeline* divide a execução de uma instrução em estágios e sobrepõe instruções diferentes: enquanto uma é executada, outra pode ser decodificada e uma terceira buscada. Assim, após o enchimento do pipeline, a CPU pode concluir aproximadamente uma instrução por ciclo.

O cache mantém cópias de dados e instruções usados com frequência perto da CPU, reduzindo acessos à RAM, que é muito mais lenta. No Cortex-A53 do Raspberry Pi, pipeline, cache e execução mais avançada fazem o desempenho real superar o que uma comparação simples de clocks sugere.

Um MCU simples pode ser mais previsível em tempo real porque possui arquitetura e memória menos complexas. Um *cache miss*, por exemplo, pode acrescentar atraso variável; interrupções também perturbam a sequência temporal. Menos otimizações dinâmicas não significam maior desempenho, mas podem significar menor variação no tempo de resposta.
