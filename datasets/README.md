# Datasets

## Project Structure

```
datasets/
├── eggs/              # Egg parasite images
├── larvae/            # Larvae parasite images
├── cysts/             # Cyst parasite images
├── datasets_parasites.zip     # Complete dataset with images and masks
├── split_dataset.py           # Script for train/test splitting
├── split_incremental.py       # Script for incremental data splits
└── run_splits.sh              # Bash script to run splitting on all datasets
```

## Contents

This folder contains three parasite image datasets used for training and evaluating classification models:

- **Eggs** - 9 classes of parasitic eggs
- **Larvae** - 6 classes of parasitic larvae
- **Cysts** - 2 classes of parasitic cysts

Each dataset folder contains raw images that are organized and split using the provided Python scripts.

## Dataset Organization

Each dataset folder (eggs/, larvae/, cysts/) contains:
- A `images/` folder with PNG image files
- A `mask/` folder with PNG mask files
- A `files.txt` containing list of all images (created after organizing)
- A `splits/` folder (created after running split scripts) with train/test split files
- A `splits_incremental/` folder (created by split_incremental.py) with JSON data descriptors for incremental percentages

## Pre-existing Splits

**Pre-computed dataset splits are already included** in each dataset folder (`eggs/splits`, `larvae/splits`, `cysts/splits`) for reproducibility. These splits were created with 3-fold cross-validation and stratified sampling (50% training data).

You can use these pre-existing splits directly without running the splitting scripts:
- `train1.csv`, `test1.csv` - Fold 1
- `train2.csv`, `test2.csv` - Fold 2
- `train3.csv`, `test3.csv` - Fold 3

Similarly, pre-computed incremental splits in `splits_incremental/` folders contain JSON descriptors for different data percentages (5%, 25%, 50%, 75%, 100%).

To regenerate splits with different parameters, follow the "How to Prepare Datasets" section below.

## Python Scripts

### `split_dataset.py`

Creates stratified or random train/test splits from a dataset.

**Purpose**: Organize raw images into multiple k-fold train/test splits with different sampling strategies.

**Usage**:
```bash
python split_dataset.py <input_csv> <output_folder> <num_splits> <sampling_type> <parameter>
```

**Parameters**:
- `input_csv`: CSV/TXT file listing all images in the dataset
- `output_folder`: Output directory for split files (e.g., `eggs/splits`)
- `num_splits`: Number of cross-validation folds to create (typically 3)
- `sampling_type`: Sampling strategy
  - `1` = Percentage of smallest class samples
  - `2` = Percentage per class (stratified)
  - `3` = Fixed number per class (constrained to 0.9 × smallest class)
  - `4` = Random sampling, ignoring class labels
- `parameter`: Percentage [0,1] or number of images per class

**Output**: Creates files like `train1.csv`, `test1.csv`, `train2.csv`, `test2.csv`, etc.

**Example**:
```bash
python split_dataset.py eggs/files.txt eggs/splits 3 2 0.5
```

### `split_incremental.py`

Generates incremental data descriptors with varying percentages of training data.

**Purpose**: Create data splits with 5%, 25%, 50%, 75%, and 100% of training data for studying model performance with different dataset sizes.

**Percentages**: `[5, 25, 50, 75, 100]`

**Output Format**: JSON files with structure:
```json
{
    "training": ["000001_00000341.png", "000002_00000342.png", ...],
    "validation": [],
    "test": []
}
```

### `run_splits.sh`

Bash script that automates splitting for all datasets.

**Purpose**: Execute `split_dataset.py` on all datasets (cysts, eggs, larvae) with consistent parameters.

**Configuration** (edit inside script):
```bash
NUM_SPLITS=3              # Number of folds
SAMPLING_TYPE=2           # Stratified by class
PARAMETER=0.5             # 50% training data
```

**Usage**:
```bash
chmod +x run_splits.sh
./run_splits.sh
```

**What it does**:
1. Iterates over each dataset (cysts, eggs, larvae)
2. Runs `split_dataset.py` with specified parameters
3. Creates `splits/` folders with train/test CSVs for each dataset

## How to Prepare Datasets

### Step 1: Organize Images

Place raw images in their respective folders. Each dataset (eggs, larvae, cysts) contains `images/` and `mask/` subfolders:
```
datasets/
├── eggs/
│   ├── images/
│   │   ├── 000001_00000123.png
│   │   ├── 000002_00000124.png
│   │   └── ...
│   └── mask/
│       ├── 000001_00000123.png
│       ├── 000002_00000124.png
│       └── ...
├── larvae/
│   ├── images/
│   │   └── ...
│   └── mask/
│       └── ...
└── cysts/
    ├── images/
    │   └── ...
    └── mask/
        └── ...
```

### Step 2: Create File List

Generate a `files.txt` for each dataset listing all images from the `images/` folder:
```bash
ls eggs/images/*.png > eggs/files.txt
ls larvae/images/*.png > larvae/files.txt
ls cysts/images/*.png > cysts/files.txt
```

### Step 3: Run Dataset Splitting

Option A - Use the bash script to split all datasets:
```bash
cd datasets/
./run_splits.sh
```

Option B - Split individual datasets (referencing images folder):
```bash
python split_dataset.py eggs/images/*.png eggs/splits 3 2 0.5
python split_dataset.py larvae/images/*.png larvae/splits 3 2 0.5
python split_dataset.py cysts/images/*.png cysts/splits 3 2 0.5
```

### Step 4: Create Incremental Splits (Optional)

Generate incremental data descriptors for each dataset and split:
```bash
python split_incremental.py
```

This creates JSON files for each percentage level (5%, 25%, 50%, 75%, 100%).

## Output Structure

After running split scripts, the structure becomes:

```
datasets/
├── eggs/
│   ├── images/
│   │   └── [PNG image files]
│   ├── mask/
│   │   └── [PNG mask files]
│   ├── files.txt
│   ├── splits/
│   │   ├── train1.csv
│   │   ├── test1.csv
│   │   ├── train2.csv
│   │   ├── test2.csv
│   │   ├── train3.csv
│   │   └── test3.csv
│   └── splits_incremental/
│       └── [JSON descriptors for each percentage]
├── larvae/
│   ├── images/
│   ├── mask/
│   ├── files.txt
│   ├── splits/
│   │   └── [similar structure]
│   └── splits_incremental/
│       └── [similar structure]
└── cysts/
    ├── images/
    ├── mask/
    ├── files.txt
    ├── splits/
    │   └── [similar structure]
    └── splits_incremental/
        └── [similar structure]
```

## Notes

- Images are expected to have labels encoded in filename prefixes (e.g., `000001_00000123.png` → label 0, class 1)
- Sampling strategies ensure representative train/test distributions
- Multiple splits enable k-fold cross-validation
- Incremental percentages help evaluate data efficiency of models