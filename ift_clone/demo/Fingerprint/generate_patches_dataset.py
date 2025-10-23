# Campinas State University (Unicamp)
# Laboratory of Image Data Science (https://lids.ic.unicamp.br/)
# Author: André Igor Nóbrega da Silva
# email :  a203758@dac.unicamp.br
# date  : 2022-11-21
# generates a patch dataset of base P

import sys
import os

import numpy as np
from PIL import Image, ImageDraw
import glob

from utils import read_mnt_file

def main():
    # Program usage
    if (len(sys.argv) != 5):
        print("Error in generate_patches_dataset.py. Usage: <P1> <P2> <P3> <P4>\n")
        print("P1: folder with all fingerprint images (.png)")
        print("P2: folder with all mintuias files (.mnt)")
        print("P3: patch size (int)")
        print("P4: output folder name")
        exit()
    
    # reading input args
    images = sorted(glob.glob(sys.argv[1] + '/*.png'))
    mnts   = sorted(glob.glob(sys.argv[2] + '/*.mnt'))

    if len(images) != len(mnts):
        print("Error in generate_pathes_dataset.py:\n number of images must be equal to number of minutias")
        exit(0)
    
    patch_size = int(sys.argv[3])

    # Creating output directories
    output_dir = sys.argv[4] + '/'

    try:
        os.mkdir(sys.argv[4]) # experiment directory
    except FileExistsError:
        print('Warning. Output folder already exists. May overwrite files.')
    
    os.makedirs(output_dir, exist_ok=True)

    # Extracting patches from images
    for i in range(len(images)):
        # print(images[i])
        if i % 10 == 0:
            print("Processing image {}/{}".format(i + 1, len(images)))
        image_mnts = read_mnt_file(mnts[i])
        for j in range(len(image_mnts)):
            x, y, theta = image_mnts[j]


            # before rotating, we need to crop a patch a little bigger than the desired, so we dont leave artefacts in the patch
            theta_max = np.pi/4 # the worst artefact happens when theta is equal to 45 degrees

            # solving x_max and y_max for theta_max
            adjustment_space = np.ceil(int(patch_size) * np.cos(theta_max))
            x_min, x_max     = x - adjustment_space, x + adjustment_space
            y_min, y_max     = y - adjustment_space, y + adjustment_space

            # x_min, x_max = x - int(patch_size / 2), x + int(patch_size / 2)
            # y_min, y_max = y - int(patch_size / 2), y + int(patch_size / 2)
            image = Image.open(images[i]).crop((x_min, y_min, x_max, y_max))

            # Normalize according to angle (mnt always points in direction of positive x-axis)
            image = image.rotate(theta * 180 / np.pi, Image.BILINEAR)
            # cropping again to the desired patch size
            x_crop, y_crop = image.size

            # getting the patch center
            x_crop, y_crop = int(x_crop/2), int(y_crop/2)

            x_crop_min, x_crop_max = x_crop - int(patch_size/2), x_crop + int(patch_size/2)
            y_crop_min, y_crop_max = y_crop - int(patch_size/2), y_crop + int(patch_size/2)

            image = image.crop((x_crop_min, y_crop_min, x_crop_max, y_crop_max))

            # Saving patch
            image_name = images[i].split('.')[-2].split('/')[-1]
            patch_name = output_dir + image_name + f'_p{j}.png'
            image.save(patch_name)
        

if __name__ == '__main__':
    main()
