# Controle — Besourineo

Esta página descreve o firmware atual do Besourineo: como os sensores são lidos, como o controlador PD corrige a trajetória e como o robô é comandado pelo controle remoto.

!!! tip "Código-fonte"
    O firmware completo está em [`dados/main.c`](dados/main.c). Ele substitui o `Core/Src/main.c` gerado pelo STM32CubeMX; todo o código próprio fica dentro dos blocos `USER CODE`, então pode ser regerado pelo CubeMX sem perder nada.

---

## Sumário

- [Estrutura do firmware](#estrutura-do-firmware)
- [Modos de operação e teclas](#modos-de-operacao-e-teclas)
- [Leitura do sensor frontal](#leitura-do-sensor-frontal)
- [Calibração](#calibracao)
- [Controlador PD](#controlador-pd)
- [Acionamento dos motores](#acionamento-dos-motores)
- [Decodificação do controle remoto](#decodificacao-do-controle-remoto)
- [Parâmetros do firmware](#parametros-do-firmware)
- [Telemetria pela serial](#telemetria-pela-serial)
- [Procedimento de teste](#procedimento-de-teste)
- [Histórico de versões](#historico-de-versoes)

---

## Estrutura do firmware

O laço principal não usa `delay`. A cada volta ele executa três funções:

```c
while (1)
{
  tratar_ir();      /* lê a última tecla recebida e troca de modo */
  executar_modo();  /* executa uma iteração do modo atual */
  pisca_led();      /* LED da placa pisca a cada 0,5 s: o programa está vivo */
}
```

| Bloco | Funções principais |
| :--- | :--- |
| Tempo | `dwt_iniciar`, `us_desde`, `atraso_us` (contador de ciclos DWT, resolução de 1/72 µs) |
| Motores | `motor_um`, `motores` |
| Sensor frontal | `qtr_ler_bruto`, `calib_atualizar`, `qtr_normalizar`, `qtr_posicao` |
| Sensores laterais | `qre_esq`, `qre_dir` |
| Modos | `mudar_modo`, `tratar_ir`, `executar_modo` |
| Controle remoto | `HAL_GPIO_EXTI_Callback` (interrupção do PA8) |
| Serial | `_write` redireciona o `printf` para a USART1 |

---

## Modos de operação e teclas

| Tecla | Modo / ação | O que faz |
| :---: | :--- | :--- |
| **OK** | Parado | Para os motores |
| **1** | Sensores | Mostra na serial os valores brutos, normalizados, a posição da linha e os QRE |
| **2** | Teste dos motores | Motor A frente e ré, motor B frente e ré, ambos frente e ré (1 s cada) |
| **3** | Seguir linha | Liga o controlador PD (exige calibração) |
| **4** | Calibrar | Gira para os dois lados por 3 s registrando mínimo e máximo de cada sensor |
| **5** / **6** | Frente / ré | As duas rodas giram até apertar OK |
| **Setas** | Manual | Anda enquanto a tecla é segurada; para 200 ms depois de soltar |
| **\*** / **#** | Velocidade | Diminui / aumenta a velocidade base |
| **7** / **9** | Kd | Diminui / aumenta o ganho derivativo |
| **0** | Cor da linha | Alterna entre linha branca e linha preta |

---

## Leitura do sensor frontal

O QTR-8RC é lido pelo **tempo de descarga** de cada sensor:

1. Os oito pinos viram saída em nível alto por 10 µs, carregando os capacitores.
2. Os pinos viram entrada, e o firmware mede quanto tempo cada um leva para cair a nível baixo (limite de 2500 µs).
3. Superfície clara reflete mais luz e descarrega rápido (valor baixo); superfície escura descarrega devagar (valor alto).

Para trocar o modo dos oito pinos rapidamente, o firmware escreve direto no registrador `CRL` das portas A e B, em vez de chamar a HAL pino a pino.

Depois da calibração, cada leitura é convertida para a escala de 0 a 1000 (1000 = sensor sobre a linha). Com a linha branca, a escala é invertida. A posição é a média ponderada dos sensores:

```text
posição = Σ (valorᵢ × i × 1000) / Σ valorᵢ      → 0 (sensor 1) … 7000 (sensor 8), centro = 3500
```

- Valores abaixo de **50** são ignorados como ruído.
- A linha só conta como encontrada se algum sensor passar de **200**.
- Se a linha sumir, a posição fica no extremo onde ela foi vista por último (0 ou 7000), e o robô continua virando para esse lado.

---

## Calibração

A cada chamada, `calib_atualizar` faz **10 leituras** e só aceita um novo mínimo ou máximo que se repita em todas elas, como a biblioteca `QTRSensors` da Pololu. Isso descarta picos de ruído dos motores. Durante 3 s o robô alterna o sentido de giro a cada 750 ms para passar todos os sensores sobre a linha. No fim, os valores mínimo e máximo de cada sensor aparecem na serial.

!!! note "A calibração se perde ao desligar"
    Os valores ficam na RAM. É preciso calibrar (tecla 4) toda vez que o robô é ligado.

---

## Controlador PD

O erro é a distância da linha ao centro:

```text
erro     = (posição − 3500) × SENTIDO_CORRECAO
correção = (erro × Kp_x100 + (erro − erro_anterior) × Kd_x100) / 100
```

Os ganhos são inteiros multiplicados por 100, para evitar contas em ponto flutuante na Blue Pill (que não tem FPU). Os valores iniciais vêm do firmware em Arduino que já funcionava (`codDefinitivo`), convertidos da escala de PWM 0–255 para 0–3599 (fator 14,1):

| Ganho | Arduino | STM3 (`_x100`) |
| :--- | :---: | :---: |
| Kp | 0,04 | 56 |
| Kd | 1,02 | 1440 |

No primeiro ciclo depois de entrar no modo seguir, `erro_anterior` recebe o próprio erro, para o termo D não dar um tranco na partida.

### Redução de velocidade nas curvas

Quando |erro| passa de **1500**, a velocidade base cai proporcionalmente ao erro, até o mínimo de **700**. Isso dá tempo para o robô fazer curvas fechadas sem perder a linha.

---

## Acionamento dos motores

```c
esq = base + correção;
dir = base − correção;
/* limitados a 0 … 3599 */
```

Como no firmware em Arduino, no modo seguir as rodas **nunca giram para trás**: a roda de dentro da curva no máximo para. Isso evita que o robô gire no próprio eixo.

No TB6612FNG, cada motor recebe o sentido por dois pinos (IN1/IN2) e a velocidade pelo PWM do TIM4 a 20 kHz. Velocidade zero aplica freio (IN1 = IN2 = 1). As constantes `INVERTE_MOTOR_A` e `INVERTE_MOTOR_B` corrigem motores montados ao contrário.

---

## Decodificação do controle remoto

O receptor IR gera uma interrupção a cada borda de descida no PA8. O firmware mede o tempo entre bordas com o DWT e reconhece o protocolo **NEC**:

| Intervalo entre bordas | Significado |
| :--- | :--- |
| ≈ 13,5 ms | Início de quadro |
| ≈ 11,25 ms | Repetição (tecla segurada) |
| ≈ 1,125 ms | Bit 0 |
| ≈ 2,25 ms | Bit 1 |

Cada quadro tem 32 bits: endereço, endereço invertido, comando e comando invertido. O comando só é aceito se o byte invertido conferir. Toda tecla recebida aparece na serial (`IR: end=0x.. cmd=0x..`), o que serve para mapear controles diferentes: basta trocar os `#define IR_...`.

---

## Parâmetros do firmware

| Constante | Valor | Função |
| :--- | :---: | :--- |
| `PWM_MAX` | 3599 | Counter Period do TIM4 (20 kHz) |
| `VEL_BASE_INICIAL` | 850 | Velocidade base do seguidor |
| `VEL_PASSO` | 140 | Passo das teclas \* e # |
| `VEL_MIN_CURVA` | 700 | Velocidade mínima nas curvas |
| `ERRO_CURVA` | 1500 | Erro a partir do qual a velocidade cai |
| `VEL_MANUAL` | 1800 | Velocidade no controle pelas setas |
| `VEL_TESTE` | 1500 | Velocidade dos testes (teclas 2, 5 e 6) |
| `VEL_CALIBRA` | 1100 | Velocidade de giro na calibração |
| `KP_X100` / `KD_X100` | 56 / 1440 | Ganhos iniciais do PD |
| `KD_PASSO` | 70 | Passo das teclas 7 e 9 |
| `SENTIDO_CORRECAO` | 1 | Troque para −1 se o robô corrigir para o lado errado |
| `LINHA_BRANCA_INICIAL` | 1 | 1 = linha branca em fundo preto |
| `QTR_TIMEOUT_US` | 2500 | Tempo máximo de leitura de cada sensor |
| `INVERTE_MOTOR_A` / `B` | 0 / 1 | Inversão de sentido de cada motor |

---

## Telemetria pela serial

A serial usa a USART1 a **115200 bauds, 8N1**. No Linux, com o FTDI conectado:

```bash
picocom -b 115200 /dev/ttyUSB0
```

Exemplo de linha do modo sensores (tecla 1):

```text
bruto: 2500 2500  410  180  175  390 2500 2500  | norm:    0    0  820 1000  985  790    0    0  | pos=3510  | QRE esq=0 dir=0
```

---

## Procedimento de teste

1. **Rodas fora do chão, tecla 2:** confirme o sentido de cada motor e ajuste `INVERTE_MOTOR_A/B`.
2. **Tecla 1:** passe a linha sob o sensor; a posição deve ir de 0 a 7000.
3. **Robô sobre a linha, tecla 4:** calibração. Confira na serial se o máximo ficou bem acima do mínimo em todos os sensores.
4. **Tecla 1 de novo:** com a linha embaixo do lado esquerdo, a posição deve ficar abaixo de 3500. Se ficar acima, use `SENTIDO_CORRECAO −1`.
5. **Tecla 3:** seguir linha. Ajuste a velocidade com \* e # e o Kd com 7 e 9.

---

## Histórico de Versões

| Versão | Descrição | Autor(es) | Data de Produção | Revisor(es) |
| :----: | --------- | --------- | :--------------: | :--------------: |
| `1.0` | Documentação do firmware de testes com controle PD e controle IR | [Felipe das Neves](https://github.com/FelipeFreire-gf) | 29/10/2026 | — |
