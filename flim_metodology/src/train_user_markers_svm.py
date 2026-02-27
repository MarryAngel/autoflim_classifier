import os
import sys

# Receive input parameters
if len(sys.argv) != 5:
    print("Usage: python train_marker_user_svm.py <dataset> <last_encoder_layer> <split> <reduction_markers>")
    print("Datasets available: eggs, larvae, ...")
    print("last_encoder_layer: Last encoder layer (int)")
    print("split: Split number (int)")
    print('reduction markers: true (center pixel) or false (all markers)')
    exit()
    
dataset_name = str(sys.argv[1])
layer = int(sys.argv[2])
split = int(sys.argv[3])
reduction_markers = sys.argv[4].lower()

# Set build directory and change working directory
build_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', dataset_name, 'build'))
os.chdir(build_dir)

# Remove previously created directories and files
cmd = f"rm -rf bag flim layer[0-{layer}] layer{layer}_train{split} superpixels train.csv seeds_files.txt"
os.system(cmd)

# retrieve the manual markings in dir_marker and copy to bag folder
if not os.path.exists("bag"):
    os.makedirs("bag")

# list files in dir_marker
dir_marker = f"../../../flim_builder/{dataset_name}/split{split}"

# copy files from dir_marker to bag
for file in os.listdir(dir_marker):
    if file.endswith("-seeds.txt"):
        src = os.path.join(dir_marker, file)
        dst = os.path.join("bag", file)
        os.system(f"cp {src} {dst}")

# open each file in bag to reduce the number of points
if reduction_markers == 'true':
    for file in os.listdir("bag"):
        # open the txt file and read the lines
        if file.endswith("-seeds.txt"):
            all_lines = []
            with open(os.path.join("bag", file), "r") as f:
                all_lines = f.readlines()
            qtd_markers, dim1, dim2 = all_lines[0].split(" ")
            # reduce the number of markers to 1 (center pixel)
            data_lines = all_lines[1:]
            groups = {}
            for i in range(0, len(data_lines), 5):
                idx = i//5+1
                group = data_lines[i:i+5]
                if len(group) >= 3:
                    groups[idx] = tuple(int(x) for x in group[2].split())
                else:
                    groups[idx] = None
            # rewrite the file with the new number of markers and the center pixel information
            new_qtd_markers = len(groups)
            with open(os.path.join("bag", file), "w") as f:
                f.write(f"{new_qtd_markers} {dim1} {dim2}")
                for idx, info in groups.items():
                    if info is not None:
                        f.write(f"{info[0]} {info[1]} {info[2]} {info[3]} {info[4]}\n")


# Create seeds_files.txt with files in bag directory
cmd = "ls -v bag >> seeds_files.txt"
os.system(cmd)

with open("seeds_files.txt", "r") as f:
    # Rename files in bag directory from name_image-seeds.txt to name_image-fpts.txt
    for line in f:
        file1 = line.strip()
        basename = file1.split("-")[0]
        file2 = f"{basename}-fpts.txt"
        cmd = f"mv -f bag/{file1} bag/{file2}"
        os.system(cmd)

# Convert all training images to MImage and put them in layer0 directory
cmd = f"iftConvertImagesToMImages train{split}.csv layer0"
os.system(cmd)

# Create layer directories and flim directory
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
