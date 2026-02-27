import os
import sys
import shutil

# Receive input parameters
if len(sys.argv) != 3:
    print("Usage: python user_marker.py <dataset> <reduction markers>")
    print("Datasets available: eggs, larvae, ...")
    print("Reduction markers: true (center pixel) or false (all markers)")
    exit()
    
dataset_name = str(sys.argv[1])
reduction_markers = sys.argv[2].lower() 

if dataset_name not in ['eggs', 'larvae', 'cistos']:
    print(f"Dataset {dataset_name} not recognized. Available datasets: eggs, larvae, cistos.")
    exit()
elif dataset_name == 'eggs':
    num_classes = 8
elif dataset_name == 'larvae':
    num_classes = 2
elif dataset_name == 'cistos':
    num_classes = 6

print(f"Dataset: {dataset_name}")
print(f"Reduction markers: {reduction_markers}")

split = [1,2,3]
seed = 42
img_per_class = 1

# Get absolute directories
base_dir = os.path.abspath(os.path.join(os.path.dirname(__file__)))
src_dir = os.path.join(base_dir, 'src')
dataset_dir = os.path.join(base_dir, dataset_name)
build_dir = os.path.join(dataset_dir, 'build')

if reduction_markers == 'true':
    output_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'extras', 'exp', dataset_name, 'user_selected_markers', 'reduction_markers'))
else:
    output_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'extras', 'exp', dataset_name, 'user_selected_markers', 'all_markers'))

os.makedirs(output_dir, exist_ok=True)

for s in split:
    cmd = f"python src/train_user_markers_svm.py {dataset_name} 3 {s} {reduction_markers}"
    os.system(cmd)

    cmd = f"python src/deploy_flim_svm.py {dataset_name} 3 {s}"
    os.system(cmd)

    # move the directories: results_3_{split}    
    results_dest = os.path.join(output_dir, f'split{s}')
    
    results_src = os.path.join(build_dir, f'results_3_{s}')
    os.makedirs(results_dest, exist_ok=True)
    shutil.move(results_src, results_dest)
    
    # copy the directories: bag to the output folder 
    results_src = os.path.join(build_dir, 'bag')
    os.makedirs(results_dest, exist_ok=True)
    shutil.copytree(results_src, os.path.join(results_dest, 'bag'))


