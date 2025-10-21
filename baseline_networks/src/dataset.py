from config import get_dataset_paths

from torch.utils.data import Dataset, DataLoader

class BuildDataset(Dataset):
    def __init__(self, dataset_name, split, percentage):
        self.dataset_name = dataset_name
        self.split = split
        
    
    def __len__():
        pass
    
    def __getitem__():
        pass
        
def create_dataloader(dataset_name, split, percentage):
    
    dataset = BuildDataset(
        dataset_name = dataset_name,
        split = split
    )
    
    pass