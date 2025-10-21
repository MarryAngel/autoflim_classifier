from pathlib import Path

# ==================== BASE PATHS ====================

# Project root 
PROJECT_ROOT = Path(__file__).parent.resolve()

# Main directories
SRC_DIR = PROJECT_ROOT / 'src'
DATASETS_DIR = PROJECT_ROOT / 'datasets'
NOTEBOOKS_DIR = PROJECT_ROOT / 'notebooks'

# ==================== DATASETS ====================

DATASETS = {
    'cistos': DATASETS_DIR / 'cistos',
    'eggs': DATASETS_DIR / 'eggs',
    'larvae': DATASETS_DIR / 'larvae'
}

# ==================== FUNCTIONS ====================

def get_dataset_paths(dataset_name):
    """Returns paths for a specific dataset.

    Args:
        dataset_name (str): The name of the dataset ('cistos', 'eggs', or 'larvae').

    Returns:
        Path: The path to the dataset.
    """
    
    if dataset_name not in DATASETS:
        raise ValueError(f"Dataset '{dataset_name}' not found. Available: {list(DATASETS.keys())}")
    
    dataset_root = DATASETS[dataset_name]
    print(f"{dataset_root=}")
    
    return {
        'root': dataset_root,
        'images': dataset_root / 'images',
        'splits': dataset_root / 'splits',
        'splits_': dataset_root / 'splits_incremental'  # Added splits_ path
    }