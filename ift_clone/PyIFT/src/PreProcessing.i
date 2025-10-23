%include "iftImage.i"
%include "iftPlane.i"

%newobject iftBrainMRIPreProcessing;
%feature("autodoc", "2");
iftImage *iftBrainMRIPreProcessing(   iftImage *mri, int nbits,    iftPlane *msp_in,    iftImage *mask,
                                      iftImage *ref_mri,    iftImage *ref_mask, bool skip_n4,
                                   bool skip_median_filter, bool skip_msp_alignment, bool skip_hist_matching,
                                   iftPlane **msp_out);

