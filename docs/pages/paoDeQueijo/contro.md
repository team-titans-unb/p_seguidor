#  Documentação - Robô Pão de Queijo

Este documento descreve a infraestrutura de software e a estratégia de controle projetadas para o robô seguidor de linha.

---

##  Sumário
- [Mapeamento de Hardware no Controle](#mapeamento-de-hardware-no-controle)
- [Estratégia de Controle da Malha Principal (PID)](#estratégia-de-controle-da-malha-principal-pid)
- [Leitura de Odometria (Encoders)](#leitura-de-odometria-encoders)
- [Tratamento dos Sensores (QRE-8D)](#tratamento-dos-sensores-qre-8d)
- [Arquitetura de Software e Classes](#arquitetura-de-software-e-classes)
- [Exemplo de Implementação](#-exemplo-de-implementação)
- [Histórico de Versões](#histórico-de-versões)

---

## Mapeamento de Hardware no Controle

O mapeamento de pinos e as definições de software abaixo garantem o acoplamento correto com os recursos de hardware da ESP32.

### Tabela de Pinagem

| Componente | Função | Pino / Canal |
| :--- | :--- | :--- |
| **Sensor QTR (LEDON)** | Controle do Emissor LED | Pino `19` |
| **Sensores QTR (Array)** | Leitura de Linha (8 sensores) | Pinos: `18, 5, 17, 16, 4, 0, 2, 15` |
| **Motor Direito** | Controle de Direção / PWM | Pino `32` / Canal `0` |
| **Motor Esquerdo** | Controle de Direção / PWM | Pino `33` / Canal `1` |
| **Encoder Esquerdo** | Canal A / Canal B | Pino `36` / Pino `39` |
| **Encoder Direito** | Canal A / Canal B | Pino `35` / Pino `34` |

### Configurações de Software (`SPECS`)

* **Frequência do PWM:** 50 kHz (`FREQ 50000`) — Garante operação silenciosa dos motores, acima da faixa audível humana.
* **Resolução do PWM:** 8 bits (`RES 8`) — Permite o controle de velocidade em uma escala de 0 a 255.
* **Velocidade Base:** 120 (`BASE_VEL`) — Velocidade nominal de cruzeiro do robô em trechos retos.
* **Quantidade de Sensores:** 8 (`SENSOR_QNT`)
* **Centro do Sensor (Setpoint):** `2500` — Valor de referência para manter o robô centralizado na linha.

---

## Estratégia de Controle da Malha Principal (PID)

O firmware implementa três pilares essenciais para assegurar precisão e estabilidade dinâmica durante a navegação na pista:

### 1. Rotina de Calibração (`calibrationRoutine`)
Ao iniciar, o robô rotaciona levemente para a direita executando 100 leituras consecutivas com a biblioteca `QTRSensors`. Esse processo mapeia os limiares de máxima e mínima refletância do ambiente (calibração de preto e branco), adaptando o robô às variações de luz da pista.

### 2. Algoritmo de Controle PID (`pid`)
Para corrigir os desvios de trajetória, o sistema calcula o erro em relação ao centro da linha e aplica correções matemáticas baseadas em tempo real ($dt$):
* **P (Proporcional):** Fornece uma correção diretamente proporcional ao erro atual.
* **I (Integral):** Acumula o erro ao longo do tempo para eliminar desvios residuais de alinhamento.
* **D (Derivativo):** Avalia a velocidade de variação do erro para amortecer a resposta física e mitigar oscilações abruptas.

A equação matemática aplicada no código é:

$$Output = (Kp \times P) + (Ki \times I) + (Kd \times D)$$

### 3. Temporização Dinâmica
O loop de controle monitora o tempo de execução através da função `micros()`. O cálculo de tempo decorrido ($dt$) assegura que os ganhos Integral e Derivativo sejam independentes de variações no tempo de processamento das tarefas da CPU.

---

## Leitura de Odometria (Encoders)

O robô está equipado com encoders de quadratura mapeados nos pinos de interrupção da ESP32. 
* As leituras utilizam sub-rotinas de interrupção rápida (**ISR** - *Interrupt Service Routines*) marcadas com o atributo `IRAM_ATTR` para garantir execução imediata diretamente na memória RAM do chip.
* Esta infraestrutura foi projetada para calcular a distância percorrida e a velocidade real de cada roda, permitindo futuras implementações de controle de velocidade em malha fechada e mapeamento de curvas.

---

## Tratamento dos Sensores (QRE-8D)

A leitura da linha é feita por um array reflexivo compatível com o módulo **QRE-8D** (8 sensores de leitura por descarga de capacitor RC).
* O método `qtr.readLineWhite(sensorValues)` faz a leitura analógica/digital dos 8 canais em paralelo e calcula a posição estimada do robô sobre uma linha branca usando uma média ponderada.
* O pino `LEDON` (`19`) atua controlando dinamicamente os emissores infravermelhos, permitindo desligá-los quando o robô estiver ocioso para economizar energia do conjunto de baterias.

---

## Arquitetura de Software e Classes

O projeto adota práticas de Programação Orientada a Objetos (POO) para isolar o acoplamento do código físico. A complexidade de baixo nível do hardware fica encapsulada de forma protegida dentro da classe `Motors`.

###  Abstração dos Motores (`Motors` Class)

A classe utiliza a API moderna do ecossistema ESP32 (`ledcAttachChannel`), vinculando de maneira nativa os pinos do driver à estrutura do periférico de hardware LEDC do chip.

#### Métodos da Classe (API)

| Método | Tipo de Retorno | Parâmetros | Descrição |
| :--- | :---: | :--- | :--- |
| **`Motors`** | Construtor | `int`, `int`, `int`, `int`, `int`, `int` | Instancia a classe configurando pinos, canais, frequência e resolução. Invoca o método interno `config()`. |
| **`setRightSpeed`** | `void` | `int velocity` | Atualiza o ciclo de trabalho (Duty Cycle) do motor direito via `ledcWrite`. |
| **`setLeftSpeed`** | `void` | `int velocity` | Atualiza o ciclo de trabalho (Duty Cycle) do motor esquerdo via `ledcWrite`. |
| **`setSpeeds`** | `void` | `int velocity_R`, `int velocity_L` | Atualiza a velocidade de ambos os motores ao mesmo tempo. |

---

##  Exemplo de Implementação

Abaixo encontra-se a estrutura básica do loop principal do firmware:

```cpp
#include <Arduino.h>
#include <motors.h>
#include <QTRSensors.h>

// Instanciação dos periféricos
QTRSensors qtr;
Motors motors(IN1_R, IN1_L, CHN_R, CHN_L, FREQ, RES);

uint16_t sensorValues[8];
int center = 2500;
int16_t BASE_VEL = 120;

void setup() {
  motors.setSpeeds(0, 0); // Inicia parado
  
  qtr.setTypeRC();
  qtr.setSensorPins((const uint8_t[]){18, 5, 17, 16, 4, 0, 2, 15}, 8);
  
  Serial.begin(115200);
  delay(500);
  
  calibrationRoutine(); // Executa calibração em pista
}

void loop() {
  // Executa o cálculo do PID e atualiza motores continuamente
  uint16_t pos = qtr.readLineWhite(sensorValues);
  int16_t error = center - pos;
  int16_t correction = pid(error);
  
  int16_t vel_R = constrain(BASE_VEL + correction, 0, 255);
  int16_t vel_L = constrain(BASE_VEL - correction, 0, 255);
  
  motors.setSpeeds(vel_R, vel_L);
}
```
## Histórico de Versões

| Versão | Descrição | Autor(es) | Data de Produção | Revisor(es) |
| :----: | --------- | --------- | :--------------: | :--------------: | 
| `1.0` | Modelagem inicial do readme | [Felipe das Neves](https://github.com/FelipeFreire-gf) | 02/03/2026 | ✓ |
| `1.1` | Modelagem e complementação de seções técnicas | Thamires Ellen | 29/05/2026 | ✓ | 
