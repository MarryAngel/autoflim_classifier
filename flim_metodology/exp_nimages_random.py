import os
import sys
import shutil
from tqdm import tqdm

# Receive input parameters
if len(sys.argv) != 2:
    print("Usage: python exp_nimages_random.py <dataset_name>")
    print("Datasets available: eggs, larvae, cistos")
    exit()
    
dataset_name = str(sys.argv[1])

if dataset_name not in ['eggs', 'larvae', 'cistos']:
    print(f"Dataset {dataset_name} not recognized. Available datasets: eggs, larvae, cistos.")
    exit()
elif dataset_name == 'eggs':
    num_classes = 8
    nsuperpixels = [25, 50]
elif dataset_name == 'larvae':
    num_classes = 2
    nsuperpixels = [25, 50, 150,200]
elif dataset_name == 'cistos':
    num_classes = 6
    nsuperpixels = [25, 50]

splits = [1, 2, 3]
img_per_class_init = 1
img_per_class_final = 5
seed = 42

# Get absolute directories
base_dir = os.path.abspath(os.path.join(os.path.dirname(__file__)))
src_dir = os.path.join(base_dir, 'src')
dataset_dir = os.path.join(base_dir, dataset_name)
build_dir = os.path.join(dataset_dir, 'build')
output_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'extras', 'exp', dataset_name, 'nimages_random'))


def mover_resultados(split, num_superpixel, nimage):
    """Move os resultados para a pasta de output"""
    results_dest = os.path.join(output_dir, f'super{num_superpixel}', f'split{split}', f'imgperclass{nimage}')
    os.makedirs(results_dest, exist_ok=True)
    
    # Mover results_3_{split}
    results_src = os.path.join(build_dir, f'results_3_{split}')
    shutil.move(results_src, results_dest)
    
    # Copiar train.csv
    train_csv_src = os.path.join(build_dir, 'train.csv')
    shutil.copy(train_csv_src, results_dest)
    
    # Copiar train{split}
    train_split_src = os.path.join(build_dir, f'train{split}')
    shutil.copytree(train_split_src, f'{results_dest}/train{split}', dirs_exist_ok=True)


def executar_pipeline_inicial(dataset_name, num_classes, split, img_per_class, seed, num_superpixel):
    """Executa o pipeline de treinamento inicial"""
    os.system(f"python src/choose_images_flim_train.py {dataset_name} {num_classes} {split} {img_per_class} {seed}")
    os.system(f"python src/train_flim_svm.py {dataset_name} 3 {split} {num_superpixel}")
    os.system(f"python src/deploy_flim_svm.py {dataset_name} 3 {split}")


# Loop principal com tqdm
total_iterations = len(nsuperpixels) * len(splits) * img_per_class_final
pbar = tqdm(total=total_iterations, desc="Processamento geral")

for num_superpixel in nsuperpixels:
    for split in splits:
        for nimage in range(img_per_class_init, img_per_class_final + 1):
            print(f"✓ Iniciando: Superpixels={num_superpixel}, Split={split}, Imagens por classe={nimage}")
            pbar.set_description(f"Super{num_superpixel} Split{split} - Inicial")
            executar_pipeline_inicial(dataset_name, num_classes, split, nimage, seed, num_superpixel)
            mover_resultados(split, num_superpixel, nimage)
            pbar.update(1)
        

pbar.close()
print("✓ Processamento concluído!")