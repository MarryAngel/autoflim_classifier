"""
Training module for neural network models on parasite classification tasks.

This module provides utilities for training, validation, and evaluation of
classification models with support for early stopping and metric tracking.
"""

import os
import json
from pathlib import Path

import torch
import torch.nn as nn
from tqdm import tqdm
import numpy as np

from src.dataset import DataModuleParasite
from src import models


# ==================== RANDOM SEED ====================

seed = 42
torch.manual_seed(seed)
np.random.seed(seed)


# ==================== HELPER CLASSES ====================

class AverageMeter:
    """
    Utility class to compute and store the average and current value of metrics.

    Attributes
    ----------
    val : float
        Current value.
    avg : float
        Average value across all updates.
    sum : float
        Cumulative sum of all values.
    count : int
        Number of updates.

    Example
    -------
    >>> meter = AverageMeter()
    >>> meter.update(0.5, n=32)
    >>> meter.update(0.6, n=32)
    >>> print(meter.avg)
    0.55
    """

    def __init__(self):
        """Initialize the meter with zero values."""
        self.reset()

    def reset(self) -> None:
        """Reset all counters to zero."""
        self.val = 0
        self.avg = 0
        self.sum = 0
        self.count = 0

    def update(self, val: float, n: int = 1) -> None:
        """
        Update the meter with a new value.

        Parameters
        ----------
        val : float
            Value to add to the meter.
        n : int, optional
            Weight/count associated with this value (default: 1).
            Usually the batch size.
        """
        self.val = val
        self.sum += val * n
        self.count += n
        self.avg = self.sum / self.count


# ==================== TRAINING FUNCTIONS ====================

def evaluate(
    model: nn.Module,
    val_loader: torch.utils.data.DataLoader,
    criterion: nn.Module,
    device: str
) -> tuple:
    """
    Evaluate model performance on validation set.

    Runs the model in evaluation mode (disables dropout, batch norm updates)
    and computes loss and accuracy metrics without gradient computation.

    Parameters
    ----------
    model : nn.Module
        Neural network model to evaluate.
    val_loader : torch.utils.data.DataLoader
        DataLoader for validation set.
    criterion : nn.Module
        Loss function (e.g., CrossEntropyLoss).
    device : str
        Device to run computations ('cuda' or 'cpu').

    Returns
    -------
    tuple
        (avg_loss, avg_accuracy) both as float values.

    Notes
    -----
    - Model is placed in eval mode with torch.no_grad()
    - No gradients are computed or stored
    - Batch normalization uses running statistics
    """
    model.eval()

    val_loss_avg = AverageMeter()
    val_acc_avg = AverageMeter()

    with torch.no_grad():
        for inputs, labels in tqdm(val_loader, desc="Validation", leave=False):
            # Move data to device
            inputs, labels = inputs.to(device), labels.to(device)

            # Forward pass
            outputs = model(inputs)
            loss = criterion(outputs, labels)

            # Compute predictions
            _, preds = torch.max(outputs, 1)

            # Compute batch accuracy
            batch_acc = (preds == labels).sum().item() / labels.size(0)

            # Update meters
            val_loss_avg.update(loss.item(), labels.size(0))
            val_acc_avg.update(batch_acc, labels.size(0))

    return val_loss_avg.avg, val_acc_avg.avg


def train_epoch(
    model: nn.Module,
    train_loader: torch.utils.data.DataLoader,
    criterion: nn.Module,
    optimizer: torch.optim.Optimizer,
    device: str
) -> tuple:
    """
    Train model for one epoch.

    Performs forward/backward passes on all batches, updates parameters
    via optimizer, and tracks loss and accuracy metrics.

    Parameters
    ----------
    model : nn.Module
        Neural network model to train.
    train_loader : torch.utils.data.DataLoader
        DataLoader for training set.
    criterion : nn.Module
        Loss function (e.g., CrossEntropyLoss).
    optimizer : torch.optim.Optimizer
        Optimization algorithm (e.g., Adam).
    device : str
        Device to run computations ('cuda' or 'cpu').

    Returns
    -------
    tuple
        (avg_loss, avg_accuracy) for the epoch.

    Notes
    -----
    - Model is placed in train mode
    - Gradients are computed and parameter updates occur
    - Batch normalization updates running statistics
    """
    model.train()

    train_loss_avg = AverageMeter()
    train_acc_avg = AverageMeter()

    for inputs, labels in tqdm(train_loader, desc="Training", leave=False):
        # Move data to device
        inputs, labels = inputs.to(device), labels.to(device)

        # Zero gradients from previous iteration
        optimizer.zero_grad()

        # Forward pass
        outputs = model(inputs)
        loss = criterion(outputs, labels)

        # Backward pass
        loss.backward()
        optimizer.step()

        # Compute predictions
        _, preds = torch.max(outputs, 1)

        # Compute batch accuracy
        batch_acc = (preds == labels).sum().item() / labels.size(0)

        # Update meters
        train_loss_avg.update(loss.item(), labels.size(0))
        train_acc_avg.update(batch_acc, labels.size(0))

    return train_loss_avg.avg, train_acc_avg.avg


# ==================== MAIN TRAINING LOOP ====================

def train_loop(
    config: dict,
    dataloaders: dict,
    path: str,
    dataset_name: str,
    model_name: str,
    type_model: str,
    pre_trained: bool = False
) -> dict:
    """
    Complete training loop with early stopping across multiple splits and percentages.

    Trains separate models for each combination of split and data percentage,
    tracking metrics and saving best checkpoints based on validation accuracy.

    Parameters
    ----------
    config : dict
        Configuration dictionary containing:
        - percentage (list): Data percentages to train on (e.g., [1, 5, 25, 50, 75, 100])
        - split (list): K-fold split indices (e.g., [1, 2, 3])
        - num_classes (int): Number of output classes
        - num_epochs (int): Maximum number of training epochs
        - device (str): Device to run training ('cuda' or 'cpu')
        - lr (float): Learning rate
        - batch_size (int): Batch size
        - num_workers (int): Number of data loading workers

    dataloaders : dict
        Nested dictionary structure: dataloaders[split][percentage]['train'/'val']
        Each entry is a PyTorch DataLoader.

    path : str
        Base directory path for saving model checkpoints and results.

    dataset_name : str
        Name of dataset (e.g., 'eggs', 'larvae', 'cysts').

    model_name : str
        Name identifier for the model (used in checkpoint filenames).

    type_model : str
        Architecture type (e.g., 'shufflenetv2', 'resnet18').
        Must be registered in models.MODEL_REGISTRY.

    pre_trained : bool, optional
        If True, loads ImageNet pre-trained weights (default: False).

    Returns
    -------
    dict
        Training history with structure:
        {
            percentage: {
                split: {
                    'train_loss': [epoch1, epoch2, ...],
                    'train_acc': [epoch1, epoch2, ...],
                    'val_loss': [epoch1, epoch2, ...],
                    'val_acc': [epoch1, epoch2, ...]
                }
            }
        }

    Notes
    -----
    Early Stopping Strategy:
    - Monitors validation accuracy
    - Saves best model checkpoint when validation accuracy improves
    - Stops training if no improvement for 20 consecutive epochs
    - Saves checkpoints to: {path}/best_models_{dataset_name}/

    Example
    -------
    >>> config = {
    ...     'percentage': [25, 50, 100],
    ...     'split': [1, 2, 3],
    ...     'num_classes': 9,
    ...     'num_epochs': 100,
    ...     'device': 'cuda',
    ...     'lr': 0.0001,
    ...     'batch_size': 16,
    ...     'num_workers': 4
    ... }
    >>> historic = train_loop(
    ...     config, dataloaders, './results', 'eggs',
    ...     'shufflenetv2_scratch', 'shufflenetv2',
    ...     pre_trained=False
    ... )
    """
    historic_train = {}
    patience = 20  # Early stopping patience (epochs without improvement)

    # Iterate over all data percentages
    for perc in config['percentage']:
        historic_train[perc] = {}

        # Iterate over all k-fold splits
        for split in config['split']:
            # print(f"\n{'='*60}")
            # print(f"Training: Split {split} | Percentage {perc}%")
            # print(f"{'='*60}")

            # Create fresh model for this split+percentage combination
            base_model, criterion, optimizer, scheduler = models.create_model(
                type_model=type_model,
                description_path=None,
                config=config,
                pre_trained=pre_trained
            )

            best_val_acc = 0.0
            epochs_no_improve = 0

            # Initialize history tracking
            historic_train[perc][split] = {
                'train_loss': [],
                'train_acc': [],
                'val_loss': [],
                'val_acc': []
            }

            # Training epochs
            for epoch in tqdm(
                range(config['num_epochs']),
                desc=f"Epochs (Split {split}, Perc {perc}%)",
                leave=False
            ):
                # Validation phase
                val_loss, val_acc = evaluate(
                    base_model,
                    dataloaders[split][perc]['val'],
                    criterion,
                    config['device']
                )

                # Training phase
                train_loss, train_acc = train_epoch(
                    base_model,
                    dataloaders[split][perc]['train'],
                    criterion,
                    optimizer,
                    config['device']
                )

                # Record metrics
                historic_train[perc][split]['train_loss'].append(train_loss)
                historic_train[perc][split]['train_acc'].append(train_acc)
                historic_train[perc][split]['val_loss'].append(val_loss)
                historic_train[perc][split]['val_acc'].append(val_acc)

                # ==================== EARLY STOPPING ====================
                if val_acc > best_val_acc:
                    best_val_acc = val_acc
                    epochs_no_improve = 0

                    # Save best model checkpoint
                    checkpoint_dir = f'{path}/best_models_{dataset_name}'
                    os.makedirs(checkpoint_dir, exist_ok=True)
                    checkpoint_path = (
                        f'{checkpoint_dir}/{model_name}_best_split{split}_perc{perc}.pth'
                    )
                    torch.save(base_model.state_dict(), checkpoint_path)
                    
                else:
                    epochs_no_improve += 1

                    # Stop if no improvement for `patience` epochs
                    if epochs_no_improve >= patience:
                        print(
                            f'Early stopping triggered at epoch {epoch + 1} '
                            f'(split {split}, percentage {perc}%) - '
                            f'No improvement for {patience} epochs'
                        )
                        break

            # Update learning rate scheduler at end of training for this split
            scheduler.step()

    return historic_train