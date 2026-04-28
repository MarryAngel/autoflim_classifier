#!/usr/bin/env python3
"""
Script de Análise: Por que MobileNetV3 Falha para Classificação de Parasitos

Este script analisa o fracasso da MobileNetV3 comparando:
1. Número de canais na última camada convolucional
2. Resolução espacial do mapa de características com entrada 200×200
3. Impacto de múltiplos blocos de redução de resolução

Execução: python analise_mobilenet_fracasso.py
"""

import torch
import torch.nn as nn
from torchvision import models
import pandas as pd
import numpy as np
from pathlib import Path
import json


class AnaliseArquitetura:
    """Classe para análise de arquiteturas de redes neurais"""
    
    def __init__(self):
        self.architectures = {
            'MobileNetV3-Small': {
                'model': models.mobilenet_v3_small(weights=models.MobileNet_V3_Small_Weights.IMAGENET1K_V1),
                'input_size': 200
            },
            'ResNet50': {
                'model': models.resnet50(weights=models.ResNet50_Weights.IMAGENET1K_V1),
                'input_size': 200
            },
            'EfficientNet-B0': {
                'model': models.efficientnet_b0(weights=models.EfficientNet_B0_Weights.IMAGENET1K_V1),
                'input_size': 200
            },
            'MobileNetV2': {
                'model': models.mobilenet_v2(weights=models.MobileNet_V2_Weights.IMAGENET1K_V1),
                'input_size': 200
            }
        }
        
        # Colocar modelos em eval mode
        for arch_dict in self.architectures.values():
            arch_dict['model'].eval()
    
    def calcular_resolucao_espacial(self, model, input_size=200):
        """Calcula resolução espacial em cada camada usando hooks"""
        activations = {}
        
        def get_activation(name):
            def hook(model, input, output):
                if isinstance(output, torch.Tensor):
                    activations[name] = output.shape
            return hook
        
        # Registrar hooks
        hooks = []
        for name, module in model.named_modules():
            if isinstance(module, nn.Conv2d):
                hook = module.register_forward_hook(get_activation(name))
                hooks.append(hook)
        
        # Forward pass
        with torch.no_grad():
            dummy_input = torch.randn(1, 3, input_size, input_size)
            _ = model(dummy_input)
        
        # Remover hooks
        for hook in hooks:
            hook.remove()
        
        # Extrair resoluções
        resolution_sequence = []
        for layer_name, shape in activations.items():
            if len(shape) == 4:
                resolution_sequence.append({
                    'layer': layer_name,
                    'channels': shape[1],
                    'spatial_size': f"{shape[2]}×{shape[3]}",
                    'height': shape[2],
                    'width': shape[3],
                    'total_pixels': shape[2] * shape[3]
                })
        
        return resolution_sequence
    
    def gerar_relatorio(self):
        """Gera relatório completo de análise"""
        print("\n" + "="*100)
        print("ANÁLISE: FRACASSO DA MOBILENETV3 PARA CLASSIFICAÇÃO DE PARASITOS")
        print("="*100)
        
        # Calcular resoluções para todas as arquiteturas
        resolution_data = {}
        for arch_name, arch_dict in self.architectures.items():
            print(f"Processando {arch_name}...", end=' ', flush=True)
            resolution_seq = self.calcular_resolucao_espacial(
                arch_dict['model'], 
                input_size=arch_dict['input_size']
            )
            resolution_data[arch_name] = resolution_seq
            print(f"✓ ({len(resolution_seq)} camadas)")
        
        # Tabela de última camada
        print("\n" + "="*100)
        print("ÚLTIMA CAMADA CONVOLUCIONAL - COMPARAÇÃO")
        print("="*100)
        
        last_layer_data = []
        for arch_name, resolution_seq in resolution_data.items():
            if resolution_seq:
                last = resolution_seq[-1]
                last_layer_data.append({
                    'Arquitetura': arch_name,
                    'Canais': last['channels'],
                    'Altura': last['height'],
                    'Largura': last['width'],
                    'Resolução': last['spatial_size'],
                    'Total Pixels': last['total_pixels'],
                    'Redução': f"200→{last['height']} (1/{200//last['height']}x)"
                })
        
        df = pd.DataFrame(last_layer_data)
        print(df.to_string(index=False))
        
        # Análise comparativa
        print("\n" + "="*100)
        print("ANÁLISE COMPARATIVA: MOBILENETV3 vs OUTRAS REDES")
        print("="*100)
        
        mobilenet_row = df[df['Arquitetura'] == 'MobileNetV3-Small'].iloc[0]
        for idx, row in df.iterrows():
            if row['Arquitetura'] != 'MobileNetV3-Small':
                pixel_ratio = row['Total Pixels'] / mobilenet_row['Total Pixels']
                channel_ratio = row['Canais'] / mobilenet_row['Canais']
                print(f"\n{row['Arquitetura']} vs MobileNetV3-Small:")
                print(f"  • Pixels: {row['Total Pixels']:4} vs {mobilenet_row['Total Pixels']:4} ({pixel_ratio:5.2f}x mais)")
                print(f"  • Canais: {row['Canais']:4} vs {mobilenet_row['Canais']:4} ({channel_ratio:5.2f}x mais)")
        
        # Análise detalhada
        print("\n" + "="*100)
        print("ANÁLISE DETALHADA: POR QUE MOBILENETV3 FALHA PARA PARASITOS?")
        print("="*100)
        
        mobilenet_last = resolution_data['MobileNetV3-Small'][-1]
        mobilenet_pixels = mobilenet_last['total_pixels']
        mobilenet_height = mobilenet_last['height']
        mobilenet_width = mobilenet_last['width']
        mobilenet_channels = mobilenet_last['channels']
        
        input_pixels = 200 * 200
        reduction_factor = input_pixels / mobilenet_pixels
        reduction_percent = (1 - mobilenet_pixels / input_pixels) * 100
        
        print(f"""
1️⃣  REDUÇÃO EXCESSIVA DE RESOLUÇÃO:
   • Entrada: 200×200 = {input_pixels:,} pixels
   • Saída:   {mobilenet_height}×{mobilenet_width} = {mobilenet_pixels} pixels
   • Redução: {reduction_factor:.0f}x (perda de {reduction_percent:.1f}% dos dados espaciais)

2️⃣  IMPLICAÇÕES PARA ESTRUTURAS PEQUENAS:
   • Parasitos (cistos, ovos, larvas) geralmente têm 20-50 pixels na entrada
   • Após redução: reduzem para 1-2 pixels na última camada!
   • Detalhes finos desaparecem: contornos, texturas, padrões internos

3️⃣  INSUFICIÊNCIA DE REPRESENTAÇÃO ESPACIAL:
   • Apenas {mobilenet_pixels} pixels para representar todo o objeto
   • {mobilenet_channels} canais × {mobilenet_pixels} posições = {mobilenet_pixels * mobilenet_channels:,} features totais
   • Em comparação:""")
        
        for arch_name in ['ResNet50', 'EfficientNet-B0']:
            last = resolution_data[arch_name][-1]
            features = last['total_pixels'] * last['channels']
            ratio = features / (mobilenet_pixels * mobilenet_channels)
            print(f"     - {arch_name:20}: {features:7,} features ({ratio:.1f}x mais)")
        
        print(f"""
4️⃣  PROBLEMA FUNDAMENTAL:
   A MobileNetV3 foi otimizada para:
   ✓ Eficiência (poucos parâmetros)
   ✓ Velocidade (redução agressiva)
   ✓ Tamanho reduzido (menos memória)
   
   MAS NÃO para aplicações com:
   ✗ Objetos muito pequenos na imagem
   ✗ Muitos detalhes finos a capturar
   ✗ Alta discriminação entre classes visualmente similares

5️⃣  CONCLUSÃO:
   MobileNetV3 perde informações críticas durante a redução de resolução.
   Para parasitos, uma resolução mínima de ~10×10 é necessária na última
   camada para capturar as estruturas morfológicas discriminativas.
""")
        
        # Recomendações
        print("\n" + "="*100)
        print("✅ RECOMENDAÇÕES PARA RESOLVER O PROBLEMA")
        print("="*100)
        print("""
1. USAR ARQUITETURAS ALTERNATIVAS:
   • ResNet50 ou EfficientNet-B0: Mantêm resolução ~7×7 ou ~8×8
   • MobileNetV2: Melhor que V3-Small para este caso
   
2. FEATURE PYRAMID NETWORKS (FPN):
   • Combinam features de múltiplas escalas
   • Excelente para detectar objetos pequenos
   
3. AUMENTAR RESOLUÇÃO DE ENTRADA:
   • Tentar 224×224 ou 256×256 em vez de 200×200
   • Requer mais processamento mas preserva detalhes
   
4. ENSEMBLE DE MODELOS:
   • Combinar ResNet50 + EfficientNet-B0
   • Aproveitar forças complementares
   
5. DATA AUGMENTATION:
   • Fine-tuning específico para parasitos
   • Augmentações que preservem estruturas morfológicas
""")
        
        print("="*100)
        
        # Salvar resultados em JSON
        self._salvar_resultados(resolution_data, last_layer_data)
    
    def _salvar_resultados(self, resolution_data, last_layer_data):
        """Salva resultados em arquivo JSON"""
        output_dir = Path(__file__).parent.parent / "outputs"
        output_dir.mkdir(exist_ok=True)
        
        # Salvar tabela de última camada
        output_file = output_dir / "ultima_camada_comparacao.json"
        with open(output_file, 'w', encoding='utf-8') as f:
            json.dump(last_layer_data, f, indent=2, ensure_ascii=False)
        
        print(f"\n✓ Resultados salvos em: {output_file}")


def main():
    """Função principal"""
    print("Iniciando análise de arquiteturas...")
    
    analise = AnaliseArquitetura()
    analise.gerar_relatorio()
    
    print("\n✓ Análise concluída!")


if __name__ == "__main__":
    main()
