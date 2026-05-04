# Eletrônica — Esquemático Base Seguidor 1 (Pão de Queijo)

## Sumário

- [Metadados do esquemático](#metadados-do-esquematico)
- [Visualização do esquemático](#visualizacao-do-esquematico)
- [Alimentação e regulação](#alimentacao-e-regulacao)
- [MCU (ESP32) e pinagem relevante](#mcu-esp32-e-pinagem-relevante)
- [Motores, ponte H e encoders](#motores-ponte-h-e-encoders)
- [Sensores e E/S](#sensores-e-es)
- [Observações de projeto (notas da folha)](#observacoes-de-projeto-notas-da-folha)
- [Resumo dos parâmetros principais](#resumo-dos-parametros-principais)
- [Referências bibliográficas](#referencias-bibliograficas)
- [Histórico de versões](#historico-de-versoes)

---

## Metadados do esquemático

Os dados abaixo foram extraídos da exportação do esquemático (**Seguidor1Base-2.jpg**) e do projeto **Seguidor1Base** no Altium (documento **Seguidor1Base-1.pdf**, quando aplicável).

<font size="3">
    <p style="text-align: center">
        <b>Tabela 1:</b> Identificação do desenho (fonte: esquemático exportado)
    </p>
</font>

<div align="center">
  <table>
    <thead>
      <tr>
        <th>Campo</th>
        <th>Valor indicado no desenho</th>
      </tr>
    </thead>
    <tbody>
      <tr>
        <td>Título</td>
        <td>Esquemático Base Seguidor 1</td>
      </tr>
      <tr>
        <td>Arquivo de esquemático</td>
        <td><code>Seguidor1Base.SchDoc</code></td>
      </tr>
      <tr>
        <td>Folha</td>
        <td>1 de 1</td>
      </tr>
      <tr>
        <td>Revisão do documento</td>
        <td>26.04.2026</td>
      </tr>
      <tr>
        <td>Autor (campo APPROVALS / AUTHOR)</td>
        <td>Henrique de O. Bernardes</td>
      </tr>
      <tr>
        <td>Formato / escala (legenda)</td>
        <td>A3 — escala 1:2 (campos SIZE / SCALE da borda)</td>
      </tr>
      <tr>
        <td>Propriedade / distribuição</td>
        <td>Documento Altium — texto padrão de propriedade na borda da folha</td>
      </tr>
    </tbody>
  </table>
</div>

!!! tip "Atenção"
    A figura interativa desta página usa **`dados/Seguidor1Base-2.jpg`**. O PDF do projeto, encontra-se em `docs/pages/paoDeQueijo/dados/Seguidor1Base-1.pdf`.

---

## Visualização do esquemático

<font size="3">
    <p style="text-align: center">
        <b>Figura 1:</b> Esquemático robô pão de queijo
        <br>
    </p>
</font>

<div align="center">
  <img src="blob:null/d6bfdd02-e5e5-4553-a657-35a18675c6e8" width="600">
</div>

<font size="3">
    <p style="text-align: center">
       
   
</font>

---

## Alimentação e regulação

A folha descreve a cadeia de tensões e os reguladores associados ao barramento do robô.

### Tensões e blocos citados no PDF

| Tensão / bloco | Observação (extraído do esquemático) |
| :--- | :--- |
| **+7 V** | Entrada associada ao bloco **REGULADOR 7V-5V** |
| **+5 V** | Barramento **+5V** — alimentação de periféricos e etapa de potência (ex.: ponte H / motores conforme folha) |
| **+3V3** | **+3V3** — típico para lógica do ESP32 e circuitos de 3,3 V |
| **MC7805** | Regulador linear **MC7805** (série 78xx, 5 V) com capacitores de entrada/saída indicados na folha |

### Capacitores e filtragem (valores explícitos no texto do PDF)

| Valor | Contexto típico na folha |
| :--- | :--- |
| **10 µF** | Desacoplamento / bulk próximo aos reguladores |
| **100 nF** | Capacitores **NF** (notação “NF” na folha — uso de desacoplamento de alta frequência) |
| **0,33 µF** | Associado à etapa do regulador (valor citado junto ao símbolo na folha) |

??? info "Detalhes — pinagem de alimentação no conector do ESP32 (trecho da folha)"

    No desenho aparecem, entre outros, os pinos lógicos do módulo **MCU - ESP32**: **3V3**, **EN**, **GND**, **5V0** e diversos **GPIO** listados na borda do símbolo (ex.: GPIO36, GPIO39, GPIO34, GPIO35, GPIO32, GPIO33, GPIO25, GPIO26, GPIO27, GPIO14, GPIO12, GPIO13, GPIO9, GPIO10, GPIO11, etc.).

---

## MCU (ESP32) e pinagem relevante

O núcleo do controle é o **MCU - ESP32**. A seguir, os **pontos mais importantes** mapeados a partir do texto do PDF (nomes de rede / rótulos na folha).

### Encoders (quatro fios por eixo)

| Função na folha | GPIO (ESP32) |
| :--- | :--- |
| Encoder — canal associado a **GPIO36** | **GPIO36_ENCODER** |
| Encoder — canal associado a **GPIO39** | **GPIO39_ENCODER** |
| Encoder — canal associado a **GPIO34** | **GPIO34_ENCODER** |
| Encoder — canal associado a **GPIO35** | **GPIO35_ENCODER** |

### Entradas digitais nomeadas na folha

| Sinal na folha | GPIO |
| :--- | :--- |
| **IN1** | **GPIO32_IN1** (GPIO32) |
| **IN3** | **GPIO33_IN3** (GPIO33) |

### Observação de projeto (registrada no próprio PDF)

> **O GND DEVE SER COMPARTILHADO ENTRE A ENTRADA E SAÍDA COMO ESPECIFICADO PELA FABRICANTE.**

Esta nota aparece explicitamente na folha, em geral associada ao **regulador** / exigência do fabricante do CI — deve ser respeitada no layout e na montagem.

---

## Motores, ponte H e encoders

### Ponte H

| Item | Valor / designação no PDF |
| :--- | :--- |
| CI ponte H | **L298N Mini** |
| Ramificação do desenho | Bloco **PONTE-H** / **MOTORES** alimentado em **+5V** (conforme rótulos da folha) |

### Motores

| Lado | Motor citado |
| :--- | :--- |
| Motor esquerdo / direito | **100:1 Micro Metal Gearmotor HPCB** (aparece nas duas metades do desenho) |

### Encoder (parâmetros elétricos citados na folha)

| Parâmetro | Valor no PDF |
| :--- | :--- |
| Alimentação do encoder (**Encoder Vcc**) | **2,7 V a 18 V** |
| Saídas | **Encoder channel A output**, **Encoder channel B output** |
| Referência de terra | **Encoder GND** |

### Potência dos motores (rótulos do conector na folha)

- **Motor power M1 (“+” terminal)**  
- **Motor power M2**

### Nota de EMI (texto da folha)

> Os capacitores deveriam ser adicionados o mais próximo possível aos polos dos motores para redução de **EMI**.

---

## Sensores e E/S

### Sensor de refletância frontal

| Item | Designação no PDF |
| :--- | :--- |
| Módulo | **QRE-8D** |
| Descrição na folha | **SENSOR REFLETÂNCIA FRONTAL** |
| Alimentação do bloco | **VCC** / **GND** (3,3 V no desenho, conforme rótulo **+3V3** próximo ao símbolo) |

### LEDs e barramento digital (trecho D0–D8)

A folha associa linhas nomeadas **D8 … D0**, **LEDON** e conexões a pinos do ESP32 (ex.: uso de prefixos **NLGPIO\*** em várias redes — interpretação: nets nomeadas para roteamento / lógica no projeto Altium).

---

## Observações de projeto (notas da folha)

1. **Terra comum do regulador:** compartilhar GND entre entrada e saída conforme fabricante (texto integral na folha).  
2. **EMI em motores:** posicionar capacitores o mais próximo possível dos terminais dos motores.  
3. **GPIO “input only”:** na folha há região marcada como **INPUT ONLY** para determinados pinos do ESP32 — respeitar restrições do chip (pinos apenas de entrada no ESP32).  
4. **Revisão e folhas:** o arquivo analisado é a **folha 1 de 3**; blocos como **MC7805**, **L298N**, sensores e motores podem continuar nas demais folhas do mesmo `SchDoc`.

---

## Resumo dos parâmetros principais

<font size="3">
    <p style="text-align: center">
        <b>Tabela 2:</b> Parâmetros elétricos e funcionais destacados (fonte: esquemático — Seguidor1Base-2.jpg)
    </p>
</font>

<div align="center">
  <table>
    <thead>
      <tr>
        <th>Domínio</th>
        <th>Parâmetro</th>
        <th>Valor / atribuição</th>
      </tr>
    </thead>
    <tbody>
      <tr>
        <td>Alimentação</td>
        <td>Réguas</td>
        <td>+7 V → regulação; +5 V; +3,3 V</td>
      </tr>
      <tr>
        <td>Regulador 5 V</td>
        <td>CI</td>
        <td>MC7805 + capacitores (10 µF, 100 nF, 0,33 µF conforme folha)</td>
      </tr>
      <tr>
        <td>MCU</td>
        <td>Modelo</td>
        <td>ESP32 (rótulo “MCU - ESP32”)</td>
      </tr>
      <tr>
        <td>Encoder</td>
        <td>Faixa de Vcc</td>
        <td>2,7 V a 18 V</td>
      </tr>
      <tr>
        <td>Motores</td>
        <td>Tipo</td>
        <td>100:1 Micro Metal Gearmotor HPCB</td>
      </tr>
      <tr>
        <td>Driver</td>
        <td>Ponte H</td>
        <td>L298N Mini</td>
      </tr>
      <tr>
        <td>Linha</td>
        <td>Sensor frontal</td>
        <td>QRE-8D</td>
      </tr>
      <tr>
        <td>GPIO</td>
        <td>Encoders</td>
        <td>36, 39, 34, 35</td>
      </tr>
      <tr>
        <td>GPIO</td>
        <td>Entradas IN1 / IN3</td>
        <td>32, 33</td>
      </tr>
    </tbody>
  </table>
</div>

??? info "Lista bruta de GPIO citados na borda do símbolo ESP32 (PDF)"

    Na ordem de aparição do texto extraído do PDF, a folha lista pinos entre outros: **GPIO36, GPIO39, GPIO34, GPIO35, GPIO32, GPIO33, GPIO25, GPIO26, GPIO27, GPIO14, GPIO12, GPIO13, GPIO9, GPIO10, GPIO11**, além de **GPIO23, GPIO22, GPIO1, GPIO3, GPIO21, GPIO19, GPIO18, GPIO5, GPIO17, GPIO16, GPIO4, GPIO0, GPIO2, GPIO15**, redes nomeadas com prefixo **NLGPIO** (convenção de *net label* no Altium), barramento **SCK**, **D0/D1**, etc. A validação final de conexão deve ser feita **no próprio esquemático** (nets e *designators* de componente).

---

## Referências bibliográficas

> [1] Projeto Altium **Seguidor1Base** — esquemático **Esquemático Base Seguidor 1**, autor **Henrique de O. Bernardes**. Exportação usada na documentação: **`docs/pages/paoDeQueijo/dados/Seguidor1Base-2.jpg`**; PDF de referência: **`docs/pages/paoDeQueijo/dados/Seguidor1Base-1.pdf`**.

> [2] Espressif Systems. **ESP32 Series Datasheet** — pinout, limites de tensão por GPIO e notas de hardware. Consultar a versão aplicável ao módulo usado na montagem.

> [3] STMicroelectronics / datasheet do **L298** (família) — correntes máximas, dissipation e recomendações de **GND comum** e diodos de roda livre quando aplicável ao **L298N Mini** montado no projeto.

---

## Histórico de versões

| Versão | Descrição | Autor(es) | Data de produção | Revisor(es) | Observações |
| :----: | --------- | --------- | :--------------: | :-----------: | ----------- |
| `1.0` | Modelagem inicial do readme | [Felipe das Neves](https://github.com/FelipeFreire-gf) | 02/03/2026 | — | Conteúdo inicial curto. |
| `1.1` | Página modelada (sumário, tabelas) e parâmetros extraídos do esquemático; visualização com **Seguidor1Base-2.jpg** em `dados/` | [Felipe das Neves](https://github.com/FelipeFreire-gf) | 04/05/2026 | — | Imagem do esquemático no lugar do PDF embutido. |
