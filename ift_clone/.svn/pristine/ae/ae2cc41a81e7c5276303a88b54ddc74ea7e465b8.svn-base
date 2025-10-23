import sys
import os
import glob
import time

import torch
# import pyift.pyift as ift
import numpy as np
from torch_snippets import *


from dataset import PatchDataset, prep, PatchDatasetPython, prep_python
from architecture import SiameseNetwork

def save_feature_vector(feature_vector, filename):
    np.save(filename, feature_vector)


device = 'cuda' if torch.cuda.is_available() else 'cpu' # colab offers limited gpu acess

if __name__ == '__main__':
    if len(sys.argv) != 4:
        print("usage: patches_deep_feature_extraction.py <P1> <P2> <P3>")
        print("<P1>: patches folder path")
        print("<P2>: pre trained deep learning model path")
        print("<P3>: output path with extracted features")
        exit(0)
    
    start_time = time.time()

    ################################# Reading input variables ############################

    mimgs_folder = sys.argv[1]
    model_path   = sys.argv[2]
    output_path  = sys.argv[3]
    os.makedirs(output_path, exist_ok=True)

    model = SiameseNetwork().to(device)
    model.load_state_dict(torch.load(model_path))
    model.eval()

    # mimgs = glob.glob(mimgs_folder + '/*.mimg')

    ## BASE P FORMAT
    mimgs = glob.glob(mimgs_folder + '/*.png')
    mimgs = [item.split('/')[-1].split('_')[0] for item in mimgs]
    mimgs = sorted(list(set(mimgs)))

    # SD 27 FORMAT

    # mimgs = glob.glob(mimgs_folder + '/*.png')
    # mimgs = [item.split('/')[-1].rsplit('_', 1)[0] for item in mimgs]
    # mimgs = sorted(list(set(mimgs)))

    
    ################################# Extracting patch features ###########################

    for i, mimg_path in enumerate(mimgs):
        # image_name = mimg_path.split('/')[-1].replace('.mimg', '')
        image_name = mimg_path
        print('Progress: {}/{}'.format(i + 1, len(mimgs)))
        # print(mimgs_folder, image_name)
        dataset    = PatchDatasetPython(mimgs_folder, image_name, prep_python)
        dataloader = DataLoader(dataset, batch_size = len(dataset), shuffle = False)

        for ix, data in enumerate(dataloader):
            with torch.no_grad():
                patches  = data.to(device)
                features = model(patches)
                
                # detach from GPU and normalize

                features = features.detach().cpu().numpy()
                features = (features / np.linalg.norm(features, axis = 1).reshape(-1,1))
                save_feature_vector(features, output_path + f'/{image_name}') 


    print("Feature extraction finished with sucess in {}s".format(time.time() - start_time))






