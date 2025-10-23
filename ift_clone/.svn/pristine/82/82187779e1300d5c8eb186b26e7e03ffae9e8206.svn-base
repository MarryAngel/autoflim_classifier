
# LIDS Fingerprint Module

This is a collection of fingerprint programs, developed as part of MSc. projects in partnership with Griaule. It is a complete pipeline for identification of latent fingerprints, consisting of the following modules:

* Normalization + Gabor Filtering*
* Segmentation
* Binarization
* Multi-scale skeletonization
* Minutiae extraction

![Example Image](https://drive.google.com/uc?id=1KhJoy5OYy-gNbo7lTKc2ld-PTZ8clxHC)

## Authors

- [@andrenobrega](a203758@dac.unicamp.br)
- [@afalcao](alexandre.falcao@gmail.com)
- [@waltercasas](w187990@dac.unicamp.br)


## Acknowledgements

 - [Griaule](https://griaule.com/)
 - [Computer Institute - Unicamp](https://ic.unicamp.br/)


## Run Locally

Each module can be executed separately. A set of shell scripts was prepared to make the whole process easier.

### Pre-Processing

The Pre-processing module can be executed locally using the _preprocessing.sh_ script, which has the following inputs:

    1. Folder with fingerprint images
    2. Folder with gabor images (this is the result of the first step, that should be executed separatelly)
    3. Input FPModel parameters
    4. Output folder with gabor masks
    5. Output folder with binarized gabor
    6. Output folder with skeleton
    7. Output folder with minuciae points
    8. Output folder with drawed minuciaes


Go to source directory

```bash
  cd ift/demo/Fingerprint
```

Create a sample folder, with a few fingerprint images. Be sure to have stored a folder with all gabor fingerprint images

Run sample example

```bash
sh preprocessing.sh sample/ path_to_gabor_images FPModelParameters.json gabor_masks gabor_bin skel mnts draw_mnts
```
