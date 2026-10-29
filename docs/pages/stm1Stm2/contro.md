# Controle — Hammer-S

Esta página descreve como o trabalho é dividido entre as duas placas e como elas se comunicam.

!!! warning "Em desenvolvimento"
    O firmware deste robô ainda está sendo escrito. A lógica de seguir linha (leitura do QTR-8RC, controle PD e comandos pelo controle remoto) já existe no firmware do [Besourineo](../stm3/contro.md) e será portada para a STM1, adaptada à pinagem desta placa.

---

## Sumário

- [Divisão de responsabilidades](#divisao-de-responsabilidades)
- [Periféricos usados na STM1](#perifericos-usados-na-stm1)
- [Periféricos usados na STM2](#perifericos-usados-na-stm2)
- [Comunicação entre as placas](#comunicacao-entre-as-placas)
- [Diferenças para o firmware do Besourineo](#diferencas-para-o-firmware-do-besourineo)
- [Histórico de versões](#historico-de-versoes)

---

## Divisão de responsabilidades

| Placa | Responsabilidade |
| :--- | :--- |
| **STM1** | Malha de controle: leitura do QTR-8RC, cálculo do erro, controle dos motores, contagem dos encoders, marcas laterais (QRE), comandos IR, buzzer e registro no micro SD |
| **STM2** | Interface: mostrar no display o modo atual, a velocidade, os ganhos e a telemetria recebida da STM1 |

Separar as funções evita que o desenho no display, que é lento, atrase a malha de controle.

---

## Periféricos usados na STM1

| Periférico | Uso |
| :--- | :--- |
| TIM4 (CH1, CH2) | PWM dos motores, 20 kHz (Prescaler 0, Counter Period 3599) |
| TIM2 (CH1, CH2) | Encoder esquerdo, modo encoder TI1 e TI2 |
| TIM3 (CH1, CH2) | Encoder direito, modo encoder TI1 e TI2 |
| EXTI8 | Receptor IR, decodificação do protocolo NEC |
| SPI2 | Micro SD (iniciar com prescaler 128, ≤ 400 kHz) |
| USART1 | Comunicação com a STM2 |
| DWT | Contador de ciclos para medir o tempo de descarga do QTR-8RC |

## Periféricos usados na STM2

| Periférico | Uso |
| :--- | :--- |
| SPI1 | Display ST7789 (Transmit Only Master) |
| TIM3 (CH3) | PWM do brilho do display (BLK) |
| USART1 | Comunicação com a STM1 |
| USART2 | Serial de depuração no FTDI |

---

## Comunicação entre as placas

- **Interface:** USART1 nas duas placas, 8N1, com TX e RX cruzados.
- **Sentido STM1 → STM2:** telemetria periódica (modo, posição da linha, velocidades, ganhos).
- **Sentido STM2 → STM1:** comandos vindos da interface, se houver.

!!! note "Protocolo"
    O formato das mensagens ainda será definido. Uma opção simples é um quadro com byte de início, tipo da mensagem, dados e checksum, enviado por DMA na STM1 para não travar a malha de controle.

---

## Diferenças para o firmware do Besourineo

| Item | Besourineo | STM1 deste robô |
| :--- | :--- | :--- |
| Sensores laterais | PB4 / PB5 | PA11 / PA0 |
| PA9 / PA10 | FTDI | Comunicação com a STM2 |
| Encoders | Não tem | TIM2 e TIM3 |
| Buzzer | Não tem | PC14 |
| Motor esquerdo | Canal A do TB6612 | Canal B do TB6612 |

---

## Histórico de Versões

| Versão | Descrição | Autor(es) | Data de Produção | Revisor(es) |
| :----: | --------- | --------- | :--------------: | :--------------: |
| `1.0` | Arquitetura de controle com duas placas | [Felipe das Neves](https://github.com/FelipeFreire-gf) | 29/10/2026 | — |
