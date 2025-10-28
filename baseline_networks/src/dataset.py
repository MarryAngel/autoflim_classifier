import os
import json
from PIL import Image

from config import get_dataset_paths
from config import get_split_path_incremental

from torch.utils.data import Dataset, DataLoader

def pil_loader(image_path):
    with open(image_path, "rb") as f:
        img = Image.open(f)
        return img.convert("RGB")

class DatasetParasite(Dataset):
    """
    Dataset que lê um JSON com chaves ('training'|'validation'|'test')
    contendo listas de caminhos (ex: "000001_00000112.png").
    Os caminhos no JSON serão combinados com o diretório do arquivo JSON.
    Retorna (image, path) por item. Pode passar transform se desejar.
    """
    def __init__(self, dataset_name, set, split, percentage, transform=None):
        self.dataset_name = dataset_name
        self.set = set
        self.split = split
        self.percentage = percentage
        self.transform = transform

        split_path_json = get_split_path_incremental(dataset_name, split, percentage)
        if not os.path.isfile(split_path_json):
            raise FileNotFoundError(f"Split JSON not found: {split_path_json}")

        with open(split_path_json, "r", encoding="utf-8") as f:
            data = json.load(f)

        filenames = data[self.set]
        dataset_paths = get_dataset_paths(dataset_name)
        base_dir = dataset_paths['images']  
        
        self.samples = [os.path.join(base_dir, p) for p in filenames]

    def __len__(self):
        return len(self.samples)

    def __getitem__(self, idx):
        path = self.samples[idx]
        img_name = os.path.basename(path)
        label = int(img_name.split("_")[0])
        img = pil_loader(path)
        
        if self.transform:
            img = self.transform(img)
        
        return img, label

class DataModuleParasite:
    """
    Gerencia DataLoaders de train/val/test usando DatasetParasite e
    torch.utils.data.DataLoader.

    Uso:
      dm = DataModuleParasite(dataset_name, percentage, batch_size=32, num_workers=4, transform=...)
      dm.setup()  # opcional — os datasets também são criados automaticamente ao chamar os dataloaders
      train_loader = dm.train_dataloader()
      val_loader = dm.val_dataloader()
      test_loader = dm.test_dataloader()

    Também existe get_dataloaders() que retorna um dict {'train','val','test'}.
    """
    def __init__(self, dataset_name, split, percentage, batch_size=32, num_workers=4, transform=None):
        self.dataset_name = dataset_name
        self.split = split
        self.percentage = percentage
        self.batch_size = batch_size
        self.num_workers = num_workers
        self.transform = transform

        self.train_dataset = None
        self.val_dataset = None
        self.test_dataset = None

    def setup(self):
        """Cria os datasets (chamável explicitamente se desejar)."""
        self.train_dataset = DatasetParasite(self.dataset_name, "training", split=self.split, percentage=self.percentage, transform=self.transform)
        self.val_dataset = DatasetParasite(self.dataset_name, "validation", split=self.split, percentage=self.percentage, transform=self.transform)
        self.test_dataset = DatasetParasite(self.dataset_name, "test", split=self.split, percentage=self.percentage, transform=self.transform)

    def _ensure_datasets(self):
        if self.train_dataset is None or self.val_dataset is None or self.test_dataset is None:
            self.setup()

    def train_dataloader(self, batch_size=None, shuffle=True):
        """Retorna DataLoader para training (shuffle=True por default)."""
        self._ensure_datasets()
        bs = batch_size or self.batch_size
        return DataLoader(self.train_dataset, batch_size=bs, shuffle=shuffle, num_workers=self.num_workers)

    def val_dataloader(self, batch_size=None, shuffle=False):
        """Retorna DataLoader para validation (shuffle=False por default)."""
        self._ensure_datasets()
        bs = batch_size or self.batch_size
        return DataLoader(self.val_dataset, batch_size=bs, shuffle=shuffle, num_workers=self.num_workers, pin_memory=True)

    def test_dataloader(self, batch_size=None, shuffle=False):
        """Retorna DataLoader para test (shuffle=False por default)."""
        self._ensure_datasets()
        bs = batch_size or self.batch_size
        return DataLoader(self.test_dataset, batch_size=bs, shuffle=shuffle, num_workers=self.num_workers, pin_memory=True)

    def get_dataloaders(self):
        """Retorna dict com {'train','val','test'} DataLoaders."""
        return {
            "train": self.train_dataloader(),
            "val": self.val_dataloader(),
            "test": self.test_dataloader(),
        }