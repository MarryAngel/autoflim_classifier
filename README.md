# FLIM-based convolutional encoder


## Organization
<!-- Capturar a organização da pasta: tree -L 5 -I "ift|__pycache__|*.pyc" flim_classification (executar em Documents) -->
```bash
flim_classification
├── baselines_networks                  # 
│   ├── datasets                        # Datasets used in base networks
│   │   ├── cistos                      #
│   │   │   ├── images/                 # 
│   │   │   ├── label/                  #
│   │   │   ├── splits/                 #   
│   │   │   ├── splits_incremental/     #
│   │   ├── eggs                        #
│   │   ├── larvae                      #
│   ├── notebooks                       #    
│   ├── src                             #
├── flim_metodology                     #
│   ├── cistos                          #
│   ├── eggs                            #
│   ├── larvae                          #
│   ├── notebooks                       #
│   └── src                             #
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

## Instalation

### Local Setup

```bash
git clone https://github.com/MarryAngel/flim_classification.git
cd flim_classification
python -m venv venv
source venv/bin/activate    
pip install -r requirements.txt
```