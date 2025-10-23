FLAGS = -fPIC -std=gnu11 -Wall -Wno-unused-result -pedantic

BIN = $(NEWIFT_DIR)/bin

LIBSVM_DIR = $(NEWIFT_DIR)/externals/libsvm
LIBNIFTI_DIR= $(NEWIFT_DIR)/externals/libnifti
LIBJPEG_DIR= $(NEWIFT_DIR)/externals/libjpeg
LIBPNG_DIR= $(NEWIFT_DIR)/externals/libpng
TSNE_DIR= $(NEWIFT_DIR)/externals/tsne
ZLIB=$(NEWIFT_DIR)/externals/zlib

LIBIFT_INC   = -I $(NEWIFT_DIR)/include
LIBIFT_LD	= -L $(NEWIFT_DIR)/lib -lift
LIBSVM_INC   = -I $(LIBSVM_DIR)/include 
LIBSVM_LD	= -L $(LIBSVM_DIR)/lib -lstdc++
LIBCBLAS_INC = -I /usr/local/opt/openblas/include
LIBCBLAS_LD  = -L /usr/local/opt/openblas/lib -L /lib64/atlas-sse3 -L /usr/lib/atlas-base -llapack -lblas -lcblas

LIBNIFTI_INC = -I $(LIBNIFTI_DIR)/include
LIBJPEG_INC = -I $(LIBJPEG_DIR)/include
TSNE_INC = -I $(TSNE_DIR)/include
ZLIB_INC = -I $(ZLIB)/include

EXTERNALS_LD = -lm

OS=$(shell uname -s)

# For Mac OS, add a special flag to enable the openmp. Install: brew install libomp
# Otherwise, add the openmp flag commonly 
ifeq ($(OS), Darwin)
	EXTERNALS_LD += -Xpreprocessor -fopenmp -lomp # -lomp is required to link the program, lib, and openmp
	OPENMP_FLAGS = -Xpreprocessor -fopenmp
	OPENMP_LINK = -lomp
else
	EXTERNALS_LD += -fopenmp
	OPENMP_FLAGS = -fopenmp
	OPENMP_LINK = -lgomp
endif

INCLUDES = $(LIBIFT_INC) $(LIBSVM_INC) $(LIBCBLAS_INC) $(LIBNIFTI_INC) $(LIBJPEG_INC) $(TSNE_INC) $(ZLIB_INC)
LIBS	 = $(LIBIFT_LD) $(LIBSVM_LD) $(LIBCBLAS_LD) $(EXTERNALS_LD)

# Separate libs for NVCC (without direct OpenMP flags)
NVCC_LIBS_BASE = $(LIBIFT_LD) $(LIBSVM_LD) $(LIBCBLAS_LD) -lm

# Base flags for C compilation (without OpenMP)
BASE_FLAGS = -fPIC -std=gnu11 -Wall -Wno-unused-result -pedantic

ifeq ($(IFT_DEBUG), 1)
	export FLAGS += -pg -g -fsanitize=address -fsanitize=leak -DIFT_DEBUG=1 $(OPENMP_FLAGS)
	BASE_FLAGS += -pg -g -DIFT_DEBUG=1
else
	export FLAGS += -O3 $(OPENMP_FLAGS)
	BASE_FLAGS += -O3
endif

# Enable GPU support - UNCOMMENT THE NEXT LINE TO ENABLE GPU
IFT_GPU=1

#CUDA path in case IFT_GPU is enabled
export CUDA_DIR1=/usr/local/cuda
export CUDA_DIR2=/opt/cuda

ifeq ($(IFT_GPU), 1)
	export FLAGS += -DIFT_GPU=1
	BASE_FLAGS += -DIFT_GPU=1
	INCLUDES += -I $(CUDA_DIR1)/include
	INCLUDES += -I $(CUDA_DIR2)/include
	LIBS	 += -L $(CUDA_DIR1)/lib64 -L $(CUDA_DIR2)/lib64 -lcublas -lcudart -lcudadevrt
	MAKE_CMD=$(MAKE) IFT_GPU=1
	
	# CUDA compiler settings
	NVCC = nvcc
	NVCC_FLAGS = -O3 -arch=sm_50
	
	# Pass host compiler flags properly to NVCC (excluding OpenMP)
	HOST_COMPILER_FLAGS = $(BASE_FLAGS)
	NVCC_FLAGS += $(foreach flag,$(HOST_COMPILER_FLAGS),-Xcompiler $(flag))
	
	# CUDA libraries
	CUDA_LIBS = -L $(CUDA_DIR1)/lib64 -L $(CUDA_DIR2)/lib64 -lcublas -lcudart -lcudadevrt
	
	# Complete NVCC libs (base libs + CUDA libs + OpenMP for linking)
	ifeq ($(OS), Darwin)
		NVCC_LIBS_COMPLETE = $(NVCC_LIBS_BASE) $(CUDA_LIBS) -Xlinker -lomp
	else
		NVCC_LIBS_COMPLETE = $(NVCC_LIBS_BASE) $(CUDA_LIBS) -lgomp
	endif
else
	MAKE_CMD=$(MAKE)
endif

libift:
	cd $(NEWIFT_DIR); $(MAKE_CMD); cd -;

# Rule for regular C files
$@.c: $@.c
	$(MAKE) libift
	$(CC) $(FLAGS) $@.c -o $(BIN)/$@ $(INCLUDES) $(LIBS)

# Special rule for GPU version
iftCAGpu: iftCAGpu.cu
	$(MAKE) libift
ifeq ($(IFT_GPU), 1)
	$(NVCC) $(NVCC_FLAGS) -x cu iftCAGpu.cu -o $(BIN)/iftCAGpu $(INCLUDES) $(NVCC_LIBS_COMPLETE)
else
	@echo "Error: GPU support is not enabled. Set IFT_GPU=1 in Makefile"
	@exit 1
endif

clean:
	rm -rf $(BIN)/*;