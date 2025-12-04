#include "ift.h"
#include "include/lapacke_wrapper.h"
#define color_space LABNorm2_CSPACE

typedef struct {
  int seed_idx;
  int elem;
  int label;
  int selected;
  float similarity_coefficient;
} SeedScore;

iftDataSet *ExtractPatchDataset(
  iftMImage *mimg, iftLabeledSet *seeds, iftAdjRel *A
) {
  int n_seeds = iftLabeledSetSize(seeds);
  int n_features = A->n * mimg->m;
  printf(
    "[INFO] Building a dataset of %d patches with %d features each\n",
    n_seeds, n_features
  );
  iftDataSet *patch_dataset = iftCreateDataSet(n_seeds, n_features);

  iftLabeledSet *s_aux = seeds;
  int s = 0;
  while (s_aux != NULL) {
    iftVoxel u = iftMGetVoxelCoord(mimg, s_aux->elem);
    patch_dataset->sample[s].id = s_aux->elem;
    patch_dataset->sample[s].truelabel = s_aux->label;

    int j = 0;
    for (int k = 0; k < A->n; k++) {
      iftVoxel v = iftGetAdjacentVoxel(A, u, k);
      if (iftMValidVoxel(mimg, v)) {
        int q = iftMGetVoxelIndex(mimg, v);
        for (int b=0; b < mimg->m; b++) {
          patch_dataset->sample[s].feat[j++] = mimg->val[q][b];
        }
      }
      else {
        for (int b=0; b < mimg->m; b++) {
          patch_dataset->sample[s].feat[j++] = 0;
        }
      }
    }
    s_aux = s_aux->next;
    s++;
  }

  iftSetStatus(patch_dataset, IFT_TRAIN);

  return patch_dataset;
}

// Double check how to compute the Covariance
iftMatrix *ComputeCovarianceMatrix(iftDataSet *dataset) {
  int n_samples = dataset->nsamples;
  int n_features = dataset->nfeats;

  // Compute mean for each feature
  float *mean = iftAllocFloatArray(n_features);
  for (int i = 0; i < n_samples; i++) {
    for (int j = 0; j < n_features; j++) {
      mean[j] += dataset->sample[i].feat[j];
    }
  }
  for (int j = 0; j < n_features; j++) {
    mean[j] /= n_samples;
  }

  // Compute covariance matrix: S = (1/n) * sum[(x_i - mean)(x_i - mean)^T]
  iftMatrix *covariance_matrix = iftCreateMatrix(n_features, n_features);
  for (int i = 0; i < n_features; i++) {
    // Symmetric matrix
    for (int j = i; j < n_features; j++) {
      float sum = 0.0;
      for (int s = 0; s < n_samples; s++) {
        float diff_i = dataset->sample[s].feat[i] - mean[i];
        float diff_j = dataset->sample[s].feat[j] - mean[j];
        sum += diff_i * diff_j;
      }
      float covariance_value = sum / n_samples;
      iftMatrixElem(covariance_matrix, j, i) = covariance_value;
      iftMatrixElem(covariance_matrix, i, j) = covariance_value;
    }
  }

  iftFree(mean);

  return covariance_matrix;
}

float ComputeMahalanobisDistance(
  float *feature_1, float *feature_2, iftMatrix *cov_inv, int n_features
) {
  float *diff = iftAllocFloatArray(n_features);
  for (int i = 0; i < n_features; i++) {
    diff[i] = feature_1[i] - feature_2[i];
  }

  float *tmp = iftAllocFloatArray(n_features);
  for (int i = 0; i < n_features; i++) {
    for (int j = 0; j < n_features; j++) {
      tmp[i] += iftMatrixElem(cov_inv, j, i) * diff[j];
    }
  }

  float distance = 0.0;
  for (int i = 0; i < n_features; i++) {
    distance += diff[i] * tmp[i];
  }

  iftFree(diff);
  iftFree(tmp);

  return sqrtf(fabsf(distance));
}

SeedScore *ComputeSimilarityCoefficientes(
  iftDataSet *dataset, iftLabeledSet *seeds
) {
  int n_samples = dataset->nsamples;
  int n_features = dataset->nfeats;

  printf("[INFO] Computing covariance matrix...\n");
  iftMatrix *covariance_matrix = ComputeCovarianceMatrix(dataset);

  printf("[INFO] Computing inverse covariance matrix...\n");
  // Copies covariance matrix (LAPACK will modify it) -- Separate this function
  iftMatrix *covariance_inv_matrix = iftCopyMatrix(covariance_matrix);
  // Cholesky decomposition (covariance matrices are positive definite)
  printf("[INFO] Verifying Cholesky Decomposition...\n");
  int info = LAPACKE_spotrf(
    LAPACK_ROW_MAJOR,
    'L',
    n_features,
    covariance_inv_matrix->val,
    n_features
  );

  if (info == 0) {
    printf("[INFO] Computing inverse using Cholesky...\n");
    info = LAPACKE_spotri(
      LAPACK_ROW_MAJOR,
      'L',
      n_features,
      covariance_inv_matrix->val,
      n_features
    );

    if (info != 0) {
      printf("[ERROR] Cholesky inversion failed\n");
      iftDestroyMatrix(&covariance_matrix);
      iftDestroyMatrix(&covariance_inv_matrix);
      return NULL;
    }

    // Copy lower triangle to upper triangle
    for (int i = 0; i < n_features; i++) {
      for (int j = i + 1; j < n_features; j++) {
        iftMatrixElem(covariance_inv_matrix, i, j) = iftMatrixElem(covariance_inv_matrix, j, i);
      }
    }
  }
  else {
    printf("[INFO] Falling back to LU as covariance matrix is not positive definite\n");
    // Verifies LU factorization
    iftDestroyMatrix(&covariance_inv_matrix);
    covariance_inv_matrix = iftCopyMatrix(covariance_matrix);
    int *ipiv = malloc(n_features * sizeof(int));
    info = LAPACKE_sgetrf(
      LAPACK_ROW_MAJOR,
      n_features,
      n_features,
      covariance_inv_matrix->val,
      n_features,
      ipiv
    );

    if (info != 0) {
      printf("[ERROR] LU factorization failed\n");
      iftFree(ipiv);
      iftDestroyMatrix(&covariance_matrix);
      iftDestroyMatrix(&covariance_inv_matrix);
      return NULL;
    }

    // Invert using LU
    info = LAPACKE_sgetri(
      LAPACK_ROW_MAJOR,
      n_features,
      covariance_inv_matrix->val,
      n_features,
      ipiv
    );

    free(ipiv);

    if (info != 0) {
      printf("[ERROR] LU inversion failed\n");
      iftDestroyMatrix(&covariance_matrix);
      iftDestroyMatrix(&covariance_inv_matrix);
      return NULL;
    }
  }

  // Store seed information
  SeedScore *scores = (SeedScore *) calloc(
    n_samples, sizeof(SeedScore)
  );
  iftLabeledSet *s_aux = seeds;
  for (int i = 0; i < n_samples && s_aux != NULL; i++) {
    scores[i].seed_idx = i;
    scores[i].elem = s_aux->elem;
    scores[i].label = s_aux->label;
    scores[i].similarity_coefficient = 0.0;
    s_aux = s_aux->next;
  }

  printf("[INFO] Computing pairwise Mahalanobis distances...\n");
  // Compute pairwise distances for each sample in the patches dataset
  #pragma omp parallel for schedule(dynamic)
  for (int i = 0; i < n_samples; i++) {
    float sum_distances = 0.0;

    for (int j = 0; j < n_samples; j++) {
      if (i != j) {
        float dist = ComputeMahalanobisDistance(
          dataset->sample[i].feat,
          dataset->sample[j].feat,
          covariance_inv_matrix,
          n_features
        );
        sum_distances += dist;
      }
    }
    scores[i].similarity_coefficient = sum_distances;
  }

  iftDestroyMatrix(&covariance_matrix);
  iftDestroyMatrix(&covariance_inv_matrix);

  return scores;
}

int CompareSeedScore(const void *a, const void *b) {
  SeedScore *score_a = (SeedScore *)a;
  SeedScore *score_b = (SeedScore *)b;

  float diff = score_b->similarity_coefficient - score_a->similarity_coefficient;

  if (diff > 0) return 1;
  if (diff < 0) return -1;
  return 0;
}

// Write the pseudo code, understanding each step
iftLabeledSet *SelectSeedsBySimilarity(
  SeedScore *scores, int n_seeds, float *mean_similarity, float perc_bofp
) {
  *mean_similarity = 0.0;
  for (int i = 0; i < n_seeds; i++) {
    *mean_similarity += scores[i].similarity_coefficient;
  }
  *mean_similarity /= n_seeds;
  printf("[INFO] Mean similarity coefficient: %.4f\n", *mean_similarity);

  int n_above = 0, n_below = 0;
  float max_sim = scores[0].similarity_coefficient;
  float min_sim = scores[0].similarity_coefficient;

  for (int i = 0; i < n_seeds; i++) {
    if (scores[i].similarity_coefficient >= *mean_similarity) {
      n_above++;
    }
    else {
      n_below++;
    }

    if (scores[i].similarity_coefficient > max_sim) {
      max_sim = scores[i].similarity_coefficient;
    }
    if (scores[i].similarity_coefficient < min_sim) {
      min_sim = scores[i].similarity_coefficient;
    }
  }

  printf("[INFO] Similarity range: [%.4f, %.4f]\n", min_sim, max_sim);
  printf("[INFO] Seeds above mean: %d, below mean: %d\n", n_above, n_below);

  qsort(scores, n_seeds, sizeof(SeedScore), CompareSeedScore);

  int n_keep = (int)(n_seeds * perc_bofp);
  // float threshold = *mean_similarity;
  float threshold = scores[n_keep-1].similarity_coefficient;

  iftLabeledSet *selected_seeds = NULL;
  for (int i = 0; i < n_seeds; i++) {
    if (scores[i].similarity_coefficient >= threshold) {
      scores[i].selected = 1;
      iftInsertLabeledSet(&selected_seeds, scores[i].elem, scores[i].label);
    }
    else {
      scores[i].selected = 0;
    }
  }

  printf("Reduced labeled set has size of %d\n", iftLabeledSetSize(selected_seeds));

  return selected_seeds;
}

int main(int argc, char **argv) {
  timer *tstart = iftTic();
  int memory_start = iftMemoryUsed();

  if (argc != 7) {
    iftError(
      "Usage: iftSelectBoFP <input_imgs> <input_seeds> <ksize> <kdilation> <output_folder> <perc_bofp>\n"
      "   input_imgs: Path to folder with input images\n"
      "   input_seeds: Path to folder with seeds files\n"
      "   ksize: kernel size for patch extraction (e.g., 5 for 5x5)\n"
      "   kdilation: dilation rate (e.g., 1 for no dilation)\n"
      "   output_folder: Output folder to save reduced seeds\n"
      "   perc_bofp: Output folder to save reduced seeds\n",
      "main"
    );
  }

  iftFileSet *fs = iftLoadFileSetFromDirBySuffix(
    argv[2], "-seeds.txt", 1
  );
  int ksize = atoi(argv[3]);
  int kdilation = atoi(argv[4]);
  char *output_folder = argv[5];
  iftMakeDir(output_folder);
  char *img_path = iftAllocCharArray(512);
  char *output_path = iftAllocCharArray(512);
  float perc_bofp = atof(argv[6]);

  for (size_t i=0; i < fs->n; i++) {
    char *img_filename = iftFilename(fs->files[i]->path, "-seeds.txt");
    printf("[INFO] Processing image %s\n", img_filename);

    // Reads image and performs conversion to LAB Norm 2 Color Space
    sprintf(img_path, "%s/%s.png", argv[1], img_filename);
    iftImage *img = iftReadImageByExt(img_path);
    iftMImage *mimg = iftImageToMImage(img, color_space);

    printf(
      "[INFO] Image size: %d x %d x %d | Bands: %ld\n",
      mimg->xsize, mimg->ysize, mimg->zsize, mimg->m
    );

    printf("[INFO] Reading seeds file...\n");
    int img_Z = iftIs3DMImage(mimg) ? 3 : 2;
    iftLabeledSet *seeds = iftReadLabeledSet(fs->files[i]->path, img_Z);
    int n_seeds = iftLabeledSetSize(seeds);
    printf("[INFO] Loaded %d seeds!\n", n_seeds);

    printf("[INFO] Creating adjacency relation...\n");
    iftAdjRel *A;
    if (img_Z == 3) {
      A = iftCuboidWithDilationForConv(
        ksize, ksize, ksize, kdilation, kdilation, kdilation
      );
    }
    else {
      A = iftRectangularWithDilation(
        ksize, ksize, kdilation, kdilation
      );
    }
    printf("[INFO] Adjacency size: %d spels!\n", A->n);

    printf("[INFO] Step 1: Extracting patches features and building patches dataset\n");
    iftDataSet *patch_dataset = ExtractPatchDataset(mimg, seeds, A);

    printf("[INFO] Step 2: Computing similarity coefficients (Mahalanobis distance-based)\n");
    SeedScore *scores = ComputeSimilarityCoefficientes(patch_dataset, seeds);

    printf("[INFO] Step 3: Selecting seeds based on similarity threshold\n");
    float mean_similarity;
    iftLabeledSet *selected_seeds = SelectSeedsBySimilarity(
      scores, n_seeds, &mean_similarity, perc_bofp
    );
    int n_selected = iftLabeledSetSize(selected_seeds);
    float reduction_percent = (100.0 * (n_seeds - n_selected)) / ( (float) n_seeds);

    printf("*******************************************\n");
    printf("[INFO] Original seeds: %d\n", n_seeds);
    printf("[INFO] Reduced seeds: %d\n", n_selected);
    printf("[INFO] Reduce percentage: %f\n", reduction_percent);
    printf("*******************************************\n");

    sprintf(output_path, "%s/%s-fpts.txt", output_folder, img_filename);
    iftWriteLabeledSet(
      selected_seeds, mimg->xsize, mimg->ysize, mimg->zsize, output_path
    );
    printf("[INFO] Reduced labeled set saved to %s\n", output_path);
    sprintf(output_path, "%s/%s-scores.csv", output_folder, img_filename);
    FILE *fp = fopen(output_path, "w");
    if (fp) {
      fprintf(fp, "seed_idx;voxel_idx;label;similarity_coefficient;selected\n");
      for (int i =0; i < n_seeds; i++) {
        fprintf(
          fp, "%d;%d;%d;%.6f;%d\n",
          scores[i].seed_idx,
          scores[i].elem,
          scores[i].label,
          scores[i].similarity_coefficient,
          scores[i].selected
        );
      }
      fclose(fp);
      printf("[INFO] Similarity scores saved to %s\n", output_path);
    }

    iftFree(scores);
    iftFree(img_filename);
    iftDestroyImage(&img);
    iftDestroyMImage(&mimg);
    iftDestroyLabeledSet(&seeds);
    iftDestroyAdjRel(&A);
    iftDestroyDataSet(&patch_dataset);
    iftDestroyLabeledSet(&selected_seeds);
  }

  iftFree(img_path);
  iftDestroyFileSet(&fs);

  printf("\n ** Done! ** \n");
  puts(iftFormattedTime(iftCompTime(tstart, iftToc())));
  int memory_end = iftMemoryUsed();
  iftVerifyMemory(memory_start, memory_end);

  return 0;
}