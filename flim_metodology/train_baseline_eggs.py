"""
Baseline training pipeline for eggs dataset (WITHOUT filter reduction).
Trains SVM without filter selection across all splits.
Used for comparison with filter_reduction results.
"""
import os
import shutil

def train_baseline_inline(build_dir, split, num_superpixels):
    """Inline baseline training without filter reduction"""
    os.chdir(build_dir)
    layer = 3
    
    # Remove previously created directories and files
    cmd = f"rm -rf bag flim layer[0-{layer}] layer{layer}_train{split} superpixels train.csv seeds_files.txt"
    os.system(cmd)
    
    # Create train.csv with selected images in train{split} folder
    cmd = f"ls -v train{split}/* >> train.csv"
    os.system(cmd)
    
    # Create superpixels for selected images
    with open("train.csv", "r") as f:
        for line in f:
            file_in = line.strip()
            basename = file_in.split("/")[1].split(".")[0]
            ext = file_in.split("/")[1].split(".")[1].strip()
            file_out = f"superpixels/{basename}.{ext}"
            file_mask = f"masks/{basename}.{ext}"
            if os.path.exists("./masks"):
                cmd = f"iftDISF {file_in} 1000 {num_superpixels} {file_out} {file_mask}"
            else:
                cmd = f"iftDISF {file_in} 1000 {num_superpixels} {file_out}"
            os.system(cmd)
    
    # Create bag directory with superpixel centers
    cmd = "iftSeedsFromSuperpixels superpixels bag 1"
    os.system(cmd)
    
    # Create seeds_files.txt with files in bag directory
    cmd = "ls -v bag >> seeds_files.txt"
    os.system(cmd)
    
    with open("seeds_files.txt", "r") as f:
        for line in f:
            file1 = line.strip()
            basename = file1.split("-")[0]
            file2 = f"{basename}-fpts.txt"
            cmd = f"mv -f bag/{file1} bag/{file2}"
            os.system(cmd)
    
    # Convert all training images to MImage and put them in layer0 directory
    cmd = f"iftConvertImagesToMImages train{split}.csv layer0"
    os.system(cmd)
    
    # Create layer directories WITHOUT filter reduction (baseline)
    for i in range(1, layer + 1):
        cmd = f"iftCreateLayerModel bag arch2D.json {i} flim"
        os.system(cmd)
        cmd = f"iftMergeLayerModels arch2D.json {i} flim"
        os.system(cmd)
        cmd = f"iftEncodeMergedLayer arch2D.json {i} flim"
        os.system(cmd)
    
    # Move layerN to layerN_train{split}
    cmd = f"mv layer{layer} layer{layer}_train{split}"
    os.system(cmd)
    
    # Create layerN_train{split}_id_image.txt and layerN_train{split}.zip
    cmd = f"iftActivDataSet layer{layer}_train{split} images 1 1"
    os.system(cmd)
    
    # Train SVM and create svm_{layer}_{split}.zip
    cmd = f"iftSupTrainBySVM layer{layer}_train{split}.zip 0 0 1e2 0 svm_{layer}_{split}.zip"
    os.system(cmd)

# Main execution
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
output_dir = os.path.abspath(os.path.join(base_dir, '..', 'extras', 'exp', dataset_name, 'baseline'))

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
        
        # Step 2: Train WITHOUT filter reduction (baseline)
        print(f"\n[2/3] Training WITHOUT filter reduction (baseline) for split {s}...")
        cmd = f"cd {build_dir} && python ../../src/train_flim_svm_baseline.py {dataset_name} 3 {s} {num_superpixels}"
        ret = os.system(cmd)
        if ret != 0:
            print(f"ERROR in train_flim_svm_baseline step - trying alternative method...")
            # Run inline baseline training
            train_baseline_inline(build_dir, s, num_superpixels)
        
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
print("Baseline training complete!")
print(f"Results saved to: {output_dir}")
print("=" * 80)

