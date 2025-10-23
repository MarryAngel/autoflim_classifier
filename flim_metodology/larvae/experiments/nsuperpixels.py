import os
import subprocess
import shutil
from tqdm import tqdm
import time

# Get absolute directories
current_file_path = os.path.abspath(__file__)
dataset_name = os.path.basename(os.path.dirname(os.path.dirname(current_file_path)))

base_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
src_dir = os.path.join(base_dir, 'src')

dataset_dir = os.path.join(base_dir, dataset_name)
build_dir = os.path.join(dataset_dir, 'build')

def run_script(script_name, args, description=""):
    """Run a Python script with arguments and return the result."""
    
    script_path = os.path.join(src_dir, script_name)
    cmd = f"python {script_path} {args}"
    
    print(f"🔄 Running: {description} - {script_name}")
    start_time = time.time()
    
    result= subprocess.run(cmd, shell=True, capture_output=True, text=True)
    
    elapsed_time = time.time() - start_time
    print(f"✅ Finished in {elapsed_time:.4f}s: {description}")
    
    return result
    
def rename_and_move_results(random_state, superpixel, nfeat):
    """Rename result files, append nfeat, and move them to the target directory."""
    
    results_dir = os.path.join(build_dir, 'results_3_1')
    target_dir = os.path.join(base_dir, dataset_name, 'experiments', 'nsuperpixels', 'results', f'super{superpixel}')
    os.makedirs(target_dir, exist_ok=True)

    # Rename files
    classified_img = f'seed{random_state}_layer3_test1-classified-images.csv'
    results_csv = f'seed{random_state}_results.csv'
    os.rename(os.path.join(results_dir, 'layer3_test1-classified-images.csv'),
              os.path.join(results_dir, classified_img))
    os.rename(os.path.join(results_dir, 'results.csv'),
              os.path.join(results_dir, results_csv))

    # Append nfeat to results file
    results_path = os.path.join(results_dir, results_csv)
    with open(results_path, 'a') as f:
        f.write(f"\nnfeat: {nfeat}")

    # Move files to target directory
    shutil.move(os.path.join(results_dir, classified_img),
                os.path.join(target_dir, classified_img))
    shutil.move(os.path.join(results_dir, results_csv),
                os.path.join(target_dir, results_csv))

def copy_superpixels_folder(random_state, superpixel):
    """Copy the superpixels folder to the target directory, renaming it with the random_state."""

    src_superpixels = os.path.join(build_dir, 'superpixels')
    dest_dir = os.path.join(base_dir, 'experiments', 'nsuperpixels', 'results', f'super{superpixel}', f'superpixels_seed{random_state}')
    os.makedirs(dest_dir, exist_ok=True)
    for item in os.listdir(src_superpixels):
        s = os.path.join(src_superpixels, item)
        d = os.path.join(dest_dir, item)
        if os.path.isdir(s):
            shutil.copytree(s, d, False, None)
        else:
            shutil.copy2(s, d)

def main():
    # Experiment parameters
    superpixels = [5, 15, 25, 50, 75, 100, 150, 200]  
    random_indices = [42, 2735, 6854, 7580, 8900, 123, 456, 789, 1011, 1213]
    split = 1
    num_classes = 2
    num_images_per_class = 1
    last_encoder = 3  

    total = len(superpixels) * len(random_indices)

    print(f"🚀 Starting {total} experiments ({len(superpixels)} superpixels × {len(random_indices)} seeds)")

    with tqdm(total=total, desc="Experiments", position=0, leave=True) as pbar:
        for i, superpixel in enumerate(superpixels):
            for j, random_state in enumerate(random_indices):
                current_exp = i * len(random_indices) + j + 1

                # Update progress bar description with current experiment info
                pbar.set_description(f"Exp {current_exp}/{total} - Super{superpixel} Random State{random_state}")
                
                # Run image selection script
                run_script('choose_images_flim_train.py', 
                          f"{dataset_name} {num_classes} {split} {num_images_per_class} {random_state}", 
                          f"Image selection (random_state={random_state})")

                # Run training script
                run_script('train_flim_svm.py', 
                          f"{dataset_name} {last_encoder} {split} {superpixel}", 
                          f"SVM training (superpixel={superpixel})")

                # Run deploy script and capture output
                result_deploy = run_script('deploy_flim_svm.py', 
                                         f"{dataset_name} {last_encoder} {split}", 
                                         "Deploy and evaluation")
                
                # Extract nfeat from deploy output
                try:
                    nfeat = result_deploy.stdout.split("nfeats: ")[1].split(",")[0]
                except (IndexError, AttributeError):
                    nfeat = "unknown"

                # Rename, append nfeat, and move result files
                rename_and_move_results(random_state, superpixel, nfeat)

                # Copy superpixels folder with random_state in name
                copy_superpixels_folder(random_state, superpixel)

                # Update progress
                pbar.update(1)
                
                # Show completion status
                remaining = total - current_exp
                pbar.set_postfix({
                    'Remaining': remaining,
                    'Super': superpixel,
                    'Random State': random_state,
                    'nfeat': nfeat
                })

    print("🎉 All experiments have been completed!")

if __name__ == "__main__":
    main()