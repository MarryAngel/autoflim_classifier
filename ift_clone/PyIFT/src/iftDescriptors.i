%include "iftCommon.i"
%include "iftImage.i"
%include "iftMImage.i"
%include "iftAdjacency.i"
%include "iftRadiometric.i"

%newobject iftExtract3DLBPTOPFeatsForLabels;
%feature("autodoc", "2");
iftMatrix *iftExtract3DLBPTOPFeatsForLabels(   iftImage *img,    iftImage *label_img,
                                            int n_bins, bool normalize_histograms);

%newobject iftExtractVLBPFeatsForLabels;
%feature("autodoc", "2");
iftMatrix *iftExtractVLBPFeatsForLabels(   iftImage *img,    iftImage *label_img,
                                        int n_bins, bool normalize_hist);

%newobject iftExtractBICForLabels;
%feature("autodoc", "2");
 iftMatrix *iftExtractBICForLabels(   iftImage *img,    iftImage *label_img,
                                   int n_bins_per_channel);

