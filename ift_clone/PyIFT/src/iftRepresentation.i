%include "iftCommon.i"
%include "iftAdjacency.i"
%include "iftImage.i"
%include "iftRadiometric.i"
%include "iftFImage.i"
%include "iftSeeds.i"
%include "iftSegmentation.i"

%newobject iftColorMSSkel2D;
%feature("autodoc", "2");
iftImage *iftColorMSSkel2D(iftImage *bin);

%newobject iftGeodesicDistTrans;
%feature("autodoc", "2");
iftFImage *iftGeodesicDistTrans(   iftSet *S,    iftImage *mask,    iftAdjRel *A);

%newobject iftBorderDistTrans;
%feature("autodoc", "2");
iftImage  *iftBorderDistTrans(   iftImage *label, iftAdjRel *A);

%newobject iftSetDistTrans;
%feature("autodoc", "2");
  iftImage  *iftSetDistTrans(iftSet *S, int xsize, int ysize, int zsize);

%newobject iftMSSkel;
%feature("autodoc", "2");
iftFImage      *iftMSSkel(iftImage *bin);

%newobject iftMSSkel2D;
%feature("autodoc", "2");
iftFImage *iftMSSkel2D(iftImage *label_img, iftAdjRel *A, iftSide side, iftImage **dist_out,
                       iftImage **relabel_img_out);

%newobject iftEuclDistTrans;
%feature("autodoc", "2");
iftImage *iftEuclDistTrans(   iftImage *label_img,    iftAdjRel *Ain, iftSide side, iftImage **root_out,
                           iftImage **edt_label_out, iftImage **pred_out);

%newobject iftGeodesicDistTransFromSet;
%feature("autodoc", "2");
iftImage *iftGeodesicDistTransFromSet(   iftImage *mask,    iftSet *set, iftImage **root_out);

%newobject iftIntegralImage;
%feature("autodoc", "2");
iftFImage *iftIntegralImage(iftImage *img);

%newobject iftCumValueInRegion;
%feature("autodoc", "2");
iftFImage *iftCumValueInRegion(iftFImage *integral_image,
			       int Dx, int Dy, int Dz,
			       int Wx, int Wy, int Wz);

%newobject iftMedialAxisTrans2D;
%feature("autodoc", "2");
iftImage       *iftMedialAxisTrans2D(iftImage *bin, float scale_thres, iftSide side);

%newobject iftShapeReconstruction;
%feature("autodoc", "2");
iftImage       *iftShapeReconstruction(iftImage *medial_axis, int value);

%newobject iftTerminalPoints2D;
%feature("autodoc", "2");
iftImage       *iftTerminalPoints2D(iftImage *skel);

%newobject iftBranchPoints2D;
%feature("autodoc", "2");
iftImage *iftBranchPoints2D(iftImage *skel);

%newobject iftTerminalPointSet2D;
%feature("autodoc", "2");
iftSet         *iftTerminalPointSet2D(iftImage *skel);

%newobject iftBranchPointSet2D;
%feature("autodoc", "2");
  iftSet *iftBranchPointSet2D(iftImage *skel);

%newobject iftContourToArray;
%feature("autodoc", "2");
iftVoxelArray *iftContourToArray(   iftImage *contour);

%newobject iftApproxContour;
%feature("autodoc", "2");
iftVoxelArray *iftApproxContour(   iftImage *contour, double epsilon);

%newobject iftApproxVoxelArray;
%feature("autodoc", "2");
iftVoxelArray *iftApproxVoxelArray(   iftVoxelArray *array, double epsilon);

%newobject iftVoxelArrayToSet;
%feature("autodoc", "2");
iftSet *iftVoxelArrayToSet(   iftImage *image,    iftVoxelArray *array);

%newobject iftNearestInContour;
%feature("autodoc", "2");
iftSet *iftNearestInContour(   iftImage *contour,    iftSet *set,    iftImage *mask);

