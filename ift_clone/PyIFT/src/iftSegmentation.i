%include "iftAdjacency.i"
%include "iftClassification.i"
%include "iftClustering.i"
%include "iftCommon.i"
%include "iftDataSet.i"
%include "iftFiltering.i"
%include "iftFImage.i"
%include "iftImage.i"
%include "iftImageForest.i"
%include "iftImageMath.i"
%include "iftMathMorph.i"
%include "iftMetrics.i"
%include "iftMImage.i"
%include "iftRadiometric.i"
%include "iftRepresentation.i"
%include "iftSeeds.i"

%newobject iftEnhanceEdges;
%feature("autodoc", "2");
iftImage      *iftEnhanceEdges(iftImage *img, iftAdjRel *A, iftLabeledSet *seed, float alpha);

%newobject iftWatershed;
%feature("autodoc", "2");
iftImage *iftWatershed(   iftImage *basins, iftAdjRel *Ain, iftLabeledSet *seeds, iftSet *forbidden);

%newobject iftWaterCut;
%feature("autodoc", "2");
iftImage *iftWaterCut(iftMImage *mimg, iftAdjRel *Ain, iftLabeledSet *seeds, iftSet *forbidden);

%newobject iftWaterGray;
%feature("autodoc", "2");
iftImage  *iftWaterGray(iftImage *basins, iftImage *marker, iftAdjRel *A);

%feature("autodoc", "2");
void iftWaterGrayForest(iftImageForest *fst, iftImage *marker);

%newobject iftDualWaterGray;
%feature("autodoc", "2");
iftImage  *iftDualWaterGray(iftImage *domes, iftImage *marker, iftAdjRel *A);

%newobject iftOrientedWatershed;
%feature("autodoc", "2");
iftImage *iftOrientedWatershed(   iftImage *img,    iftImage *grad, iftAdjRel *Ain,
                               iftFloatArray *alpha, iftLabeledSet *seeds, iftSet *forbidden);

%feature("autodoc", "2");
int iftOtsu(   iftImage *img);

%newobject iftThreshold;
%feature("autodoc", "2");
iftImage *iftThreshold(   iftImage *img, int lowest, int highest, int value);

%newobject iftFThreshold;
%feature("autodoc", "2");
iftImage  *iftFThreshold(   iftFImage *img, float lowest, float highest, int value);

%newobject iftAboveAdaptiveThreshold;
%feature("autodoc", "2");
  iftImage  *iftAboveAdaptiveThreshold(iftImage *img, iftImage *mask, iftAdjRel *A, float perc, int niters, int value);

%newobject iftBelowAdaptiveThreshold;
%feature("autodoc", "2");
  iftImage  *iftBelowAdaptiveThreshold(iftImage *img, iftImage *mask, iftAdjRel *A, float perc, int niters, int value);

%newobject iftBorderImage;
%feature("autodoc", "2");
iftImage *iftBorderImage(   iftImage *label, bool get_margins);

%newobject iftSelectAndPropagateRegionsAboveAreaByColor;
%feature("autodoc", "2");
iftImage *iftSelectAndPropagateRegionsAboveAreaByColor(iftImage *img, iftImage *label, int area);

%newobject iftSmoothRegionsByDiffusion;
%feature("autodoc", "2");
iftImage *iftSmoothRegionsByDiffusion(   iftImage *label_img,    iftImage *orig_img,
                                      float smooth_factor, int n_iters);

%newobject iftOrientedWaterCut;
%feature("autodoc", "2");
iftImage *iftOrientedWaterCut(   iftImage *img, iftAdjRel *Ain, iftFloatArray *alpha,
                                          iftLabeledSet *seeds, iftSet *forbidden);

%newobject iftOrientedColorWaterCut;
%feature("autodoc", "2");
iftImage *iftOrientedColorWaterCut(iftMImage *mimg, iftImage *orient, iftAdjRel *Ain, float beta, iftLabeledSet *seeds);

%newobject iftEnhancedWaterCut;
%feature("autodoc", "2");
iftImage *iftEnhancedWaterCut(iftMImage *mimg, iftImage *objmap, iftAdjRel *Ain, iftLabeledSet *seeds, float alpha);

%newobject iftEnhancedWatershed;
%feature("autodoc", "2");
iftImage *iftEnhancedWatershed(iftImage *basins, iftImage *objmap, iftAdjRel *Ain, iftLabeledSet *seeds, float alpha);

%newobject iftSuperPixelMajorityVote;
%feature("autodoc", "2");
iftImage *iftSuperPixelMajorityVote(iftImage *comp, iftImage *objmap, float threshold);

%newobject iftBoundingBoxArrayToLabel;
%feature("autodoc", "2");
iftImage *iftBoundingBoxArrayToLabel(   iftImage *img,    iftBoundingBoxArray *bb_ary);

%newobject iftFindOptPath;
%feature("autodoc", "2");
iftSet *iftFindOptPath(   iftMImage *mimg, int src, int dst, float sigma, iftFImage *pathval, iftImage *pred);

%newobject iftFindOptPathWithProb;
%feature("autodoc", "2");
iftSet *iftFindOptPathWithProb(   iftMImage *mimg,    iftFImage *obj,    iftFImage *bkg, int src, int dst,
                               float sigma, float gamma, iftFImage *pathval, iftImage *pred);

%newobject iftSinglePathContour;
%feature("autodoc", "2");
iftImage *iftSinglePathContour(   iftImage *label);

%newobject iftFillContour;
%feature("autodoc", "2");
iftImage *iftFillContour(   iftImage *contour);

%newobject iftDISF;
%feature("autodoc", "2");
iftImage *iftDISF(iftMImage *mimg, iftAdjRel *A, int num_init_seeds, int num_superpixels, iftImage *mask);

%newobject iftGetIoUDetection;
%feature("autodoc", "2");
float iftGetIoUDetection(iftBoundingBox gt_bb, iftBoundingBox pred_bb);

%newobject iftDynamicTrees;
%feature("autodoc", "2");
iftImage *iftDynamicTrees(iftMImage *mimg, iftImage *seeds_in, iftImage *seeds_out);

%newobject iftSalientCompsAboveOtsu;
%feature("autodoc", "2");
iftSet *iftSalientCompsAboveOtsu(iftImage *label, iftImage *salie);

%newobject iftDelineateEggComp;
%feature("autodoc", "2");
iftImage *iftDelineateEggComp(
  iftImage *comps, iftSet *SelectedComps, iftMImage *mimg,
  float seed_erosion, float seed_dilation
);

%newobject iftSelectEggCompFromSaliencyMap;
%feature("autodoc", "2");
iftImage *iftSelectEggCompFromSaliencyMap(
  iftImage *saliency, int border_dist,
  int min_obj_area, int max_obj_area
);

%newobject iftSMansoniDelineation;
%feature("autodoc", "2");
iftImage *iftSMansoniDelineation(
  iftMImage *mimg, iftImage *saliency, int border_dist,
  int min_obj_area, int max_obj_area, float seed_erosion, 
  float seed_dilation
);

%newobject iftSMansoniDelineationADMRDT;
%feature("autodoc", "2");
iftFImage *iftSMansoniDelineationADMRDT(
    iftMImage *mimg, iftImage *saliency, int border_dist,
    int min_obj_area, int max_obj_area, float seed_erosion,
    float seed_dilation, int relaxation_iterations, float alpha_confidence
);

