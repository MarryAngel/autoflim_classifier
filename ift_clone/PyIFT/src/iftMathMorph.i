%include "iftCommon.i"
%include "iftImage.i"
%include "iftImageMath.i"
%include "iftSeeds.i"
%include "iftAdjacency.i"
%include "iftFiltering.i"
%include "iftCompTree.i"

%newobject iftLeveling;
%feature("autodoc", "2");
  iftImage *iftLeveling(iftImage *orig, iftImage *impaired);

%newobject iftDilate;
%feature("autodoc", "2");
  iftImage *iftDilate(   iftImage *img,    iftAdjRel *A,    iftImage *mask);

%newobject iftErode;
%feature("autodoc", "2");
  iftImage *iftErode(   iftImage *img,    iftAdjRel *A,    iftImage *mask);

%newobject iftClose;
%feature("autodoc", "2");
  iftImage *iftClose(   iftImage *img,    iftAdjRel *A,    iftImage *mask);

%newobject iftOpen;
%feature("autodoc", "2");
  iftImage *iftOpen(   iftImage *img,    iftAdjRel *A,    iftImage *mask);

%newobject iftAsfCO;
%feature("autodoc", "2");
  iftImage *iftAsfCO(   iftImage *img,    iftAdjRel *A,    iftImage *mask);

%newobject iftAsfOC;
%feature("autodoc", "2");
  iftImage *iftAsfOC(   iftImage *img,    iftAdjRel *A,    iftImage *mask);

%newobject iftDilateBin;
%feature("autodoc", "2");
  iftImage *iftDilateBin(   iftImage *bin, iftSet **seed, float radius);

%newobject iftErodeBin;
%feature("autodoc", "2");
iftImage *iftErodeBin(   iftImage *bin, iftSet **seed, float radius);

%newobject iftErodeLabelImage;
%feature("autodoc", "2");
iftImage *iftErodeLabelImage(   iftImage *label_img, float radius);

%newobject iftCloseBin;
%feature("autodoc", "2");
iftImage *iftCloseBin(   iftImage *bin, float radius);

%newobject iftOpenBin;
%feature("autodoc", "2");
iftImage *iftOpenBin(   iftImage *bin, float radius);

%newobject iftAsfCOBin;
%feature("autodoc", "2");
  iftImage *iftAsfCOBin(   iftImage *bin, float radius);

%newobject iftAsfOCBin;
%feature("autodoc", "2");
  iftImage *iftAsfOCBin(   iftImage *bin, float radius);

%newobject iftCloseRecBin;
%feature("autodoc", "2");
  iftImage *iftCloseRecBin(   iftImage *bin, float radius);

%newobject iftOpenRecBin;
%feature("autodoc", "2");
  iftImage *iftOpenRecBin(   iftImage *bin, float radius);

%newobject iftMorphGrad;
%feature("autodoc", "2");
  iftImage *iftMorphGrad(   iftImage *img,    iftAdjRel *A);

%newobject iftSuperiorRec;
%feature("autodoc", "2");
  iftImage *iftSuperiorRec(   iftImage *img,    iftImage *marker,    iftImage *mask);

%newobject iftInferiorRec;
%feature("autodoc", "2");
  iftImage *iftInferiorRec(   iftImage *img,    iftImage *marker,    iftImage *mask);

%newobject iftOpenRec;
%feature("autodoc", "2");
  iftImage *iftOpenRec(   iftImage *img,    iftAdjRel *A,    iftImage *mask);

%newobject iftCloseRec;
%feature("autodoc", "2");
  iftImage *iftCloseRec(   iftImage *img,    iftAdjRel *A,    iftImage *mask);

%newobject iftAsfCORec;
%feature("autodoc", "2");
  iftImage *iftAsfCORec(   iftImage *img,    iftAdjRel *A,    iftImage *mask);

%newobject iftAsfOCRec;
%feature("autodoc", "2");
  iftImage *iftAsfOCRec(   iftImage *img,    iftAdjRel *A,    iftImage *mask);

%newobject iftCloseBasins;
%feature("autodoc", "2");
  iftImage *iftCloseBasins(   iftImage *img, iftSet *seed,    iftImage *mask);

%newobject iftOpenDomes;
%feature("autodoc", "2");
  iftImage *iftOpenDomes(   iftImage *img, iftSet *seed,    iftImage *mask);

%newobject iftHClose;
%feature("autodoc", "2");
  iftImage *iftHClose(   iftImage *img, int H,    iftImage *mask);

%newobject iftHOpen;
%feature("autodoc", "2");
  iftImage *iftHOpen(   iftImage *img, int H,    iftImage *mask);

%newobject iftFastAreaClose;
%feature("autodoc", "2");
  iftImage *iftFastAreaClose(   iftImage *img, int thres);

%newobject iftFastAreaOpen;
%feature("autodoc", "2");
iftImage *iftFastAreaOpen(   iftImage *img, int thres);

%newobject iftAreaClose;
%feature("autodoc", "2");
  iftImage *iftAreaClose(   iftImage *img, int area_thres, iftCompTree *ctree);

%newobject iftVolumeClose;
%feature("autodoc", "2");
  iftImage *iftVolumeClose(   iftImage *img, int volume_thres, iftCompTree *ctree);

%newobject iftAreaOpen;
%feature("autodoc", "2");
  iftImage *iftAreaOpen(   iftImage *img, int area_thres, iftCompTree *ctree);

%newobject iftVolumeOpen;
%feature("autodoc", "2");
  iftImage *iftVolumeOpen(   iftImage *img, int volume_thres, iftCompTree *ctree);

