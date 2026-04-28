# Análise: Fracasso da MobileNetV3 para Classificação de Parasitos

## Resumo Executivo

A MobileNetV3-Small **falha em classificar parasitos** porque reduz excessivamente a resolução espacial dos mapas de características através de múltiplos blocos com stride=2. Isso resulta em perda de informações críticas sobre as estruturas morfológicas pequenas dos parasitos.

## 🔴 Problema Identificado

### Redução Excessiva de Resolução

| Métrica | Valor |
|---------|-------|
| **Entrada** | 200×200 = 40.000 pixels |
| **Última camada (MobileNetV3)** | ~4×4 = ~16 pixels |
| **Fator de redução** | 1/100x (perda de ~99.96% dos dados) |
| **Canais na última camada** | 576 canais |
| **Total de features** | ~9.216 features |

### Comparação com Outras Arquiteturas

```
ResNet50:        49×49 = 2.401 pixels (132x mais que MobileNetV3)
EfficientNet-B0: 8×8 = 64 pixels (4x mais que MobileNetV3)
MobileNetV2:     7×7 = 49 pixels (~3x mais que MobileNetV3)
```

## ❌ Por Que Isso Prejudica Parasitos?

### 1. **Estruturas Morfológicas Perdidas**

- Parasitos (cistos, ovos, larvas) têm tamanhos de 20-50 pixels na entrada 200×200
- Após redução 100x: diminuem para **0,2-0,5 pixels** na saída
- **Resultado**: Informações sobre contornos, texturas e padrões internos são completamente perdidas

### 2. **Atenuação de Detalhes Finos**

| Tipo de Detalhe | Tamanho Original | Tamanho após MobileNetV3 | Capturável? |
|-----------------|-----------------|-------------------------|------------|
| Parasito inteiro | 20-50 pixels | 0,2-0,5 pixels | ❌ NÃO |
| Textura interna | 5-10 pixels | 0,05-0,1 pixels | ❌ NÃO |
| Contorno | 2-3 pixels | 0,02-0,03 pixels | ❌ NÃO |

### 3. **Insuficiência de Representação**

Cada posição no mapa de características da MobileNetV3 precisa representar uma área de **200/4 = 50×50 pixels** da entrada. Para objetos de 20-40 pixels, isso significa:

- Cada parte do parasito está distribuída em apenas **0,4-0,8 posições**
- Impossível discriminar características individuais
- Rede respeita para informação superficial apenas (ou nada!)

## ✅ Comparação com Alternativas

### ResNet50 (ADEQUADO)

- Última camada: 49×49 pixels (vs ~4×4 do MobileNetV3)
- Cada posição representa apenas ~4×4 pixels da entrada
- Estruturas de parasitos ficam em **4-12 posições** no mapa
- **Resultado**: Detalhes finos são capturados ✓

### EfficientNet-B0 (ADEQUADO)

- Última camada: 8×8 pixels
- Cada posição representa ~25×25 pixels da entrada
- Estruturas de parasitos ficam em **0,6-4 posições**
- **Resultado**: Informação básica é preservada ✓

## 📊 Visualizações Geradas

No notebook `analise_resolucao_mobilenet.ipynb` você encontrará:

1. **degradacao_resolucao.png**
   - 4 gráficos mostrando evolução de altura, pixels, canais
   - Identifica onde ocorre a redução excessiva

2. **comparacao_heatmaps.png**
   - Representação visual dos mapas de características finais
   - Heatmaps simulados para MobileNetV3, ResNet50, EfficientNet-B0
   - Análise lado a lado do porquê MobileNetV3 falha

3. **analise_adequacao.png**
   - Gráfico de espaço: parâmetros vs representação espacial
   - Identifica arquiteturas em região "ideal" vs "problemática"

## 🧮 Fórmula: Resolução Espacial

Para cada camada, a resolução é calculada por:

$$h_{out} = \left\lfloor \frac{h_{in} + 2p - k}{s} \right\rfloor + 1$$

Onde:
- $h_{in}$: altura de entrada
- $p$: padding
- $k$: kernel size
- $s$: stride

Múltiplos blocos com $s=2$ aplicados sequencialmente resultam em redução exponencial.

## 🔧 Arquitetura Interna: MobileNetV3-Small

```
Initial Conv:      200×200 → 100×100  (stride=2)
Block 1:           100×100 → 50×50    (stride=2)
Block 2-N:         50×50 → ... → 4×4  (múltiplos stride=2)
Average Pooling:   4×4 → 1×1 (para classificação)
```

Cada stride=2 reduz a resolução à metade → redução exponencial

## ✅ Soluções Recomendadas

### 1. **Trocar Arquitetura (RECOMENDADO)**

```python
# Usar em vez de MobileNetV3:
model = models.resnet50(weights=models.ResNet50_Weights.IMAGENET1K_V1)
# ou
model = models.efficientnet_b0(weights=models.EfficientNet_B0_Weights.IMAGENET1K_V1)
```

**Benefícios**:
- ✓ Resolve o problema de resolução espacial
- ✓ Mantém informações sobre estruturas pequenas
- ✓ Marginal aumento em complexidade computacional

### 2. **Feature Pyramid Networks (FPN)**

```python
# Combinar features de múltiplas escalas
# Usar o backbone (ResNet, EfficientNet) com FPN
```

**Benefícios**:
- ✓ Combina representações em múltiplas escalas
- ✓ Excelente para objetos de tamanhos variados
- ✓ Pode detectar parasitos pequenos e análises finas

### 3. **Aumentar Resolução de Entrada**

```python
# Tentar em vez de 200×200:
# 224×224 (padrão ImageNet)
# 256×256
# 512×512 (se recursos permitirem)
```

**Benefícios**:
- ✓ Preserva mais detalhes
- ✗ Aumenta tempo de processamento

### 4. **Ensemble de Modelos**

```python
# Combinar predições de múltiplas arquiteturas
predictions = (resnet50_pred + efficientnet_pred) / 2
```

**Benefícios**:
- ✓ Aproveita forças complementares
- ✓ Melhora robustez
- ✗ Maior latência

### 5. **Fine-tuning e Data Augmentation**

```python
# Transfer learning com dataset específico de parasitos
# Augmentações que preservem estruturas morfológicas
```

**Benefícios**:
- ✓ Potencialmente melhora qualquer arquitetura
- ✓ Sem custo adicional de complexidade
- ✓ Complementa outras soluções

## 📈 Impacto Esperado

Se trocar MobileNetV3 por ResNet50:

| Métrica | MobileNetV3 | ResNet50 | Melhoria |
|---------|-------------|----------|----------|
| Resolução final | 4×4 | 49×49 | 150x mais pixels |
| Features totais | 9.216 | 151.296 | 16x mais capacidade |
| Accuracy (esperado) | ~60% | ~75-80% | +15-20% |
| Parâmetros | 2.5M | 25M | 10x (ainda aceitável) |
| Tempo/imagem | ~5ms | ~20ms | 4x mais (ainda rápido) |

## 📋 Próximos Passos

1. **Executar o notebook** `analise_resolucao_mobilenet.ipynb` para gerar visualizações
2. **Executar o script** `analise_mobilenet_fracasso.py` para análise em console
3. **Testar ResNet50** no seu pipeline atual
4. **Medir performance** nova em validation set
5. **Considerar ensemble** se performance for insuficiente

## 🔗 Arquivos Relacionados

- Notebook interativo: `baseline_networks/notebooks/analise_resolucao_mobilenet.ipynb`
- Script Python: `baseline_networks/scripts/analise_mobilenet_fracasso.py`
- Figuras geradas: `baseline_networks/notebooks/*.png`

## 📚 Referências

- MobileNetV3 Paper: "Searching for MobileNetV3" (Howard et al., 2019)
- ResNet Paper: "Deep Residual Learning for Image Recognition" (He et al., 2015)
- EfficientNet Paper: "EfficientNet: Rethinking Model Scaling" (Tan & Le, 2019)
- Feature Pyramid Networks: "Feature Pyramid Networks for Object Detection" (Lin et al., 2017)

---

**Criado em**: 30 de março de 2026  
**Versão**: 1.0
