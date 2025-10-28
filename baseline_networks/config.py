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
    """Returns paths for a specific dataset"""
    
    if dataset_name not in DATASETS:
        raise ValueError(f"Dataset '{dataset_name}' not found. Available: {list(DATASETS.keys())}")
    
    dataset_root = DATASETS[dataset_name]
    
    return {
        'root': dataset_root,
        'images': dataset_root / 'images',
        'splits': dataset_root / 'splits',
        'splits_incremental': dataset_root / 'splits_incremental'  
    }
    
def get_split_path_incremental(dataset_name, split, percentage):
    """Returns the full path of an incremental split file."""
    
    path_name = get_dataset_paths(dataset_name)
    path_json = f"{path_name['splits_incremental']}/split{split}/data_descriptor_perc{percentage}.json"
    return path_json