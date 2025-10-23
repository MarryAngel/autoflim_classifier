import os
import sys

def main():
    # Check command line arguments
    if len(sys.argv) != 5:
        print("Usage: python add_images_train_flim.py <num_classes> <split> <input_csv>")
        print("Datasets available: eggs, larvae, ...")
        print("num_classes: number of classes")
        print("split: split identifier")
        print("input_csv: path to misclassified.csv")
        sys.exit(1)

    dataset_name = str(sys.argv[1])
    num_classes = int(sys.argv[2])
    split = int(sys.argv[3])
    input_csv = sys.argv[4]

    # Set build directory and change working directory
    build_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', dataset_name, 'build'))
    os.chdir(build_dir)

    # Read misclassified.csv and create list of image entries
    with open(input_csv, "r") as f:
        image_lines = [line.strip() for line in f]

    # Remove header if present
    if image_lines and image_lines[0].startswith("Misclassified Image; Certainty;"):
        image_lines.pop(0)

    print(f"Total images in misclassified.csv: {len(image_lines)}")

    # Build dictionary mapping image names to certainty values
    image_certainty = {}
    for line in image_lines:
        # Extract image name and certainty value
        name = line.split("/")[1].split(";")[0]
        certainty = float(line.split(";")[1])
        image_certainty[name] = certainty

    # Sort images by certainty in descending order
    sorted_images = dict(sorted(image_certainty.items(), key=lambda item: item[1], reverse=True))

    # Prepare output directory
    output_dir = f"./train{split}"
    os.makedirs(output_dir, exist_ok=True)

    # Select and copy images for each class
    selected_counts = []
    for class_id in range(1, num_classes + 1):
        # Filter images belonging to the current class
        class_images = [name for name in sorted_images.keys() if int(name.split("_")[0]) == class_id]
        class_images.sort(key=lambda x: sorted_images[x], reverse=True)

        if class_id != num_classes:
            # Try to copy the image with highest certainty, skip if already exists
            count = 0
            for img in class_images:
                img_name = img.split("/")[-1]
                dest_path = os.path.join(output_dir, img_name)
                src_path = os.path.join("./images", img)
                if not os.path.exists(dest_path):
                    os.system(f"cp {src_path} {dest_path}")
                    print(f"Selected image for class {class_id}: {img} {sorted_images[img]}")
                    count += 1
                    break
                else:
                    print(f"Image {img_name} already exists in {output_dir}, trying next for class {class_id}.")
            selected_counts.append(count)
        else:
            # For the last class, add double the total number of images selected for other classes
            total_other = sum(selected_counts)
            needed = 2 * total_other
            count = 0
            for img in class_images:
                if count >= needed:
                    break
                img_name = img.split("/")[-1]
                dest_path = os.path.join(output_dir, img_name)
                src_path = os.path.join("./images", img)
                if not os.path.exists(dest_path):
                    os.system(f"cp {src_path} {dest_path}")
                    print(f"Selected image for class {class_id}: {img} {sorted_images[img]}")
                    count += 1
                else:
                    print(f"Image {img_name} already exists in {output_dir}, trying next for class {class_id}.")

if __name__ == "__main__":
    main()
