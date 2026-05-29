from pathlib import Path

# ==================== BASE PATHS ====================

# Project root and Baseline Networks directory
PROJECT_ROOT = Path(__file__).parent.resolve()
DATASETS_DIR = PROJECT_ROOT / 'datasets'


# ==================== DATASETS ====================

DATASETS = {
    'eggs': DATASETS_DIR / 'eggs',
    'larvae': DATASETS_DIR / 'larvae',
    'cysts': DATASETS_DIR / 'cysts'
}

# ==================== FUNCTIONS ====================

def get_dataset_paths(dataset_name: str) -> dict[str, Path]:
    """
    Return the main directory paths for a given dataset.

    Parameters
    ----------
    dataset_name : str
        Name of the dataset registered in `DATASETS`.

    Returns
    -------
    dict[str, Path]
        A dictionary containing:
        - root: dataset root directory
        - images: images directory
        - masks: masks directory
        - splits: split files directory
        - splits_incremental: incremental split files directory

    Raises
    ------
    ValueError
        If `dataset_name` is not found in `DATASETS`.
    """
    
    if dataset_name not in DATASETS:
        raise ValueError(f"Dataset '{dataset_name}' not found. Available: {list(DATASETS.keys())}")
    
    dataset_root = DATASETS[dataset_name]
    
    return {
        'root': dataset_root,
        'images': dataset_root / 'images',
        'masks': dataset_root / 'masks',
        'splits': dataset_root / 'splits',
        'splits_incremental': dataset_root / 'splits_incremental'  
    }
    
def get_split_path_incremental(dataset_name: str, split: int, percentage: float) -> Path:
    """
    Return the full path to an incremental split JSON file.

    Parameters
    ----------
    dataset_name : str
        Name of the dataset registered in `DATASETS`.
    split : int
        Split index used in the folder name.
    percentage : float
        Percentage identifier used in the file name.

    Returns
    -------
    Path
        Full path to the incremental split JSON file.

    Raises
    ------
    ValueError
        If `dataset_name` is not found in `DATASETS`.
    """
    
    path_name = get_dataset_paths(dataset_name)
    path_json = f"{path_name['splits_incremental']}/split{split}/data_descriptor_perc{percentage}.json"
    return path_json

