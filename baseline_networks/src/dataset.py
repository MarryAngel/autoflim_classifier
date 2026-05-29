import os
import json
from PIL import Image

from config import get_dataset_paths, get_split_path_incremental

from torch.utils.data import Dataset, DataLoader

def pil_loader(image_path: str) -> Image.Image:
    """
    Load an image from disk and convert to RGB.

    Parameters
    ----------
    image_path : str
        Full path to the image file.

    Returns
    -------
    Image.Image
        PIL Image object in RGB format.
    """
    with open(image_path, "rb") as f:
        img = Image.open(f)
        return img.convert("RGB")


class DatasetParasite(Dataset):
    """
    Custom PyTorch Dataset for parasite classification tasks.

    Reads a JSON file containing train/validation/test splits with image filenames.
    Labels are extracted from the filename prefix (e.g., "000001_00000112.png" → label 1).
    
    Attributes
    ----------
    dataset_name : str
        Name of the dataset (e.g., 'eggs', 'larvae', 'cysts').
    set : str
        Type of split: 'training', 'validation', or 'test'.
    split : int
        Split index (1, 2, 3, etc.).
    percentage : float
        Data percentage used (1, 5, 25, 50, 75, 100).
    transform : callable, optional
        Function to apply augmentations/preprocessing to images.
    samples : list[str]
        Full paths to all images in the current split.

    Example
    -------
    >>> dataset = DatasetParasite(
    ...     dataset_name='eggs',
    ...     set='training',
    ...     split=1,
    ...     percentage=25,
    ...     transform=transforms_compose
    ... )
    >>> img, label = dataset[0]
    """

    def __init__(
        self,
        dataset_name: str,
        set: str,
        split: int,
        percentage: float,
        transform=None
    ):
        """
        Initialize the dataset.

        Parameters
        ----------
        dataset_name : str
            Name of the dataset registered in config.DATASETS.
        set : str
            Data split type: 'training', 'validation', or 'test'.
        split : int
            Split index (used for k-fold splitting).
        percentage : float
            Percentage of data to use (e.g., 25 for 25%).
        transform : callable, optional
            Optional PyTorch transforms (resize, normalize, augmentations).

        Raises
        ------
        FileNotFoundError
            If the split JSON file does not exist.
        """
        self.dataset_name = dataset_name
        self.set = set
        self.split = split
        self.percentage = percentage
        self.transform = transform

        # Get the path to the split JSON file
        split_path_json = get_split_path_incremental(dataset_name, split, percentage)
        
        if not os.path.isfile(split_path_json):
            raise FileNotFoundError(f"Split JSON not found: {split_path_json}")

        # Load the JSON file containing train/val/test splits
        with open(split_path_json, "r", encoding="utf-8") as f:
            data = json.load(f)

        # Get filenames for the current set (train/val/test)
        filenames = data[self.set]
        
        # Get the base directory where images are stored
        dataset_paths = get_dataset_paths(dataset_name)
        base_dir = dataset_paths['images']
        
        # Create full paths for all samples
        self.samples = [os.path.join(base_dir, p) for p in filenames]

    def __len__(self) -> int:
        """
        Return the number of samples in the dataset.

        Returns
        -------
        int
            Total number of images in this split.
        """
        return len(self.samples)

    def __getitem__(self, idx: int) -> tuple:
        """
        Get a single sample from the dataset.

        Parameters
        ----------
        idx : int
            Index of the sample to retrieve.

        Returns
        -------
        tuple
            Tuple of (image, label) where:
            - image: PIL Image or torch.Tensor (if transform applied)
            - label: int, class label (0-indexed)
        """
        path = self.samples[idx]
        img_name = os.path.basename(path)
        
        # Extract label from filename (prefix before first underscore)
        # e.g., "000001_00000112.png" → label 1 → 0-indexed → 0
        label = int(img_name.split("_")[0])
        label -= 1  # Convert to 0-indexed
        
        # Load image
        img = pil_loader(path)
        
        # Apply transforms if provided
        if self.transform:
            img = self.transform(img)
        
        return img, label


class DataModuleParasite:
    """
    Data module for managing train/validation/test DataLoaders.

    Encapsulates dataset creation and DataLoader generation for the parasite
    classification pipeline. Supports lazy initialization of datasets.

    Attributes
    ----------
    dataset_name : str
        Dataset identifier ('eggs', 'larvae', 'cysts').
    split : int
        K-fold split index.
    percentage : float
        Data percentage to use.
    batch_size : int
        Batch size for DataLoaders.
    num_workers : int
        Number of workers for parallel data loading.
    transform : callable, optional
        PyTorch transforms to apply to images.
    train_dataset : DatasetParasite or None
        Training dataset (created on demand).
    val_dataset : DatasetParasite or None
        Validation dataset (created on demand).
    test_dataset : DatasetParasite or None
        Test dataset (created on demand).

    Example
    -------
    >>> dm = DataModuleParasite(
    ...     dataset_name='eggs',
    ...     split=1,
    ...     percentage=25,
    ...     batch_size=32,
    ...     num_workers=4,
    ...     transform=transforms_compose
    ... )
    >>> dataloaders = dm.get_dataloaders()
    >>> for batch_imgs, batch_labels in dataloaders['train']:
    ...     print(batch_imgs.shape, batch_labels.shape)
    """

    def __init__(
        self,
        dataset_name: str,
        split: int,
        percentage: float,
        batch_size: int = 32,
        num_workers: int = 4,
        transform=None
    ):
        """
        Initialize the data module.

        Parameters
        ----------
        dataset_name : str
            Dataset name registered in config.DATASETS.
        split : int
            Split index for k-fold validation.
        percentage : float
            Data percentage to use.
        batch_size : int, optional
            Batch size for DataLoaders (default: 32).
        num_workers : int, optional
            Number of worker processes for data loading (default: 4).
        transform : callable, optional
            Torchvision transforms to apply to images.
        """
        self.dataset_name = dataset_name
        self.split = split
        self.percentage = percentage
        self.batch_size = batch_size
        self.num_workers = num_workers
        self.transform = transform

        # Lazy-initialized datasets
        self.train_dataset = None
        self.val_dataset = None
        self.test_dataset = None

    def setup(self) -> None:
        """
        Create train/validation/test datasets.

        This method can be called explicitly or will be called automatically
        when accessing dataloaders. Datasets are created with the configured
        parameters.
        """
        self.train_dataset = DatasetParasite(
            self.dataset_name,
            "training",
            split=self.split,
            percentage=self.percentage,
            transform=self.transform
        )
        self.val_dataset = DatasetParasite(
            self.dataset_name,
            "validation",
            split=self.split,
            percentage=self.percentage,
            transform=self.transform
        )
        self.test_dataset = DatasetParasite(
            self.dataset_name,
            "test",
            split=self.split,
            percentage=self.percentage,
            transform=self.transform
        )

    def _ensure_datasets(self) -> None:
        """
        Ensure datasets are initialized before accessing them.

        Calls setup() if any dataset is None (lazy initialization).
        """
        if self.train_dataset is None or self.val_dataset is None or self.test_dataset is None:
            self.setup()

    def train_dataloader(self, batch_size: int = None, shuffle: bool = True) -> DataLoader:
        """
        Get training DataLoader.

        Parameters
        ----------
        batch_size : int, optional
            Override the default batch size. If None, uses self.batch_size.
        shuffle : bool, optional
            Whether to shuffle data (default: True for training).

        Returns
        -------
        DataLoader
            PyTorch DataLoader for training set.
        """
        self._ensure_datasets()
        bs = batch_size or self.batch_size
        return DataLoader(
            self.train_dataset,
            batch_size=bs,
            shuffle=shuffle,
            num_workers=self.num_workers
        )

    def val_dataloader(self, batch_size: int = None, shuffle: bool = False) -> DataLoader:
        """
        Get validation DataLoader.

        Parameters
        ----------
        batch_size : int, optional
            Override the default batch size.
        shuffle : bool, optional
            Whether to shuffle data (default: False for validation).

        Returns
        -------
        DataLoader
            PyTorch DataLoader for validation set.
        """
        self._ensure_datasets()
        bs = batch_size or self.batch_size
        return DataLoader(
            self.val_dataset,
            batch_size=bs,
            shuffle=shuffle,
            num_workers=self.num_workers,
            pin_memory=True
        )

    def test_dataloader(self, batch_size: int = None, shuffle: bool = False) -> DataLoader:
        """
        Get test DataLoader.

        Parameters
        ----------
        batch_size : int, optional
            Override the default batch size.
        shuffle : bool, optional
            Whether to shuffle data (default: False for test).

        Returns
        -------
        DataLoader
            PyTorch DataLoader for test set.
        """
        self._ensure_datasets()
        bs = batch_size or self.batch_size
        return DataLoader(
            self.test_dataset,
            batch_size=bs,
            shuffle=shuffle,
            num_workers=self.num_workers,
            pin_memory=True
        )

    def get_dataloaders(self) -> dict[str, DataLoader]:
        """
        Get all DataLoaders at once.

        Returns
        -------
        dict[str, DataLoader]
            Dictionary with keys 'train', 'val', 'test' containing respective DataLoaders.

        Example
        -------
        >>> dataloaders = dm.get_dataloaders()
        >>> train_loader = dataloaders['train']
        >>> val_loader = dataloaders['val']
        >>> test_loader = dataloaders['test']
        """
        return {
            "train": self.train_dataloader(),
            "val": self.val_dataloader(),
            "test": self.test_dataloader(),
        }