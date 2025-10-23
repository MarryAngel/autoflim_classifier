"""
Select images from a CSV file for training, choosing a specified number per class.

Usage:
    python choose_images_flim_train.py <num_classes> <split> <num_images_per_class> <seed>

Arguments:
    dataset: Dataset name (e.g., 'cistos', 'eggs', 'larvae').
    num_classes: Number of classes in the dataset.
    split: Split identifier (integer).
    num_images_per_class: Number of images to select per class.
    seed: Random seed for reproducibility.

This script:
    - Reads image names from a CSV file.
    - Selects a specified number of images per class.
    - Copies selected images to a target directory.
    - Logs the selection and seed to a CSV file.
"""

import os
import random
import sys

def parse_args():
    # Parse command line arguments and check usage
    if len(sys.argv) != 6:
        print("Usage: python choose_images_flim_train.py <dataset> <num_classes> <split> <num_images_per_class> <random_state>")
        sys.exit(1)
    dataset = sys.argv[1]
    num_classes = int(sys.argv[2])
    split = int(sys.argv[3])
    num_images_per_class = int(sys.argv[4])
    random_state = int(sys.argv[5])
    return dataset, num_classes, split, num_images_per_class, random_state

def read_image_names(csv_path):
    # Read image names from the CSV file and sort them
    print(f"Reading image names from {csv_path}")
    with open(csv_path, "r") as f:
        image_names = [line.strip() for line in f]
    image_names.sort()
    return image_names

def group_images_by_label(image_names):
    # Group images by their label (assumed to be the first part of the filename)
    labels_to_images = {}
    for name in image_names:
        name = name.split("/")[-1]          # Get the filename only
        label = int(name.split("_")[0])     # Extract label from filename
        labels_to_images.setdefault(label, []).append(name)
    return labels_to_images

def select_images(labels_to_images, num_classes, num_images_per_class, random_state):
    # Select a specified number of images per class using the given random state
    selected_images = []
    for class_idx in range(1, num_classes + 1):
        if class_idx in labels_to_images:
            random.seed(random_state)
            selected = random.sample(labels_to_images[class_idx], num_images_per_class)
            selected_images.extend(selected)
        else:
            print(f"Class {class_idx} not found in the dataset.")
            sys.exit(1)
    return selected_images

def copy_csv_files(split, dataset):
    # Copy train and test CSV files for the given split
    os.makedirs(f"{dataset}/build/splits", exist_ok=True)
    os.system(f"cp {dataset}/build/splits/train{split}.csv {dataset}/build/train{split}.csv")
    os.system(f"cp {dataset}/build/splits/test{split}.csv {dataset}/build/test{split}.csv")

def remove_existing_train_dir(split, dataset):
    # Remove existing train directory for the split, if it exists
    train_dir = f"{dataset}/build/train{split}"
    if os.path.exists(train_dir):
        os.system(f"rm -rf {train_dir}")

def copy_selected_images(selected_images, split, dataset):
    # Copy selected images to the output directory for the split
    output_dir = f"{dataset}/build/train{split}"
    os.makedirs(output_dir, exist_ok=True)
    for image in selected_images:
        src = f"{dataset}/build/images/{image}"
        dst = os.path.join(output_dir, image)
        os.system(f"cp {src} {dst}")

def log_selected_images(selected_images, random_state, dataset):
    # Log selected images and seed to a CSV file
    log_path = f"{dataset}/build/output/images_seed{random_state}.csv"
    if not os.path.exists(f"{dataset}/build/output"):
        os.makedirs(f"{dataset}/build/output", exist_ok=True)
    if not os.path.exists(log_path):
        open(log_path, "w").close()
    with open(log_path, "a") as f:
        f.write(f"seed: {random_state}\n")
        for image in selected_images:
            f.write(f"{image}\n")

def main():
    # Main workflow: parse args, select images, copy files, and log selection
    dataset, num_classes, split, num_images_per_class, random_state = parse_args()
    csv_path = f"{dataset}/build/splits/train{split}.csv"    
    image_names = read_image_names(csv_path)
    labels_to_images = group_images_by_label(image_names)
    selected_images = select_images(labels_to_images, num_classes, num_images_per_class, random_state)
    copy_csv_files(split, dataset)
    remove_existing_train_dir(split, dataset)
    copy_selected_images(selected_images, split, dataset)
    # log_selected_images(selected_images, random_state, dataset)
    print(f"Images selected and copied to {dataset}/build/train{split} with random_state {random_state}.")

if __name__ == "__main__":
    main()
