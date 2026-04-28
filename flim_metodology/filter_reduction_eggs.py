"""
Filter reduction training pipeline for eggs dataset.
Trains SVM with filter selection across all splits.
"""
import os
import shutil

dataset_name = 'eggs'
num_classes = 9
splits = [1, 2, 3]
nsuperpixels = [50]
seed = 42
img_per_class = 1

# Get absolute directories
base_dir = os.path.abspath(os.path.dirname(__file__))
dataset_dir = os.path.join(base_dir, dataset_name)
build_dir = os.path.join(dataset_dir, 'build')
output_dir = os.path.abspath(os.path.join(base_dir, '..', 'extras', 'exp', dataset_name, 'filter_reduction'))

print(f"Dataset: {dataset_name}")
print(f"Number of classes: {num_classes}")
print(f"Splits: {splits}")
print(f"Output directory: {output_dir}")
print()

for num_superpixels in nsuperpixels:
    for s in splits:
        print("=" * 80)
        print(f"Processing: split={s}, nsuperpixels={num_superpixels}")
        print("=" * 80)
        
        # Step 1: Choose images (already selected from all available)
        print(f"\n[1/3] Choosing images for split {s}...")
        cmd = f"cd {base_dir} && python src/choose_images_flim_train.py {dataset_name} {num_classes} {s} {img_per_class} {seed}"
        ret = os.system(cmd)
        if ret != 0:
            print(f"ERROR in choose_images step")
            continue
        
        # Step 2: Train with filter reduction
        print(f"\n[2/3] Training with filter reduction for split {s}...")
        cmd = f"cd {base_dir} && python src/train_flim_svm_filter_reduction.py {dataset_name} 3 {s} {num_superpixels}"
        ret = os.system(cmd)
        if ret != 0:
            print(f"ERROR in train_flim_svm_filter_reduction step")
            continue
        
        # Step 3: Deploy SVM (evaluate on test set)
        print(f"\n[3/3] Deploying SVM for split {s}...")
        cmd = f"cd {base_dir} && python src/deploy_flim_svm.py {dataset_name} 3 {s}"
        ret = os.system(cmd)
        if ret != 0:
            print(f"ERROR in deploy_flim_svm step")
            continue
        
        # Move results to organized output directory
        print(f"\nMoving results to {output_dir}...")
        results_dest = os.path.join(output_dir, f'super{num_superpixels}', f'split{s}')
        
        moves = [
            (os.path.join(build_dir, f'results_3_{s}'), 'results'),
            (os.path.join(build_dir, 'superpixels'), 'superpixels'),
            (os.path.join(build_dir, 'bag'), 'bag'),
            (os.path.join(build_dir, 'train.csv'), 'train.csv'),
            (os.path.join(build_dir, f'train{s}'), f'train{s}'),
        ]
        
        for src, name in moves:
            if os.path.exists(src):
                os.makedirs(results_dest, exist_ok=True)
                dst = os.path.join(results_dest, name)
                try:
                    if os.path.isdir(src):
                        shutil.rmtree(dst, ignore_errors=True)
                        shutil.move(src, dst)
                    else:
                        os.makedirs(results_dest, exist_ok=True)
                        shutil.move(src, dst)
                    print(f"  Moved: {name}")
                except Exception as e:
                    print(f"  WARNING: Could not move {name}: {e}")

print("\n" + "=" * 80)
print("Filter reduction training complete!")
print(f"Results saved to: {output_dir}")
print("=" * 80)
