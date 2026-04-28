# Libraries and configurations
import sys
from pathlib import Path
import json

# PyTorch
import torch
import torch.nn as nn
import torch.optim as optim
from torchvision import models
from torchvision import transforms
from torchinfo import summary

# Visualization and analysis
import os
import seaborn as sns
import PIL
import numpy as np
import matplotlib.pyplot as plt
from tqdm import tqdm
import pandas as pd
from sklearn.metrics import confusion_matrix, classification_report
from sklearn.metrics import cohen_kappa_score

from src.dataset import DataModuleParasite
from src import utils

# Adds the project root to the path
project_root = Path.cwd().parent
sys.path.insert(0, str(project_root))

from config import (
    get_dataset_paths
)

seed = 42
torch.manual_seed(seed)
np.random.seed(seed)

# Dicionário com os modelos disponíveis e seus pesos pré-treinados
MODEL_REGISTRY = {
    'vgg16': {
        'model_fn': models.vgg16,
        'weights': models.VGG16_Weights.IMAGENET1K_V1,
        'classifier_layer': ('classifier', 6),
        'in_features': 4096
    },
    'resnet18': {
        'model_fn': models.resnet18,
        'weights': models.ResNet18_Weights.IMAGENET1K_V1,
        'classifier_layer': ('fc', None),
        'in_features': 512
    },
    'squeezenet': {
        'model_fn': models.squeezenet1_1,
        'weights': models.SqueezeNet1_1_Weights.IMAGENET1K_V1,
        'classifier_layer': ('classifier', 1),  # Conv2d layer
        'in_features': 512
    },
    'mobilenetv3_small':{
        'model_fn':  models.mobilenet_v3_small,
        'weights': models.MobileNet_V3_Small_Weights.IMAGENET1K_V1,
        'classifier_layer': ('classifier', 3),
        'in_features': 1024       
    },
    'efficientnetb0': {
        'model_fn': models.efficientnet_b0,
        'weights': models.EfficientNet_B0_Weights.IMAGENET1K_V1,
        'classifier_layer': ('classifier', 1),
        'in_features': 1280
    },
    'efficientnetv2s': {
        'model_fn': models.efficientnet_v2_s,
        'weights': models.EfficientNet_V2_S_Weights.IMAGENET1K_V1,
        'classifier_layer': ('classifier', 1),
        'in_features': 1280
    },
    'mobilenetv2': {
        'model_fn': models.mobilenet_v2,
        'weights': models.MobileNet_V2_Weights.IMAGENET1K_V1,
        'classifier_layer': ('classifier', 1),
        'in_features': 1280
    },
    'shufflenetv2': {
        'model_fn': models.shufflenet_v2_x1_0,
        'weights': models.ShuffleNet_V2_X1_0_Weights.IMAGENET1K_V1,
        'classifier_layer': ('fc', None),
        'in_features': 1024
    }
    
}

def _replace_classifier(model, model_info, num_classes):
    """Substitui a camada de classificação do modelo."""
    layer_name, layer_idx = model_info['classifier_layer']
    in_features = model_info['in_features']
    
    if layer_idx is not None:
        if model_info['model_fn'] == models.squeezenet1_1:
            # Para squeezenet, a camada de classificação é uma Conv2d, então precisamos usar nn.Conv2d ao invés de nn.Linear
            getattr(model, layer_name)[layer_idx] = nn.Conv2d(in_features, num_classes, kernel_size=(1,1), stride=(1,1))
        else:
            # Para modelos como VGG (classifier é um Sequential)
            getattr(model, layer_name)[layer_idx] = nn.Linear(in_features, out_features=num_classes)
    else:
        # Para modelos como ResNet e ShuffleNet(fc é uma camada única)
        setattr(model, layer_name, nn.Linear(in_features, num_classes))
    
    return model

def create_model(type_model, description_path=None, config=None, pre_trained=False):
    """
    Cria uma instância do modelo especificado.
    
    Args:
        model_name: Nome do modelo ('vgg16', 'resnet50', etc.)
        description_path: Caminho para salvar a descrição do modelo
        config: Dicionário com configurações (num_classes, device, lr)
        pre_trained: Se True, carrega pesos da ImageNet
    
    Returns:
        model, criterion, optimizer, scheduler
    """
    if type_model not in MODEL_REGISTRY:
        raise ValueError(f"Modelo '{type_model}' não suportado. "
                        f"Opções: {list(MODEL_REGISTRY.keys())}")

    model_info = MODEL_REGISTRY[type_model]

    # Criar modelo com ou sem pesos pré-treinados
    if pre_trained:
        model = model_info['model_fn'](weights=model_info['weights'])
    else:
        model = model_info['model_fn'](weights=None)
    
    # Substituir camada de classificação
    model = _replace_classifier(model, model_info, config['num_classes'])
    model = model.to(config['device'])
    
    # Contar parâmetros
    total_params = sum(p.numel() for p in model.parameters())
    trainable_params = sum(p.numel() for p in model.parameters() if p.requires_grad)

    # Salvar descrição do modelo
    if description_path:
        os.makedirs(os.path.dirname(description_path), exist_ok=True)
        with open(description_path, "w") as f:
            f.write(f"Model: {type_model}\n")
            f.write(f"Pre-trained: {pre_trained}\n")
            f.write(f"Total parameters: {total_params}\n")
            f.write(f"Trainable parameters: {trainable_params}\n")
            f.write(f"Model Architecture:\n{model}\n")
    
    # Loss, Optimizer e Scheduler
    criterion = nn.CrossEntropyLoss()
    optimizer = optim.Adam(model.parameters(), lr=config['lr'])
    scheduler = optim.lr_scheduler.StepLR(optimizer, step_size=10, gamma=0.1)
    
    return model, criterion, optimizer, scheduler

