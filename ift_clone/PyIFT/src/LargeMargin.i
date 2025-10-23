%include "iftMatrix.i"
%include "iftMImage.i"

%newobject iftLMCompAnalysis;
%feature("autodoc", "2");
double *iftLMCompAnalysis(   double *data,    int *label,    double *L_in, int n, int d, int d_out,
                          int k_targets, double learn_rate, int iterations, bool verbose);

%newobject iftKernelLMCA;
%feature("autodoc", "2");
double *iftKernelLMCA(   double *gram,    int *label, int n, int dim_out,
                      int k_targets, double c, double learn_rate, int iterations, bool verbose);

%newobject iftKLMCA;
%feature("autodoc", "2");
iftDoubleMatrix *iftKLMCA(   iftDoubleMatrix *gram,    iftIntArray *label, int dim_out,
                          int k_targets, double c, double learn_rate, int iterations, bool verbose);

