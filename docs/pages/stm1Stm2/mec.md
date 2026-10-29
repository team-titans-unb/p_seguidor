# Mecânica — Hammer-S

Esta página reúne o modelo 3D do robô e a disposição dos componentes na estrutura.

---

## Sumário

- [Visualização do modelo 3D (CAD)](#visualizacao-do-modelo-3d-cad)
- [Disposição dos componentes](#disposicao-dos-componentes)
- [Histórico de versões](#historico-de-versoes)

---

## Visualização do modelo 3D (CAD)

<div style="position:relative;width:100%;aspect-ratio:4/3;">
  <iframe src="https://aluno8943.autodesk360.com/shares/public/SH90d2dQT28d5b602811441078144265f230?mode=embed"
          style="position:absolute;inset:0;width:100%;height:100%;border:0;"
          title="Modelo 3D do Hammer-S no Fusion 360"
          allowfullscreen webkitallowfullscreen mozallowfullscreen></iframe>
</div>

<p align="center"><b>Figura 1:</b> Modelo 3D interativo do Hammer-S (Fusion 360)</p>
<p align="center"><b>Autor do CAD:</b> Felipe das Neves</p>

!!! info ""
    Os projetos ainda estão sendo atualizados pela equipe e, talvez, possa demorar um pouco para renderizar.

!!! note "Como usar"
    Arraste com o botão esquerdo para girar, use a roda do mouse para aproximar e o botão de tela cheia no canto do visualizador para ampliar.

---

## Disposição dos componentes

A estrutura tem duas placas-base sobrepostas:

- **Placa superior (frente):** STM1, driver TB6612FNG, buzzer, módulo micro SD, chave liga/desliga e o step-down.
- **Placa traseira:** STM2, com espaço para o display TFT.
- **Base inferior:** os dois micromotores com encoder magnético, fixados por suportes próprios.
- **Receptor IR:** montado acima da estrutura, longe dos LEDs do sensor frontal.

!!! tip "Posição do receptor IR"
    O receptor deve ficar virado para cima ou para trás, fora do alcance dos emissores do QTR-8RC, para não sofrer interferência.

---

## Histórico de Versões

| Versão | Descrição | Autor(es) | Data de Produção | Revisor(es) |
| :----: | --------- | --------- | :--------------: | :--------------: |
| `1.0` | Modelo 3D e disposição dos componentes | [Felipe das Neves](https://github.com/FelipeFreire-gf) | 29/10/2026 | — |
| `1.1` | Visualizador interativo do Fusion 360 no lugar da imagem do CAD | [Felipe das Neves](https://github.com/FelipeFreire-gf) | 29/10/2026 | — |
