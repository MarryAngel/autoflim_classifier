# FLIM-based convolutional encoder


## Organization
<!-- Capturar a organização da pasta: tree -L 5 -I "ift|__pycache__|*.pyc" flim_classification (executar em Documents) -->

```bash
    flim_classification
    ├── baselines_networks                  # Baseline models and main experiments
    │   ├── notebooks                       # Jupyter notebooks for training, analysis and testing    
    │   ├── src                             # Implementation of models and utilities
    ├── datasets                            # Datasets used in this work
    │   ├── cysts                           # Data related to cysts parasites                       
    │   │   ├── images/                     # Original input images
    │   │   ├── label/                      # Corresponding masks/labels
    │   │   ├── splits/                     # Fixed train/test splits
    │   │   ├── splits_incremental/         # Progressive splits for incremental experiment
    │   ├── eggs                            # Equivalent structure for eggs
    │   ├── larvae                          # Equivalent structure for larvae
    │   ├── datasets_parasites.zip          # Compressed parasites dataset
    │   ├── run_splits.py                   # Created all splits
    │   ├── split_dataset.py                # Created splits
    ├── flim_metodology                     # FLIM-based convoluctional encoder
    │   ├── cistos                          # Experiments related to cyst parasites
    │   ├── eggs                            # Experiments related to eggs
    │   ├── larvae                          # Experiments related to larvae
    │   └── src                             # Implementation of FLIM-based encoder and utilities
    ├── .gitignore                          # Git ignore configuration
    ├── requirements.txt                    # Project dependencies
    ├── README.md                           # Project documentation 
    └── LICENSE                             # Project license
```

## Dataset

This work uses the intestinal parasites dataset developed by LIDS Laboratory at UNICAMP, which comprises images of multiple helminth classes (eggs and larvae) and protozoan cysts. 

The dataset is publicly available at https://github.com/LIDS-UNICAMP/intestinal-parasites-datasets. 

Unlike the original split provided by the repository, our experiments employ three independent dataset partitions, where each partition has a 50-50 split between training and testing samples. Both the original images and their corresponding segmentation masks are utilized throughout the experiments, noting that the segmentation masks are not present in the original repository.

## Libraries installation

Para utilizar a biblioteca IFT, realize o seguinte procedimento:
1. Abra o terminal na pasta `ift`  (eu renomeei de `svnift` para `ift`) e dê um  `make`  no terminal. Após isso a biblioteca será compilada e aparecerá no final a mensagem “*libift.a built … //  DONE.*” Caso queira rodar com GPU, execute o seguinte comando: `IFT_GPU=1 make`
    Pode ser que seja necessário executar `sudo apt install nvidia-cuda-toolkit` para ter o nvcc e `sudo apt install libatlas-base-dev`
2. Agora vamos arrumar o arquivo `.bashrc` . Para isso, no terminal digite `nano ~/.bashrc` . Vai até o final do arquivo que aparecerá no terminal e adicione as seguintes linhas, lembrando de alterar o caminho do diretório da ift. Após aplicar as alterações, salve o arquivo e digite no terminal `source ~/.bashrc`  para atualizar o arquivo. Se quiser pode fechar e reabrir o terminal por garantia que as novas configurações serão aplicadas.

#ift
export PATH=$PATH:/data_lids/home/maria/Documents/ift/bin
export NEWIFT_DIR=/data_lids/home/maria/Documents/ift
export IFT_DIR=/data_lids/home/maria/Documents/ift
#export IFT_GPU=1

3. após ter compilado, execute o arquivo compile.sh no terminal da seguinte forma: ./compile.sh all (caso precise, compile ele com o comando chmod +x compile.sh)

4. caso a compilação tenha dado certo, diversos programas irão aparecer dentro da pasta ift/bin


## Instalation

### Local Setup

```bash
git clone https://github.com/MarryAngel/flim_classification.git
cd flim_classification
python -m venv venv
source venv/bin/activate    
pip install -r requirements.txt
```