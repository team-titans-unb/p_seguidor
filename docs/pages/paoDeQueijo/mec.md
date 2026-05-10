# Mecânica — Esquemático Seguidor 1 (Pão de Queijo)

Este documento detalha as especificações técnicas, escolhas de design e métricas da estrutura mecânica do robô seguidor de linha Pão de Queijo.

--- 

## Sumário

- [Metadados do esquemático mecânico](#metadados-do-esquematico-mecanico)
- [Visualização do esquemático 3D (CAD)](#visualizacao-do-esquematico-cad)
- [Chassi e Estrutura](#chassi-e-estrutura)
- [Sistema de Locomoção e Rodas](#sistema-de-locomocao-e-rodas)
- [Materiais](#materiais)
- [Processo de Fabricação](#processo-de-fabricacao)
- [Análise de Massa e Centro de Gravidade](#analise-de-massa-e-centro-de-gravidade)
- [Resumo dos parâmetros principais](#resumo-dos-parametros-principais)
- [Referências bibliográficas](#referencias-bibliograficas)
- [Histórico de versões](#historico-de-versoes)

---

## Metadados do esquemático

Os dados abaixo foram extraídos das projeções técnicas geradas a partir da modelagem 3D.

<div align="center">
  <p><b>Tabela 1:</b> Identificação do desenho (fonte: esquemático exportado)</p>
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
        <td>Robô Pão de Queijo</td>
      </tr>
      <tr>
        <td>Componente</td>
        <td><code>Chassi</code></td>
      </tr>
      <tr>
        <td>Software Utilizado</td>
        <td>Fusion 360</td>
      </tr>
      <tr>
        <td>Revisão do documento</td>
        <td>07.05.2026</td>
      </tr>
      <tr>
        <td>Sistema de Medidas</td>
        <td>Milímetros (mm)</td>
      </tr>
      <tr>
        <td>Autor</td>
        <td>Gustavo Emmanuel dos S. Rodrigues</td>
      </tr>
    </tbody>
  </table>
</div>

---

## Visualização do esquemático CAD


---

## Chassi e Estrutura

O chassi foi projetado utilizando uma abordagem de **plataforma integrada**, onde a rigidez estrutural é priorizada para suportar as forças G laterais durante curvas de alta velocidade.

### Geometria e Dimensões
* **Base Principal:** Possui uma largura transversal de **145,00 mm**, o que oferece uma ampla base de sustentação, reduzindo o risco de capotamento.
* **Extensão Frontal (Haste):** Uma haste de **70,00 mm** projeta o array de sensores à frente do eixo de tração. Este comprimento foi calculado para otimizar o tempo de reação do algoritmo de controle (PID), permitindo que o robô antecipe a curva antes que o centro de massa a atinja.
* **Design de Treliça:** A haste frontal utiliza um padrão vazado em "X" (treliça), que reduz a massa na extremidade do robô, diminuindo o momento de inércia e evitando vibrações estruturais que poderiam causar ruído na leitura dos sensores.

---

## Sistema de Locomoção e Rodas

O sistema de tração é composto por dois motores independentes posicionados lateralmente na base de 145mm.

* **Suportes de Motor:** 
  
* **Configuração de Apoio:** 

---

## Materiais

Para garantir o equilíbrio entre peso e resistência, sugerem-se os seguintes materiais para a fabricação:

| Componente | Material | Justificativa |
| :--- | :--- | :--- |
| Placa Base |  |  |
| Haste Treliçada | ||
| Pneus | |  |

---

## Processo de Fabricação

1.  **Modelagem:** Realizada no Fusion 360 com foco em parametrização.
   
2.  **Manufatura Aditiva:** 
   
3.  **Montagem:** 
---

## Análise de Massa e Centro de Gravidade

A distribuição de peso foi planejada para manter o **Centro de Gravidade (CG)** o mais baixo possível e próximo ao eixo das rodas de tração.

* **Massa Estimada:** Dependente do infill (aprox. 150g - 250g para o chassi completo).
* **Equilíbrio:** O uso da treliça frontal de 70mm garante que o peso não fique excessivamente concentrado na frente, o que poderia sobrecarregar o apoio deslizante e gerar atrito desnecessário.

---

## Resumo dos Parâmetros Principais

* **Largura de bitola:** 145,00 mm
* **Comprimento da haste de sensores:** 70,00 mm
* **Largura do suporte de componentes:** 30,00 mm

---

## Referências Bibliográficas

* Autodesk Fusion 360 Documentation.
* Regulamentos de Competição de Robótica (Seguidor de Linha).

---

## Histórico de Versões

| Versão | Descrição | Autor(es) | Data de Produção | Revisor(es) |
| :----: | --------- | --------- | :--------------: | :--------------: |
| `1.0` | Modelagem inicial e documentação básica | Felipe das Neves | 02/03/2026 | ✓ |
| `1.1` |  | Thamires Ellen | 10/05/2026 | ✓ |
