%include "iftCommon.i"
%include "iftImage.i"
%include "iftFImage.i"

%newobject iftAdd;
%feature("autodoc", "2");
iftImage *iftAdd(   iftImage *img1,    iftImage *img2);

%newobject iftSub;
%feature("autodoc", "2");
iftImage *iftSub(   iftImage *img1,    iftImage *img2);

%newobject iftSubReLU;
%feature("autodoc", "2");
iftImage *iftSubReLU(   iftImage *img1,    iftImage *img2);

%newobject iftAbsSub;
%feature("autodoc", "2");
iftImage *iftAbsSub(   iftImage *img1,    iftImage *img2);

%newobject iftIntersec;
%feature("autodoc", "2");
iftImage *iftIntersec(   iftImage *label_img1,    iftImage *label_img2);

%newobject iftAnd;
%feature("autodoc", "2");
iftImage *iftAnd(   iftImage *img1,    iftImage *img2);

%newobject iftOr;
%feature("autodoc", "2");
iftImage *iftOr(   iftImage *img1,    iftImage *img2);

%newobject iftMult;
%feature("autodoc", "2");
iftImage *iftMult(   iftImage *img1,    iftImage *img2);

%newobject iftMultByIntScalar;
%feature("autodoc", "2");
iftImage *iftMultByIntScalar(   iftImage *img, int scalar);

%newobject iftAbs;
%feature("autodoc", "2");
iftImage *iftAbs(   iftImage *img);

%newobject iftXor;
%feature("autodoc", "2");
iftImage *iftXor(   iftImage *img1,    iftImage *img2);

%newobject iftComplement;
%feature("autodoc", "2");
iftImage *iftComplement(   iftImage *img);

%newobject iftComplementToAnUpperBound;
%feature("autodoc", "2");
  iftImage *iftComplementToAnUpperBound(iftImage *img, int maxval);

%newobject iftReLUImage;
%feature("autodoc", "2");
iftImage *iftReLUImage(   iftImage *img);

%newobject iftMask;
%feature("autodoc", "2");
iftImage *iftMask(   iftImage *img,    iftImage *mask);

%newobject iftBinarize;
%feature("autodoc", "2");
iftImage *iftBinarize(   iftImage *label_img);

%newobject iftAddValue;
%feature("autodoc", "2");
iftImage *iftAddValue(   iftImage *img, int val);

%newobject iftBinaryFrom1To255;
%feature("autodoc", "2");
iftImage *iftBinaryFrom1To255(   iftImage *bin);

%feature("autodoc", "2");
iftFImage *iftSQRT(iftImage    *img1);

%feature("autodoc", "2");
float iftMeanValue(   iftImage *img, iftImage *mask);

%newobject iftRoundFImage;
%feature("autodoc", "2");
iftImage *iftRoundFImage(   iftFImage *fimg);

