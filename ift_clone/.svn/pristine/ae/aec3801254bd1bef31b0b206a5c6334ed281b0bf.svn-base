"""
binarization of segmented images from FingerNet
"""

# Author:  Walter Casas <w187990@dac.unicamp.br>
#          Laboratory of Image Data Science [https://lids.ic.unicamp.br/]
#          Campinas State University [UNICAMP]
# License: BSD 3 clause


import os
import cv2
import numpy as np
import matplotlib.pyplot as plt
from skimage.io import imread, imsave



def binarize_image(folderIn, folderOut='out', suffixNameIn='_enh', suffixNameOut='_out', thresh=127):
    
    images = [img for img in os.listdir(folderIn) if suffixNameIn in img] 
    
    for image in images:
        img = imread(os.path.join(folderIn, image))
        _, thresh_binary = cv2.threshold(img, thresh, 255, cv2.THRESH_BINARY)
        name = image.replace(suffixNameIn, suffixNameOut)
        
        if not os.path.exists(folderOut):
            os.makedirs(folderOut)
        
        imsave(os.path.join(folderOut, name), thresh_binary)