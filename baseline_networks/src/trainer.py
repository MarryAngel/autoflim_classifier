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

from thop import profile
from src.dataset import DataModuleParasite
from src import models

# Adds the project root to the path
project_root = Path.cwd().parent
sys.path.insert(0, str(project_root))

from config import (
    get_dataset_paths
)

seed = 42
torch.manual_seed(seed)
np.random.seed(seed)

# ---------------------- TRAINING ------------------ #

class AverageMeter:
    def __init__(self):
        self.reset()

    def reset(self):
        self.val = 0
        self.avg = 0
        self.sum = 0
        self.count = 0

    def update(self, val, n=1):
        self.val = val
        self.sum += val * n
        self.count += n
        self.avg = self.sum / self.count
        
def evaluate(model, val_loader, criterion, device):
    model.eval()
    
    val_loss_avg = AverageMeter()
    val_acc_avg = AverageMeter()
    
    with torch.no_grad():
        for input, label in tqdm(val_loader, desc="Validation", leave=False):
            input, label = input.to(device), label.to(device)

            # Forward
            outputs = model(input)
            loss = criterion(outputs, label)

            # Predictions
            _, preds = torch.max(outputs, 1)

            # Metrics for batch
            batch_acc = (preds == label).sum().item() / label.size(0)

            val_loss_avg.update(loss.item(), label.size(0))
            val_acc_avg.update(batch_acc, label.size(0))

    return val_loss_avg.avg, val_acc_avg.avg

def train(model, train_loader, criterion, optimizer, device):
    model.train()
    
    train_loss_avg = AverageMeter()
    train_acc_avg = AverageMeter()

    for input, label in tqdm(train_loader, desc="Training", leave=False):
        input, label = input.to(device), label.to(device)

        # Forward
        optimizer.zero_grad()
        outputs = model(input)
        loss = criterion(outputs, label)

        # Backward
        loss.backward()
        optimizer.step()
        
        # Predictions 
        _, preds = torch.max(outputs, 1)
        
        # Metrics for batch
        batch_acc = (preds == label).sum().item() / label.size(0)

        train_loss_avg.update(loss.item(), label.size(0))
        train_acc_avg.update(batch_acc, label.size(0))

    return train_loss_avg.avg, train_acc_avg.avg

def train_loop(config, dataloaders, path, dataset_name, model_name, type_model, pre_trained=False):

    # Training loop with early stopping

    historic_train = {}
    patience = 20 

    for perc in config['percentage']:
        historic_train[perc] = {}
        for split in config['split']:

            base_model, criterion, optimizer, scheduler = models.create_model(type_model=type_model, description_path=None, config=config, pre_trained=pre_trained)

            best_val_acc = 0.0
            historic_train[perc][split] = {
                'train_loss': [],
                'train_acc': [],
                'val_loss': [],
                'val_acc': []
            }
            
            for epoch in tqdm(range(config['num_epochs']), desc=f"Split {split} - Percentage {perc}%", leave=False):
                
                val_loss, val_acc = evaluate(base_model, dataloaders[split][perc]['val'], criterion, config['device'])

                train_loss, train_acc = train(base_model, dataloaders[split][perc]['train'], criterion, optimizer, config['device'])
                historic_train[perc][split]['train_loss'].append(train_loss)
                historic_train[perc][split]['train_acc'].append(train_acc)
                historic_train[perc][split]['val_loss'].append(val_loss)
                historic_train[perc][split]['val_acc'].append(val_acc)
                
                # Early Stopping
                if val_acc > best_val_acc:
                    best_val_acc = val_acc
                    epochs_no_improve = 0
                    os.makedirs(f'{path}/best_models_{dataset_name}', exist_ok=True)
                    torch.save(base_model.state_dict(), f'{path}/best_models_{dataset_name}/{model_name}_best_split{split}_perc{perc}.pth')
                else:
                    epochs_no_improve += 1
                    if epochs_no_improve >= patience:
                        print(f'Early stopping at epoch {epoch+1} for split {split} and percentage {perc}%')
                        break

            val_loss, val_acc = evaluate(base_model, dataloaders[split][perc]['val'], criterion, config['device'])

            historic_train[perc][split]['val_loss'].append(val_loss)
            historic_train[perc][split]['val_acc'].append(val_acc)
                
            scheduler.step()
            
    return historic_train