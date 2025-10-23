%include "iftImage.i"
%include "iftMImage.i"
%include "iftDataSet.i"
%include "iftKmeans.i"
%include "iftMaxflow.i"

%newobject iftUncertMapToLabel;
%feature("autodoc", "2");
iftImage *iftUncertMapToLabel(   iftImage *map);

%newobject iftLabelToUncertMap;
%feature("autodoc", "2");
iftImage *iftLabelToUncertMap(   iftImage *label,    iftAdjRel *A);

%newobject iftMaybeForeground;
%feature("autodoc", "2");
iftImage *iftMaybeForeground(   iftImage *label);

%newobject iftGrabCut;
%feature("autodoc", "2");
iftImage *iftGrabCut(   iftMImage *mimg,    iftImage *regions, double beta, int n_iters);

%newobject iftGMMDataSetDist;
%feature("autodoc", "2");
iftFImage *iftGMMDataSetDist(iftDataSet *train,    iftMImage *mimg, int n_comps);

