"""
split_dataset.py

This script provides functions to split a dataset into train and test sets using different sampling strategies.
It supports random sampling and stratified sampling based on class labels, with options for percentage-based or fixed-number sampling.

Usage:
  python split_dataset.py <input_csv> <output_folder> <num_splits> <sampling_type> <parameter>

Arguments:
  [1] input_csv: CSV file containing all images of the dataset (as created by CreateCSVfileForDataset.py).
  [2] output_folder: Output folder to save the CSV files of the training and testing splits.
  [3] num_splits: Number of train/test splits (folds) to create.
  [4] sampling_type:
    1 = Sampling based on a percentage of images in the smallest class.
    2 = Sampling based on a percentage of images per class (stratified approach).
    3 = Sampling based on the number of images per class (constrained to 0.9*size of the smallest class).
    4 = Random sampling, independently of class information and according to a given percentage of training samples.
  [5] parameter: Percentage of training samples in [0,1] or the number of images per class (for approach 3).
"""

import os
import sys
import numpy as np
import random

def create_random_sampling_file(src, prefix, num_fold, percent):
  """
  Creates random train/test splits from a dataset without considering class labels.

  Args:
    src (str): Path to the input CSV file containing dataset information.
    prefix (str): Output folder path where train/test files will be saved.
    num_fold (int): Number of cross-validation folds to create.
    percent (float): Percentage of data to use for training (0.0 to 1.0).

  Returns:
    int: 1 if successful.
  """
  samples_list = []
  with open(src, "r") as ref_arquivo:
    for linha in ref_arquivo:
      samples_list.append(linha)
  total_samples = len(samples_list)
  train_size = int(total_samples * percent)

  if not os.path.exists(prefix):
    os.makedirs(prefix)

  for i in range(num_fold):
    full_dataset = list(samples_list)
    train_file = os.path.join(prefix, f"train{i+1}.csv")
    test_file = os.path.join(prefix, f"test{i+1}.csv")

    with open(train_file, 'w') as f_train:
      for _ in range(train_size):
        item = full_dataset.pop(random.randrange(len(full_dataset)))
        f_train.write(item)

    with open(test_file, 'w') as f_test:
      for item in full_dataset:
        f_test.write(item)

  return 1

def create_sampling_file(src, prefix, num_fold, percent_minor_class=None, percent_per_class=None, num_images=None):
  """
  Creates stratified train/test splits from a dataset considering class labels.
  Supports three different sampling strategies based on the parameters provided.

  Args:
    src (str): Path to the input CSV file containing dataset information.
    prefix (str): Output folder path where train/test files will be saved.
    num_fold (int): Number of cross-validation folds to create.
    percent_minor_class (float, optional): Percentage of minority class to use for sampling.
    percent_per_class (float, optional): Percentage per class for stratified sampling.
    num_images (int, optional): Fixed number of images per class.

  Returns:
    int: 1 if successful.
  """
  # First pass: determine number of classes
  n_class = 0
  with open(src, "r") as ref_arquivo:
    for linha in ref_arquivo:
      base = os.path.basename(linha)
      f = os.path.splitext(base)[0]
      val = f.split("_")
      label = val[0]
      n_class = max(n_class, int(label))

  hist_label = np.zeros(n_class)
  samples_list = [[] for _ in range(n_class)]

  # Second pass: organize samples by class
  with open(src, "r") as ref_arquivo:
    for linha in ref_arquivo:
      base = os.path.basename(linha)
      f = os.path.splitext(base)[0]
      val = f.split("_")
      label = int(val[0])
      hist_label[label - 1] += 1
      samples_list[label - 1].append(linha)

  minor = int(np.min(hist_label))

  # Determine sampling strategy
  per_class = {}
  if percent_minor_class is not None:
    for i in range(n_class):
      per_class[i] = int(minor * percent_minor_class)
  elif percent_per_class is not None:
    for i in range(n_class):
      per_class[i] = int(hist_label[i] * percent_per_class)
  elif num_images is not None:
    if minor < num_images:
      num_images = int(0.9 * minor)
    for i in range(n_class):
      per_class[i] = num_images

  if not os.path.exists(prefix):
    os.makedirs(prefix)

  for i in range(num_fold):
    full_dataset = [list(class_samples) for class_samples in samples_list]
    train_file = os.path.join(prefix, f"train{i+1}.csv")
    test_file = os.path.join(prefix, f"test{i+1}.csv")

    with open(train_file, 'w') as f_train:
      for j in range(n_class):
        for _ in range(per_class[j]):
          item = full_dataset[j].pop(random.randrange(len(full_dataset[j])))
          f_train.write(item)

    with open(test_file, 'w') as f_test:
      for j in range(n_class):
        for item in full_dataset[j]:
          f_test.write(item)

  return 1

if __name__ == "__main__":
  if len(sys.argv) != 6:
    print("Usage:")
    print("python " + sys.argv[0] + " <input_csv> <output_folder> <num_splits> <sampling_type> <parameter>")
    print("[1] input_csv: CSV file containing all images of the dataset.")
    print("[2] output_folder: Output folder for train/test splits.")
    print("[3] num_splits: Number of splits (folds).")
    print("[4] sampling_type:")
    print("    1 = percentage of images in the smallest class.")
    print("    2 = percentage of images per class (stratified).")
    print("    3 = fixed number of images per class (max 0.9*minority class).")
    print("    4 = random sampling by percentage.")
    print("[5] parameter: Percentage [0,1] or number of images per class (for type 3).")
  else:
    src = sys.argv[1]
    prefix = sys.argv[2]
    folds = int(sys.argv[3])
    sampling_type = int(sys.argv[4])
    param = sys.argv[5]

    if sampling_type == 1:
      result = create_sampling_file(src, prefix, folds, percent_minor_class=float(param))
    elif sampling_type == 2:
      result = create_sampling_file(src, prefix, folds, percent_per_class=float(param))
    elif sampling_type == 3:
      result = create_sampling_file(src, prefix, folds, num_images=int(param))
    else:
      result = create_random_sampling_file(src, prefix, folds, float(param))

    if result == 1:
      print("The train and test files have been successfully created.")
