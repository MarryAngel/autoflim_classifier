%include "iftCommon.i"
%include "iftImage.i"
%include "iftFImage.i"

%newobject iftLinearStretch;
%feature("autodoc", "2");
iftImage *iftLinearStretch(iftImage *img, double f1, double f2, double g1, double g2);

%newobject iftNormalize;
%feature("autodoc", "2");
iftImage *iftNormalize(   iftImage *img, double minval, double maxval);

%newobject iftNormalizeWithNoOutliersInRegion;
%feature("autodoc", "2");
iftImage *iftNormalizeWithNoOutliersInRegion(   iftImage *img,    iftImage *region,
                                            int minval, int maxval, float perc);

%newobject iftNormalizeWithNoOutliers;
%feature("autodoc", "2");
iftImage *iftNormalizeWithNoOutliers(   iftImage *img, int minval, int maxval, float perc);

%newobject iftMatchHistogram;
%feature("autodoc", "2");
iftImage *iftMatchHistogram(   iftImage *img,    iftImage *img_mask,
                               iftImage *ref,    iftImage *ref_mask);

