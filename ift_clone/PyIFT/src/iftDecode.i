%include "iftImage.i"

%newobject iftSimpleAdaptiveDecoder;
%feature("autodoc", "2");
iftMImage *iftSimpleAdaptiveDecoder(iftMImage *mimg);

%newobject iftProbabilityBasedAdaptiveDecoder;
%feature("autodoc", "2");
iftMImage *iftProbabilityBasedAdaptiveDecoder(
  iftMImage *mimg, char *model_dir, char *img_basename, int layer
);

%newobject iftMeanBasedAdaptiveDecoder;
%feature("autodoc", "2");
iftMImage *iftMeanBasedAdaptiveDecoder(
  iftMImage *mimg, char *model_dir, char *img_basename, int layer
);

