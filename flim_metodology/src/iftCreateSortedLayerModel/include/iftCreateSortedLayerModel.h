#ifndef _IFT_CA_
#define _IFT_CA_
#include "ift.h"
typedef struct iftFilterScore {
  // --------<Filter data >--------
  int filter_index;
  int class_id;
  // --------< Similarity/distance metrics >--------
  // [TODO] Se fosse vetor talvez seria mais fácil calcular tudo
  float intra_cos; // cosine distance
  float inter_cos;
  float intra_corr; // correlation distance
  float inter_corr;
  float intra_cross_corr; // cross_correlation distance
  float inter_cross_corr;
  // --------< Filter scores for each similarity/distance metric >--------
  float cosine_score;
  float correlation_score;
  float cross_correlation_score;
  float composite_score; // Score combining all scores
} iftFilterScore;

typedef struct iftFilterScoreSummary {
  // BETTER CODE THIS STRUCTURE
  int n_classes;
  int class_id;
  float avg_score[4]; // cos, corr, cross_cor, and composite
  float max_score[4];
  float min_score[4];
  int n_filters;
  iftFilterScore *sorted_filters;
  int difficulty; // 0 (easy) 1 (medium) 2 (hard) 3 (very hard)
} iftFilterScoreSummary;

iftAdjRel *GetPatchAdjacency(iftMImage *mimg, iftFLIMLayer layer);
void iftDownsamplesLabeledSet(
  iftMImage *src, iftMImage *dst, iftLabeledSet **S
);
iftDataSet * GetFeaturesDataset(
  iftMImage *mimg, iftMImage *mfeatures, iftFLIMArch *arch, int layer,
  iftLabeledSet **S, int class
);
iftMatrix *ComputeDistanceMatrix(
  iftDataSet *Z, float (metric) (float *, float *, int)
);
iftMatrix *VerifySimilarities(iftDataSet *Z, iftMatrix *M);
iftFilterScore * ComputeFilterScores(iftDataSet *Z, iftMatrix *M);
iftFilterScoreSummary *AnalyseScores(
  iftFilterScore *filter_scores, int n_total_filters, int n_classes
);
void iftDestroyFilterScoreSummary(iftFilterScoreSummary **summary);
void iftReportFilterScoring(
  iftFilterScoreSummary *summary, int n_total_filters, int n_classes
);
iftDataSet *iftComposeLayerFilterBank(
  iftFilterScoreSummary *summary, iftDataSet *merged_patches_dataset,
  iftFLIMArch *arch, int layer
);
void iftSetKernelParameters(
  iftDataSet *Z, iftMatrix **kernels, int **true_labels, float **bias
);
void iftSaveBiasOrWeights(
  char *basepath, void *W, int n_kernels, iftCDataType type
);

#endif