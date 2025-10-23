
%feature("autodoc", "2");
int iftLocalMaximum(   iftImage *weights, int p,    iftAdjRel *disk);

%feature("autodoc", "2");
int iftFurthestInError(   iftImage *source,    iftImage *target,    iftImage *mask, int p);

%newobject iftQueryForAnchorsPosition;
%feature("autodoc", "2");
iftSet *iftQueryForAnchorsPosition(   iftImage *gt_contour,    iftImage *source_mask,
                                      iftImage *gt_mask,    iftSet *anchors);

%newobject iftFurtherThanThreshold;
%feature("autodoc", "2");
iftSet *iftFurtherThanThreshold(   iftVoxelArray *anchors,
                                   iftImage *mask,
                                float threshold);

