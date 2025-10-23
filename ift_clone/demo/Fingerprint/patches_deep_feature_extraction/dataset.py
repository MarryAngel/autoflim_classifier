import glob
import torch
from torch_snippets import *

# import pyift.pyift as ift

class PatchDataset(Dataset):
    def __init__(self, image_path, transform=None):

        self.image_path = image_path
        self.patches    = ift.ReadMImage(self.image_path).AsNumPy().squeeze(0)
        self.nitems     = self.patches.shape[-1]

        self.transform = transform


    def __getitem__(self, ix):
        patch = self.patches[: , :, ix].astype('uint8')

        if self.transform:
            patch = self.transform(patch)
        return patch

    def __len__(self):
        return self.nitems
    

class PatchDatasetPython(Dataset):
    def __init__(self, patches_path, image, transform=None):

        self.patches_path = patches_path
        self.patches      = glob.glob(patches_path + f'/{image}*')

        self.image        = image
        self.nitems       = len(self.patches)

        self.transform = transform


    def __getitem__(self, ix):
        patch = f'{self.patches_path}/{self.image}_p{ix}.png'
        patch = read(patch)

        if self.transform:
            patch = self.transform(patch)
        return patch

    def __len__(self):
        return self.nitems

prep_python = transforms.Compose([
    transforms.ToPILImage(),
    transforms.ToTensor(),
    transforms.Normalize((0), (1))
])

prep = transforms.Compose([
    transforms.ToPILImage(),
    transforms.Resize((128,128), interpolation = transforms.InterpolationMode.BILINEAR, max_size = None, antialias = True),
    transforms.ToTensor(),
    transforms.Normalize((0), (1))
])