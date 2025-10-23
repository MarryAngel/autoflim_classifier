%include "iftDataSet.i"
%include "iftImage.i"
%include "iftMImage.i"

%newobject iftRegErrorMagnitude;
%feature("autodoc", "2");
iftImage *iftRegErrorMagnitude(   iftImage *img,    iftImage *template_img,    iftImage *bias);

%newobject iftMeanRegErrorMagnitude;
%feature("autodoc", "2");
iftImage *iftMeanRegErrorMagnitude(   iftFileSet *img_set,    iftImage *template_img, bool add_stdev_error);

%newobject iftISFOnRegErrors;
%feature("autodoc", "2");
iftImage *iftISFOnRegErrors(   iftImage *img,    iftImage *reg_error_mag,    iftImage *template_img,
                               iftImage *label_img,    iftDblArray *alphas,    iftDblArray *betas,
                               iftDblArray *thres_factors,    iftDblArray *min_dist_to_borders,
                               iftIntArray *n_seeds_on_correct_region);

%newobject iftISFOnRegErrorsFast;
%feature("autodoc", "2");
iftImage *iftISFOnRegErrorsFast(   iftImage *img,    iftImage *reg_error_mag,    iftImage *template_img,
                                   iftImage *label_img,    iftDblArray *alphas,    iftDblArray *betas,
                                   iftDblArray *thres_factors,    iftDblArray *min_dist_to_borders,
                                   iftIntArray *n_seeds_on_correct_region);

%newobject iftGridSamplingOnDomes;
%feature("autodoc", "2");
    iftIntArray *iftGridSamplingOnDomes(   iftImage *img,    iftImage *bin_mask, int n_samples_on_flat_region,
                                        float thres_factor, iftImage **domes_out);

%newobject iftExtractSupervoxelHistRegErrorsFeats;
%feature("autodoc", "2");
iftDataSet **iftExtractSupervoxelHistRegErrorsFeats(   iftImage *test_reg_error_mag,    iftImage *test_svoxels_img,
                                                       iftFileSet *train_set, int n_bins, int *n_svoxels_out);

%newobject iftExtractSupervoxelHistRegErrorsFeatsDilation;
%feature("autodoc", "2");
iftDataSet **iftExtractSupervoxelHistRegErrorsFeatsDilation(   iftImage *test_img,    iftImage *test_svoxels_img,
                                                               iftFileSet *train_set,    iftImage *template_img,
                                                            int n_bins, float radius,    iftImage *bias, int *n_svoxels_out);

%newobject iftExtractSupervoxelBandHistFeats;
%feature("autodoc", "2");
iftDataSet **iftExtractSupervoxelBandHistFeats(   iftMImage *test_filt_img,    iftImage *test_svoxels_img,
                                                  iftFileSet *train_set, int n_bins, int *n_svoxels_out);

%newobject iftWriteSupervoxelDataSets;
%feature("autodoc", "2");
void iftWriteSupervoxelDataSets(   iftDataSet **Zarr, int n_supervoxels, const char *out_dir);

%newobject iftComputeLinearAttenuationWeightsByEDT;
%feature("autodoc", "2");
iftFImage *iftComputeLinearAttenuationWeightsByEDT(   iftImage *label_img, float max_attenuation_factor);

%newobject iftComputeExponentialAttenuationWeightsByEDT;
%feature("autodoc", "2");
iftFImage *iftComputeExponentialAttenuationWeightsByEDT(   iftImage *label_img, float max_attenuation_factor, float exponent);

%newobject iftWeightedRegErrorMagnitude;
%feature("autodoc", "2");
iftImage *iftWeightedRegErrorMagnitude(   iftImage *img,    iftImage *template_img,    iftFImage *weights);

%newobject iftRemoveSVoxelsByVolAndMeanRegError;
%feature("autodoc", "2");
iftImage *iftRemoveSVoxelsByVolAndMeanRegError(   iftImage *svoxels_img,    iftImage *reg_error_mag,
                                               int min_vol, float min_mean_reg_error_on_svoxel);

%newobject iftISFOnAttentionMap;
%feature("autodoc", "2");
iftImage *iftISFOnAttentionMap(   iftImage *img,    iftImage *attention_map,    iftImage *target_img,
                                  iftImage *label_img,    iftDblArray *alphas,    iftDblArray *betas,
                                  iftDblArray *thres_factors,    iftDblArray *min_dist_to_borders,
                                  iftIntArray *n_seeds_on_correct_region);

%newobject iftExtractSupervoxelBICFeats;
%feature("autodoc", "2");
iftDataSet **iftExtractSupervoxelBICFeats(   iftImage *test_img,    iftImage *test_svoxels_img,
                                             iftFileSet *train_set, int n_bins_per_channel,
                                          int *n_svoxels_out);

%newobject iftExtractSupervoxelAttentionMapFeats;
%feature("autodoc", "2");
iftDataSet **iftExtractSupervoxelAttentionMapFeats(   iftImage *test_attention_map,    iftImage *test_svoxels_img,
                                                      iftFileSet *train_set, int n_bins, int *n_svoxels_out);

%newobject iftExtractSupervoxelLBPFeats;
%feature("autodoc", "2");
iftDataSet **iftExtractSupervoxelLBPFeats(   iftImage *test_img,    iftImage *test_svoxels_img,
                                             iftFileSet *train_set, int n_bins, int *n_svoxels_out);

%newobject iftExtractSupervoxelVLBPFeats;
%feature("autodoc", "2");
iftDataSet **iftExtractSupervoxelVLBPFeats(   iftImage *test_img,    iftImage *test_svoxels_img,
                                              iftFileSet *train_set, int n_bins, int *n_svoxels_out);

%newobject iftExtractSupervoxelTextureFeats;
%feature("autodoc", "2");
iftDataSet **iftExtractSupervoxelTextureFeats(   iftImage *test_img,    iftImage *test_attention_map,
                                                 iftImage *test_svoxels_img,    iftFileSet *train_set,
                                                 iftFileSet *train_attention_map_set,
                                              int n_bins, int *n_svoxels_out);

