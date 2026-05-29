# arquivo que irá gerar os arquivos data_descriptor_percX.json que estará dentro de DATASETS/split_incremental/split{split}/layer{layer}

#formato do arquivo json final
# {
#     "training": [
#         "000001_00000341.png",
#         "000002_00000342.png",
#         ...],
#     "validation": [],
#     "test": []
# }

import os
import json  

percentages = [5, 25, 50, 75, 100]



