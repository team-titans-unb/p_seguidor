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

<div align="center">
  <img src="https://raw.githubusercontent.com/team-titans-unb/p_seguidor/main/docs/pages/paoDeQueijo/dados/Seguidor1cad-superior.JPG" width="600">
  <p><b>Figura 1:</b> Vista Superior </p>
</div>


<div align="center">
  <img src="https://raw.githubusercontent.com/team-titans-unb/p_seguidor/main/docs/pages/paoDeQueijo/dados/Seguidor1cad-lateral.JPG" width="600">
  <p><b>Figura 2:</b> Vista lateral </p>
</div>



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

* **Suportes de Motor:** Integrados diretamente à estrutura do chassi principal por meio de abraçadeiras personalizadas impressas em 3D e fixadas com parafusos M3. Este design garante o alinhamento perfeito do eixo dos micromotores (padrão de engrenagem metálica 20D), eliminando folgas que poderiam prejudicar a odometria e a resposta dinâmica do robô.
  
* **Configuração de Apoio:** Configuração do tipo Differential Drive (Diferencial) com duas rodas de tração traseiras e um apoio deslizante esférico (caster wheel ou ball caster) de poliacetal (POM) na extremidade frontal, posicionado logo abaixo da haste de sensores. Essa disposição minimiza o atrito estático com a pista e mantém a distância constante entre os sensores de refletância e o solo.

---

## Materiais

Para garantir o equilíbrio entre peso e resistência, sugerem-se os seguintes materiais para a fabricação:

| Componente | Material | Justificativa |
| :--- | :--- | :--- |
| Placa Base | A definir (Alternativas: ABS ou PLA) | Escolhido devido à facilidade de fabricação por impressão 3D e boa resistência mecânica estrutural. |
| Haste Treliçada | A definir (Alternativas: PLA ou PETG) | Material leve e rígido, ideal para manter a haste de sensores firme sem adicionar peso excessivo na frente. |
| Pneus | Silicone | Materiais que oferecem propriedades elásticas e boa aderência (grip) com o solo para evitar derrapagen |

---

## Processo de Fabricação

1.  **Modelagem:** Realizada no Fusion 360 com foco em parametrização.
   
2.  **Manufatura Aditiva:** Os componentes estruturais (chassi e haste) são fabricados via FDM. Configurações recomendadas: altura de camada de 0,2 mm, preenchimento (infill) de 30% em padrão giroidal para otimizar a relação resistência/peso, e 4 paredes perimetrais para garantir fixação firme dos parafusos.
   
3.  **Montagem:** Fixação mecânica utilizando parafusos e porcas M3 de aço inoxidável auto-travantes para evitar o afrouxamento devido às vibrações de alta frequência dos motores. Os sensores e placas eletrônicas são fixados na largura de suporte dedicada de 30,00 mm por meio de espaçadores de nylon.
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
| `1.1` | Documentação inicial | Thamires Ellen | 10/05/2026 | ✓ |
