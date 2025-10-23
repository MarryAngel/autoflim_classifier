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
    
    result = subprocess.run(cmd, shell=True, capture_output=True, text=True)
    
    elapsed_time = time.time() - start_time
    print(f"✅ Finished in {elapsed_time:.2f}s: {description}")
    
    return result

def rename_and_move_results(random_state, nimage, superpixel, nfeat):
    """Rename result files, append nfeat, and move them to the target directory."""
    
    results_dir = os.path.join(build_dir, 'results_3_1')
    target_dir = os.path.join(base_dir, dataset_name, 'experiments', 'nimages', 'results', f'super{superpixel}_{nimage}images')
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

def main():
    # Experiment parameters
    nimages = [2, 3, 4, 5]
    random_indices = [42, 2735, 6854, 7580, 8900, 123, 456, 789, 1011, 1213]
    superpixel = 50
    split = 1
    nclass = 8
    last_layer = 3

    total = len(nimages) * len(random_indices)

    print(f"🚀 Starting {total} experiments")

    with tqdm(total=total, desc="Experiments", position=0, leave=True) as pbar:
        for i, nimage in enumerate(nimages):
            for j, random_state in enumerate(random_indices):
                current_exp = i * len(random_indices) + j + 1

                # Update progress bar description with current experiment info
                pbar.set_description(f"Exp {current_exp}/{total} - Super{superpixel} Random State{random_state}")

                # Run image selection script
                run_script('choose_images_flim_train.py', 
                          f"{dataset_name} {nclass} {split} {nimage} {random_state}", 
                          f"Image selection (random_state={random_state})")

                # Run training script
                run_script('train_flim_svm.py', 
                          f"{dataset_name} {last_layer} {split} {superpixel}", 
                          f"SVM training (superpixel={superpixel})")

                # Run deploy script and capture output
                result_deploy = run_script('deploy_flim_svm.py',
                                         f"{dataset_name} {last_layer} {split}",
                                         "Deploy and evaluation")
                
                # Extract nfeat from deploy output
                try:
                    nfeat = result_deploy.stdout.split("nfeats: ")[1].split(",")[0]
                except (IndexError, AttributeError):
                    nfeat = "unknown"

                # Rename, append nfeat, and move result files
                rename_and_move_results(random_state, nimage, superpixel, nfeat)

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