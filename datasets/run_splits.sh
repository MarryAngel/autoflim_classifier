#!/bin/bash

echo "Starting dataset splitting process..."

# Define the list of datasets
DATASETS=("cysts" "eggs" "larvae" "test")

# Define common parameters
NUM_SPLITS=3
SAMPLING_TYPE=2
PARAMETER=0.5

# Iterate over each dataset in the list
for DATASET in "${DATASETS[@]}"; do
    INPUT_FILE="${DATASET}/files.txt"
    # Create the output folder with a dynamic name based on the dataset
    OUTPUT_FOLDER="${DATASET}/splits"
    
    # Create the output directory for the current dataset if it doesn't exist
    mkdir -p "${OUTPUT_FOLDER}"
    
    echo "Processing dataset: ${DATASET}"

    # Run the python script
    # python src/split_dataset.py "${INPUT_FILE}" "${OUTPUT_FOLDER}" "${NUM_SPLITS}" "${SAMPLING_TYPE}" "${PARAMETER}"
    python split_dataset.py "${INPUT_FILE}" "${OUTPUT_FOLDER}" "${NUM_SPLITS}" "${SAMPLING_TYPE}" "${PARAMETER}"
    
    echo "Finished processing ${DATASET}."
    echo "-----------------------------------"
done

echo "All datasets have been processed."