"""
Model creation and management module for parasite classification.

This module provides utilities for creating and configuring pre-trained
neural network models with custom classification heads.
"""

import os
from pathlib import Path

import torch
import torch.nn as nn
import torch.optim as optim
from torchvision import models


# ==================== MODEL REGISTRY ====================
"""
Registry of supported neural network architectures.

Each entry contains:
- model_fn: torchvision model constructor function
- weights: pre-trained weights from ImageNet1K
- classifier_layer: tuple of (module_name, layer_index)
- in_features: input features to the classification layer
"""
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
    'mobilenetv3_small': {
        'model_fn': models.mobilenet_v3_small,
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


# ==================== HELPER FUNCTIONS ====================

def _replace_classifier(model: nn.Module, model_info: dict, num_classes: int) -> nn.Module:
    """
    Replace the classification head of a pre-trained model.

    This function modifies the final layer of a neural network to output
    predictions for a custom number of classes instead of ImageNet's 1000.

    Parameters
    ----------
    model : nn.Module
        Pre-trained PyTorch model.
    model_info : dict
        Model metadata from MODEL_REGISTRY containing:
        - classifier_layer: (module_name, layer_index)
        - in_features: input dimension to the classifier
        - model_fn: original model constructor function
    num_classes : int
        Number of output classes for the new task.

    Returns
    -------
    nn.Module
        Model with modified classification head.

    Notes
    -----
    Handles different classifier architectures:
    - Sequential modules (VGG): modify specific layer by index
    - SqueezeNet: uses Conv2d instead of Linear for classification
    - Single layer (ResNet, ShuffleNet): replace entire layer
    """
    layer_name, layer_idx = model_info['classifier_layer']
    in_features = model_info['in_features']

    if layer_idx is not None:
        # Handle Sequential modules (VGG, MobileNet, etc.)
        if model_info['model_fn'] == models.squeezenet1_1:
            # SqueezeNet uses Conv2d for classification
            getattr(model, layer_name)[layer_idx] = nn.Conv2d(
                in_features, num_classes, kernel_size=(1, 1), stride=(1, 1)
            )
        else:
            # Other models with sequential classifiers (VGG)
            getattr(model, layer_name)[layer_idx] = nn.Linear(
                in_features, out_features=num_classes
            )
    else:
        # Handle single Linear layer (ResNet, ShuffleNet, etc.)
        setattr(model, layer_name, nn.Linear(in_features, num_classes))

    return model


def _save_model_description(
    model: nn.Module,
    type_model: str,
    pre_trained: bool,
    description_path: str
) -> None:
    """
    Save model architecture and parameter information to a file.

    Parameters
    ----------
    model : nn.Module
        PyTorch model to describe.
    type_model : str
        Model architecture name.
    pre_trained : bool
        Whether pre-trained weights were used.
    description_path : str
        Full path where to save the description file.
    """
    os.makedirs(os.path.dirname(description_path), exist_ok=True)

    total_params = sum(p.numel() for p in model.parameters())
    trainable_params = sum(p.numel() for p in model.parameters() if p.requires_grad)

    with open(description_path, "w") as f:
        f.write(f"Model: {type_model}\n")
        f.write(f"Pre-trained: {pre_trained}\n")
        f.write(f"Total parameters: {total_params:,}\n")
        f.write(f"Trainable parameters: {trainable_params:,}\n")
        f.write(f"\nModel Architecture:\n")
        f.write(f"{model}\n")


# ==================== MAIN FUNCTION ====================

def create_model(
    type_model: str,
    description_path: str = None,
    config: dict = None,
    pre_trained: bool = False
) -> tuple:
    """
    Create a neural network model with custom classification head.

    Loads a pre-trained architecture from torchvision (optionally), replaces
    the classification layer for the target number of classes, and prepares
    training utilities (criterion, optimizer, scheduler).

    Parameters
    ----------
    type_model : str
        Architecture name. Must be a key in MODEL_REGISTRY.
        Supported: 'vgg16', 'resnet18', 'squeezenet', 'mobilenetv3_small',
                   'efficientnetb0', 'efficientnetv2s', 'mobilenetv2', 'shufflenetv2'
    description_path : str, optional
        Path to save model architecture and parameter details.
        If None, no description file is created.
    config : dict, optional
        Configuration dictionary containing:
        - num_classes (int): Number of output classes
        - device (str): Device to place model ('cuda', 'cpu')
        - lr (float): Learning rate for optimizer
    pre_trained : bool, optional
        If True, loads ImageNet pre-trained weights (default: False).

    Returns
    -------
    tuple
        (model, criterion, optimizer, scheduler) where:
        - model (nn.Module): Neural network on specified device
        - criterion (nn.Module): CrossEntropyLoss
        - optimizer (optim.Optimizer): Adam optimizer
        - scheduler (optim.lr_scheduler): StepLR scheduler

    Raises
    ------
    ValueError
        If type_model is not in MODEL_REGISTRY.
    KeyError
        If config missing required keys ('num_classes', 'device', 'lr').

    Examples
    --------
    >>> config = {
    ...     'num_classes': 9,
    ...     'device': 'cuda',
    ...     'lr': 0.001
    ... }
    >>> model, criterion, optimizer, scheduler = create_model(
    ...     type_model='shufflenetv2',
    ...     config=config,
    ...     pre_trained=True
    ... )
    >>> print(model)
    ShuffleNet(...)
    """
    # Validate model type
    if type_model not in MODEL_REGISTRY:
        raise ValueError(
            f"Model '{type_model}' not supported. "
            f"Available options: {list(MODEL_REGISTRY.keys())}"
        )

    # Validate config
    if config is None:
        raise ValueError("config dictionary is required")
    
    required_keys = {'num_classes', 'device', 'lr'}
    missing_keys = required_keys - set(config.keys())
    if missing_keys:
        raise KeyError(f"config missing required keys: {missing_keys}")

    # Retrieve model metadata
    model_info = MODEL_REGISTRY[type_model]

    # ==================== Create Model ====================
    if pre_trained:
        # Load with ImageNet pre-trained weights
        model = model_info['model_fn'](weights=model_info['weights'])
    else:
        # Create model from scratch (random initialization)
        model = model_info['model_fn'](weights=None)

    # Replace classification layer
    model = _replace_classifier(model, model_info, config['num_classes'])

    # Move model to target device
    model = model.to(config['device'])

    # ==================== Save Description ====================
    if description_path:
        _save_model_description(
            model=model,
            type_model=type_model,
            pre_trained=pre_trained,
            description_path=description_path
        )

    # ==================== Training Setup ====================
    # Loss function for multi-class classification
    criterion = nn.CrossEntropyLoss()

    # Adam optimizer
    optimizer = optim.Adam(model.parameters(), lr=config['lr'])

    # Learning rate scheduler: reduce LR by 0.1 every 10 epochs
    scheduler = optim.lr_scheduler.StepLR(
        optimizer, step_size=10, gamma=0.1
    )

    return model, criterion, optimizer, scheduler