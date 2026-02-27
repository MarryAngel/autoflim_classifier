import os
import subprocess
import shutil
from tqdm import tqdm
import time

# dataset_name = 'eggs'
# num_classes = 8

# dataset_name = 'larvae'
# num_classes = 2

dataset_name = 'cistos'
num_classes = 6

split = [1,2,3]
seed = 42
img_per_class = 1

# Get absolute directories

base_dir = os.path.abspath(os.path.join(os.path.dirname(__file__)))
src_dir = os.path.join(base_dir, 'src')
dataset_dir = os.path.join(base_dir, dataset_name)
build_dir = os.path.join(dataset_dir, 'build')
output_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'extras', 'exp', dataset_name, 'user_marker_all'))

for s in split:
    cmd = f"python src/train_marker_all_user_svm.py {dataset_name} 3 {s}"
    os.system(cmd)

    cmd = f"python src/deploy_flim_svm.py {dataset_name} 3 {s}"
    os.system(cmd)

    # mover os diretórios: results_3_{split} e bag para a pasta de output
    results_dest = os.path.join(output_dir, f'split{s}')
    
    results_src = os.path.join(build_dir, f'results_3_{s}')
    os.makedirs(results_dest, exist_ok=True)
    shutil.move(results_src, results_dest)
    
    results_src = os.path.join(build_dir, 'bag')
    os.makedirs(results_dest, exist_ok=True)
    shutil.move(results_src, results_dest)

