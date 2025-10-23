import torch
from torch_snippets import *

def convBlock(ni, no):
    return nn.Sequential(
        nn.Conv2d(ni, no, kernel_size=3, padding=1, bias=False), #, padding_mode='reflect'),
        nn.BatchNorm2d(no),
        nn.ReLU(inplace=True),
        nn.MaxPool2d(kernel_size=3, stride=2, padding=1)
    )

class SiameseNetwork(nn.Module):
    def __init__(self):
        super(SiameseNetwork, self).__init__()
        self.features = nn.Sequential(
            convBlock(1,16),
            convBlock(16,32),
            convBlock(32,64),
            nn.Dropout(0.5),
            nn.Flatten(),
            nn.Linear(64*16*16, 256), nn.ReLU(inplace=True),
            nn.Linear(256, 128), nn.ReLU(inplace=True),
            nn.Linear(128, 64)
        )

    def forward(self, input1):
        output1 = self.features(input1)
        return output1
