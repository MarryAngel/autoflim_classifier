%include "iftCommon.i"
%include "iftImage.i"
%include "iftMatrix.i"
%include "iftMetrics.i"
%include "iftSeeds.i"

%feature("autodoc", "2");
double iftDiceSimilarity(   iftImage *bin_source,    iftImage *bin_target);

%newobject iftASSD;
%feature("autodoc", "2");
double iftASSD(   iftImage *bin_source,    iftImage *bin_target);

%newobject iftGeneralBalancedCoeff;
%feature("autodoc", "2");
double iftGeneralBalancedCoeff(   iftImage *bin_source,    iftImage *bin_target);

%newobject iftShannonEntropy;
%feature("autodoc", "2");
double          iftShannonEntropy(   iftImage *image);

