"""
Utility functions for model training, evaluation, and result visualization.

This module provides helper functions for:
- Creating DataLoaders for multiple splits and percentages
- Computing FLOPs and parameters
- Generating classification reports with mean/std
- Plotting training history and aggregated metrics
- Evaluating models on test sets
- Saving and managing results
"""

import sys
import os
import json
from pathlib import Path

import torch
import torch.nn as nn
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from tqdm import tqdm
from sklearn.metrics import (
    confusion_matrix, 
    classification_report, 
    cohen_kappa_score
)
from thop import profile

from src.dataset import DataModuleParasite
from src import models


# ==================== RANDOM SEED ====================

seed = 42
torch.manual_seed(seed)
np.random.seed(seed)


# ==================== DATA LOADING ====================

def create_dataloaders(config: dict, transforms) -> dict:
    """
    Create DataLoaders for all splits and percentages.

    Generates train/validation/test DataLoaders for each combination of
    k-fold split and data percentage, organized in a nested dictionary.

    Parameters
    ----------
    config : dict
        Configuration dictionary containing:
        - dataset_name (str): Dataset identifier
        - split (list): K-fold split indices (e.g., [1, 2, 3])
        - percentage (list): Data percentages (e.g., [1, 5, 25, 50, 75, 100])
        - batch_size (int): Batch size for DataLoaders
        - num_workers (int): Number of data loading workers
    transforms : callable
        PyTorch transforms to apply to images (resize, normalize, augmentation).

    Returns
    -------
    dict
        Nested dictionary: dataloaders[split][percentage] = {
            'train': DataLoader,
            'val': DataLoader,
            'test': DataLoader
        }

    Example
    -------
    >>> config = {
    ...     'dataset_name': 'eggs',
    ...     'split': [1, 2, 3],
    ...     'percentage': [25, 50, 100],
    ...     'batch_size': 16,
    ...     'num_workers': 4
    ... }
    >>> dataloaders = create_dataloaders(config, transforms)
    >>> train_loader = dataloaders[1][25]['train']
    """
    dataloader = {}

    for split in config['split']:
        dataloader[split] = {}
        for perc in config['percentage']:
            # Create data module for this split+percentage combination
            data_module = DataModuleParasite(
                dataset_name=config['dataset_name'],
                split=split,
                percentage=perc,
                batch_size=config['batch_size'],
                num_workers=config['num_workers'],
                transform=transforms
            )
            # Initialize datasets
            data_module.setup()
            
            # Store DataLoaders
            dataloader[split][perc] = {
                'train': data_module.train_dataloader(),
                'val': data_module.val_dataloader(),
                'test': data_module.test_dataloader()
            }

    return dataloader


# ==================== MODEL ANALYSIS ====================

def count_flops(model: nn.Module, input_size: tuple = (1, 3, 200, 200)) -> tuple:
    """
    Count FLOPs and parameters in a model.

    Uses THOP library to compute floating point operations and parameter count
    for a neural network given a specific input size.

    Parameters
    ----------
    model : nn.Module
        PyTorch neural network model.
    input_size : tuple, optional
        Input tensor shape (batch, channels, height, width).
        Default: (1, 3, 200, 200)

    Returns
    -------
    tuple
        (flops, params) where:
        - flops: Total floating point operations
        - params: Total number of parameters

    Example
    -------
    >>> model = models.resnet18()
    >>> flops, params = count_flops(model, input_size=(1, 3, 224, 224))
    >>> print(f"FLOPs: {flops:,}, Parameters: {params:,}")
    """
    dummy_input = torch.randn(input_size).to(next(model.parameters()).device)
    flops, params = profile(model, inputs=(dummy_input,), verbose=False)
    return flops, params


# ==================== REPORTING UTILITIES ====================

def _convert_keys_to_int(obj) -> dict:
    """
    Recursively convert string dictionary keys to integers where possible.

    JSON serialization converts integer keys to strings. This function
    reverses that conversion for compatibility with nested dictionaries.

    Parameters
    ----------
    obj : dict or any
        Object (typically a loaded JSON dict) to convert.

    Returns
    -------
    dict or any
        Object with string keys converted to integers where applicable.

    Example
    -------
    >>> data = {'1': {'2': 'value'}, '3': 'other'}
    >>> converted = _convert_keys_to_int(data)
    >>> print(converted)
    {1: {2: 'value'}, 3: 'other'}
    """
    if isinstance(obj, dict):
        new = {}
        for k, v in obj.items():
            try:
                new_k = int(k)
            except (ValueError, TypeError):
                new_k = k
            new[new_k] = _convert_keys_to_int(v)
        return new
    return obj


def generate_mean_std_report(
    results_data: dict,
    percentage: float,
    split_list: list
) -> pd.DataFrame:
    """
    Generate aggregated classification report with mean and std across splits.

    Computes mean and standard deviation of precision, recall, f1-score, and
    support metrics across multiple k-fold splits for a given data percentage.

    Parameters
    ----------
    results_data : dict
        Test results dictionary with structure:
        results_data[percentage][split] = {
            'classification_report_dict': scikit-learn report dict,
            'kappa': Cohen's kappa score
        }
    percentage : float
        Data percentage to aggregate (e.g., 25).
    split_list : list
        List of split indices to aggregate (e.g., [1, 2, 3]).

    Returns
    -------
    pd.DataFrame or None
        DataFrame with class-wise metrics (mean ± std) if successful,
        None if required keys are missing.

    Notes
    -----
    - Each row represents a class or average metric
    - Columns: precision, recall, f1-score, support
    - Includes overall accuracy and Cohen's kappa metrics
    - NaN values are replaced with empty strings

    Example
    -------
    >>> df = generate_mean_std_report(results, 25, [1, 2, 3])
    >>> print(df)
                precision          recall       f1-score     support
    0             0.9234 ± 0.0156  0.9123 ± 0.0234  ...
    accuracy      0.9200 ± 0.0145  ...
    """
    reports_list = []
    kappa_list = []

    # ==================== Collect Data ====================
    for split in split_list:
        data_split = results_data[percentage][split]
        
        # Verify required keys exist
        if 'classification_report_dict' in data_split:
            reports_list.append(data_split['classification_report_dict'])
            kappa_list.append(data_split['kappa'])
        else:
            return None

    # ==================== Prepare DataFrame ====================
    keys = list(reports_list[0].keys())
    
    # Remove 'accuracy' if present (handled separately)
    if 'accuracy' in keys:
        keys.remove('accuracy')

    # Create DataFrame with class indices and metric columns
    final_df = pd.DataFrame(
        index=keys + ['accuracy', 'Cohen Kappa'],
        columns=['precision', 'recall', 'f1-score', 'support']
    )

    # ==================== Fill Class Metrics ====================
    for key in keys:
        for metric in ['precision', 'recall', 'f1-score', 'support']:
            # Collect values across all splits
            values = [r[key][metric] for r in reports_list]
            mean_val = np.mean(values)
            std_val = np.std(values)

            # Format based on metric type
            if metric == 'support':
                final_df.loc[key, metric] = f"{int(mean_val)}"
            else:
                final_df.loc[key, metric] = f"{mean_val:.4f} ± {std_val:.4f}"

    # ==================== Fill Accuracy ====================
    acc_values = [r['accuracy'] for r in reports_list]
    final_df.loc['accuracy', 'f1-score'] = (
        f"{np.mean(acc_values):.4f} ± {np.std(acc_values):.4f}"
    )
    total_support = int(np.mean([r['macro avg']['support'] for r in reports_list]))
    final_df.loc['accuracy', 'support'] = f"{total_support}"

    # ==================== Fill Cohen Kappa ====================
    k_mean = np.mean(kappa_list)
    k_std = np.std(kappa_list)
    final_df.loc['Cohen Kappa', 'f1-score'] = f"{k_mean:.4f} ± {k_std:.4f}"

    return final_df.fillna('')


# ==================== VISUALIZATION ====================

def loss_and_accuracy_split(
    config: dict,
    path: str,
    model_name: str,
    show_plot: bool = False
) -> None:
    """
    Plot training history separately for each split and percentage combination.

    Creates a grid of subplots showing loss and accuracy curves for training,
    validation across all k-fold splits and data percentages.

    Parameters
    ----------
    config : dict
        Configuration dictionary containing:
        - dataset_name (str): Dataset name
        - percentage (list): Data percentages
        - split (list): K-fold split indices
    path : str
        Directory containing the training history JSON file.
    model_name : str
        Model identifier (used in filename).
    show_plot : bool, optional
        Whether to display the plot (default: False, saves instead).

    Notes
    -----
    - Saves high-resolution PNG to: {path}/training_history_{dataset}_{model_name}.png
    - Left y-axis: Loss (blue/red lines)
    - Right y-axis: Accuracy (green/magenta dashed lines)
    - Each subplot title shows split and data percentage
    """
    # Load training history
    with open(
        f'{path}/{model_name}_historic_{config["dataset_name"]}.json', 'r'
    ) as f:
        historic_train = json.load(f)

    # Create grid of subplots
    fig, axes = plt.subplots(
        len(config['percentage']),
        len(config['split']),
        figsize=(18, 4 * len(config['percentage']))
    )

    # Plot each split+percentage combination
    for perc_idx, perc in enumerate(config['percentage']):
        for split_idx, split in enumerate(config['split']):
            ax = axes[perc_idx, split_idx]

            historic = historic_train[str(perc)][str(split)]

            epochs = range(1, len(historic['train_loss']) + 1)
            epochs_val = range(0, len(historic['val_loss']))

            # ==================== Loss Axis ====================
            ax_loss = ax
            ax_loss.plot(
                epochs, historic['train_loss'],
                'b-', label='Train Loss', linewidth=2
            )
            ax_loss.plot(
                epochs_val, historic['val_loss'],
                'r-', label='Val Loss', linewidth=2
            )
            ax_loss.set_xlabel('Epochs')
            ax_loss.set_ylabel('Loss', color='black')
            ax_loss.tick_params(axis='y', labelcolor='black')

            # ==================== Accuracy Axis ====================
            ax_acc = ax.twinx()
            ax_acc.plot(
                epochs, historic['train_acc'],
                'g--', label='Train Acc', linewidth=2
            )
            ax_acc.plot(
                epochs_val, historic['val_acc'],
                'm--', label='Val Acc', linewidth=2
            )
            ax_acc.set_ylabel('Accuracy', color='black')
            ax_acc.tick_params(axis='y', labelcolor='black')

            # ==================== Formatting ====================
            if perc == 1:
                ax.set_title(
                    f'Split {split} - 1 image per class',
                    fontsize=12, fontweight='bold'
                )
            else:
                ax.set_title(
                    f'Split {split} - Percentage {perc}%',
                    fontsize=12, fontweight='bold'
                )

            ax.grid(True, alpha=0.3)

            # Combine legends from both axes
            lines1, labels1 = ax_loss.get_legend_handles_labels()
            lines2, labels2 = ax_acc.get_legend_handles_labels()
            ax.legend(
                lines1 + lines2, labels1 + labels2,
                loc='upper right', fontsize=9
            )

    plt.suptitle(
        f'Training History - {config["dataset_name"]} Dataset ({model_name})',
        fontsize=16, fontweight='bold', y=0.995
    )
    plt.tight_layout()
    plt.savefig(
        f'{path}/training_history_{config["dataset_name"]}_grid_{model_name}.png',
        dpi=300, bbox_inches='tight'
    )

    if show_plot:
        plt.show()
    else:
        plt.close()


def loss_and_accuracy_aggregate(
    config: dict,
    path: str,
    model_name: str,
    show_plot: bool = False
) -> None:
    """
    Plot aggregated (averaged) loss and accuracy across all splits per percentage.

    For each data percentage, computes mean loss/accuracy across k-fold splits
    to visualize overall training performance without split variability.

    Parameters
    ----------
    config : dict
        Configuration dictionary containing:
        - dataset_name (str): Dataset name
        - percentage (list): Data percentages
        - split (list): K-fold split indices
    path : str
        Directory containing the training history JSON file.
    model_name : str
        Model identifier (used in filename).
    show_plot : bool, optional
        Whether to display plots (default: False, saves instead).

    Notes
    -----
    - Creates separate figure for each percentage
    - Truncates histories to minimum length to handle early stopping variability
    - Saves to: {path}/aggregated_loss_accuracy_{dataset}_perc{percentage}.png
    - Displays mean loss/accuracy with consistent visual style
    """
    # Load training history
    with open(
        f'{path}/{model_name}_historic_{config["dataset_name"]}.json', 'r'
    ) as f:
        historic_train = json.load(f)

    agg_rows = []

    # ==================== Aggregate Across Splits ====================
    for perc in config['percentage']:
        all_train_loss = []
        all_val_loss = []
        all_train_acc = []
        all_val_acc = []

        # Collect histories for all splits at this percentage
        for split in config['split']:
            historic = historic_train[str(perc)][str(split)]
            all_train_loss.append(historic['train_loss'])
            all_val_loss.append(historic['val_loss'])
            all_train_acc.append(historic['train_acc'])
            all_val_acc.append(historic['val_acc'])

        # Handle early stopping: truncate to minimum length
        min_len = min(len(x) for x in all_train_loss)

        all_train_loss_trunc = [x[:min_len] for x in all_train_loss]
        all_val_loss_trunc = [x[:min_len] for x in all_val_loss]
        all_train_acc_trunc = [x[:min_len] for x in all_train_acc]
        all_val_acc_trunc = [x[:min_len] for x in all_val_acc]

        # Compute mean across splits
        avg_train_loss = np.mean(all_train_loss_trunc, axis=0)
        avg_val_loss = np.mean(all_val_loss_trunc, axis=0)
        avg_train_acc = np.mean(all_train_acc_trunc, axis=0)
        avg_val_acc = np.mean(all_val_acc_trunc, axis=0)

        agg_rows.append({
            'percentage': perc,
            'avg_train_loss': avg_train_loss,
            'avg_val_loss': avg_val_loss,
            'avg_train_acc': avg_train_acc,
            'avg_val_acc': avg_val_acc,
        })

    # ==================== Plot Aggregated Results ====================
    for agg in agg_rows:
        epochs = range(1, len(agg['avg_train_loss']) + 1)
        epochs_val = range(0, len(agg['avg_val_loss']))

        fig, ax1 = plt.subplots(figsize=(10, 4))

        # Loss curves
        ax1.plot(
            epochs, agg['avg_train_loss'],
            label='Train Loss', color='blue', linewidth=2
        )
        ax1.plot(
            epochs_val, agg['avg_val_loss'],
            label='Val Loss', color='red', linewidth=2
        )
        ax1.set_xlabel('Epochs')
        ax1.set_ylabel('Loss')
        ax1.grid(True, alpha=0.3)

        # Accuracy curves (secondary y-axis)
        ax2 = ax1.twinx()
        ax2.plot(
            epochs, agg['avg_train_acc'],
            label='Train Acc', color='green', linestyle='--', linewidth=2
        )
        ax2.plot(
            epochs_val, agg['avg_val_acc'],
            label='Val Acc', color='purple', linestyle='--', linewidth=2
        )
        ax2.set_ylabel('Accuracy')

        # Combine legends
        lines1, labels1 = ax1.get_legend_handles_labels()
        lines2, labels2 = ax2.get_legend_handles_labels()
        ax1.legend(lines1 + lines2, labels1 + labels2, loc='lower right')

        # Title with percentage info
        if agg['percentage'] == 1:
            ax1.set_title('1 image per class - Avg Loss/Accuracy Across Splits')
        else:
            ax1.set_title(
                f'Percentage {agg["percentage"]}% - Avg Loss/Accuracy Across Splits'
            )

        plt.savefig(
            (f'{path}/aggregated_loss_accuracy_{config["dataset_name"]}'
             f'_perc{agg["percentage"]}.png'),
            dpi=300, bbox_inches='tight'
        )

        plt.tight_layout()
        if show_plot:
            plt.show()
        else:
            plt.close()


# ==================== TEST EVALUATION ====================

def evaluate_test_set(
    config: dict,
    path: str,
    model_name: str,
    dataloaders: dict,
    type_model: str,
    pre_trained: bool = False
) -> dict:
    """
    Evaluate trained models on test set and compute classification metrics.

    Loads trained model checkpoints for each split+percentage combination,
    evaluates on test set, and computes confusion matrix, Cohen's kappa,
    and classification reports.

    Parameters
    ----------
    config : dict
        Configuration dictionary containing:
        - device (str): Device for inference ('cuda' or 'cpu')
        - dataset_name (str): Dataset name
        - split (list): K-fold split indices
        - percentage (list): Data percentages
        - num_classes (int): Number of output classes
        - lr (float): Learning rate (needed for model creation)
    path : str
        Base directory containing model checkpoints.
    model_name : str
        Model checkpoint identifier.
    dataloaders : dict
        Nested dictionary dataloaders[split][percentage]['test'].
    type_model : str
        Model architecture type.
    pre_trained : bool, optional
        Whether model was pre-trained (default: False).

    Returns
    -------
    dict
        Nested dictionary results[percentage][split] containing:
        - confusion_matrix (list): Confusion matrix as nested list
        - kappa (float): Cohen's kappa score
        - classification_report_dict (dict): Scikit-learn report dict
        - classification_report_str (str): Formatted classification report

    Notes
    -----
    - Expects checkpoints at: {path}/best_models_{dataset_name}/{model_name}_best_split{split}_perc{perc}.pth
    - No gradient computation during evaluation
    - Models placed in eval mode for batch norm/dropout handling
    """
    results = {}

    for perc in config['percentage']:
        results[perc] = {}

        for split in config['split']:
            # Create model matching checkpoint configuration
            model_eval, _, _, _ = models.create_model(
                type_model=type_model,
                config=config,
                pre_trained=pre_trained
            )

            # Load trained checkpoint
            checkpoint_path = (
                f'{path}/best_models_{config["dataset_name"]}'
                f'/{model_name}_best_split{split}_perc{perc}.pth'
            )
            model_eval.load_state_dict(
                torch.load(checkpoint_path, map_location=config['device'])
            )
            model_eval.to(config['device'])
            model_eval.eval()

            all_preds = []
            all_labels = []

            # ==================== Inference ====================
            with torch.no_grad():
                for inputs, labels in tqdm(
                    dataloaders[split][perc]['test'],
                    desc=f'Testing Split {split} Perc {perc}%',
                    leave=False
                ):
                    inputs = inputs.to(config['device'])
                    labels = labels.to(config['device'])

                    outputs = model_eval(inputs)
                    _, preds = torch.max(outputs, 1)

                    all_preds.extend(preds.cpu().numpy())
                    all_labels.extend(labels.cpu().numpy())

            # ==================== Compute Metrics ====================
            cm = confusion_matrix(all_labels, all_preds)
            kappa = cohen_kappa_score(all_labels, all_preds)
            report_dict = classification_report(
                all_labels, all_preds,
                zero_division=0, output_dict=True
            )
            report_str = classification_report(
                all_labels, all_preds,
                zero_division=0
            )

            results[perc][split] = {
                'confusion_matrix': cm.tolist(),
                'kappa': kappa,
                'classification_report_dict': report_dict,
                'classification_report_str': report_str
            }

    return results


def calculate_and_save_reports(
    config: dict,
    path: str,
    model_name: str
) -> None:
    """
    Generate and save aggregated classification reports.

    Loads test results JSON, aggregates metrics (mean ± std) across splits
    for each data percentage, and saves to a unified text file.

    Parameters
    ----------
    config : dict
        Configuration dictionary containing:
        - dataset_name (str): Dataset name
        - percentage (list): Data percentages
        - split (list): K-fold split indices
    path : str
        Directory containing test results JSON file.
    model_name : str
        Model identifier (used in filenames).

    Notes
    -----
    - Loads from: {path}/{model_name}_test_results_{dataset_name}.json
    - Saves to: {path}/{model_name}_aggregated_classification_report_{dataset_name}.txt
    - Removes existing report file before appending new results
    - Reports include precision, recall, f1-score, support, accuracy, Cohen's kappa
    """
    # Load test results
    with open(
        f'{path}/{model_name}_test_results_{config["dataset_name"]}.json', 'r'
    ) as f:
        results = json.load(f)

    # Convert string keys back to integers
    results = _convert_keys_to_int(results)

    # Remove existing report file (will append new results)
    report_path = (
        f'{path}/{model_name}_aggregated_classification_report'
        f'_{config["dataset_name"]}.txt'
    )
    if os.path.exists(report_path):
        os.remove(report_path)

    # ==================== Generate Reports ====================
    for perc in config['percentage']:
        df_result = generate_mean_std_report(results, perc, config['split'])

        if df_result is not None:
            # Append to unified report file
            with open(report_path, 'a') as f:
                if perc == 1:
                    f.write(f"\n>> Percentage: 1 image per class\n")
                else:
                    f.write(f"\n>> Percentage: {perc}%\n")
                f.write(df_result.fillna('').to_string())
                f.write("\n\n")

    print(f"Aggregated classification reports saved to {report_path}")