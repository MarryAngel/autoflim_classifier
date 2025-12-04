import os
import subprocess
import shutil
from tqdm import tqdm
import time

dataset_name = 'cistos'
num_classes = 6
split = [1,2,3]
nsuperpixels = [5, 15, 25, 50, 75, 100, 150, 200]
nsuperpixels = [50]
seed = 42
img_per_class = 1

# Get absolute directories

base_dir = os.path.abspath(os.path.join(os.path.dirname(__file__)))
src_dir = os.path.join(base_dir, 'src')
dataset_dir = os.path.join(base_dir, dataset_name)
build_dir = os.path.join(dataset_dir, 'build')
output_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'extras', 'exp', dataset_name, 'nsuperpixels_reduction'))

for num_superpixels in nsuperpixels:
    for s in split:
        cmd = f"python src/choose_images_flim_train.py {dataset_name} {num_classes} {s} {img_per_class} {seed}"
        os.system(cmd)

        cmd = f"python src/train_flim_svm_filter_reduction.py {dataset_name} 3 {s} {num_superpixels}"
        os.system(cmd)
        
        cmd = f"python src/deploy_flim_svm.py {dataset_name} 3 {s}"
        os.system(cmd)
        
        # mover os diretórios: results_3_{split}, superpixels, bag e o arquivo train.csv para a ásta de output
        results_dest = os.path.join(output_dir, f'super{num_superpixels}', f'split{s}')
        
        results_src = os.path.join(build_dir, f'results_3_{s}')
        os.makedirs(results_dest, exist_ok=True)
        shutil.move(results_src, results_dest)
        
        results_src = os.path.join(build_dir, 'superpixels')
        os.makedirs(results_dest, exist_ok=True)
        shutil.move(results_src, results_dest)

        results_src = os.path.join(build_dir, 'bag')
        os.makedirs(results_dest, exist_ok=True)
        shutil.move(results_src, results_dest)

        results_src = os.path.join(build_dir, 'train.csv')
        os.makedirs(results_dest, exist_ok=True)
        shutil.move(results_src, results_dest)

        results_src = os.path.join(build_dir, f'train{s}')
        os.makedirs(results_dest, exist_ok=True)
        shutil.move(results_src, results_dest)

        # shutil.copytree(results_src, results_dest, dirs_exist_ok=True)