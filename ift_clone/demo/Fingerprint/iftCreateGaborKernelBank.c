#include "ift.h"

#define GABOR_KERNEL_SIZE 27
#define SIGMA_X 4
#define SIGMA_Y 4
#define MAX_FREQ 12 // 0.08 Hz
#define MIN_FREQ 5 // 0.20 Hz



void printKernelBank(iftMMKernel *K_bank, int xsize, int ysize){
  /*
  Prints a Gabor iftMMKernel.
  Each kernel has 2 bands representing the real and the imaginary parts 
  */
  char filename[200];
  iftVoxel u;
  int j, p, dmin[3];
  iftImage *img;

  for (int i = 0; i < K_bank->nkernels; i++){
    img = iftCreateImage(xsize, ysize, 1);
    dmin[0]=IFT_INFINITY_INT;
    dmin[1]=IFT_INFINITY_INT;
    dmin[2]=IFT_INFINITY_INT;
    float min_weight = IFT_INFINITY_FLT;
    float max_weight = IFT_INFINITY_FLT_NEG;

    for (j=0; j < K_bank->A->n; j++) {
      if (K_bank->A->dx[j]<dmin[0])
        dmin[0] = K_bank->A->dx[j];
      if (K_bank->A->dy[j]<dmin[1])
        dmin[1] = K_bank->A->dy[j];
      if (K_bank->A->dz[j]<dmin[2])
        dmin[2] = K_bank->A->dz[j];
    }

    for (j=0; j < K_bank->A->n; j++){
      if(K_bank->weight[i][0].val[j] > max_weight)
        max_weight = K_bank->weight[i][0].val[j];
      
      if (K_bank->weight[i][0].val[j] < min_weight)
        min_weight = K_bank->weight[i][0].val[j];
    }

    for (j=0; j < K_bank->A->n; j++) {
      K_bank->weight[i][0].val[j] = 255.0*(K_bank->weight[i][0].val[j] - min_weight)/(max_weight - min_weight);
    }

    for (j=0; j < K_bank->A->n; j++) {
      u.x = K_bank->A->dx[j]-dmin[0];
      u.y = K_bank->A->dy[j]-dmin[1];
      u.z = K_bank->A->dz[j]-dmin[2];
      p   = iftGetVoxelIndex(img,u);
      img->val[p] = iftRound(K_bank->weight[i][0].val[j]);
    }
    sprintf(filename,"kernel-%d.png", i);
    iftWriteImageByExt(img, filename);
    iftDestroyImage(&img);
  }
  
}



int main(int argc, char *argv[]) {
  if (argc != 4)
  {
    printf("usage iftCreateGaborKernelBank: <P1> <P2> <P3>\n");
    printf("<P1>: Input Number of Orientations\n");
    printf("<P2>: Input Number of Frequencies\n");
    printf("<P3>: Output filename (.npy)\n");
    exit(0);
  }

  int n_orientations = atoi(argv[1]);
  int n_frequencies = atoi(argv[2]);

  iftMMKernel *K = iftGaborKernelBank(GABOR_KERNEL_SIZE, n_orientations, n_frequencies, MAX_FREQ, MIN_FREQ, SIGMA_X, SIGMA_Y);

  // iftWriteMMKernel(K, argv[3]);
  iftMatrix *M = iftMMKernelToMatrix(K);
  iftMatrix *M_t = iftTransposeMatrix(M);
  printKernelBank(K, GABOR_KERNEL_SIZE, GABOR_KERNEL_SIZE);

  iftWriteMatrix(M_t, argv[3]);

  iftDestroyMatrix(&M);
  iftDestroyMatrix(&M_t);
  iftDestroyMMKernel(&K);
}
