# REFERÊNCIA RÁPIDA: Por que MobileNetV3 Falha

## 📊 Problema em Números

```
┌─────────────────────────────────────────────────────────────────┐
│ ENTRADA: 200×200 = 40.000 pixels                               │
└─────────────────────────────────────────────────────────────────┘
                           │
                    stride=2 ConvBlock
                           │
                      100×100 = 10.000 px
                           │
                    stride=2 ConvBlock
                           │
                       50×50 = 2.500 px
                           │
                    stride=2 ConvBlock (x múltiplos)
                           │
                        ▼  ▼  ▼
┌─────────────────────────────────────────────────────────────────┐
│ SAÍDA (MobileNetV3): ~4×4 = ~16 pixels  ← 🔴 INADEQUADO        │
└─────────────────────────────────────────────────────────────────┘

REDUÇÃO: 40.000 → 16 = 100x (perda de 99.96% os dados!)
```

## 🎯 Por Que Isso Prejudica Parasitos

```
Parasita na entrada (20×20 pixels):
┌────────────────────────────┐
│  Estrutura Morfológica     │  ← Detalhes importantes
│  - Contornos              │
│  - Texturas               │
│  - Padrões internos       │
└────────────────────────────┘

Após redução 100x (cada pixel representa 50×50 da entrada):
█  ← Reduzido para ~0,16 pixels! 
   (Informação PERDIDA!)
```

## 📈 Comparação com Alternativas

```
┌──────────────────────┬──────────┬──────────────┬───────────────┐
│ Arquitetura          │ Última   │ Pixels       │ Adequado?     │
│                      │ Camada   │ (últimos)    │               │
├──────────────────────┼──────────┼──────────────┼───────────────┤
│ MobileNetV3-Small    │ 4×4      │ 16 pixels    │ ❌ NO         │
│ MobileNetV2          │ 7×7      │ 49 pixels    │ ⚠️  MARGINAL  │
│ EfficientNet-B0      │ 8×8      │ 64 pixels    │ ✓ OK          │
│ ResNet50             │ 49×49    │ 2.401 pixels │ ✓✓ EXCELENTE  │
└──────────────────────┴──────────┴──────────────┴───────────────┘
```

## 🧮 Mapeamento: Pixels Entrada → Pixels Saída

```
MobileNetV3 (4×4 saída):
Cada posição na saída representa 50×50 pixels da entrada

Parasita 20×20:
20÷50 = 0.4 posições na saída
↓
Informação PERDIDA ❌

─────────────────────────────────

ResNet50 (49×49 saída):
Cada posição na saída representa 4.08×4.08 pixels da entrada

Parasita 20×20:
20÷4.08 = ~4.9 posições na saída
↓
Detalhes PRESERVADOS ✓
```

## 🔴 Problema em 5 Pontos

```
1. REDUÇÃO EXCESSIVA
   200×200 → 4×4 (100x redução)
   
2. ESTRUTURAS PERDIDAS
   Parasitas 20-50px → 0.2-0.5px
   
3. INSUFICIÊNCIA ESPACIAL
   16 posições para objeto inteiro + variância
   
4. INCOMPATIBILIDADE COM ENTRADA
   Projetado para imagens 224×224 otimizadas
   
5. FALTA DE DETALHES FINOS
   Contornos, texturas, padrões internos perdidos
```

## ✅ Soluções (por Ordem de Efetividade)

```
1. TROCAR PARA ResNet50         ← RECOMENDADO
   • 49×49 na última camada (150x mais pixels)
   • Qualidade comprovada em visão computacional
   • Trade-off: +15ms por imagem (ainda rápido)

2. USAR EfficientNet-B0          ← BOA ALTERNATIVA
   • Balanço entre eficiência e performance
   • 8×8 na última camada (4x mais que MobileNetV3)
   • Requer menos memória que ResNet50

3. ENSEMBLE ResNet50 + EfficientNet
   • Combina forças de ambas
   • Melhor robustez
   • Trade-off: 2x latência (ambos modelos)

4. AUMENTAR RESOLUÇÃO ENTRADA
   • De 200×200 para 224×224 ou 256×256
   • Mantém proporção de dados
   • Pequeno aumento em tempo

5. DATA AUGMENTATION + FINE-TUNING
   • Aproveita dataset específico de parasitos
   • Complementa outras soluções
   • Sem custo adicional de complexidade
```

## 💻 Quick Start

```python
# ANTES (MobileNetV3 - ❌ INADEQUADO)
model = models.mobilenet_v3_small(weights=models.MobileNet_V3_Small_Weights.IMAGENET1K_V1)

# DEPOIS (ResNet50 - ✓ RECOMENDADO)
model = models.resnet50(weights=models.ResNet50_Weights.IMAGENET1K_V1)

# Ou usar o helper:
from scripts.solucoes_alternativas import TransferLearningModels
model, _ = TransferLearningModels.create_model_resnet50(num_classes=3)
```

## 📏 Estatísticas Esperadas

```
┌────────────────────┬──────────────┬──────────────┬──────────┐
│ Métrica            │ MobileNetV3  │ ResNet50     │ Ganho    │
├────────────────────┼──────────────┼──────────────┼──────────┤
│ Accuracy           │ ~60%         │ ~75-80%      │ +15-20%  │
│ Tempo/imagem       │ 5ms          │ 20ms         │ +15ms    │
│ Pixels (última)    │ 16           │ 2.401        │ 150x     │
│ Parâmetros         │ 2.5M         │ 25M          │ 10x      │
│ Memória GPU        │ 200MB        │ 500MB        │ +300MB   │
└────────────────────┴──────────────┴──────────────┴──────────┘
```

## 📚 Referências para Aprofundamento

- Notebook completo: `baseline_networks/notebooks/analise_resolucao_mobilenet.ipynb`
- Script Python: `baseline_networks/scripts/analise_mobilenet_fracasso.py`
- Soluções práticas: `baseline_networks/scripts/solucoes_alternativas.py`
- Documento completo: `baseline_networks/ANALISE_MOBILENET_FRACASSO.md`

## 🎓 Conceitos-Chave

**Por que stride=2 é agressivo para parasitos:**
- Reduz resolução à metade a cada aplicação
- Múltiplas aplicações = redução exponencial
- ~Redução 6-7 strides = 64-128x de redução total
- Para objetos pequenos, é assassino

**O que ResNet50 faz melhor:**
- Mantém mais blocos sem stride=2
- Usa residual connections para preservar features
- Mantém 49×49 vs 4×4 do MobileNetV3
- Projetado para ImageNet com objetos vários tamanhos

**Por que inputs 200×200 específicos:**
- Não é padrão (ImageNet usa 224×224)
- MobileNetV3 ainda mais reduz por não ser otimizado
- Se possível, considerar aumentar para 224×224

---

**Conclusão: Trocar MobileNetV3 por ResNet50 resolverá o problema.**
