#include "ift.h"

#define SIGMA_X 4
#define SIGMA_Y 4
#define MAX_FREQ 12 // 0.08 Hz
#define MIN_FREQ 8 // 0.20 Hz
#define GABOR_KERNEL_SIZE 17
#define COHERENCE_WINDOW_SIZE 5
#define COHERENCE_THRESHOLD 0.95
#define ADJ_RADIUS 8
#define VARIANCE_BLK_SIZE 8
#define VARIANCE_THRESHOLD 0.1
#define GAUSSIAN_KERNEL_STDEV 11
#define MEDIAN_FILTER_RADIUS 7
#define VECTOR_SMOOTHING 1

// #define DEBUG 1 // uncomment this line if wish to see filters and activations


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

iftImage *GetMask(iftImage *bin, float radius)
{
  iftImage *mask     = iftAsfCOBin(bin,radius);
  iftImage *cbasins  = iftCloseBasins(mask,NULL,NULL);
  iftDestroyImage(&mask);
  return(cbasins);
}

float *getFrequencyArray(int n_frequencies){
  float *freq      = (float *)calloc(n_frequencies, sizeof(float));
  float freq_step  = (float)(MAX_FREQ - MIN_FREQ) / n_frequencies;
  /*Getting array of frequencies*/
  for (int i = 0; i < n_frequencies; i++){
      freq[i] = 1/(MIN_FREQ + i * freq_step);
  }
  return freq;
}

float *getOrientationArray(int n_orientations){
  float *theta = (float *)calloc(n_orientations, sizeof(float));
  float theta_step = IFT_PI / n_orientations;
  /*Getting array of orientations*/
  for (int i = 0; i < n_orientations; i++){
      theta[i] = i * theta_step;
  }

  // float *theta = (float *)calloc(1, sizeof(float));
  // // float theta_step = IFT_PI / 1;
  // /*Getting array of orientations*/
  // theta[0] = IFT_PI/2;

  return theta;
}

int getThetaIndex(int b, int n_frequencies){
  return (b / n_frequencies);
}

int getFreqIndex(int b, int n_frequencies) {
  return (b % n_frequencies);
}

int getBandFromThetaFreqIndex(int theta_i, int freq_i, int n_frequencies) {
  return (theta_i * n_frequencies)+freq_i;
}

void saveOrientationImage(iftMImage *mimg, char* basename) {
  iftImage *img;
  if (VECTOR_SMOOTHING == 1) {
    for (int b = 0; b < 2; b++) {
      img = iftCreateImage(mimg->xsize, mimg->ysize, mimg->zsize);
      for (int p = 0; p < img->n; p++) {
          img->val[p] = iftRound(127*(1+mimg->val[p][b]));
      } 
      char filename[200];
      // sprintf(filename,"%s.npy",basename);
      // iftWriteFImage(fimg, "orientation_image.npy");
      if (b == 0)
        sprintf(filename,"%s_cos.png",basename);
      else
        sprintf(filename,"%s_sin.png",basename);
      iftWriteImageByExt(img, filename);
      iftDestroyImage(&img);
    }
  }
  
  img = iftCreateImage(mimg->xsize, mimg->ysize, mimg->zsize);
  for (int p = 0; p < img->n; p++) {
    float theta;

    if (VECTOR_SMOOTHING == 1) {
      float arctan2theta = atan2(mimg->val[p][1],mimg->val[p][0])/2;
      if (isnan(arctan2theta)) {
        theta = 0;
      } else {
        theta = arctan2theta;
      }
    } else {
      theta = mimg->val[p][0];
    }
    
    img->val[p] = iftRound(255*(theta/IFT_PI));
  } 
  printf(" \n");
  char filename[200];
  // sprintf(filename,"%s.npy",basename);
  // iftWriteFImage(fimg, "orientation_image.npy");
  printf(" theta\n");
  sprintf(filename,"%s_theta.png",basename);
  iftWriteImageByExt(img, filename);
  iftDestroyImage(&img);
}

iftMImage *getGaussianFilteredOrientations(iftMImage *mimg) {
  int k_size = (2*(int)ceil(3*GAUSSIAN_KERNEL_STDEV)+1)/2;
  iftMImage *filtered_orientations = iftMGaussianFilter(mimg, k_size, GAUSSIAN_KERNEL_STDEV);
  // iftMKernel *K = iftGaussianMKernel2D(k_size, GAUSSIAN_KERNEL_STDEV, 2);

  // iftMImage *filtered_orientations = iftMLinearFilter(mimg, K);

  return filtered_orientations;
}

iftMImage *getMedianFilteredOrientations(iftMImage *mimg) {
  float r = (float) MEDIAN_FILTER_RADIUS;
  iftAdjRel *A = iftCircular(r);
  iftMImage *filtered_orientations = iftMMedianFilter(mimg, A);
  // iftFImage *filtered_orientations = fimg;
  return filtered_orientations;
}

// iftFImage *getOrientationImage(iftMImage *mimg, int *bmax, int n_orientations, int n_frequencies, iftImage *mask) {
//   iftImage *img   = iftCreateImage(mimg->xsize, mimg->ysize, mimg->zsize);
//   iftFImage *orient_img   = iftCreateFImage(mimg->xsize, mimg->ysize, mimg->zsize);
//   // iftFImage *fimg = iftCreateFImage(mimg->xsize, mimg->ysize, mimg->zsize);
//   iftAdjRel *A    = iftRectangular(ADJ_RADIUS,ADJ_RADIUS);//iftCircular(ADJ_RADIUS);

//   for (int p = 0; p < mimg->n; p++){
//     int max  = IFT_INFINITY_INT_NEG;
//     bmax[p] = 0; 
//     /*Searching for max band in neighborhood*/
//     iftVoxel u = iftGetVoxelCoord(img, p);
//     for (int i = 0; i < A->n; i++){
//       iftVoxel v = iftGetAdjacentVoxel(A, u, i);
//       if (iftValidVoxel(img, v)){
//         int q = iftGetVoxelIndex(img, v);
//         for (int b = 0; b < mimg->m; b++){
//           if (mimg->val[q][b] > max){
//             max = mimg->val[q][b];
//             bmax[p] = b;
//           }
//         }
//       }
//     }
//     if (mask->val[p]) {
//       int band = bmax[p];
//       int theta_i = getThetaIndex(band, n_frequencies);
//       float *orientation_arr = getOrientationArray(n_orientations);
//       orient_img->val[p] = orientation_arr[theta_i];
//     }
//   }

//   iftDestroyAdjRel(&A);
//   return orient_img;
// }

iftMImage *getOrientationVectorImageSum(iftMImage *mimg, int *bmax, int n_orientations, int n_frequencies, iftImage *mask) {
  iftImage *img   = iftCreateImage(mimg->xsize, mimg->ysize, mimg->zsize);
  iftMImage *orient_img   = iftCreateMImage(mimg->xsize, mimg->ysize, mimg->zsize, 2);
  // iftFImage *fimg = iftCreateFImage(mimg->xsize, mimg->ysize, mimg->zsize);
  iftAdjRel *A    = iftCircular(ADJ_RADIUS);//iftCircular(ADJ_RADIUS);
  float *orientation_arr = getOrientationArray(n_orientations);

  for (int p = 0; p < mimg->n; p++){
    float max_sum = IFT_INFINITY_FLT_NEG;
    int   max_id  = 0;
    float *band_sum = (float *)calloc(mimg->m, sizeof(float));

    /* Getting the sum in the neighbourhood for each band */
    iftVoxel u = iftGetVoxelCoord(img, p);
    for (int b = 0; b < mimg->m; b++){
      band_sum[b] = 0.0;
      for (int i = 0; i < A->n; i++){
        iftVoxel v = iftGetAdjacentVoxel(A, u, i);
        if (iftValidVoxel(img, v)){
          int q = iftGetVoxelIndex(img, v);
          band_sum[b] += mimg->val[q][b];
        }
      }
    }
    /* Finding the band with the highest sum */

    for (int b = 0; b < mimg->m; b++){
      if (band_sum[b] > max_sum){
        max_sum = band_sum[b];
        max_id  = b;
      }
    }
    if (mask->val[p]) {
      int band = max_id;
      int theta_i = getThetaIndex(band, n_frequencies);

      if (VECTOR_SMOOTHING == 0) {
        // Band 0 = Band 1 = theta
        orient_img->val[p][0] = orientation_arr[theta_i];
        orient_img->val[p][1] = orientation_arr[theta_i];
      } else {
        // Band 0 = cosine
        orient_img->val[p][0] = cos(2*orientation_arr[theta_i]);
        // Band 1 = sine
        orient_img->val[p][1] = sin(2*orientation_arr[theta_i]);
      }

    }

    iftFree(band_sum);
  }

  iftFree(orientation_arr);

  iftDestroyAdjRel(&A);
  return orient_img;
}

// finds most voted bands and saves value in bmax
void getMostVotedGaborBandByPixelInWindow(iftMImage *mimg, iftImage *img, int *bmax, iftImage *mask){
  // iftFImage *fimg = iftCreateFImage(mimg->xsize, mimg->ysize, mimg->zsize);
  iftAdjRel *A    = iftCircular(ADJ_RADIUS);

  for (int p = 0; p < mimg->n; p++) {
    int* voted_bands = iftAllocIntArray(A->n);
    int* index = iftAllocIntArray(A->n);

    int max  = IFT_INFINITY_INT_NEG;
    bmax[p] = 0; 
    /*Searching for max band in neighborhood*/
    iftVoxel u = iftGetVoxelCoord(img, p);

    for (int i = 0; i < A->n; i++){
      iftVoxel v = iftGetAdjacentVoxel(A, u, i);
      if (iftValidVoxel(img, v)){
        int q = iftGetVoxelIndex(img, v);
        for (int b = 0; b < mimg->m; b++){
          if (mimg->val[q][b] > max){
            max = mimg->val[q][b];
            bmax[p] = b;
          }
        }
        // bmax[p] is max activation band of pixel p
        voted_bands[i] = bmax[p];
      } else {
        voted_bands[i] = -1;
      }
      index[i] = i;
    }
    
    iftQuickSort(voted_bands, index, 0, A->n-1, IFT_INCREASING);

    int max_freq = 0, most_voted = 0, curr_freq = 1;

    for (int i = 0; i < A->n - 1; i++) {
      if (voted_bands[i] == -1)
        continue;

      if (voted_bands[i] == voted_bands[i+1]) {
        curr_freq += 1;
      } else {
        curr_freq = 1;
      }

      if (curr_freq > max_freq) {
        max_freq = curr_freq;
        most_voted = voted_bands[i];
      }

    }

    bmax[p] = most_voted;
    // bmax[p] = 0;

    // printf("Most Voted = %d\n", most_voted);
    // assert(1==0);

    iftFree(voted_bands);
    iftFree(index);

  }

  iftDestroyAdjRel(&A);
}




iftMImage *getOrientationVectorImage(iftMImage *mimg, int *bmax, int n_orientations, int n_frequencies, iftImage *mask) {
  iftImage *img   = iftCreateImage(mimg->xsize, mimg->ysize, mimg->zsize);
  iftMImage *orient_img   = iftCreateMImage(mimg->xsize, mimg->ysize, mimg->zsize, 2);
  // iftFImage *fimg = iftCreateFImage(mimg->xsize, mimg->ysize, mimg->zsize);
  iftAdjRel *A    = iftCircular(ADJ_RADIUS);//iftCircular(ADJ_RADIUS);
  float *orientation_arr = getOrientationArray(n_orientations);

  getMostVotedGaborBandByPixelInWindow(mimg, img, bmax, mask);

  // getMaxGaborActivByPixelInWindow(mimg, bmax, n_frequencies, mask);

  for (int p = 0; p < mimg->n; p++){
    if (mask->val[p]) {
      // int max  = IFT_INFINITY_INT_NEG;
      // bmax[p] =   0; 
      /*Searching for max band in neighborhood*/
      // iftVoxel u = iftGetVoxelCoord(img, p);
      // for (int i = 0; i < A->n; i++){
      //   iftVoxel v = iftGetAdjacentVoxel(A, u, i);
      //   if (iftValidVoxel(img, v)){
      //     int q = iftGetVoxelIndex(img, v);
      //     for (int b = 0; b < mimg->m; b++){
      //       if (mimg->val[q][b] > max){
      //         max = mimg->val[q][b];
      //         bmax[p] = b;
      //       }
      //     }
      //   }
      // }
      int band = bmax[p];
      int theta_i = getThetaIndex(band, n_frequencies);
      
      if (VECTOR_SMOOTHING == 0) {
        // Band 0 = Band 1 = theta
        orient_img->val[p][0] = orientation_arr[theta_i];
        orient_img->val[p][1] = orientation_arr[theta_i];
      } else {
        // Band 0 = cosine
        orient_img->val[p][0] = cos(2*orientation_arr[theta_i]);
        // Band 1 = sine
        orient_img->val[p][1] = sin(2*orientation_arr[theta_i]);
      }
    }
  }

  iftDestroyImage(&img);
  iftFree(orientation_arr);
  iftDestroyAdjRel(&A);
  return orient_img;
}

int getClosestThetaIndexFromOrientation(float orientation, int n_orientations, int n_freq) {
  float *orientation_arr = getOrientationArray(n_orientations);
  float min_diff = IFT_INFINITY_FLT;
  int closest_theta_i = -1;

  for (int i = 0; i < n_orientations; i++) {
    float diff = fabs(orientation - orientation_arr[i]);

    if (diff < min_diff) {
      min_diff = diff;
      closest_theta_i = i;
    }
  }

  free(orientation_arr);

  return closest_theta_i;
}

int getClosestFreqIndexFromValue(float f, int n_freq) {
  float *freq_arr = getFrequencyArray(n_freq);
  float min_diff = IFT_INFINITY_FLT;
  int closest_freq_i = -1;

  for (int i = 0; i < n_freq; i++) {
    float diff = fabs(f - freq_arr[i]);

    if (diff < min_diff) {
      min_diff = diff;
      closest_freq_i = i;
    }
  }

  free(freq_arr);

  return closest_freq_i;
}

void saveOrientationField(iftImage *smooth_orientation_img, iftImage *mask, int *thetas, char *filename) {
  iftAdjRel *A = iftRectangular(8, 8);

  FILE *fptr;

  if(iftFileExists(filename)) {
    iftRemoveFile(filename);
  }

  // float *thetas_cos = (float *)calloc(smooth_orientation_img->n, sizeof(float));

  // for (int i = 0; i < smooth_orientation_img->n; i++) thetas_cos[i] = cosf()
  // Open a file in append mode
  fptr = fopen(filename, "a");

  // Append some text to the file
  fprintf(fptr, "%d %d\n", iftRound((float)smooth_orientation_img->xsize/8.0), iftRound((float)smooth_orientation_img->ysize/8.0));

  for (int i = 0; i < smooth_orientation_img->ysize; i += 8) {
    for (int j = 0; j < smooth_orientation_img->xsize; j += 8) { 
      iftVoxel u;
      u.x = j + 4;
      u.y = i + 4;

      float sum_cos = 0;
      float sum_sin = 0;
      float count = 0;
      int invalid = 0;
      for (int k = 0; k < A->n; k++){
        iftVoxel v = iftGetAdjacentVoxel(A, u, k);
        if (iftValidVoxel(smooth_orientation_img, v)){
          int q = iftGetVoxelIndex(smooth_orientation_img, v);

          if (mask->val[q] == 0) {
            invalid = 1;
            sum_cos = -1;
            sum_sin = -1;
            continue;
          }

          // sum += thetas[q]; // thetas[q] != cos(2 * theta_hat[q])
          sum_cos += cos(2 * (float)thetas[q] * IFT_PI / 180);
          sum_sin += sin(2 * (float)thetas[q] * IFT_PI / 180);
          count += 1;
        }
      }

      int mean_orient;
      if (invalid == 0) {
        float mean_orient_f = atan2((sum_sin/count),(sum_cos/count))/2;
        if (mean_orient_f < 0) mean_orient_f += IFT_PI;
        mean_orient_f       = iftRound(mean_orient_f * 180 / IFT_PI);
        mean_orient     = (int)(mean_orient_f);
      } else {
        mean_orient     = -1;
      }
      // int mean_orient     = iftRound(sum/count);
      
      fprintf(fptr, "%d ", mean_orient);
      
    }
    fprintf(fptr, "\n");
  }
  fclose(fptr);

  iftDestroyAdjRel(&A);
  return;
}

iftImage *getFilteredImgFromOrientationImg(iftMImage *orient_mimg, iftMImage *mimg, int *bmax, int* thetas, int n_orientations, int n_frequencies, iftImage *mask) {
  iftImage *img     = iftCreateImage(orient_mimg->xsize, orient_mimg->ysize, orient_mimg->zsize);
  iftFImage *fimg   = iftCreateFImage(orient_mimg->xsize, orient_mimg->ysize, orient_mimg->zsize);
  float image_max   = IFT_INFINITY_FLT_NEG;
  float image_min   = IFT_INFINITY_FLT;



  for (int p = 0; p < orient_mimg->n; p++) {
    if (mask->val[p]){
      int freq_i = getFreqIndex(bmax[p], n_frequencies);
      float theta;

      if (VECTOR_SMOOTHING == 0) {
	      theta = orient_mimg->val[p][1];
      } else {
        theta = atan2(orient_mimg->val[p][1], orient_mimg->val[p][0])/2;
        if (theta < 0)
          theta = (IFT_PI)+theta;
      }

      int theta_deg = iftRound(iftDegrees(theta));
      thetas[p] = theta_deg;
      
      int theta_i = getClosestThetaIndexFromOrientation(theta, n_orientations, n_frequencies);

      int b = getBandFromThetaFreqIndex(theta_i, freq_i, n_frequencies);

      fimg->val[p] = mimg->val[p][b];
      // if (fimg->val[p]<0)
	    //   fimg->val[p]=0;

      if (image_max < fimg->val[p]) image_max = fimg->val[p];
      if (image_min > fimg->val[p]) image_min = fimg->val[p];
    }
  }

  // calculates mean for stdev
  float mean = 0;
  for (int p = 0; p < fimg->n; p++){
    mean += fimg->val[p]/fimg->n;
  }

  // calculates stdev for zscore
  float var = 0;
  for (int p = 0; p < fimg->n; p++){
    var += (fimg->val[p] - mean)*(fimg->val[p] - mean)/fimg->n;
  }
  float stdev = sqrtf(var);

  printf("mean = %f\nstdev = %f\n", mean, stdev);

  for (int p = 0; p < img->n; p++){
    float zscore = (fimg->val[p] - mean)/stdev;

    img->val[p] = iftRound(iftSigm(zscore)*255);
  }

  iftDestroyFImage(&fimg);

  return img;
}




iftImage *getSumGaborActivByPixelInWindow(iftMImage *mimg, int *bmax, int n_frequencies, iftImage *mask){
  iftImage *img   = iftCreateImage(mimg->xsize, mimg->ysize, mimg->zsize);
  iftFImage *fimg = iftCreateFImage(mimg->xsize, mimg->ysize, mimg->zsize);
  iftAdjRel *A    = iftCircular(ADJ_RADIUS);


  for (int p = 0; p < mimg->n; p++){
    float max_sum = IFT_INFINITY_FLT_NEG;
    int   max_id  = 0;
    float *band_sum = (float *)calloc(mimg->m, sizeof(float));

    /* Getting the sum in the neighbourhood for each band */
    iftVoxel u = iftGetVoxelCoord(img, p);
    for (int b = 0; b < mimg->m; b++){
      band_sum[b] = 0.0;
      for (int i = 0; i < A->n; i++){
        iftVoxel v = iftGetAdjacentVoxel(A, u, i);
        if (iftValidVoxel(img, v)){
          int q = iftGetVoxelIndex(img, v);
          band_sum[b] += mimg->val[q][b];
        }
      }
    }
    /* Finding the band with the highest sum */

    for (int b = 0; b < mimg->m; b++){
      if (band_sum[b] > max_sum){
        max_sum = band_sum[b];
        max_id  = b;
      }
    }
    if (mask->val[p])
      fimg->val[p] = mimg->val[p][max_id];
    
    free(band_sum);
  }


  float image_max = IFT_INFINITY_FLT_NEG;
  float image_min = IFT_INFINITY_FLT;

  for (int i = 0; i < fimg->n; i++){
    if (image_max < fimg->val[i]) image_max = fimg->val[i];
    if (image_min > fimg->val[i]) image_min = fimg->val[i];
  }

  for (int i = 0; i < img->n; i++){
    img->val[i] = (int)(255.0 * (fimg->val[i] - image_min)/(image_max - image_min));
  }


  iftDestroyAdjRel(&A);
  return img;
}

iftImage *getFrequencyImageFromSelectedBands(iftMImage *mimg, int *bmax, int n_frequencies, iftImage *mask) {
  iftImage *freq_img = iftCreateImage(mimg->xsize, mimg->ysize, mimg->zsize);
  float* f_array = getFrequencyArray(n_frequencies);
  for (int p = 0; p < freq_img->n; p++) {

    int b = bmax[p];

    int freq_i = getFreqIndex(b, n_frequencies);
    float f = f_array[freq_i];

    float max_freq = iftMaxFloatArray(f_array, n_frequencies);
    float min_freq = iftMinFloatArray(f_array, n_frequencies);
    float norm_value = (f - min_freq) /  (max_freq - min_freq) * 255;
    freq_img->val[p] = iftRound(norm_value);
  }

  return freq_img;
}

iftImage *getSmoothFrequencyImage(iftImage *freq_img, int*bmax, int n_frequencies) {
  iftImage *output_img = iftCreateImageFromImage(freq_img);
  iftFImage *freq_fimg = iftImageToFImage(freq_img);

  int k_size = (2*(int)ceil(3*GAUSSIAN_KERNEL_STDEV)+1)/2;
  
  iftFImage* filtered_freq_fimg = iftFGaussianFilter(freq_fimg, k_size, GAUSSIAN_KERNEL_STDEV);
  
  float* freq_arr = getFrequencyArray(n_frequencies);
  float max_freq = iftMaxFloatArray(freq_arr, n_frequencies);
  float min_freq = iftMinFloatArray(freq_arr, n_frequencies);

  for (int p = 0; p < filtered_freq_fimg->n; p++) {

    float f = filtered_freq_fimg->val[p];
    float f_ = min_freq + (max_freq - min_freq)*f/255;
    int f_i = getClosestFreqIndexFromValue(f_, n_frequencies);

    float new_f = freq_arr[f_i];

    float norm_value = (new_f - min_freq) /  (max_freq - min_freq) * 255;
    output_img->val[p] = iftRound(norm_value);

    int theta_i = getThetaIndex(bmax[p], n_frequencies);

    bmax[p] = getBandFromThetaFreqIndex(theta_i, f_i, n_frequencies);
  }

  iftFree(freq_arr);
  iftDestroyFImage(&freq_fimg);
  return output_img;
}


iftImage *getFrequestGaborActivByPixelInWindow(iftMImage *mimg, int *bmax, int n_frequencies, iftImage *mask){
  iftImage *img   = iftCreateImage(mimg->xsize, mimg->ysize, mimg->zsize);
  iftFImage *fimg = iftCreateFImage(mimg->xsize, mimg->ysize, mimg->zsize);
  iftAdjRel *A    = iftCircular(ADJ_RADIUS);//iftCircular(ADJ_RADIUS);


  for (int p = 0; p < mimg->n; p++){
    float max_freq   = IFT_INFINITY_FLT_NEG;
    int   max_id     = 0;
    float *band_freq = (float *)calloc(mimg->m, sizeof(float));

    /* Getting the freq in the neighbourhood for each band */
    iftVoxel u = iftGetVoxelCoord(img, p);
    for (int i = 0; i < A->n; i++){
      float band_max = IFT_INFINITY_FLT_NEG;
      iftVoxel v = iftGetAdjacentVoxel(A, u, i);
      if (iftValidVoxel(img, v)){
        int q = iftGetVoxelIndex(img, v);
        for (int b = 0; b < mimg->m; b++){
          if (band_max < mimg->val[q][b]){
            band_max = mimg->val[q][b];
            max_id = b;
          }
        }
      }
      if (band_max != IFT_INFINITY_FLT_NEG)
        band_freq[max_id] += 1;
    }
    max_id = 0;

    for (int b = 0; b < mimg->m; b++){
      if (max_freq < band_freq[b]){
        max_freq = band_freq[b];
        max_id = b;
      }
    }

    if (mask->val[p])
          fimg->val[p] = mimg->val[p][max_id];

    free(band_freq);
  }


  float image_max = IFT_INFINITY_FLT_NEG;
  float image_min = IFT_INFINITY_FLT;

  for (int i = 0; i < fimg->n; i++){
    if (image_max < fimg->val[i]) image_max = fimg->val[i];
    if (image_min > fimg->val[i]) image_min = fimg->val[i];
  }

  for (int i = 0; i < img->n; i++){
    img->val[i] = (int)(255.0 * (fimg->val[i] - image_min)/(image_max - image_min));
  }


  iftDestroyAdjRel(&A);
  return img;
}

char *Basename(char *path)
{
  char *basename     = iftBasename(path);
  iftSList *slist    = iftSplitString(basename,"/");
  strcpy(basename,slist->tail->elem);
  iftDestroySList(&slist);
  return(basename);
}

float *gaborFilter(iftAdjRel *A, float theta, float freq){
    float *gabor = (float *)calloc(A->n, sizeof(float));

    for (int i = 0; i < A->n; i++) {
        int x = A->dx[i];
        int y = A->dy[i];

        float x0 =  x * sin(theta) + y * cos(theta);
        float y0 = -x * cos(theta) + y * sin(theta); 

        gabor[i] = exp(-0.5 * (((x0 * x0)/(SIGMA_X * SIGMA_X)) + ((y0 * y0)/(SIGMA_Y * SIGMA_Y)))) * cos(2 * IFT_PI * freq * x0);
    }

    return gabor;
}


float maxArray(int n, float *arr){
  float max = IFT_INFINITY_FLT_NEG;
  for (int i = 0; i < n; i++){
    if (arr[i] > max) max = arr[i];
  }

  return max;
}

float minArray(int n, float *arr){
  float min = IFT_INFINITY_FLT;
  for (int i = 0; i < n; i++){
    if (arr[i] < min) min = arr[i];
  }

  return min;
}

float meanArray(int n, float *arr){
  float mean = 0.0;
  for (int i = 0; i < n; i++){
    mean += arr[i];
  }
  mean /= n;
  return mean;
}



float imageMean(iftImage *img){
    float mean = 0.0;
    for (int i = 0; i < img->n; i++){
        mean += (float) img->val[i];
    }
    mean = mean / img->n;
    return mean;
}

float imageVar(iftImage *img, float mean){
    float var = 0.0;
    for (int i = 0; i < img->n; i++){
        var += (float)(img->val[i] - mean) * (img->val[i] - mean);
    }
    return var;
}

iftImage *normalizeFingerprint(iftImage *img, int m0, float var0){
    float mean = imageMean(img);
    float var  = imageVar(img, mean);


    iftImage *normalized_img = iftCreateImageFromImage(img);
    for (int i = 0; i < img->n; i++){
        // if   (img->val[i] > mean) normalized_img->val[i] = (int)(m0 + sqrt(var0/var * (img->val[i] - mean) * (img->val[i] - mean)));
        // else                      normalized_img->val[i] = (int)(m0 - sqrt(var0/var * (img->val[i] - mean) * (img->val[i] - mean)));
        normalized_img->val[i] = (int)((img->val[i] - mean)/sqrt(var));
    }

    return normalized_img;
}


void RectifiedLinearUnit(iftMImage *mimg) {
  for (int p = 0; p < mimg->n; p++){
    for (int b = 0; b < mimg->m; b++) {
      if (mimg->val[p][b]<0)
	mimg->val[p][b]=0.0;
    }
  }
}

iftMImage *normalizeBandsByZscore(iftMImage *mimg) {
  iftMImage *normalized_mimg = iftCreateMImage(mimg->xsize, mimg->ysize, mimg->zsize, mimg->m);


  // compute mean and standard deviation
  for (int b = 0; b < mimg->m; b++) {
    double var = 0; 
    double mean = 0;

    // mean
    for (int p = 0; p < mimg->n; p++){
      mean += (double)mimg->val[p][b];
    }
    mean /= (double)(mimg->n);

    // variance
    for (int p = 0; p < mimg->n; p++){
      var += pow((double)mimg->val[p][b] - mean, 2);
    }
    var /= (double)(mimg->n);
    double stdev = sqrt(var);

    for (int p = 0; p < mimg->n; p++){
      double z_score = ((double)mimg->val[p][b] - mean)/stdev;
      normalized_mimg->val[p][b] =  (int)(iftSigm(z_score)*255.0);
    }
  }
  
  return normalized_mimg; 
}

iftMImage *normalizeVolumeByZscore(iftMImage *mimg) {
  iftMImage *normalized_mimg = iftCreateMImage(mimg->xsize, mimg->ysize, mimg->zsize, mimg->m);


  // compute mean and standard deviation
  double var = 0; 
  double mean = 0;
  // mean
  for (int p = 0; p < mimg->n; p++){
    for (int b = 0; b < mimg->m; b++) {
      mean += (double)mimg->val[p][b];
    }
  }
  mean /= (double)(mimg->n * mimg->m);

  // variance
  for (int p = 0; p < mimg->n; p++){
    for (int b = 0; b < mimg->m; b++) {
      var += pow((double)mimg->val[p][b] - mean, 2);
    }
  }
  var /= (double)(mimg->n * mimg->m);

  double stdev = sqrt(var);
  printf("volume mean=%lf ,stdev=%lf\n", mean, stdev);

  // update values in normalized image
  for (int p = 0; p < mimg->n; p++) {
    for (int b = 0; b < mimg->m; b++) {

      // standerdized pixel value
      double z_score = ((double)mimg->val[p][b] - mean)/stdev;

      normalized_mimg->val[p][b] =  (int)(iftSigm(z_score)*255.0);
    }
    // printf("\n");
    // printf("band %d: mean=%f ,stdev=%lf\n", b, mean, stdev);
  }
  // iftWriteMImageBands(normalized_mimg, "output_gabor_band");
  return normalized_mimg;
}

iftImage *normalizeImageByZscore(iftImage *img) {
  iftImage *normalized_img = iftCreateImage(img->xsize, img->ysize, img->zsize);

  // image mean value
  float var = 0; 
  float mean = 0;
  for (int p = 0; p < img->n; p++){
    mean += (float)img->val[p];
  }
  mean /= (float)img->n;
  // image standard deviation
  for (int p = 0; p < img->n; p++){
    var += (float)pow((float)img->val[p] - mean, 2);
  }
  var /= ((float)img->n);
  float stdev = (float)sqrt(var);

  for (int i = 0; i < img->n; i++) {

    // standardized pixel value
    float z_score = ((float)img->val[i] - mean)/stdev;
    
    normalized_img->val[i] = (int)(255*iftSigm(z_score));
  }
  // printf("\n");
  printf("mean=%f ,stdev=%lf\n", mean, stdev);

  // iftWriteMImageBands(normalized_img, "output_gabor_band");
  return normalized_img;
}



void printFilteredImageBank(iftMImage *fimg_bank, char basename[], int filter_id){
  /*
  Prints a bank of filtered images from gabor filtering.
  */
  char filename[200];
  double max, min;
  max = IFT_INFINITY_DBL_NEG;
  min = IFT_INFINITY_DBL;
  int img_index_counter = 0;
  iftImage *img;
  
  for (int i = 0; i < fimg_bank->m; i = i + 2){
    img = iftCreateImage(fimg_bank->xsize, fimg_bank->ysize, fimg_bank->zsize);
    for (int j = 0; j < fimg_bank->n; j++){
      img->val[j] = sqrt(fimg_bank->val[j][i]*fimg_bank->val[j][i] + fimg_bank->val[j][i + 1] * fimg_bank->val[j][i + 1]);
      if (img->val[j] > max)
        max = img->val[j];
      if (img->val[j] < min)
        min = img->val[j];
    }
    for (int j = 0; j < fimg_bank->n; j++){
      img->val[j] = 255.0 * (img->val[j] - min)/(max - min);
    }
    if (basename[2] == 'a') // checking if printing images from space
      sprintf(filename,"%s_filtered_image-%d.png",basename, img_index_counter++);
    
    else // printing images from spectrum
      sprintf(filename,"%s_filtered_image-%d.png",basename, filter_id);
    
    iftWriteImageByExt(img, filename);
    iftDestroyImage(&img);
  
  
  }
}

void normalizeAndExtractMaskInPlace(iftMImage *mimg, iftImage *mask){
  iftMImage *aux =   iftCopyMImage(mimg);
  float meani = 0.0;
  float stdevi  = 0.0;

  // Calculating initial mean and standard deviation
  for (int p = 0; p < mimg->n; p++){
    meani += mimg->val[p][0]; // FP images are grayscale, they only have one band.
  }
  meani /= mimg->n;
  for (int p = 0; p < mimg->n; p++){
    stdevi += (mimg->val[p][0] - meani) * (mimg->val[p][0] - meani); 
  }
  stdevi = sqrt(stdevi/mimg->n);

  // normalizing auxiliar image
  for (int p = 0; p < mimg->n; p++){
    aux->val[p][0] = (mimg->val[p][0] - meani) / stdevi;
  }

  // finding segmentation mask using local standard deviation
  iftAdjRel *A = iftCircular(VARIANCE_BLK_SIZE);
  //iftAdjRel *A = iftCircular(5.0);
  for (int p = 0; p < mimg->n; p++){
    iftVoxel u = iftGetVoxelCoord(mask, p);

    int blk_counter = 0;
    float blk_mean  = 0.0;
    float blk_var   = 0.0;
    for (int i = 0; i < A->n; i++){
      iftVoxel v = iftGetAdjacentVoxel(A, u, i);
      if (iftValidVoxel(mask, v)){
        int q = iftGetVoxelIndex(mask, v);
        blk_mean += aux->val[q][0];
        blk_counter++;
      }
    }
    blk_mean /= blk_counter;
    for (int i = 0; i < A->n; i++){
      iftVoxel v = iftGetAdjacentVoxel(A, u, i);
      if (iftValidVoxel(mask, v)){
        int q = iftGetVoxelIndex(mask, v);
        blk_var += (aux->val[q][0] - blk_mean) * (aux->val[q][0] - blk_mean);
      }
    }
    blk_var /= blk_counter;
    
    if (blk_var > VARIANCE_THRESHOLD)
      mask->val[p] = 255;
  }

  iftDestroyAdjRel(&A);

  // calculating new mean and variance, only in segmentation mask
  float meanf        = 0.0;
  float stdevf         = 0.0;
  int   mask_counter = 0;

  for (int p = 0; p < mimg->n; p++){
    if (mask->val[p]){
      meanf += mimg->val[p][0];
      mask_counter++;
    }
  }
  meanf /= mask_counter;

  for (int p = 0; p < mimg->n; p++){
    if (mask->val[p]){
      stdevf += (mimg->val[p][0] - meanf) * (mimg->val[p][0] - meanf);
    }
  }
  stdevf = sqrtf(stdevf/mask_counter);
  
  for (int p = 0; p < mimg->n; p++){
    if (mask->val[p]){
      mimg->val[p][0] = (mimg->val[p][0] - meanf) / stdevf;
    }
    else 
      mimg->val[p][0] = 0;
  }

  iftImage *new_mask = iftSelectLargestComp(mask, NULL);
  iftCopyImageInplace(new_mask, mask);
  iftDestroyImage(&new_mask);
  iftDestroyMImage(&aux);
}

int main(int argc, char *argv[])
{
  timer           *tstart=NULL;

  
  if (argc != 6) {
    printf("usage iftSimpleGaborBank: <P1> <P2> <P3> <P4>\n");
    printf("<P1>: input folder with grayscale images (.png)\n");
    printf("<P2>: number of different orientations\n");
    printf("<P3>: number of different frequencies\n");
    printf("<P4>: output folder for filtered gabor images\n");
    printf("<P5>: output folder for mask images\n");
    exit(0);
  }

  /* Read input parameters */
  
  char *orig_dir    = argv[1];
  iftFileSet *fs    = iftLoadFileSetFromDirBySuffix(orig_dir,".png", 1);  
  int nimages       = fs->n;
  char *gabor_dir   = argv[4];
  char *masks_dir   = argv[5];
  iftMakeDir(gabor_dir);
  iftMakeDir(masks_dir);
  iftMakeDir("of_output");
  char filename[200];

  tstart = iftTic();

  int n_orientations = atoi(argv[2]);
  int n_frequencies  = atoi(argv[3]);

  iftMMKernel *K_bank      = iftGaborKernelBank(GABOR_KERNEL_SIZE, n_orientations, n_frequencies, MAX_FREQ, MIN_FREQ, SIGMA_X, SIGMA_Y);
  /* Apply Gabor filtering and save result */
  
  for (int i=0; i < nimages; i++) {
    printf("Applying gabor filtering on image %d/%d...\n", i+1, nimages);


    timer *t0 = iftTic();

    char *basename        = iftFilename(fs->files[i]->path,".png");
    iftImage *orig        = iftReadImageByExt(fs->files[i]->path);

    int img_size = orig->xsize;
    assert(orig->xsize == orig->ysize);

    float resize_scale = 1;

    iftImage *orig_resized = iftResizeImage(orig, iftRound(((float)img_size)/resize_scale), iftRound(((float)img_size)/resize_scale), 1);

    iftImage  *mask          = iftCreateImageFromImage(orig_resized);  
    iftMImage *mimg          = iftImageToMImage(orig_resized, GRAY_CSPACE);
    normalizeAndExtractMaskInPlace(mimg, mask);

    // t1 - Initalization, Normalization and segmentation mask
    timer *t1 = iftToc();
    printf("---- t1 : Initalization, Normalization and segmentation mask ----\n");
    puts(iftFormattedTime(iftCompTime(t0, t1)));
    t1 = iftTic();
    /* Convoluting with a bank of gabor filters */

    iftMImage   *padded_mimg = iftMAddFrame(mimg, GABOR_KERNEL_SIZE - 1, GABOR_KERNEL_SIZE - 1, 0, 0);
    iftMImage   *fimg_bank   = iftMMLinearFilter(padded_mimg, K_bank);
    iftMImage   *aux         = fimg_bank;
    fimg_bank                = iftMRemFrame(fimg_bank, (GABOR_KERNEL_SIZE - 1)/2, (GABOR_KERNEL_SIZE - 1)/2, 0);
    iftDestroyMImage(&aux);
    // iftMImage *fimg_bank_norm = normalizeVolumeByZscore(fimg_bank);

    // t2 - Convoluting with bank of gabor filters
    timer *t2 = iftToc();
    printf("---- t2 : Convoluting with bank of gabor filters ----\n");
    puts(iftFormattedTime(iftCompTime(t1, t2)));
    t2 = iftTic();

    /* Array to save band of max activation */
    int *bmax = (int *)calloc(mimg->n, sizeof(int));

    // Array to save orientation field thetas
    int *thetas = (int *)calloc(mimg->n, sizeof(int));
    
    /* Constructing gabor image from band of activations */

    iftMImage *orientation_vectorimg = getOrientationVectorImage(fimg_bank, bmax, n_orientations, n_frequencies, mask);
    sprintf(filename, "%s_orient", iftBasename(argv[4]));
    saveOrientationImage(orientation_vectorimg, filename);

    // t3 - Calculate orientation vector Image
    timer *t3 = iftToc();
    printf("---- t3 : Calculate orientation vector Image ----\n");
    puts(iftFormattedTime(iftCompTime(t2, t3)));
    t3 = iftTic();


    iftMImage *smooth_orientation_img = getGaussianFilteredOrientations(orientation_vectorimg);
    // iftMImage *smooth_orientation_img = getMedianFilteredOrientations(orientation_vectorimg);

    // t4 - Smoothen and save Orientation Field
    timer *t4 = iftToc();
    printf("---- t4 : Smoothen Orientation Field ----\n");
    puts(iftFormattedTime(iftCompTime(t3, t4)));
    t4 = iftTic();

    iftImage *freq_img =  getFrequencyImageFromSelectedBands(smooth_orientation_img, bmax, n_frequencies, mask);
    iftImage *smooth_freq_img = getSmoothFrequencyImage(freq_img, bmax, n_frequencies);
    sprintf(filename,"%s_freq/%s.png",gabor_dir,basename);
    iftWriteImageByExt(smooth_freq_img, filename);

  
    iftImage *filtered_image = getFilteredImgFromOrientationImg(smooth_orientation_img, fimg_bank, bmax, thetas, n_orientations, n_frequencies, mask);
    sprintf(filename, "of_output/%s.dir", basename);
    saveOrientationField(filtered_image, mask, thetas, filename);


    // t5 - Get final image from orientation field
    timer *t5 = iftToc();
    printf("---- t5 ----\n");
    puts(iftFormattedTime(iftCompTime(t4, t5)));

    // filtered_image = getMostVotedGaborActivByPixelInWindow(fimg_bank, bmax, n_frequencies, mask);
    // iftImage *filtered_image = getSumGaborActivByPixelInWindow(fimg_bank, bmax, n_frequencies, mask);
    //iftImage *filtered_image = getFrequestGaborActivByPixelInWindow(fimg_bank, bmax, n_frequencies, mask);

    

    float scale = ((float)img_size)/((float)orig_resized->xsize);
    // printf("%d x %f = %f\n", filtered_image->xsize, scale, filtered_image->xsize * scale);
    iftImage *filtered_image_rescaled = iftInterp2D(filtered_image, scale, scale);


    /* Saving results */
    sprintf(filename,"%s/%s.png",gabor_dir,basename);
    iftWriteImageByExt(filtered_image_rescaled, filename);


    iftImage *mask_rescaled = iftInterp2D(mask, scale, scale);
    iftImage *mask_rescaled_bin = iftThreshold(mask_rescaled, 30, 255, 255);

    sprintf(filename, "%s/%s.png", masks_dir, basename);
    iftWriteImageByExt(mask_rescaled_bin, filename);

    #ifdef DEBUG
      printKernelBank(K_bank, GABOR_KERNEL_SIZE, GABOR_KERNEL_SIZE);
      // printFilteredImageBank(fimg_bank_norm, "space", 0);
    #endif

    /* Building and save binary image */
      //    iftAdjRel *A = iftCircular(3.0);
    // iftImage* bin_image = iftBelowAdaptiveThreshold(filtered_image, NULL, A, 0.9, 2, 255);
    /* iftImage* bin_image = iftThreshold(filtered_image, 1, 255, 255); */
    //iftDestroyAdjRel(&A);
    // iftImage *open_image = iftOpenBin(bin_image,1.0);

    /* sprintf(filename, "%s_bin.png", argv[4]); */
    /* iftWriteImageByExt(bin_image, filename); */
    /* iftDestroyImage(&bin_image); */
      
    iftDestroyImage(&orig);
    iftDestroyImage(&orig_resized);
    iftDestroyImage(&filtered_image_rescaled);
    iftDestroyImage(&mask_rescaled);
    iftDestroyImage(&mask_rescaled_bin);
    iftDestroyImage(&mask);
    iftDestroyImage(&filtered_image);
    iftDestroyMImage(&smooth_orientation_img);
    iftDestroyMImage(&orientation_vectorimg);
    iftDestroyMImage(&mimg);
    iftDestroyMImage(&padded_mimg);
    iftDestroyMImage(&fimg_bank);

    free(bmax);
    free(thetas);
    iftFree(basename);
  
  }
  iftDestroyMMKernel(&K_bank);

  iftDestroyFileSet(&fs);

  puts("\nDone...");
  puts(iftFormattedTime(iftCompTime(tstart, iftToc())));
  

  return 0;
}
