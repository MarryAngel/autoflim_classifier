#!/bin/bash

# Define the list of datasets
DATASETS=("cistos" "eggs" "larvae")

# Define common parameters
NUM_SPLITS=3
SAMPLING_TYPE=2
PARAMETER=0.5

# Iterate over each dataset in the list
for DATASET in "${DATASETS[@]}"; do
    INPUT_FILE="${DATASET}/build/files.txt"
    # Create the output folder with a dynamic name based on the dataset
    OUTPUT_FOLDER="${DATASET}/build/splits"
    
    # Create the output directory for the current dataset if it doesn't exist
    mkdir -p "${OUTPUT_FOLDER}"
    
    echo "Processing dataset: ${DATASET}"

    # Run the python script
    python src/split_dataset.py "${INPUT_FILE}" "${OUTPUT_FOLDER}" "${NUM_SPLITS}" "${SAMPLING_TYPE}" "${PARAMETER}"

    echo "Finished processing ${DATASET}."
    echo "-----------------------------------"
done

echo "All datasets have been processed."