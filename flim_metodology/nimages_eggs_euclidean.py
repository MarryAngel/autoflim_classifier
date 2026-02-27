import os
import shutil
from tqdm import tqdm

dataset_name = 'eggs'
num_classes = 8
splits = [1, 2, 3]
nsuperpixels = [25, 50]
img_per_class_init = 1
img_per_class_final = 5
seed = 42

# Get absolute directories
base_dir = os.path.abspath(os.path.join(os.path.dirname(__file__)))
src_dir = os.path.join(base_dir, 'src')
dataset_dir = os.path.join(base_dir, dataset_name)
build_dir = os.path.join(dataset_dir, 'build')
output_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'extras', 'exp', dataset_name, 'nimages_euclidean_noredution'))


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


def adicionar_imagem_e_retreinar(dataset_name, num_classes, split, num_superpixel):
    """Adiciona uma imagem e retreina o modelo"""
    # Avaliação de protótipos
    p1 = f"{dataset_name}/build/layer3_train{split}.zip"
    p2 = f"{dataset_name}/build/train.csv"
    p3 = f"{dataset_name}/build/misclassified.csv"
    p4 = 1
    os.system(f"iftPrototypeEvaluation {p1} {p2} {p3} {p4}")
    
    # Adicionar imagem
    os.system(f"python src/add_images_train_flim.py {dataset_name} {num_classes} {split} misclassified.csv")
    
    # Retreinamento
    os.system(f"python src/train_flim_svm.py {dataset_name} 3 {split} {num_superpixel}")
    os.system(f"python src/deploy_flim_svm.py {dataset_name} 3 {split}")


# Loop principal com tqdm
total_iterations = len(nsuperpixels) * len(splits) * img_per_class_final
pbar = tqdm(total=total_iterations, desc="Processamento geral")

for num_superpixel in nsuperpixels:
    for split in splits:
        # Treinamento inicial com 1 imagem por classe
        pbar.set_description(f"Super{num_superpixel} Split{split} - Inicial")
        executar_pipeline_inicial(dataset_name, num_classes, split, img_per_class_init, seed, num_superpixel)
        mover_resultados(split, num_superpixel, 1)
        pbar.update(1)
        
        # Adicionar imagens incrementalmente (de 2 até img_per_class_final)
        for nimage in range(2, img_per_class_final + 1):
            pbar.set_description(f"Super{num_superpixel} Split{split} - Img {nimage}")
            adicionar_imagem_e_retreinar(dataset_name, num_classes, split, num_superpixel)
            mover_resultados(split, num_superpixel, nimage)
            pbar.update(1)

pbar.close()
print("✓ Processamento concluído!")