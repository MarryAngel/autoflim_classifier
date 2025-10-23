#include "include/iftCreateSortedLayerModel.h"
#define color_space LABNorm2_CSPACE

iftAdjRel *GetPatchAdjacency(iftMImage *mimg, iftFLIMLayer layer)
{
  iftAdjRel *A;

  if (iftIs3DMImage(mimg))
  {
    printf("[INFO] Defining 3D Adjacency\n");
    A = iftCuboidWithDilationForConv(
      layer.kernel_size[0],
      layer.kernel_size[1],
      layer.kernel_size[2],
      layer.dilation_rate[0],
      layer.dilation_rate[1],
      layer.dilation_rate[2]
    );
  }
  else
  {
    printf("[INFO] Defining 2D Adjacency\n");
    A = iftRectangularWithDilationForConv(
      layer.kernel_size[0],
      layer.kernel_size[1],
      layer.dilation_rate[0],
      layer.dilation_rate[1]
    );
  }

  return (A);
}

// If necessary, downsample and updates the labeled set (remove identical elements)
void iftDownsamplesLabeledSet(
  iftMImage *src, iftMImage *dst, iftLabeledSet **S
) {
  // Downsample features directly here, them remove from bellow (Merge elements in the same place)
  iftLabeledSet *S_aux = (*S);
  iftLabeledSet *S_downsampled = NULL;
  float scale[3];
  scale[0] = (float) dst->xsize / (float) src->xsize;
  scale[1] = (float) dst->ysize / (float) src->ysize;
  scale[2] = (float) dst->zsize / (float) src->zsize;
  while (S_aux != NULL) {
    iftVoxel u = iftMGetVoxelCoord(src, S_aux->elem);
    u.x = (int)((float) u.x * scale[0] + 0.5);
    u.y = (int)((float) u.y * scale[1] + 0.5);
    u.z = (int)((float) u.z * scale[2] + 0.5);
    int tmp_elem = iftMGetVoxelIndex(dst, u);

    if (!iftLabeledSetHasElement(S_downsampled, tmp_elem)) {
      iftInsertLabeledSetMarkerAndHandicap(
        &S_downsampled, tmp_elem, S_aux->label, S_aux->marker,
        S_aux->handicap
      );
    }
    S_aux = S_aux->next;
  }
  printf("[INFO] Old Set had %d elements\n", iftLabeledSetSize(*S));
  printf("[INFO] New Set has %d elements\n", iftLabeledSetSize(S_downsampled));

  iftDestroyLabeledSet(S);
  (*S) = S_downsampled;
}

iftDataSet * GetFeaturesDataset(
  iftMImage *mimg, iftMImage *mfeatures, iftFLIMArch *arch, int layer,
  iftLabeledSet **S, int class
) {
    iftMImage *maux = NULL;
    if (layer == 1) {
      printf("[INFO] No downsampling for first layer\n");
      maux = mimg;
    } else {
      printf("[INFO] Downsampling labeled set if necessary (has downsampling?)\n");
      maux = mfeatures;
      iftDownsamplesLabeledSet(mimg, mfeatures, S);
    }

    iftAdjRel *A = GetPatchAdjacency(maux, arch->layer[layer - 1]);
    int n_features = A->n * maux->m;
    printf("[INFO] Adjacency relationship defines %d features\n", n_features);

    iftDataSet *patch_dataset = iftCreateDataSet(
      iftLabeledSetSize(*S), n_features
    );


    iftLabeledSet *Saux = *S;
    int s = 0;
    while (Saux != NULL) {
      iftVoxel u = iftMGetVoxelCoord(mimg, Saux->elem);

      patch_dataset->sample[s].id = Saux->elem;
      patch_dataset->sample[s].weight = (float)Saux->handicap;
      patch_dataset->sample[s].truelabel = class;
      
      // Reading only seeds from a specific class into the dataset
      patch_dataset->nclasses = 1;
      
      // Iterates over the mimage reading the features
      int j = 0;
      for (int k = 0; k < A->n; k++) {
        iftVoxel v = iftGetAdjacentVoxel(A, u, k);
        if (iftMValidVoxel(maux, v)) {
          int q = iftMGetVoxelIndex(maux, v);
          for (int b = 0; b < maux->m; b++) {
            patch_dataset->sample[s].feat[j] = maux->val[q][b];
            j++;
          }
        }
        else {
          for (int b = 0; b < maux->m; b++) {
            patch_dataset->sample[s].feat[j] = 0;
            j++;
          }
        }
      }

      s++;
      Saux = Saux->next;
    }

    iftSetStatus(patch_dataset, IFT_TRAIN);
    iftAddStatus(patch_dataset, IFT_SUPERVISED);

    return patch_dataset;
}

iftMatrix *ComputeDistanceMatrix(
  iftDataSet *Z, float (metric) (float *, float *, int)
) {
  iftMatrix *M = iftCreateMatrix(
    Z->nsamples, Z->nsamples
  );
  
  #pragma omp parallel for schedule(dynamic)
  for (size_t i=0; i < M->nrows; i++) {
    float distance; // Private variable for each thread
    for (size_t j=0; j < M->ncols; j++) {
      if (i == j) {
        // matrix, col, row
        distance = 0;
      }
      else {
        distance = iftCosineDistance2(
          Z->sample[i].feat,
          Z->sample[j].feat,
          Z->nfeats
        );
      }

      iftMatrixElem(M, j, i) = distance;
    }
  }

  return M;
}

iftMatrix *VerifySimilarities(iftDataSet *Z, iftMatrix *M) {
    // 6 columns:
    // 0. Intra Class Average Similarity
    // 1. Inter Class Average Similarity  
    // 2. Intra Class Average Correlation
    // 3. Inter Class Average Correlation
    // 4. Intra Class Average Cross-Correlation
    // 5. Inter Class Average Cross-Correlation
    // 6 cols x Z->nsamples rows
    iftMatrix *similarity_M = iftCreateMatrix(6, Z->nsamples);
    
    // Parallelize the outer loop - each sample processed independently
    #pragma omp parallel for schedule(dynamic, 1)
    for (size_t i = 0; i < Z->nsamples; i++) {
        // Private variables for each thread
        int n_intra = 0, n_inter = 0;
        int i_class = Z->sample[i].truelabel;
        float sum_similarity_intra = 0.0f, sum_similarity_inter = 0.0f;
        float sum_cor_intra = 0.0f, sum_cor_inter = 0.0f;
        float sum_cross_cor_intra = 0.0f, sum_cross_cor_inter = 0.0f;
        
        for (size_t j = 0; j < Z->nsamples; j++) {
            if (i == j) continue;
            
            float correlation_dist = iftCorrCoefDist(
                Z->sample[i].feat, Z->sample[j].feat, Z->nfeats
            );
            float cross_correlation_dist = iftCrossCorrCoefDist(
                Z->sample[i].feat, Z->sample[j].feat, Z->nfeats
            );

            int j_class = Z->sample[j].truelabel;
            
            if (i_class == j_class) {
                // INTRA-class (same class)
                n_intra++;
                sum_similarity_intra += iftMatrixElem(M, j, i);
                sum_cor_intra += correlation_dist;
                sum_cross_cor_intra += cross_correlation_dist;
            } else {
                // INTER-class (different classes)
                n_inter++;
                sum_similarity_inter += iftMatrixElem(M, j, i);
                sum_cor_inter += correlation_dist;
                sum_cross_cor_inter += cross_correlation_dist;
            }
        }
        
        // Store results
        iftMatrixElem(similarity_M, 0, i) = sum_similarity_intra / n_intra;
        iftMatrixElem(similarity_M, 1, i) = sum_similarity_inter / n_inter;
        iftMatrixElem(similarity_M, 2, i) = sum_cor_intra / n_intra;
        iftMatrixElem(similarity_M, 3, i) = sum_cor_inter / n_inter;
        iftMatrixElem(similarity_M, 4, i) = sum_cross_cor_intra / n_intra;
        iftMatrixElem(similarity_M, 5, i) = sum_cross_cor_inter / n_inter;
    }

    return similarity_M;
}

iftFilterScore * ComputeFilterScores(iftDataSet *Z, iftMatrix *M) {
  printf("[INFO] Computing scores of %d filters\n", M->nrows);
  iftFilterScore *scores = (iftFilterScore *) calloc(
    M->nrows, sizeof(iftFilterScore)
  );

  for (size_t i=0; i < M->nrows; i++) {
    scores[i].filter_index = i;
    scores[i].class_id = Z->sample[i].truelabel;
    
    scores[i].intra_cos = iftMatrixElem(M, 0, i);
    scores[i].inter_cos = iftMatrixElem(M, 1, i);
    scores[i].intra_corr = iftMatrixElem(M, 2, i);
    scores[i].inter_corr = iftMatrixElem(M, 3, i);
    scores[i].intra_cross_corr = iftMatrixElem(M, 4, i);
    scores[i].inter_cross_corr = iftMatrixElem(M, 5, i);

    // [TODO] Verify best approaches to compute the composite score
    scores[i].cosine_score = scores[i].intra_cos - scores[i].inter_cos;
    scores[i].correlation_score = scores[i].inter_corr - scores[i].intra_corr;
    scores[i].cross_correlation_score = scores[i].inter_cross_corr - scores[i].intra_cross_corr;
    scores[i].composite_score = 0.3f * scores[i].cosine_score +
                                0.6f * scores[i].correlation_score +
                                0.1f * scores[i].cross_correlation_score;
  }

  return scores;
}

int iftcompareFiltersScore(const void *a, const void *b) {
  iftFilterScore *f1 = (iftFilterScore*) a;
  iftFilterScore *f2 = (iftFilterScore*) b;

  if (f1->composite_score > f2->composite_score) {
    return -1;
  }
  if (f1->composite_score < f2->composite_score) {
    return 1;
  }

  return 0;
}


iftFilterScoreSummary *AnalyseScores(
  iftFilterScore *filter_scores, int n_total_filters, int n_classes
) {
  iftFilterScoreSummary *summary = (iftFilterScoreSummary *) calloc(
    n_classes, sizeof(iftFilterScoreSummary)
  );
  summary->n_classes = n_classes;
  printf("[INFO] Sorting the filters\n");
  // [TODO] Better improve it to avoid re selecting filters
  // Iterate over each class scoring its filters
  for (size_t class_id = 0; class_id < n_classes; class_id++) {
    // [TODO] Optimize it --- MANDATORY
    // Computes n o filters per class, them allocates scores for class
    int class_filter_count = 0;
    for (size_t i = 0; i < n_total_filters; i++) {
      if (filter_scores[i].class_id == class_id) {
        class_filter_count++;
      }
    }
    iftFilterScore *class_filters_scores = (iftFilterScore*) calloc(
      class_filter_count, sizeof(iftFilterScore)
    );
    int j = 0;
    for (size_t i = 0; i < n_total_filters; i++) {
      if (filter_scores[i].class_id == class_id) {
        class_filters_scores[j++] = filter_scores[i];
      }
    }
    // sorts class_filters_scores
    qsort(
      class_filters_scores, class_filter_count, sizeof(iftFilterScore),
      iftcompareFiltersScore
    );

    //cos, corr, cross_cor, and composite
    float sums[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    for (size_t i = 0; i < class_filter_count; i++) {
      sums[0] += class_filters_scores[i].cosine_score;
      sums[1] += class_filters_scores[i].correlation_score;
      sums[2] += class_filters_scores[i].cross_correlation_score;
      sums[3] += class_filters_scores[i].composite_score;
    }

    summary[class_id].class_id = class_id;
    for (size_t i = 0; i < 4; i++) {
      summary[class_id].avg_score[i] = sums[i] / class_filter_count;
    }
    summary[class_id].max_score[0] = class_filters_scores[0].cosine_score;
    summary[class_id].max_score[1] = class_filters_scores[0].correlation_score;
    summary[class_id].max_score[2] = class_filters_scores[0].cross_correlation_score;
    summary[class_id].max_score[3] = class_filters_scores[0].composite_score;
    summary[class_id].min_score[0] = class_filters_scores[class_filter_count - 1].cosine_score;
    summary[class_id].min_score[1] = class_filters_scores[class_filter_count - 1].correlation_score;
    summary[class_id].min_score[2] = class_filters_scores[class_filter_count - 1].cross_correlation_score;
    summary[class_id].min_score[3] = class_filters_scores[class_filter_count - 1].composite_score;
    summary[class_id].n_filters = class_filter_count;
    // Points to the sorted filter
    summary[class_id].sorted_filters = class_filters_scores;
  }

  return summary;
}

void iftDestroyFilterScoreSummary(iftFilterScoreSummary **summary) {
  if (summary == NULL || *summary == NULL) return;
  
  for (int i = 0; i < (*summary)->n_classes; i++) {
    if ((*summary)[i].sorted_filters != NULL) {
      iftFree((*summary)[i].sorted_filters);
      (*summary)[i].sorted_filters = NULL;
    }
  }

  iftFree(*summary);
  *summary = NULL;
}

void iftReportFilterScoring(
  iftFilterScoreSummary *summary, int n_total_filters, int n_classes
) {
  printf("\n######## FILTER SCORING REPORT ########\n");
  printf("Total filters: %d, Classes: %d\n\n", n_total_filters, n_classes);
  
  for (int i = 0; i < n_classes; i++) {
    printf("[INFO] CLASS %d: %d filters\n", i, summary[i].n_filters);
    
    if (summary[i].n_filters == 0) {
      printf("  No filters for this class\n\n");
      continue;
    }
    
    printf("  - Average scores: Cosine=%.4f Correlation=%.4f Cross-Correlation=%.4f Composite=%.4f\n",
            summary[i].avg_score[0], summary[i].avg_score[1], 
            summary[i].avg_score[2], summary[i].avg_score[3]);
            
    printf("  - Max scores:     Cosine=%.4f Correlation=%.4f Cross-Correlation=%.4f Composite=%.4f\n",
            summary[i].max_score[0], summary[i].max_score[1],
            summary[i].max_score[2], summary[i].max_score[3]);
            
    printf("  - Min scores:     Cosine=%.4f Correlation=%.4f Cross-Correlation=%.4f Composite=%.4f\n",
            summary[i].min_score[0], summary[i].min_score[1],
            summary[i].min_score[2], summary[i].min_score[3]);
  }
  printf("################################################################\n");
}

iftDataSet *iftComposeLayerFilterBank(
  iftFilterScoreSummary *summary, iftDataSet *merged_patches_dataset,
  iftFLIMArch *arch, int layer
) {
  /* [TODO] How to normalize the selected filters?

  Currently, I normalize all the filters (the selected ones as a single Dataset)
  */
  int total_filters = arch->layer[layer - 1].noutput_channels;
  printf("[INFO] Learning a total of %d\n", total_filters);
  iftDataSet *selected_filters = iftCreateDataSet(
    total_filters, merged_patches_dataset->nfeats
  );
  /* [TODO] Select N Filters according to class difficulties
  
  Currently, I select the same ammount of classes for each filter.
  */
  int n_filters_per_class = total_filters / merged_patches_dataset->nclasses;
  printf("[INFO] Learning %d filter(s) for each class \n", n_filters_per_class);
  int class_idx = 1;
  int best_filter_idx = 0;
  for(size_t i = 0; i < total_filters; i++) {
    int filter_index = summary[class_idx].sorted_filters[best_filter_idx].filter_index;
    iftCopySample(
      &merged_patches_dataset->sample[filter_index],
      &selected_filters->sample[i],
      merged_patches_dataset->nfeats,
      true
    );
    best_filter_idx++;

    if ((best_filter_idx % n_filters_per_class) == 0) {
      best_filter_idx = 0;
      class_idx++;
    }
  }
  iftSetStatus(selected_filters, IFT_TRAIN);
  iftAddStatus(selected_filters, IFT_SUPERVISED);
  iftNormalizeDataSetByZScoreInPlace(selected_filters, NULL, arch->stdev_factor);

  return selected_filters;
}

void iftSetKernelParameters(
  iftDataSet *Z, iftMatrix **kernels, int **true_labels, float **bias
) {
  *kernels = iftCreateMatrix(Z->nsamples, Z->nfeats);
  *true_labels = iftAllocIntArray(Z->nsamples);

  for (size_t s = 0; s <  Z->nsamples; s++) {
    (*true_labels)[s] = Z->sample[s].truelabel;

    iftUnitNorm(Z->sample[s].feat, Z->nfeats);
    for (size_t row = 0; row < Z->nfeats; row++) {
      iftMatrixElem(*kernels, s, row) = Z->sample[s].feat[row];
    }
  }

  *bias = iftAllocFloatArray((*kernels)->ncols);
  for (size_t col = 0; col < (*kernels)->ncols; col++) {
    for (size_t row = 0; row < (*kernels)->nrows; row++) {
      iftMatrixElem((*kernels), col, row) = iftMatrixElem(
        (*kernels), col, row
      ) / Z->fsp.stdev[row];
      (*bias)[col] -= (Z->fsp.mean[row] * iftMatrixElem((*kernels), col, row));
    }
  }
}

void iftSaveBiasOrWeights(
  char *filename, void *W, int n_kernels, iftCDataType type
) {
  FILE *fp;

  fp = fopen(filename, "w");
  fprintf(fp, "%d\n", n_kernels);
  for (int k = 0; k < n_kernels; k++) {
    if (type == IFT_FLT_TYPE) {
      fprintf(fp, "%f ", ((float*)W)[k]);
    } else if (type == IFT_INT_TYPE) {
      fprintf(fp, "%d ", ((int*)W)[k]);
    }
  }
  fprintf(fp, "\n");
  fclose(fp);
}



/*
Running for layer 1:

iftCreateSortedLayerModel \
  /home/john_doe/msc/2_dataset/parasites_with_mask/eggs/orig/ \
  None ../../flim_crispim/BoFP_runs/split1/bofp_sp_with_mask/ \
  ../../flim_crispim/arch2D.json {layer=1} \
  ../../flim_crispim/BoFP_runs/split1/model/

Running for layer 2 to N:

iftCreateSortedLayerModel \
  /home/john_doe/msc/2_dataset/parasites_with_mask/eggs/orig/ \
  layer1/ \
  ../../flim_crispim/BoFP_runs/split1/bofp_sp_with_mask/ \
  ../../flim_crispim/arch2D.json {layer=2} ../../flim_crispim/BoFP_runs/split1/model/
*/
int main(int argc, char **argv) {
  timer *tstart;
  int memory_start, memory_end;
  memory_start = iftMemoryUsed();
  tstart = iftTic();

  if (argc != 7) {
    iftError(
      "Usage: iftCreateSortedLayerModel P1 P2 P3 P4 P5 P6\n"
      "P1: input folder with original input images (e.g., .png, .jpeg, .nii.gz, .mimg)\n"
      "P2: input folder with features - For Layer > 1 (.mimg)\n"
      "P3: input folder with feature points (-fpts.txt)\n"
      "P4: input network architecture (.json)\n"
      "P5: input layer for patch definition (1, 2, 3, etc)\n"
      "P6: output folder with the models\n",
      "main"
    );
  }

  // iftFileSet *fs = iftLoadFileSetFromDirBySuffix(
  //   argv[3], "-seeds.txt", 1
  // );
  iftFileSet *fs = iftLoadFileSetFromDirBySuffix(
    argv[3], "-fpts.txt", 1
  );
  iftFLIMArch *arch = iftReadFLIMArch(argv[4]);
  int layer = atoi(argv[5]);
  char *filename = iftAllocCharArray(512);
  char *model_dir = argv[6];
  iftMakeDir(model_dir);

  iftDataSet **patch_datasets = (iftDataSet**) calloc(fs->n, sizeof(iftDataSet*));
  int n_classes = 0;
  for (size_t i=0; i < fs->n; i++) {
      // char *img_basename = iftFilename(fs->files[i]->path, "-seeds.txt");
      char *img_basename = iftFilename(fs->files[i]->path, "-fpts.txt");
      printf("[INFO] Processing image %s\n", img_basename);
      int class = atoi(iftSplitString(img_basename, "-")->head->elem);
      if (class > n_classes) {
        n_classes++;
      }
      /*Handles reading both the input image and converting it to .mimg for the
      first layer, as well as reading .mimg directly for subsequent layers.*/
      sprintf(filename, "%s/%s.png", argv[1], img_basename);
      iftImage *input_img = iftReadImageByExt(filename);
      iftMImage *input_mimg = iftImageToMImage(input_img, color_space);
      sprintf(filename, "%s/%s.mimg", argv[2], img_basename);
      iftMImage *features = NULL;
      if (layer > 1) {
        features = iftReadMImage(filename);
      }

      // Construct dataset of Kernels for the image, it will
      // It is better to do everything inside the function itself!
      iftLabeledSet *S = NULL;
      if (iftIs3DMImage(input_mimg)) {
        S = iftReadLabeledSet(fs->files[i]->path, 3);
      }
      else {
        S = iftReadLabeledSet(fs->files[i]->path, 2);
      }
      /*
      The method GetFeaturesDataset implements the following:

      1. Get the patch adjacency from architecture values;
      2. It creates that patch dataset (Size of labeled set and n Features)
      3. If layer !- 1, computes the scale parameter (features / input_img)
      4. Iterate over each seed and:
        4.1. Updates the elem (pixel index) if necessary (pooling)
        4.2. Sets dataset elemen, handicap, and truelabel (from image file label)
        4.3. Reads the patches as the dataset features
      */
      patch_datasets[i] = GetFeaturesDataset(input_mimg, features, arch, layer, &S, class);
      /*[TODO] Verify normalization
        Normalizing separately each image dataset is not a good option:
      We have notice that with the purpose of computing similarities,
      normalizing the dataset with all patches renders better results. Also, no 
      normalization at all also renders good results. 
              **Once Pipeline is finished end-to-end verify the best approach**
      */
      // iftNormalizeDataSetByZScoreInPlace(
      //   patch_datasets[i], NULL, arch->stdev_factor
      // );

      iftDestroyImage(&input_img);
      iftDestroyMImage(&input_mimg);
      iftDestroyLabeledSet(&S);
      if (features != NULL) {
        iftDestroyMImage(&features);
      }
      iftFree(img_basename);
  }

  iftDataSet *merged_patches_dataset = iftMergeDataSetArray(
    patch_datasets, fs->n, true
  );
  //[TODO] Verify normalization
  iftDataSet *normalized_patches_dataset = iftNormalizeDataSetByZScore(
    merged_patches_dataset, NULL, arch->stdev_factor
  );
  // iftNormalizeDataSetByZScoreInPlace(
  //   merged_patches_dataset, NULL, arch->stdev_factor
  // );
  merged_patches_dataset->nclasses = n_classes;
  normalized_patches_dataset->nclasses = n_classes;
  printf(
    "[INFO] Merged dataset has %d samples with %d features\n",
    normalized_patches_dataset->nsamples, normalized_patches_dataset->nfeats
  );
  printf("[INFO] n groups %d and n classes %d\n", normalized_patches_dataset->ngroups,
    normalized_patches_dataset->nclasses
  );

  /* [TODO] I am investigating the best design choice to enable fast modifications,
  as adding new distance metrics or different forms of composing the similarity
  matrix.
  */
  float (*metric) (float *, float *, int);
  metric = &iftCosineDistance2;
  iftMatrix *cosine_distance_M = ComputeDistanceMatrix(
    normalized_patches_dataset, metric
  );
  iftMatrix *similarity_M = VerifySimilarities(normalized_patches_dataset, cosine_distance_M);

  // [TODO] Implement scoring and selecting filters - Implement End-to-End pipeline
  // I need preliminar results to share with Maria and write the FASPEP
  iftFilterScore *filter_scores = ComputeFilterScores(
    normalized_patches_dataset, similarity_M
  );
  iftFilterScoreSummary *summary = AnalyseScores(
    filter_scores, similarity_M->nrows, normalized_patches_dataset->nclasses + 1
  );
  iftReportFilterScoring(summary, similarity_M->nrows, normalized_patches_dataset->nclasses + 1);

  iftDataSet *selected_filters = iftComposeLayerFilterBank(
    summary, merged_patches_dataset, arch, layer
  );

  // Compute Bias and Save Filters
  iftMatrix *kernels = NULL;
  int *true_labels = NULL;
  float *bias = NULL;
  printf("[INFO] Computing kernels, bias, and weights\n");
  iftSetKernelParameters(
    selected_filters, &kernels, &true_labels, &bias
  );

  printf("[INFO] Saving model parameters to %s\n", model_dir);
  sprintf(filename, "%s/conv%d-kernels.npy", model_dir, layer);
  iftWriteMatrix(kernels, filename);
  sprintf(filename, "%s/conv%d-bias.txt", model_dir, layer);
  iftSaveBiasOrWeights(filename, bias, kernels->ncols, IFT_FLT_TYPE);
  sprintf(filename, "%s/conv%d-weights.txt", model_dir, layer);
  iftSaveBiasOrWeights(filename, true_labels, kernels->ncols, IFT_INT_TYPE);

  // [TODO] Verificar como deixar de forma mais organizada a função e as estruturas referentes ao AnalyseScores
  // [TODO] Alterei a questão do eps da distancia de coseno na ift, atualizar
  // [TODO] Entender mais o score. Graduar filtros dificeis e fáceis.
  // [TODO] Selecionar os filtros, salvar modelo. Visualizar ativações.

  // Utilizar decoder 2 ou 3: iftProbabilityBasedAdaptiveDecoder e iftMeanBasedAdaptiveDecoder

  // Releases all allocated resources
  for (size_t i = 0; i < fs->n; i++) {
    iftDestroyDataSet(&patch_datasets[i]);
  }
  iftDestroyDataSet(&merged_patches_dataset);
  iftDestroyDataSet(&normalized_patches_dataset);
  iftDestroyDataSet(&selected_filters);
  free(patch_datasets);
  iftDestroyFileSet(&fs);
  iftDestroyFLIMArch(&arch);
  iftWriteMatrixCSV(cosine_distance_M, "cosine_dist_matrix.csv");
  iftWriteMatrixCSV(similarity_M, "similarity_M.csv");
  iftDestroyMatrix(&cosine_distance_M);
  iftDestroyMatrix(&similarity_M);
  iftDestroyFilterScoreSummary(&summary);
  iftFree(filter_scores);
  iftFree(filename);
  iftFree(bias);
  iftFree(true_labels);
  iftDestroyMatrix(&kernels);

  puts("\nDone ...");
  puts(iftFormattedTime(iftCompTime(tstart, iftToc())));
  memory_end = iftMemoryUsed();
  iftVerifyMemory(memory_start, memory_end);
}