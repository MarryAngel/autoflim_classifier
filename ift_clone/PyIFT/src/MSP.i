%include "iftImage.i"
%include "iftInterpolation.i"
%include "iftPlane.i"

%newobject iftAlignBrainByMSP;
%feature("autodoc", "2");
iftImage *iftAlignBrainByMSP(   iftImage *img, iftPlane **msp_out);

%newobject iftRotateImageToMSP;
%feature("autodoc", "2");
iftImage *iftRotateImageToMSP(   iftImage *img,    iftPlane *msp, iftInterpolationType interp_type);

%newobject iftMSPAsImage;
%feature("autodoc", "2");
iftImage *iftMSPAsImage(   iftPlane *msp,    iftImage *ref_img);

