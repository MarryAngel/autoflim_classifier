# Campinas State University (Unicamp)
# Laboratory of Image Data Science (https://lids.ic.unicamp.br/)
# Author: André Igor Nóbrega da Silva
# email :  a203758@dac.unicamp.br
# date  : 2022-07-20
# performs a simple gabor fingerprint enhancement in a fingerprint folder

import sys
import os


from FingerprintEnhancer import FingerprintImageEnhancer
from joblib import Parallel, delayed
import cv2 as cv
import numpy as np
from PIL import Image
import wsq

def enhance_fp(image):
    # img           = np.array(Image.open(image))
    img = cv.imread(image, 0)
    enhancer = FingerprintImageEnhancer()
    enh, bin_enh  = enhancer.enhance(img)

    enh_path     = images_folder + 'enh/' +  image.replace('wsq', 'png').split('/')[-1]
    bin_enh_path = images_folder + 'bin_enh/' +  image.replace('wsq', 'png').split('/')[-1]

    # saving enhanced image
    cv.imwrite(enh_path, enh)

    # saving binary enhanced image
    cv.imwrite(bin_enh_path, bin_enh)

    

def main():
    # Program usage
    if (len(sys.argv) != 3):
        print("Error in register.py. Usage: <P1> <P2>\n")
        print("P1: folder with all fingerprint images")
        print("P2: output folder name")
        exit()
    
    # Creating output directories
    experiment_directory = sys.argv[2] + '/'

    global images_folder
    images_folder = experiment_directory 

    try:
        os.mkdir(sys.argv[2]) # experiment directory
    except FileExistsError:
        print('Warning. Output folder already exists. May overwrite files.')
    
    os.makedirs(images_folder, exist_ok=True)
    os.makedirs(images_folder + '/bin_enh/', exist_ok=True)
    os.makedirs(images_folder + '/enh/', exist_ok=True)


    input_images_folder = sys.argv[1] + '/'
    input_images = os.listdir(input_images_folder)

    Parallel(n_jobs=96)(delayed(enhance_fp)(input_images_folder + image) for image in input_images)
        
if __name__ == '__main__':
    main()
