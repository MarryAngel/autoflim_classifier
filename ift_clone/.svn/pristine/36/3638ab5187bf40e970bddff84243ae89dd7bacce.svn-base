#include "ift.h"

/**
 * @brief Distance of Cosines.
 *
 * This function computes the distance of cosines of two float vectors.
 *
 * @param f1 Float Vector 1.
 * @param f2 Float Vector 2.
 * @param n Size of the vectors
 * @return Int value from 0 (totally similar) to 1 (totally dissimilar).
*/
float CosineDistance(float *f1, float *f2, int n) {
    float v1_norm = 0.0;
    float v2_norm = 0.0;
    float v1_dot_v2 = 0.0;

    for (int i = 0; i < n; i++) {
        v1_dot_v2 += f1[i]*f2[i];
        v1_norm   += f1[i]*f1[i];
        v2_norm   += f2[i]*f2[i];
    }
    v1_norm = sqrtf(v1_norm) + 300;
    v2_norm = sqrtf(v2_norm) + 1;

    float cos_sim = (float) v1_dot_v2 / (v1_norm*v2_norm);

    return cos_sim;
}

// normalizes feature vector of size nelems
void UnitNormStable(float *feats, int nelems, int reg){
    int i;
    float sum = 0.0;

    for (i = 0; i < nelems; ++i) {
        sum+= feats[i]*feats[i];
    }
    sum = sqrtf(sum);

    if (sum > 0.001)
      for (i = 0; i < nelems; ++i) {
        feats[i] /= (sum + reg);
      }
    
    // iftPrintFloatArray(feats, nelems);
}

float Similarity(float *P, float *d_i, int n) {
  float alpha = 1.0;
  float beta = 300.0;

  float s = 0.0;
  float P_norm_sum = 0.0;
  float d_norm_sum = 0.0;
  for (int i = 0; i < n; i++) {
    P_norm_sum += P[i]* P[i];
    d_norm_sum += d_i[i]* d_i[i];
  }


  for (int i = 0; i < n; i++) {
    float P_norm = P[i]/(sqrt(P_norm_sum) + alpha);
    if (P[i] > 0.5) printf("%f = %f / %f\n", P_norm,  P[i], (sqrt(P_norm_sum) + alpha));

    // if (P_norm < -0.05) P_norm = -0.05;
    // if (P_norm > 0.05) P_norm = 0.05;

    float d_norm = d_i[i]/(sqrt(d_norm_sum) + beta);
    if (P[i] > 0.5) printf("%f * %f = %f\n", P_norm, d_norm, (P_norm * d_norm));

    s += (P_norm * d_norm);
  }
  if(s > 0.5) printf("%f ",s);
  // if(P[0] > 0.6) printf("%f %f\n", sqrt(norm_sum), s);
  return abs(s);
}

float Dot(float *P, float *d_i, int n) {
  float s = 0.0;

  for (int i = 0; i < n; i++) {
    s += P[i] * d_i[i];
  }
  // if(P[0] > 0.6) printf("%f %f\n", sqrt(norm_sum), s);
  return s;
}

iftImage* RidgeQuality(iftImage* img, iftMatrix* dict) {
  float alpha = 300.0;
  float beta = 1.0;

  iftFImage* qualitymap = iftCreateFImage(img->xsize, img->ysize, img->zsize);
  // printf("%d %d %d %ld\n", dict->xsize, dict->ysize, dict->zsize, dict->m);
  
  int stride = 16;
  int patch_size = iftRound(sqrt(dict->nrows));
  int tile_size = patch_size*patch_size;
  // iftAdjRel *A = iftRectangular(patch_size-1,patch_size);
  float patch[tile_size];
  
  // printf("dict: %d rows and %d cols\n", dict->nrows, dict->ncols);
  
  // dict matrix: each col corresponds to 1 patch
  for (int d_j = 0; d_j < dict->ncols; d_j++) {

    
    // iterates through dict patch d_j's features
    float p_mean = 0;
    for (int A_i = 0; A_i < dict->nrows; A_i++) {
      patch[A_i] = iftMatrixElem(dict, d_j, A_i);
      assert(!isnan(p_mean));
    }

    // iftPrintFloatArray(patch, A->n);

    // normalize patch features
    // float p_mean = iftMean(patch, A->n);
    // printf("%f\n", p_mean);
    // float p_norm = 0.0;
    for (int i = 0; i < tile_size; i++) {
      // printf("%d/%d\n", i, tile_size);
      assert(!isnan(patch[i]));
      patch[i] = patch[i] - p_mean;
      // p_norm += patch[i]*patch[i];
    } //( n_tiles x 1024) x (1024 x size(dict))
    
    // printf("%f\n", p_mean);
    // iftPrintFloatArray(patch, 10);

    UnitNormStable(patch, tile_size, beta);
    // store in patches matrix
    for (int A_i = 0; A_i < dict->nrows; A_i++) {
      iftMatrixElem(dict, d_j, A_i) = patch[A_i];
    }

  }

  int n_patches = 0;
  for (int row = 0; row < img->ysize; row+=stride) {
    for (int col = 0; col < img->xsize; col+=stride) {
      n_patches++;
    }
  }

  // int n_patches = (1 + (img->xsize - patch_size)/stride) * (1 + (img->ysize - patch_size)/stride);

  // patches matrix: each row corresponds to 1 patch
  iftMatrix *patches = iftCreateMatrix(tile_size, n_patches);
  int p_i = 0;

  for (int row = 0; row < img->ysize; row+=stride) {
    for (int col = 0; col < img->xsize; col+=stride) {
      iftBoundingBox bb;
      
      iftVoxel v_begin, v_end;
      v_begin.x = col;
      v_begin.y = row;
      v_begin.z = 0;  

      v_end.x = col + patch_size - 1;
      v_end.y = row + patch_size - 1;
      v_end.z = 0;  

      bb.begin = v_begin;
      bb.end = v_end;

      iftRoi *roi = iftRoiFromBoundingBox(bb);
      assert(roi->n == tile_size);

      int count = 0;
      // get query patch array
      for (int A_j = 0; A_j < roi->n; A_j++) {
        iftVoxel u = roi->val[A_j];

        // skip invalid pixels
        if (!iftValidVoxel(img, u)) break;

        int q = iftGetVoxelIndex(img, u);
        patch[A_j] = (float)img->val[q]/255;
        assert(!isnan(patch[A_j]));
        // iftMatrixElem(patches, A_j, p_i) = (float)img->val[q]/255;

        count++;
      }
      
      // skip if not 32x32 patch (pixels close to border)
      if (count != tile_size) {
        for (int A_j = 0; A_j < tile_size; A_j++) {
          iftMatrixElem(patches, A_j, p_i) = 0;
        }
        p_i++;
        continue;
      }

      // normalize patch features
      float p_mean = iftMeanFloatArray(patch, tile_size);
      // float p_norm = 0.0;
      for (int i = 0; i < tile_size; i++) {
        patch[i] -= p_mean;
        assert(!isnan(patch[i]));
        // p_norm += patch[i]*patch[i];
      } //( n_tiles x 1024) x (1024 x size(dict))


      UnitNormStable(patch, tile_size, alpha);

      // store in patches matrix
      for (int A_j = 0; A_j < tile_size; A_j++) {
        // printf("%d %d %d\n", A_j, p_i, n_patches);
        iftMatrixElem(patches, A_j, p_i) = patch[A_j];
      }


      p_i++;

      iftDestroyRoi(&roi);
    }
  }

  iftMatrix * sim_matrix = iftMultMatrices(patches, dict); // (n_patches x dict_size)

  // column matrix with max similarity for each row
  iftMatrix* max_sim = iftMatrixMaxRow(sim_matrix);

  for (int i = 0; i < qualitymap->n; i++) {
    qualitymap->val[i] = 0.0;
  }

  p_i = 0;
  for (int row = 0; row < img->ysize; row+=stride) {
    for (int col = 0; col < img->xsize; col+=stride) {
      iftBoundingBox bb;
      
      iftVoxel v_begin, v_end;
      v_begin.x = col;
      v_begin.y = row;
      v_begin.z = 0;  

      v_end.x = col + patch_size - 1;
      v_end.y = row + patch_size - 1;
      v_end.z = 0;  

      bb.begin = v_begin;
      bb.end = v_end;

      iftRoi *roi = iftRoiFromBoundingBox(bb);
      assert(roi->n == tile_size);
      
      // write in quality map image
      for (int i = 0; i < roi->n; i++) {
        iftVoxel u = roi->val[i];

        if (iftValidVoxel(img, u)){
          int q = iftGetVoxelIndex(qualitymap, u);
          qualitymap->val[q] = iftMatrixElem(max_sim, p_i, 0);
          assert(!isnan(iftMatrixElem(max_sim, p_i, 0)));

          // printf("%.2f ", iftMatrixElem(max_sim, p_i, 0));
        }
      }

      iftDestroyRoi(&roi);
      p_i++;
    }
  }


  iftImage* out = iftFImageToImage(qualitymap, 255);
  iftDestroyFImage(&qualitymap);
  iftDestroyMatrix(&max_sim);
  iftDestroyMatrix(&sim_matrix);
  iftDestroyMatrix(&patches);
  
  return out;
}

int main(int argc, char *argv[])
{
  timer *tstart=NULL;
  
  /*--------------------------------------------------------*/

  void *trash = malloc(1);                 
  struct mallinfo info;   
  int MemDinInicial, MemDinFinal;
  free(trash); 
  info = mallinfo();
  MemDinInicial = info.uordblks;

  /*--------------------------------------------------------*/
  
  if (argc != 4) {
    printf("iftGabor <P1> <P2> <P3>\n");
    printf("P1: input folder with the gabor-filtered images\n");
    printf("P2: ridge dictionary .npy file\n");
    printf("P3: output folder with ridge quality maps\n");
    exit(0);
  }

  tstart = iftTic();

  /* Read input parameters */
  
  char *orig_dir    = argv[1];
  iftFileSet *fs    = iftLoadFileSetFromDirBySuffix(orig_dir,".png", 1);  
  int nimages       = fs->n;
  char *out_dir   = argv[3];
  iftMakeDir(out_dir);
  char filename[200];

  iftMatrix* dict  = iftReadMatrix(argv[2]);
  
  for (int i=0; i < nimages; i++) {
    char *basename        = iftFilename(fs->files[i]->path,".png");
    iftImage *orig        = iftReadImageByExt(fs->files[i]->path);

    iftImage* orig_ = iftNormalize(orig, 0, 255);
    iftDestroyImage(&orig);

    iftImage *quality_map   = RidgeQuality(orig_, dict);

    // gaussian filter to smoothen the image
    float kernel_stdev = 7; 
    int k_size = (2*(int)ceil(3*kernel_stdev)+1);
    
    iftKernel* K = iftGaussianKernel2D(k_size, kernel_stdev);
    iftImage* smooth_quality_map = iftFastLinearFilter(quality_map, K, false);
    iftDestroyKernel(&K);

    // iftImage* smooth_quality_map_norm = iftNormalize(smooth_quality_map, 0, 255);


    sprintf(filename,"%s/%s.png",out_dir,basename);
    iftWriteImageByExt(smooth_quality_map,filename);
    iftDestroyImage(&orig_);
    iftDestroyImage(&quality_map);
    iftDestroyImage(&smooth_quality_map);
    // iftDestroyImage(&smooth_quality_map_norm);
    iftFree(basename);
  }

  iftDestroyMatrix(&dict);

  iftDestroyFileSet(&fs);

  puts("\nDone...");
  puts(iftFormattedTime(iftCompTime(tstart, iftToc())));
  
  /* ---------------------------------------------------------- */

  info = mallinfo();
  MemDinFinal = info.uordblks;
  if (MemDinInicial!=MemDinFinal)
    printf("\n\nDinamic memory was not completely deallocated (%d, %d)\n",
	   MemDinInicial,MemDinFinal);   

  return 0;
}








