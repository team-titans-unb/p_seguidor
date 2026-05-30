# Documentação do Robô Pão de Queijo

--- 

Este documento descreve a infraestrutura de software e a estratégia de controle projetadas para o robô seguidor de linha, mapeadas diretamente com o hardware definido em `Seguidor1Base.SchDoc`.

---

## 📌 Sumário
- [Mapeamento de Hardware no Controle](#mapeamento-de-hardware-no-controle)
- [Estratégia de Controle da Malha Principal (PID)](#estrategia-de-controle-da-malha-principal-pid)
- [Leitura de Odometria (Encoders)](#leitura-de-odometria-encoders)
- [Tratamento dos Sensores (QRE-8D)](#tratamento-dos-sensores-qre-8d)
- [Estrutura do Código Base](#estrutura-do-codigo-base)

---

## 🔌 Mapeamento de Hardware no Controle

O firmware executado no **ESP32** faz a interface direta com os atuadores e sensores através da seguinte pinagem homologada:

### Atuação (Ponte H L298N Mini)
Os pinos de entrada da ponte H controlam o sentido de rotação e a velocidade via PWM. Os pinos definidos no esquemático são:
* **Motor Esquerdo (Canais de Direção):** Vinculado ao sinal **IN1** (GPIO32).
* **Motor Direito (Canais de Direção):** Vinculado ao sinal **IN3** (GPIO33).

### Entrada de Sinais (Restrição Input-Only)
Como indicado nas notas de projeto, os pinos de leitura dos encoders utilizam pinos estritamente de entrada (*Input Only*) do ESP32:
* **Encoder Motor A (Canais):** GPIO36 e GPIO39.
* **Encoder Motor B (Canais):** GPIO34 e GPIO35.

---

## 📐 Estratégia de Controle da Malha Principal (PID)

O robô opera em **malha fechada** utilizando um algoritmo de controle Proporcional, Integral e Derivativo ($PID$) para calcular a correção de trajetória baseada no erro de centralização na linha.

O sinal de correção $u(t)$ é calculado por:

$$u(t) = K_p \cdot e(t) + K_i \int_{0}^{t} e(\tau) d\tau + K_d \frac{de(t)}{dt}$$

Onde:
* $e(t)$ é o erro instantâneo gerado pela leitura da placa de sensores **QRE-8D**.
* O resultado do sinal calculado determina o diferencial de velocidade aplicado às rodas:

$$\text{Velocidade}_{\text{Esquerda}} = \text{Velocidade}_{\text{Base}} + u(t)$$
$$\text{Velocidade}_{\text{Direita}} = \text{Velocidade}_{\text{Base}} - u(t)$$

---

## 🔄 Leitura de Odometria (Encoders)

Os motores **100:1 Micro Metal Gearmotor HPCB** possuem encoders integrados de efeito Hall. Como os pinos do ESP32 utilizados (34, 35, 36, 39) não possuem resistores de *pull-up* internos acionáveis por software, o circuito físico deve garantir o nível lógico correto.

### Configuração das Interrupções
Para garantir precisão milimétrica em altas velocidades, a leitura dos encoders utiliza interrupções de hardware externas (`RISING` ou `CHANGE`), computando os pulsos em tempo real para cálculo de velocidade linear individual e odometria da pista.

---

## 👁️ Tratamento dos Sensores (QRE-8D)

O módulo frontal **QRE-8D** gera leituras analógicas ou digitais através do barramento de dados `D0–D8`. 

1. **Normalização:** Os valores brutos lidos do barramento são calibrados entre `0` (superfície totalmente clara) e `1000` (superfície totalmente escura).
2. **Cálculo de Centro de Massa (Média Ponderada):** A posição da linha sob o array de sensores é calculada pela equação:

$$\text{Posição} = \frac{\sum_{i=0}^{n} (S_i \cdot W_i)}{\sum_{i=0}^{n} S_i}$$

Onde $S_i$ representa a leitura normalizada do sensor $i$, e $W_i$ representa o peso ponderado da distância daquele sensor em relação ao centro geométrico do robô.

---

## 💻 Estrutura do Código Base (C++ / Arduino IDE)

Abaixo encontra-se a arquitetura inicial recomendada para o gerenciamento das tarefas de controle utilizando o ecossistema do ESP32:

```cpp
#include <Arduino.h>

// Definição dos Pinos de acordo com o Esquemático Base
const int PIN_IN1 = 32; // Controle Motor Esquerdo
const int PIN_IN3 = 33; // Controle Motor Direito

const int PIN_ENC_A1 = 36; // Input Only - Sem pull-up interno
const int PIN_ENC_A2 = 39; // Input Only - Sem pull-up interno
const int PIN_ENC_B1 = 34; // Input Only - Sem pull-up interno
const int PIN_ENC_B2 = 35; // Input Only - Sem pull-up interno

// Parâmetros do Controle PID
float Kp = 1.5;
float Ki = 0.0;
float Kd = 0.8;

int erroAnterior = 0;
float integral = 0;
const int velocidadeBase = 180; // Escala PWM (0-255)

void setup() {
    // Configurações de saídas para Ponte H L298N Mini
    pinMode(PIN_IN1, OUTPUT);
    pinMode(PIN_IN3, OUTPUT);

    // Configuração dos pinos Input-Only dos Encoders
    // Nota: Certifique-se de que a placa possui resistores de pull-up físicos!
    pinMode(PIN_ENC_A1, INPUT);
    pinMode(PIN_ENC_A2, INPUT);
    pinMode(PIN_ENC_B1, INPUT);
    pinMode(PIN_ENC_B2, INPUT);
}

void loop() {
    int erroAtual = obterErroPista(); // Processa dados do QRE-8D
    
    float P = erroAtual;
    integral += erroAtual;
    float D = erroAtual - erroAnterior;
    
    float controle = (Kp * P) + (Ki * integral) + (Kd * D);
    erroAnterior = erroAtual;
    
    // Cálculo do sinal diferencial para os motores
    int velEsq = velocidadeBase + controle;
    int velDir = velocidadeBase - controle;
    
    // Saturação dos sinais nos limites do PWM do ESP32
    velEsq = constrain(velEsq, 0, 255);
    velDir = constrain(velDir, 0, 255);
    
    // Comandos de escrita física nos pinos de potência
    analogWrite(PIN_IN1, velEsq);
    analogWrite(PIN_IN3, velDir);
}

int obterErroPista() {
    // Lógica para leitura do barramento D0-D8 e cálculo do erro central
    return 0; 
}


---

## Histórico de Versões

| Versão | Descrição | Autor(es) | Data de Produção | Revisor(es) |
| :----: | --------- | --------- | :--------------: | :--------------: | 
| `1.0` | Modelagem inicial do readme | [Felipe das Neves](https://github.com/FelipeFreire-gf) | 02/03/2026 | ✓ |
| `1.1` | Modelagem inicial | Thamires Ellen | 29/05/2026 | ✓ | 
