%include "iftDataSet.i"
%include "iftImage.i"
%include "iftMImage.i"



typedef enum {
    IFT_RIGHT_BRAIN_SIDE, IFT_LEFT_BRAIN_SIDE
} iftBrainSide;

%newobject iftBrainAsymMap;
%feature("autodoc", "2");
iftImage *iftBrainAsymMap(   iftImage *img,    iftImage *bias);

%newobject iftMeanBrainAsymMap;
%feature("autodoc", "2");
iftImage *iftMeanBrainAsymMap(   iftFileSet *img_set, bool add_stdev_asymmetries);

%newobject iftMeanBrainDiffMap;
%feature("autodoc", "2");
iftImage *iftMeanBrainDiffMap(   iftFileSet *img_set,    iftImage *template_img, bool add_stdev_asymmetries);

%newobject iftGridSamplingByBrainAsymmetries;
%feature("autodoc", "2");
iftIntArray * iftGridSamplingByBrainAsymmetries(   iftImage *asym_map,    iftImage *bin_mask,
                                                int min_samples_on_symmetries, float thres_factor);

%newobject iftBuildBrainHemisphereMImage;
%feature("autodoc", "2");
iftMImage *iftBuildBrainHemisphereMImage(   iftImage *img);

%newobject iftBuildBrainHemisphereMImageAsym;
%feature("autodoc", "2");
iftMImage *iftBuildBrainHemisphereMImageAsym(   iftImage *img,    iftImage *asym_map);

%newobject iftSymmISF;
%feature("autodoc", "2");
iftImage *iftSymmISF(   iftImage *img,    iftImage *bin_mask, float alpha, float beta, float thres_factor,
                     float min_dist_to_border, int n_seeds_on_symmetric_regions,    iftImage *normal_asymmap);

%newobject iftSymmOISF;
%feature("autodoc", "2");
iftImage *iftSymmOISF(   iftImage *img,    iftImage *bin_mask, int n_supervoxels, float alpha, float beta,
                      float gamma,    iftImage *normal_asymmap, float thres_factor);

%newobject iftExtractBrainSide;
%feature("autodoc", "2");
iftImage *iftExtractBrainSide(   iftImage *img, iftBrainSide side);

%newobject iftExtractSupervoxelHAAFeats;
%feature("autodoc", "2");
iftDataSet **iftExtractSupervoxelHAAFeats(   iftImage *test_img,    iftImage *test_sym_svoxels,
                                             iftFileSet *train_set, int n_bins,    iftImage *normal_asym_map,
                                          int *n_svoxels_out);

%newobject iftExtractSupervoxelBICAsymmFeats;
%feature("autodoc", "2");
iftDataSet **iftExtractSupervoxelBICAsymmFeats(   iftImage *test_img,    iftImage *test_sym_svoxels,
                                                  iftFileSet *train_set, int n_bins_per_channel,    iftImage *normal_asym_map,
                                               int *n_svoxels_out);

%newobject iftBuildRefDataSupervoxelDataSets;
%feature("autodoc", "2");
void iftBuildRefDataSupervoxelDataSets(iftDataSet **Zarr, int n_supervoxels, const char *test_img_path,
                                       const char *test_supervoxels_path,    iftFileSet *train_set);

