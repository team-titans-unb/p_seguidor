# Documentação - Robô Pão de Queijo

Este documento descreve a infraestrutura de software, o mapeamento de hardware e a estratégia de controle implementados na versão atual do robô seguidor de linha baseado na ESP32.

> **Importante:** Esta documentação reflete o estado atual do firmware. Algumas estruturas, como o controlador PID completo e a leitura de encoders, já possuem suporte inicial no código, porém ainda não estão totalmente implementadas.

---

# Sumário

* [Mapeamento de Hardware no Controle](#mapeamento-de-hardware-no-controle)
* [Tratamento dos Sensores (QRE-8D)](#tratamento-dos-sensores-qre-8d)
* [Estratégia de Controle da Malha Principal](#estratégia-de-controle-da-malha-principal)
* [Leitura de Odometria (Encoders)](#leitura-de-odometria-encoders)
* [Arquitetura de Software e Classes](#arquitetura-de-software-e-classes)
* [Exemplo de Implementação](#exemplo-de-implementação-código-do-firmware)
* [Histórico de Versões](#histórico-de-versões)

---

# Mapeamento de Hardware no Controle

O projeto utiliza uma ESP32 como unidade principal de processamento, responsável pela leitura dos sensores, cálculo das correções de trajetória e acionamento dos motores.

## Tabela de Pinagem

| Componente         | Função Física             | Pino / Canal                      | Observação                           |
| ------------------ | ------------------------- | --------------------------------- | ------------------------------------ |
| Sensor QTR (LEDON) | Controle dos emissores IR | GPIO 19                           | Utilizado pela biblioteca QTRSensors |
| Sensores QTR       | Leitura da linha          | GPIOs 18, 5, 17, 16, 4, 0, 2 e 15 | Configuração RC                      |
| Motor Direito      | PWM                       | GPIO 32 / Canal 0                 | Controle de velocidade               |
| Motor Esquerdo     | PWM                       | GPIO 33 / Canal 1                 | Controle de velocidade               |
| Encoder Esquerdo   | Canal A/B                 | GPIOs 36 e 39                     | Estrutura definida                   |
| Encoder Direito    | Canal A/B                 | GPIOs 35 e 34                     | Estrutura definida                   |

## Configurações do Firmware

| Parâmetro                             | Valor    |
| ------------------------------------- | -------- |
| Frequência PWM (`FREQ`)               | 50000 Hz |
| Resolução PWM (`RES`)                 | 8 bits   |
| Velocidade Base (`BASE_VEL`)          | 120      |
| Quantidade de Sensores (`SENSOR_QNT`) | 8        |
| Centro da Linha (`center`)            | 2500     |
| Ganho Proporcional (`Kp`)             | 0.5      |
| Ganho Integral (`Ki`)                 | 0.0      |
| Ganho Derivativo (`Kd`)               | 0.0      |

A frequência de 50 kHz foi escolhida para reduzir ruídos audíveis e fornecer uma resposta mais suave aos motores.

---

# Tratamento dos Sensores (QRE-8D)

O robô utiliza um conjunto de oito sensores reflexivos configurados através da biblioteca `QTRSensors` em modo RC.

## Funcionamento

Cada sensor mede a refletância da superfície por meio do tempo de descarga de um circuito RC.

### Superfície Branca

* Maior reflexão da luz infravermelha;
* Descarga mais rápida;
* Leitura menor.

### Superfície Preta

* Menor reflexão da luz infravermelha;
* Descarga mais lenta;
* Leitura maior.

## Determinação da Posição da Linha

A função:

```cpp
qtr.readLineWhite(sensorValues);
```

retorna a posição estimada da linha utilizando uma média ponderada dos sensores ativos.

O valor retornado é comparado ao ponto de referência:

```cpp
center = 2500;
```

gerando o erro utilizado pelo controlador.

## Controle dos Emissores

O pino `LEDON` é utilizado pela biblioteca QTRSensors para controlar os emissores infravermelhos do conjunto de sensores.

---

# Estratégia de Controle da Malha Principal

A navegação do robô é baseada na leitura contínua da posição da linha e no cálculo de correções diferenciais aplicadas aos motores.

## Rotina de Calibração

Durante a inicialização, o robô executa uma rotina de calibração dos sensores:

```cpp
motors.setSpeeds(100, 0);

for(uint16_t i = 0; i < 100; i++){
    qtr.calibrate();
}
```

Esse procedimento permite que a biblioteca registre os valores mínimos e máximos observados pelos sensores, melhorando a robustez da leitura.

---

## Controle Proporcional (P)

Embora a estrutura do código tenha sido preparada para um controlador PID completo, a versão atual opera apenas com a componente proporcional.

O erro é calculado por:

[
erro = centro - posição
]

A correção aplicada aos motores é dada por:

[
output = K_p \times erro
]

onde:

[
K_p = 0.5
]

As componentes integral e derivativa encontram-se desabilitadas:

[
K_i = 0
]

[
K_d = 0
]

Portanto, o comportamento atual do sistema corresponde a um controlador proporcional (P).

## Aplicação da Correção

A correção calculada é aplicada diferencialmente:

```cpp
vel_R = BASE_VEL + correction;
vel_L = BASE_VEL - correction;
```

Em seguida, os valores são limitados:

```cpp
constrain(valor, 0, 255);
```

garantindo que permaneçam dentro da faixa válida do PWM.

---

## Temporização

O firmware calcula o tempo entre iterações utilizando:

```cpp
micros();
```

A variável `dt` é atualizada continuamente e foi incorporada à estrutura do controlador para futuras expansões do PID.

---

# Leitura de Odometria (Encoders)

A infraestrutura para utilização de encoders já foi definida no projeto.

## Pinagem Reservada

| Encoder  | Canal A | Canal B |
| -------- | ------- | ------- |
| Esquerdo | GPIO 36 | GPIO 39 |
| Direito  | GPIO 35 | GPIO 34 |

## Estado Atual

A variável de contagem já está presente:

```cpp
volatile long encoderCount = 0;
```

e a rotina de interrupção foi declarada:

```cpp
void IRAM_ATTR readEncoder(int encoder);
```

Entretanto, a implementação da leitura dos pulsos e do cálculo de deslocamento ainda não foi concluída nesta versão do firmware.

Portanto, nenhuma funcionalidade de odometria é utilizada atualmente pelo sistema de navegação.

---

# Arquitetura de Software e Classes

O projeto utiliza Programação Orientada a Objetos (POO) para organizar os módulos responsáveis pelo controle do robô.

## Classe Motors

A classe `Motors` encapsula as rotinas de acionamento dos motores e abstrai os detalhes de configuração do PWM da ESP32.

### Responsabilidades

* Inicialização dos canais PWM;
* Configuração dos pinos de saída;
* Aplicação das velocidades dos motores;
* Limitação dos valores enviados ao hardware.

## Métodos Utilizados

### Construtor

```cpp
Motors(
  IN1_R,
  IN1_L,
  CHN_R,
  CHN_L,
  FREQ,
  RES
);
```

Responsável pela configuração inicial do sistema de acionamento.

### Controle de Velocidade

```cpp
motors.setSpeeds(vel_R, vel_L);
```

Atualiza simultaneamente a velocidade dos motores direito e esquerdo.

---

# Exemplo de Implementação (Código do Firmware)

Trecho simplificado do loop principal:

```cpp
uint16_t pos = qtr.readLineWhite(sensorValues);

int16_t error = center - pos;

int16_t correction = pid(error);

int16_t vel_R = BASE_VEL + correction;
int16_t vel_L = BASE_VEL - correction;

vel_R = constrain(vel_R, 0, 255);
vel_L = constrain(vel_L, 0, 255);

motors.setSpeeds(vel_R, vel_L);
```

Implementação atual da função de controle:

```cpp
int16_t pid(int16_t error){
    int16_t output = Kp * error;
    return output;
}
```

Como `Ki` e `Kd` estão configurados como zero, o comportamento efetivo do sistema corresponde a um controlador proporcional.

---

# Histórico de Versões

| Versão | Descrição | Autor(es) | Data de Produção | Revisor(es) |
| :----: | --------- | --------- | :--------------: | :--------------: | 
| `1.0` | Modelagem inicial do readme | [Felipe das Neves](https://github.com/FelipeFreire-gf) | 02/03/2026 | ✓ |
| `1.1` | Modelagem e complementação de seções técnicas | [Thamires Ellen](https://github.com/thamiresellensa) | 29/05/2026 | ✓ | 
