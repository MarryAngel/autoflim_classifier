
## Organization

```bash
flim_metodology
├── cistos                              # Baseline models and main experiments
│   ├── build                           # Datasets used in base networks
│   ├── experiments 
│   ├── notebooks 
│   │   ├── cistos                      # Data related to cyst parasites
├── eggs                                # Equivalent structure for eggs
├── larvae                              # Equivalent structure for larvae
├── src                                 # Implementation of FLIM-based encoder and utilities
└── README.md                           # Documentation 
```

## Informações

Para rodar os experimentos, a pasta build tem que conter:
- arquivo "images" com as imagens originais
- arquivo "masks" com as máscaras de segmentação dos parasitas
- arquivo "filex.txt" com o nome de todas as imagens 
- arquivo "arch2D.json" com a arquitetura do encoder
- arquivos "train1" e o seu equivalente "test1" que são a particação dos dados (no caso está sendo trabalhado com 3 splits - eles estão presentes na pasta splits)
- arquivo train1 (train2 ou train3) com as imagens selecionadas para realizar o treinamento [pode usar o programa choose_images_flim_train.py para selecionar um conjunto de imagens iniciais] 

## Arquivos gerados quando treina
 - bag: dicionário de pontos centrais para cada superpixel
 - flim: 
 - layer0, layer1, layer2, ...:
 - layer3_train1: 
 - superpixels: 
 - layer3_train1_id_image.txt
 - layer3_train1.zip
 - seeds_files.txt
 - train.csv: arquivo csv com as imagens de treino que estão na pasta train1
 - svm_3_1.zip