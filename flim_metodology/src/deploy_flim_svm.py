import os
import sys

# Check for correct number of input arguments
if len(sys.argv) != 4:
    print("python deploy_flim_svm.py <P1> <P2> <P3>")
    print("P1: Datset name (eggs, larvae, ...)")
    print("P2: last encoder layer")
    print("P3: split")
    exit()

dataset_name = str(sys.argv[1])
layer = int(sys.argv[2])
split = int(sys.argv[3])

# Set build directory and change working directory
build_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', dataset_name, 'build'))
os.chdir(build_dir)

# Remove previously created directories and files
remove_cmd = "rm -rf layer[0-{}] layer{}_test{}".format(layer, layer, split)
os.system(remove_cmd)

# Create the layer0 folder with selected test images
convert_cmd = "iftConvertImagesToMImages test{}.csv layer0".format(split)
os.system(convert_cmd)

# Run the encoder convolution block for each layer using the configured model on all test images
for i in range(1, layer + 1):
    encode_cmd = "iftEncodeMergedLayer arch2D.json {} flim".format(i)
    os.system(encode_cmd)

# Move the folder layer{layer} to layer{layer}_test{split}
move_cmd = "mv layer{} layer{}_test{}".format(layer, layer, split)
os.system(move_cmd)

# Prepare the dataset for classification by creating a .zip file with the processed data
activ_cmd = "iftActivDataSet layer{}_test{} images 2 1".format(layer, split)
os.system(activ_cmd)

# Classify the data using the SVM classifier and store accuracy and kappa in the results file
svm_cmd = "iftClassifyBySVM layer{}_test{}.zip svm_{}_{}.zip results_{}_{}".format(layer, split, layer, split, layer, split)
os.system(svm_cmd)
