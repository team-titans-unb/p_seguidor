# Besourineo

Seguidor de linha com **uma STM32 Blue Pill**, usado como plataforma de testes do firmware. A placa lê o sensor frontal QTR-8RC, controla os motores pelo TB6612FNG com um controlador PD e recebe comandos de um controle remoto infravermelho. Toda a telemetria sai pela serial (FTDI).

---

## Componentes principais

| Função | Componente |
| :--- | :--- |
| Microcontrolador | STM32 Blue Pill (STM32F103C8T6, 72 MHz) |
| Sensor frontal | Pololu QTR-8RC |
| Sensores laterais | 2× QRE (lidos como digitais) |
| Driver dos motores | TB6612FNG |
| Controle remoto | Receptor IR 38 kHz + controle de 17 teclas (NEC) |
| Registro de dados | Módulo micro SD (SPI2, reservado) |
| Telemetria | Adaptador USB-serial FTDI (USART1, 115200 8N1) |

## Divisão das páginas

- [Mecânica](mec.md): modelo 3D interativo do Fusion 360.
- [Eletrônica](eletr.md): pinagem e configuração do CubeMX.
- [Controle](contro.md): funcionamento do firmware, modos e parâmetros.

---

## Histórico de Versões

| Versão | Descrição | Autor(es) | Data de Produção | Revisor(es) |
| :----: | --------- | --------- | :--------------: | :--------------: |
| `1.0` | Criação da página do robô | [Felipe das Neves](https://github.com/FelipeFreire-gf) | 29/10/2026 | — |
