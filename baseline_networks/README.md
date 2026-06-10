# Baseline Networks

## Project Structure

```
baseline_networks/
├── notebooks/          # Jupyter notebooks for training
└── src/                # Reusable Python modules
```

## Contents - Notebooks

This folder contains Jupyter notebooks to train and evaluate different neural network architectures (baseline models) on FLIM datasets:
- **EfficientNetB0** - Efficient neural network with balance between accuracy and computational cost
- **EfficientNetV2** - Improved EfficientNet version with faster training
- **MobileNetV2** - Mobile-optimized model, lightweight and fast
- **MobileNetV3 Small** - Compact version of MobileNetV3, even more efficient
- **ResNet18** - Classic residual network with 18 layers
- **ShuffleNetV2** - Architecture optimized for speed with channel shuffling
- **SqueezeNet** - Compact model with few parameters
- **VGG16** - Classic deep network with simple convolution

## Supported Datasets

Each notebook trains/evaluates on:
- **Eggs** - 9 classes
- **Larvae** - 6 classes  
- **Cysts** - 2 classes

## How to Run

### Prerequisites
```bash
# Activate virtual environment
source venv/bin/activate

# Install dependencies
pip install -r ../requirements.txt
```

### Run a Notebook
```bash
# Open notebook in Jupyter
jupyter notebook <model>/<model>.ipynb

# Example: train EfficientNetB0
jupyter notebook efficientnetb0.ipynb
```

### Configuration
Inside each notebook, edit the `CONFIG_*` blocks to:
- Choose dataset (eggs, larvae, cysts)
- Adjust `batch_size`, `num_epochs`, `lr` (learning rate)
- Configure `percentage` (fraction of data to train)
- Select data splits
- Mode: train from scratch or fine-tune with ImageNet weights

## Output

Models and results are saved in folders like:
```
<model>/<model>_scratch/eggs/
<model>/<model>_pretrained/eggs/
```

Containing:
- Model checkpoints (.pth)
- JSON reports with metrics
- Confusion matrices and plots

## Python Modules - src/

The `src/` folder contains reusable modules imported by notebooks to abstract common functionality:

### `dataset.py`
- **DatasetParasite**: Custom PyTorch Dataset that loads parasite images
  - Reads train/validation/test splits from JSON files
  - Extracts labels from filename prefixes
  - Supports augmentations via `transforms`
- **DataModuleParasite**: High-level data manager
  - Abstracts DataLoader creation
  - Manages multiple splits and data percentages
  - Simplified interface for training

### `models.py`
- **MODEL_REGISTRY**: Registry of supported architectures
  - VGG16, ResNet18, SqueezeNet, MobileNetV3 Small, EfficientNet, etc.
  - Each model includes pre-trained ImageNet weights
  - Configuration of custom classification layers
- Functions to create and adapt models with custom classification heads

### `trainer.py`
- **AverageMeter**: Helper class to track metrics
- **Training functions**: 
  - Train an epoch (forward pass, backward pass, optimization)
  - Validate on validation data
  - Early stopping based on metrics
  - Loss and accuracy tracking

### `utils.py`
- **DataLoader creation**: `create_dataloaders()` for all splits/percentages
- **FLOPs computation**: Model profiling for computational cost analysis
- **Report generation**: Classification metrics with mean/std
- **Visualization**: 
  - Training history (loss, accuracy)
  - Confusion matrices
  - Aggregated metrics
- **Evaluation**: Evaluate on test datasets with metric computation
- **Result management**: Save and load checkpoints and JSON reports

