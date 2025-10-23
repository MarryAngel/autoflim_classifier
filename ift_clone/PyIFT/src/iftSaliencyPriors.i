%include "iftSaliency.i"

%newobject iftFilterSaliencyByEllipseMatching;
%feature("autodoc", "2");
iftImage *iftFilterSaliencyByEllipseMatching(iftImage *salie_map, int tensor_npairs, int min_size, int max_size, float score_threshold);

