%include "iftAdjacency.i"
%include "iftCommon.i"
%include "iftCurve.i"
%include "iftDataSet.i"
%include "iftFImage.i"
%include "iftImage.i"
%include "iftImageMath.i"
%include "iftInterpolation.i"
%include "iftMathMorph.i"
%include "iftMImage.i"
%include "iftPlane.i"
%include "iftRepresentation.i"
%include "iftSeeds.i"

%feature("autodoc", "2");
void       iftDrawSeeds(iftImage *img, iftLabeledSet *S);

%feature("autodoc", "2");
void       iftDrawPoints(iftImage *img, iftSet *S, iftColor YCbCr, iftAdjRel *B);

%feature("autodoc", "2");
void       iftDrawBorders(iftImage *img, iftImage *label, iftAdjRel *A, iftColor YCbCr, iftAdjRel *B);

%feature("autodoc", "2");
void iftDrawLabels(iftImage *img,    iftImage *label_img,    iftColorTable *cmap,
                      iftAdjRel *A, bool fill, float alpha);

%feature("autodoc", "2");
void       iftDrawObject(iftImage *img, iftImage *bin, iftColor YCbCr, iftAdjRel *B);

%feature("autodoc", "2");
void       iftDrawRoots(iftImage *img, iftImage *root, iftColor YCbCr, iftAdjRel *B);

%newobject iftDraw2DFeatureSpace;
%feature("autodoc", "2");
iftImage *iftDraw2DFeatureSpace(iftDataSet *Z, iftFeatureLabel opt, iftSampleStatus status);

%newobject iftColorizeComp;
%feature("autodoc", "2");
iftImage *iftColorizeComp(iftImage *label);

%newobject iftColorizeImageByLabels;
%feature("autodoc", "2");
iftImage *iftColorizeImageByLabels(iftImage *orig, iftImage *label, iftColorTable *ctb);

%newobject iftColorizeCompOverImage;
%feature("autodoc", "2");
iftImage *iftColorizeCompOverImage(iftImage *orig, iftImage *label);

%feature("autodoc", "2");
void iftDrawBoundingBoxBordersInPlace(iftImage *img, iftBoundingBox bb, iftColor YCbCr, iftAdjRel *A, bool drawCentralPoint);

