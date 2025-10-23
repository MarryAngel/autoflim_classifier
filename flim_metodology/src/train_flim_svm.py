import os
import sys

# Receive input parameters
if len(sys.argv) != 5:
    print("Usage: python train_flim_svm.py <dataset> <last_encoder_layer> <split> <num_superpixels>")
    print("Datasets available: eggs, larvae, ...")
    print("last_encoder_layer: Last encoder layer (int)")
    print("split: Split number (int)")
    print("num_superpixels: Number of superpixels (int)")
    exit()
    
dataset_name = str(sys.argv[1])
layer = int(sys.argv[2])
split = int(sys.argv[3])
num_superpixels = int(sys.argv[4])

# Set build directory and change working directory
build_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', dataset_name, 'build'))
os.chdir(build_dir)

# Remove previously created directories and files
cmd = f"rm -rf bag flim layer[0-{layer}] layer{layer}_train{split} superpixels train.csv seeds_files.txt"
os.system(cmd)

# Create train.csv with selected images in train{split} folder
cmd = f"ls -v train{split}/* >> train.csv"
os.system(cmd)

# Create superpixels for selected images
with open("train.csv", "r") as f:
    for line in f:
        print(line)
        file_in = line.strip()
        basename = file_in.split("/")[1].split(".")[0]
        ext = file_in.split("/")[1].split(".")[1].strip()
        print(f"Processing {basename}")
        file_out = f"superpixels/{basename}.{ext}"
        file_mask = f"masks/{basename}.{ext}"
        if os.path.exists("./masks"):
            print("Using masks")
            # iftDISF <input image> <initial seeds> <final superpixels> <output label image> <mask image>
            cmd = f"iftDISF {file_in} 1000 {num_superpixels} {file_out} {file_mask}"
        else:
            # iftDISF <input image> <initial seeds> <final superpixels> <output label image>
            cmd = f"iftDISF {file_in} 1000 {num_superpixels} {file_out}"
        os.system(cmd)

# Create bag directory with superpixel centers
# iftSeedsFromSuperpixels <superpixel images dir> <output seeds dir> <seed type> <connectivity>
cmd = "iftSeedsFromSuperpixels superpixels bag 1"
os.system(cmd)

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
