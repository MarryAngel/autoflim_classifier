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
    """Runs a Python script with arguments and returns the result."""
    
    script_path = os.path.join(src_dir, script_name)
    if script_name == 'iftPrototypeEvaluation':
        # script_path = os.path.join(base_dir, dataset_name, 'build', script_name)
        script_path = script_name
        print(f"Running iftPrototypeEvaluation from {script_path}")
        cmd = f"{script_path} {args}"
    else:
        cmd = f"python {script_path} {args}"

    print(f"🔄 Running: {description} - {script_name}")
    start_time = time.time()
    
    result = subprocess.run(cmd, shell=True, capture_output=True, text=True)
    
    elapsed_time = time.time() - start_time
    print(f"✅ Finished in {elapsed_time:.2f}s: {description}")
    
    return result

def rename_and_move_results(random_state, nimage, superpixel, nfeat, technique, split):
    """Renames result files, adds nfeat, and moves them to the target directory."""

    results_dir = os.path.join(build_dir, f'results_3_{split}')
    images_dir = os.path.join(build_dir, f'train{split}')
    target_dir = os.path.join(base_dir, dataset_name, 'experiments', 'train_fiveImage', f'split{split}', f'{nimage}')
    target_image_dir = os.path.join(target_dir, f'train{split}')
    os.makedirs(target_dir, exist_ok=True)

    # Rename files
    classified_img = f'layer3_test{split}-classified-images.csv'
    results_csv = f'results.csv'
    os.rename(os.path.join(results_dir, f'layer3_test{split}-classified-images.csv'), os.path.join(results_dir, classified_img))
    os.rename(os.path.join(results_dir, 'results.csv'), os.path.join(results_dir, results_csv))

    # Append nfeat to results file
    results_path = os.path.join(results_dir, results_csv)
    with open(results_path, 'a') as f:
        f.write(f"\nnfeat: {nfeat}")

    # Move files to target directory
    shutil.move(os.path.join(results_dir, classified_img), 
                os.path.join(target_dir, classified_img))
    shutil.move(os.path.join(results_dir, results_csv), 
                os.path.join(target_dir, results_csv))
    shutil.copytree(images_dir, target_image_dir, dirs_exist_ok=True)


def experiment_pipeline(random_state, technique, distance_flag, nimages_final, superpixel, nclass, split, last_layer, output_dir):
    """Runs the experiment pipeline for a given technique and random_state."""
    nimages = 1
    # print(f"🔷 Starting experiments for random_state {random_state} with {technique} distance")
    
    # Initial image selection and training
    run_script('choose_images_flim_train.py', f"{dataset_name} {nclass} {split} {nimages} {random_state}", f"Image selection (random_state={random_state})")
    run_script('train_flim_svm.py', f"{dataset_name} {last_layer} {split} {superpixel}", f"SVM training (superpixel={superpixel})")
    
    # Capturar avaliação para 1 imagem
    result_deploy = run_script('deploy_flim_svm.py', f"{dataset_name} {last_layer} {split}", "Deploy and evaluation")
    try:
        nfeat = result_deploy.stdout.split("nfeats: ")[1].split(",")[0]
    except (IndexError, AttributeError):
        nfeat = "unknown"
    rename_and_move_results(random_state, nimages, superpixel, nfeat, technique, split)

    while nimages < nimages_final:
        path = os.path.join(base_dir, dataset_name, 'build')
        run_script('iftPrototypeEvaluation', f"{path}/layer{last_layer}_train{split}.zip {path}/train.csv {path}/{output_dir} {distance_flag}", f"Prototype evaluation ({technique})")
        run_script('add_images_train_flim.py', f"{dataset_name} {nclass} {split} {output_dir}", "Add most similar images")
        nimages += 1
        print(f"🔄 Re-training SVM with {nimages} images")
        run_script('train_flim_svm.py', f"{dataset_name} {last_layer} {split} {superpixel}", f"SVM training (superpixel={superpixel})")
    
        if nimages < nimages_final:
            result_deploy = run_script('deploy_flim_svm.py', f"{dataset_name} {last_layer} {split}", "Deploy and evaluation")
            try:
                nfeat = result_deploy.stdout.split("nfeats: ")[1].split(",")[0]
            except (IndexError, AttributeError):
                nfeat = "unknown"
            rename_and_move_results(random_state, nimages, superpixel, nfeat, technique, split)

    # Last deploy
    result_deploy = run_script('deploy_flim_svm.py', f"{dataset_name} {last_layer} {split}", "Deploy and evaluation")
    try:
        nfeat = result_deploy.stdout.split("nfeats: ")[1].split(",")[0]
    except (IndexError, AttributeError):
        nfeat = "unknown"
        
    rename_and_move_results(random_state, nimages, superpixel, nfeat, technique, split)

def main():
    # Experiment parameters
    random_indices = [123]
    superpixel = 50
    nclass = 6
    last_layer = 3
    split = 3
    nimages_final = 5
    
    output_dir = 'misclassified.csv'
    techniques = [("cossine", 0)]
    total_experiments = len(random_indices) * len(techniques) * nimages_final
    experiments_done = 0

    print(f"🚀 Starting {total_experiments} experiments for Cosine distances")

    with tqdm(total=total_experiments, desc="Experiments", position=0, leave=True) as pbar:
        for i, random_state in enumerate(random_indices):
            for j, (technique, flag) in enumerate(techniques):
                experiment_pipeline(
                    random_state=random_state,
                    technique=technique,
                    distance_flag=flag,
                    nimages_final=nimages_final,
                    superpixel=superpixel,
                    nclass=nclass,
                    split=split,
                    last_layer=last_layer,
                    output_dir=output_dir
                )
                experiments_done += 1
                percent = int((experiments_done / total_experiments) * 100)
                pbar.n = experiments_done
                pbar.set_postfix_str(f"{percent}%")
                pbar.refresh()
                print(f"Experiments remaining: {total_experiments - experiments_done}")

    print("🎉 All experiments completed!")

if __name__ == "__main__":
    main()
