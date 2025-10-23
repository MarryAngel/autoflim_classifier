import numpy as np
from scipy import ndimage, signal
from sklearn.decomposition import PCA
from sklearn.preprocessing import StandardScaler
from sklearn.linear_model import LinearRegression
import cv2
import argparse
import os

import matplotlib.pyplot as plt



M = 16
N = 16

def estimate_direction_by_Sobel(im):
    bin = np.int8(im > 127)

    gradient_sigma = 1
    sze = np.fix(6*gradient_sigma)
    if np.remainder(sze,2) == 0:
        sze = sze+1

    gauss = cv2.getGaussianKernel(int(sze),gradient_sigma)
    f = gauss * gauss.T

    fy,fx = np.gradient(f)                               #Gradient of Gaussian

    Gx = signal.convolve2d(bin, fx, mode='same')
    Gy = signal.convolve2d(bin, fy, mode='same')

    Gxx = np.power(Gx,2)
    Gyy = np.power(Gy,2)
    Gxy = Gx*Gy

    block_sigma = 7

    #Now smooth the covariance data to perform a weighted summation of the data.
    sze = np.fix(6*block_sigma)

    gauss = cv2.getGaussianKernel(int(sze), block_sigma)
    f = gauss * gauss.T  

    Gxx = ndimage.convolve(Gxx,f)
    Gyy = ndimage.convolve(Gyy,f)
    Gxy = 2*ndimage.convolve(Gxy,f)

    # Analytic solution of principal direction
    denom = np.sqrt(np.power(Gxy,2) + np.power((Gxx - Gyy),2)) + np.finfo(float).eps

    thetas = np.zeros_like(bin)
    # try:
    sin2theta = Gxy/denom                   # Sine and cosine of doubled angles
    cos2theta = (Gxx-Gyy)/denom

    # values_x, counts_x = np.unique(tile_x, return_counts=True)
    # values_y, counts_y = np.unique(tile_y, return_counts=True)
    
    # most_frequent = values_x[np.argmax(counts_x)]

    # thetas = np.pi/2 + np.arctan2(sin2theta, cos2theta)/2

    # values, counts = np.unique(thetas.flatten(), return_counts=True)
    # theta = values[np.argmax(counts)]


    orient_smooth_sigma = 7
    sze = np.fix(6*orient_smooth_sigma)
    if np.remainder(sze,2) == 0:
        sze = sze+1
    gauss = cv2.getGaussianKernel(int(sze), orient_smooth_sigma)
    f = gauss * gauss.T
    cos2theta = ndimage.convolve(cos2theta,f)                   # Smoothed sine and cosine of
    sin2theta = ndimage.convolve(sin2theta,f)                   # doubled angles

    thetas = np.pi/2 + np.arctan2(sin2theta,cos2theta)/2


    return thetas


# Function to validate the input directory
def validate_input_directory(path):
    if not os.path.isdir(path):
        raise argparse.ArgumentTypeError(f"The directory {path} does not exist.")
    return path

# Function to validate the output type
def validate_output_type(value):
    try:
        value = int(value)
        if value not in [0, 1]:
            raise ValueError
    except ValueError:
        raise argparse.ArgumentTypeError("Output type must be 0 for .png or 1 for .txt.")
    return value

# Function to validate the output directory
def validate_output_directory(path):
    if not os.path.exists(path):
        os.makedirs(path)
        print(f"Directory '{path}' created.")
    return path

# Main function to parse arguments
def parse_arguments():
    parser = argparse.ArgumentParser(description="Process input and output details.")

    # Input directory argument
    parser.add_argument(
        "input_directory",
        type=validate_input_directory,
        help="Path to the input directory containing .png files."
    )

    # Output type argument
    parser.add_argument(
        "output_type",
        type=validate_output_type,
        help="Output type: 0 for .png, 1 for .txt."
    )

    # Output directory argument
    parser.add_argument(
        "output_directory",
        type=validate_output_directory,
        help="Path to the output directory."
    )

    return parser.parse_args()

def dummy_function(image_path):
    print(f"Processing image: {image_path}")

# Function to process all .png images in the input directory
def process_images(input_directory, out_type, out_dir):
    for index, filename in enumerate(os.listdir(input_directory)):
        if filename.endswith(".png"):
            print(index)
            file_path = os.path.join(input_directory, filename)
            out_filepath = os.path.join(out_dir, filename)
            if out_type == 1:
                out_filepath = out_filepath.replace(".png", ".dir")

            im = cv2.imread(file_path, 0)
            thetas = estimate_direction_by_Sobel(im)

            tiles = [im[x:x+M,y:y+N] for x in range(0,im.shape[0],M) for y in range(0,im.shape[1],N)]
            
            # Write to image file (.png)
            if out_type == 0:
                orient_vis = np.empty_like(thetas)
                cv2.normalize(thetas, orient_vis, 0, 255, cv2.NORM_MINMAX)
                cv2.imwrite(out_filepath, orient_vis)

            # Write to text file (.dir)
            elif out_type == 1:
                tile_i = 0
                tile_orientations = [0 for _ in tiles]
                for x in range(0,im.shape[0],M):
                    for y in range(0,im.shape[1],N):
                        block = thetas[x:x+M,y:y+N]
                    # block = block[block != 0]
                    tile_thetas = np.round(block, 2)
                    values, counts = np.unique(tile_thetas, return_counts=True)
                    if len(counts) < 1:
                        most_common = 0
                    else:
                        most_common = values[np.argmax(counts)]

                    # most_common = np.mean(tile_thetas.flatten())

                    tile_orientations[tile_i] = most_common
                    tile_i += 1
                
                i = 0
                for x in range(0,im.shape[0],M):
                    for y in range(0,im.shape[1],N):
                        print(round(tile_orientations[i]/np.pi * 180), end=" ")
                        i += 1
                    print()



# Example usage
if __name__ == "__main__":
    args = parse_arguments()
    print("Input Directory:", args.input_directory)
    print("Output Type:", ".png" if args.output_type == 0 else ".txt")
    print("Output Directory:", args.output_directory)

    process_images(args.input_directory, args.output_type, args.output_directory)

        
