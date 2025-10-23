/* Description: Functions to support the creation and use of a FP
   model per individuo with a collection of filters per minucia point
   extracted from the skeleton.
   Author: Alexandre Falcão.
   Date: February 6th, 2023.
*/

#define SKEL_THRES 0.5
#define MIN_MNT_PROXIMITY 4
#define MIN_BORDER_DISTANCE 1
#define BIFURC_R 7
#define TERM_DISTANCE_THRESHOLD 5
#define RESCALE_FACTOR 2

iftImage *InterpolateBin(iftImage *bin, float sx, float sy);
iftImage **CreateImageArray(int n);
void DestroyImageArray(iftImage ***img, int n);
iftImage **ReadImageArray(int n, char *dirname, char *basename);
void WriteImageArray(iftImage **img, int n, char *dirname,
                     char *basename, char *subdirname);
void WriteImage(iftImage *img, char *dirname, char *basename,
                char *suffix);
void WritePoints(iftLabeledSet *S, char *dirname, char *basename,
                 char *suffix, iftImage *img);
void WritePointArray(iftLabeledSet **S, int n, char *dirname,
                     char *basename, char *subdirname, iftImage **timgs);
void WriteModel(iftMatrix *kernels, float *bias, int *seedpt,
                char *dirname, char *basename);
void MaskingArray(iftImage **img, int n, iftImage **mask);
void Masking(iftImage *img, iftImage *mask);
void FilterGaborBinArray(iftImage **gabor, int n, iftDict *param);
void FilterGaborBin(iftImage **gabor, iftDict *param);
iftImage *GetMask(iftImage *bin, iftDict *param);
iftImage **GetMaskArray(iftImage **bin, int n, iftDict *param);
iftImage **ThresholdImageArray(iftImage **img, int n, int Imin, int Imax, int out_val);
iftMatrix *ImageToFeatureMatrix(iftImage *img, iftDict *param);
float RandomFloat(float min, float max);
iftMatrix **GetRandomTransformations(iftImage *orig, iftDict *param);
iftImage **GetTransformedImages(iftImage *orig, iftMatrix **M, int n);
void WriteTransformations(iftMatrix **M, int n, char *dirname,
                          char *basename, char *subdirname);
iftImage *Gabor(iftImage *img);
iftImage *Skeleton(iftImage *gabor_bin);
iftImage **GaborImageArray(iftImage **img, int n);
iftImage **SkeletonArray(iftImage **gabor_bin, int n);
iftLabeledSet **GetSkeletonPointArray(iftImage **tskels, int n);
iftImage *ErodedInterpolatedMask(iftImage *mask, float sx, float sy,
                                 iftDict *param);
iftVoxel InvTransformPoint(iftImage *src, iftVoxel v, iftMatrix *InvM);
iftVoxel TransformPoint(iftImage *src, iftVoxel u, iftMatrix *M);
iftDataSet *PatchesFromSeeds(iftImage *orig, iftImage **timgs, iftMatrix **M,
                             int n, iftLabeledSet *seeds, iftDict *param);
iftDataSet *PatchesFromMntSet(iftImage *img, iftMatrix **M, int n, iftLabeledSet *S, iftDict *param);
float *MeanValues(iftDataSet *Z);
float *StdevValues(iftDataSet *Z, float *mean, iftDict *param);
void CompWriteModel(iftDataSet *Z, int n, iftDict *param,
                    char *dirname, char *basename);
void SavePointMatchImages(iftImage *skel, iftImage *src, iftImage **dst, iftImage **tskels, iftMatrix **M, int n, char *dirname);

iftLabeledSet *GetStableSkelPoints(iftImage *skel, iftImage *orig, iftImage *mask,
                                   iftImage **tskels, iftImage **timgs,
                                   iftMatrix **M, int n, iftDict *param);
iftSet *GetSkelPoints(iftImage *skel, iftImage *orig,iftImage* mask);
int *ReadSeedPoints(char *dirname, char *basename);
float *ReadFPModelBias(char *dirname, char *basename);
iftMatrix *ReadFPModelKernels(char *dirname, char *basename);
iftMatrix *mntAnglesFromDual(iftImage* skel, iftImage* skel_dual, iftImage* bin, iftImage* of_map, iftSet* mnts);

/*--------------- Public functions -------------------------------------*/

iftMatrix *ReadFPModelKernels(char *dirname, char *basename)
{
  char subdirname[200], filename[400];

  sprintf(subdirname, "%s/%s", dirname, basename);
  sprintf(filename, "%s/conv1-kernels.npy", subdirname);
  iftMatrix *kernels = iftReadMatrix(filename);

  return (kernels);
}

float *ReadFPModelBias(char *dirname, char *basename)
{
  char subdirname[200], filename[400];
  FILE *fp;
  int nvals;

  sprintf(subdirname, "%s/%s", dirname, basename);
  sprintf(filename, "%s/conv1-bias.txt", subdirname);
  fp = fopen(filename, "r");

  fscanf(fp, "%d", &nvals);
  float *bias = iftAllocFloatArray(nvals);
  for (int k = 0; k < nvals; k++)
  {
    fscanf(fp, "%f", &bias[k]);
  }
  fclose(fp);

  return (bias);
}

int *ReadSeedPoints(char *dirname, char *basename)
{
  char subdirname[200], filename[400];
  FILE *fp;
  int nvals;

  sprintf(subdirname, "%s/%s", dirname, basename);
  sprintf(filename, "%s/conv1-seedpt.txt", subdirname);
  fp = fopen(filename, "r");

  fscanf(fp, "%d", &nvals);
  int *seedpt = iftAllocIntArray(nvals);
  for (int k = 0; k < nvals; k++)
  {
    fscanf(fp, "%d", &seedpt[k]);
  }
  fclose(fp);

  return (seedpt);
}

iftImage *ErodedInterpolatedMask(iftImage *mask, float sx, float sy,
                                 iftDict *param)
{
  iftSet *S = NULL;
  float erosion_radius = iftGetDblValFromDict("mask_erosion_radius", param);
  iftImage *ero = iftErodeBin(mask, &S, erosion_radius);
  iftImage *emask = InterpolateBin(ero, sx, sy);
  iftDestroyImage(&ero);
  iftDestroySet(&S);

  return (emask);
}

iftImage *InterpolateBin(iftImage *bin, float sx, float sy)
{
  iftImage *aux = iftInterp2D(bin, sx, sy);
  iftImage *interp = iftThreshold(aux, 1, 255, 255);
  iftDestroyImage(&aux);

  return (interp);
}

iftImage **CreateImageArray(int n)
{
  iftImage **img = (iftImage **)calloc(n, sizeof(iftImage *));
  for (int i = 0; i < n; i++)
    img[i] = NULL;
  return (img);
}

void DestroyImageArray(iftImage ***img, int n)
{
  iftImage **aux = *img;

  if (aux != NULL)
  {
    for (int i = 0; i < n; i++)
    {
      if (aux[i] != NULL)
        iftDestroyImage(&aux[i]);
    }
    iftFree(aux);
    *img = NULL;
  }
}

iftImage **ReadImageArray(int n, char *dirname, char *basename)
{
  char filename[200];
  int delta = 360 / n;
  iftImage **img = CreateImageArray(n);
  for (int i = 0, theta = 0; i < n; i++, theta += delta)
  {
    sprintf(filename, "%s/%s/%s_%03d.png", dirname, basename, basename, theta);
    img[i] = iftReadImageByExt(filename);
  }
  return (img);
}

void WriteImageArray(iftImage **img, int n, char *dirname, char *basename, char *subdirname)
{
  char pathname[200], filename[400];

  sprintf(pathname, "%s/%s/%s", dirname, basename, subdirname);
  iftMakeDir(pathname);

  for (int i = 0; i < n; i++)
  {
    sprintf(filename, "%s/%s_%03d.png", pathname, basename, i);
    iftWriteImageByExt(img[i], filename);
  }
}

void WriteImage(iftImage *img, char *dirname, char *basename, char *suffix)
{
  char pathname[200], filename[400];

  if (!iftDirExists(dirname))
    iftMakeDir(dirname);

  sprintf(pathname, "%s/%s", dirname, basename);

  if (!iftDirExists(pathname))
    iftMakeDir(pathname);

  sprintf(filename, "%s/%s-%s.png", pathname, basename, suffix);
  iftWriteImageByExt(img, filename);
}

void WritePoints(iftLabeledSet *S, char *dirname, char *basename,
                 char *suffix, iftImage *img)
{
  char pathname[200], filename[400];

  if (!iftDirExists(dirname))
    iftMakeDir(dirname);

  sprintf(pathname, "%s", dirname);

  if (!iftDirExists(pathname))
    iftMakeDir(pathname);

  sprintf(filename, "%s/%s-seeds.txt", pathname, basename);
  iftWriteSeeds(S, img, filename);
}

void WritePointArray(iftLabeledSet **S, int n, char *dirname,
                     char *basename, char *subdirname, iftImage **timgs)
{
  char filename[400], pathname1[200], pathname2[300];

  if (!iftDirExists(dirname))
    iftMakeDir(dirname);

  sprintf(pathname1, "%s/%s", dirname, basename);
  if (!iftDirExists(pathname1))
    iftMakeDir(pathname1);

  sprintf(pathname2, "%s/%s", pathname1, subdirname);
  if (!iftDirExists(pathname2))
    iftMakeDir(pathname2);

  for (int i = 0; i < n; i++)
  {
    sprintf(filename, "%s/%s_%03d-seeds.txt", pathname2, basename, i);
    iftWriteSeeds(S[i], timgs[i], filename);
  }
}

void WriteModel(iftMatrix *kernels, float *bias, int *seedpt,
                char *dirname, char *basename)
{
  FILE *fp;
  char filename[400], pathname[200];

  if (!iftDirExists(dirname))
    iftMakeDir(dirname);

  sprintf(pathname, "%s/%s", dirname, basename);
  if (!iftDirExists(pathname))
    iftMakeDir(pathname);

  sprintf(filename, "%s/conv1-kernels.npy", pathname);
  iftWriteMatrix(kernels, filename);
  sprintf(filename, "%s/conv1-bias.txt", pathname);
  fp = fopen(filename, "w");
  fprintf(fp, "%d\n", kernels->ncols);
  for (int k = 0; k < kernels->ncols; k++)
  {
    fprintf(fp, "%f\n", bias[k]);
  }
  fclose(fp);
  sprintf(filename, "%s/conv1-seedpt.txt", pathname);
  fp = fopen(filename, "w");
  fprintf(fp, "%d\n", kernels->ncols);
  for (int k = 0; k < kernels->ncols; k++)
  {
    fprintf(fp, "%d\n", seedpt[k]);
  }
  fclose(fp);
}

void MaskingArray(iftImage **img, int n, iftImage **mask)
{
  for (int i = 0; i < n; i++)
  {
    for (int p = 0; p < mask[i]->n; p++)
    {
      if (mask[i]->val[p] == 0)
        img[i]->val[p] = 0;
    }
  }
}

void Masking(iftImage *img, iftImage *mask)
{
  for (int p = 0; p < mask->n; p++)
  {
    if (mask->val[p] == 0)
      img->val[p] = 0;
  }
}

iftImage **ThresholdImageArray(iftImage **img, int n, int Imin, int Imax, int out_val)
{
  iftImage **bin = CreateImageArray(n);

  for (int i = 0; i < n; i++)
    bin[i] = iftThreshold(img[i], Imin, Imax, out_val);

  return (bin);
}

void FilterGaborBinArray(iftImage **gabor_bin, int n, iftDict *param)
{
  float close_rec = (float)iftGetDblValFromDict("close_rec", param);
  int area_open = (int)iftGetLongValFromDict("area_open", param);
  int area_close = (int)iftGetLongValFromDict("area_close", param);

  for (int i = 0; i < n; i++)
  {
    iftImage *cbin = iftCloseRecBin(gabor_bin[i], close_rec);
    iftImage *obin = iftFastAreaOpen(cbin, area_open);
    iftDestroyImage(&cbin);
    cbin = iftFastAreaClose(obin, area_close);
    iftDestroyImage(&gabor_bin[i]);
    iftDestroyImage(&obin);
    gabor_bin[i] = cbin;
  }
}

void FilterGaborBin(iftImage **gabor_bin, iftDict *param)
{
  float close_rec = (float)iftGetDblValFromDict("close_rec", param);
  int area_open = (int)iftGetLongValFromDict("area_open", param);
  int area_close = (int)iftGetLongValFromDict("area_close", param);

  iftImage *cbin = iftCloseRecBin(*gabor_bin, close_rec);
  iftImage *obin = iftFastAreaOpen(cbin, area_open);
  iftDestroyImage(&cbin);
  cbin = iftFastAreaClose(obin, area_close);
  iftDestroyImage(gabor_bin);
  iftDestroyImage(&obin);
  *gabor_bin = cbin;
}

iftImage *GetMask(iftImage *bin, iftDict *param)
{
  float asf_close_open = (float)iftGetDblValFromDict("asf_close_open", param);

  iftImage *mask = iftAsfCOBin(bin, asf_close_open);
  iftImage *cbasins = iftCloseBasins(mask, NULL, NULL);
  iftDestroyImage(&mask);

  return (cbasins);
}

iftImage **GetMaskArray(iftImage **bin, int n, iftDict *param)
{
  iftImage **mask = CreateImageArray(n);
  float asf_close_open = (float)iftGetDblValFromDict("asf_close_open", param);

  for (int i = 0; i < n; i++)
  {
    mask[i] = iftAsfCOBin(bin[i], asf_close_open);
    iftImage *cbasins = iftCloseBasins(mask[i], NULL, NULL);
    iftDestroyImage(&mask[i]);
    mask[i] = cbasins;
  }

  return (mask);
}

float RandomFloat(float min, float max)
{
  float n = (float)rand() / ((float)RAND_MAX + 0.5);
  return (iftMin((float)(n * (max - min) + min), max));
}

iftMatrix **GetRandomTransformations(iftImage *orig, iftDict *param)
{
  float min_scale = (float)
      iftGetDblValFromDict("min_scale", param);
  float max_scale = (float)
      iftGetDblValFromDict("max_scale", param);
  float min_rotation = (float)
      iftGetDblValFromDict("min_rotation", param);
  float max_rotation = (float)
      iftGetDblValFromDict("max_rotation", param);
  float min_translation_x = (float)
      iftGetDblValFromDict("min_translation_x", param);
  float max_translation_x = (float)
      iftGetDblValFromDict("max_translation_x", param);
  float min_translation_y = (float)
      iftGetDblValFromDict("min_translation_y", param);
  float max_translation_y = (float)
      iftGetDblValFromDict("max_translation_y", param);
  int num_of_transformations = (int)
      iftGetLongValFromDict("num_of_transformations", param);

  iftMatrix **M = (iftMatrix **)calloc(num_of_transformations,
                                       sizeof(iftMatrix *));
  /* Generate random direct transformations */

  float sx, sy, tx, ty, theta;

  for (int t = 0; t < num_of_transformations; t++)
  {
    if (min_scale < max_scale)
    {
      sx = RandomFloat(min_scale, max_scale);
      sy = sx; // RandomFloat(min_scale,max_scale);
    }
    else
    {
      sx = sy = 1.0;
    }
    if (min_rotation < max_rotation)
      theta = RandomFloat(min_rotation, max_rotation);
    else
      theta = 0.0;

    if (min_translation_x < max_translation_x)
      tx = RandomFloat(min_translation_x, max_translation_x);
    else
      tx = 0;

    if (min_translation_y < max_translation_y)
      ty = RandomFloat(min_translation_y, max_translation_y);
    else
      ty = 0;

    iftMatrix *Rz = iftRotationMatrix(IFT_AXIS_Z, theta);
    iftMatrix *S = iftScaleMatrix(sx, sy, 1);
    iftVector V = {tx, ty, 0};
    iftMatrix *Tv = iftTranslationMatrix(V);

    iftMatrix *Maux = iftMultMatrices(S, Rz);
    M[t] = iftMultMatrices(Tv, Maux);

    iftDestroyMatrix(&S);
    iftDestroyMatrix(&Rz);
    iftDestroyMatrix(&Tv);
    iftDestroyMatrix(&Maux);
  }

  return (M);
}

iftImage **GetTransformedImages(iftImage *orig, iftMatrix **M, int n)
{
  iftImage **timgs = CreateImageArray(n);

  for (int t = 0; t < n; t++)
  {
    timgs[t] = iftTransformImageByMatrix(orig, M[t]);
  }

  return (timgs);
}

void WriteTransformations(iftMatrix **M, int n, char *dirname, char *basename, char *subdirname)
{
  char pathname[200], filename[400];

  sprintf(pathname, "%s/%s/%s", dirname, basename, subdirname);
  iftMakeDir(pathname);

  for (int t = 0; t < n; t++)
  {
    sprintf(filename, "%s/%s_%03d.npy", pathname, basename, t);
    iftWriteMatrix(M[t], filename);
  }
}

iftImage *Gabor(iftImage *img)
{
  iftMakeDir("tmp_gabor");
  iftWriteImageByExt(img, "tmp/img.png");

  int BUFFSIZE = 300;
  char filename[BUFFSIZE];

  char NEWIFT_DIR[BUFFSIZE];
  char *envvar = "NEWIFT_DIR";

  if (!getenv(envvar))
  {
    iftError("Error in Gabor. Set NEWIFT_DIR env variable.\n", "Gabor");
  }

  if (snprintf(NEWIFT_DIR, BUFFSIZE, "%s", getenv(envvar)) >= BUFFSIZE)
  {
    fprintf(stderr, "BUFSIZE of %d was too small. Aborting\n", BUFFSIZE);
    exit(1);
  }

  sprintf(filename, "python %s/demo/Fingerprint/fingerprint_enhancement.py tmp tmp_gabor", NEWIFT_DIR);

  // printf("Applying Gabor Filtering on Image %d/%d: %s\n", i+1, nimages, basename);
  system(filename);
  system("rm -rf tmp");

  iftImage *gabor_img = NULL;
  sprintf(filename, "tmp_gabor/enh/img.png");

  gabor_img = iftReadImageByExt(filename);
  iftImage *correct = iftInterp2D(gabor_img, (float)img->xsize / gabor_img->xsize, (float)img->ysize / gabor_img->ysize);
  iftDestroyImage(&gabor_img);

  gabor_img = correct;
  system("rm -rf tmp_gabor");
  return (gabor_img);
}

void deleteAnomalousBranch(iftImage *skel, iftImage *distance, int root)
{
  iftFIFO *F = iftCreateFIFO(skel->n);
  iftAdjRel *A = iftCircular(sqrt(2.0));
  // iftAdjRel *C = iftCircular(1.0);

  // iftColor RGB1, YCbCr1;
  // RGB1.val[0] = 255;
  // RGB1.val[1] = 255;
  // RGB1.val[2] = 0;
  // YCbCr1      = iftRGBtoYCbCr(RGB1,255);

  // printf("Deletando origem...\n");
  iftInsertFIFO(F, root);

  // iftVoxel u = iftGetVoxelCoord(skel, root);
  // iftDrawPoint(skel, u, YCbCr1, C, 255);

  skel->val[root] = 0;

  // RGB1.val[0] = 255;
  // RGB1.val[1] = 0;
  // RGB1.val[2] = 255;
  // YCbCr1      = iftRGBtoYCbCr(RGB1,255);

  while (!iftEmptyFIFO(F))
  {
    int p = iftRemoveFIFO(F);

    iftVoxel u = iftGetVoxelCoord(skel, p);
    for (int i = 1; i < A->n; i++)
    {
      iftVoxel v = iftGetAdjacentVoxel(A, u, i);
      if (iftValidVoxel(skel, v))
      {
        int q = iftGetVoxelIndex(skel, v);
        if (distance->val[q])
        {
          if (F->color[q] == IFT_WHITE)
          {
            // printf("Deletando pixel...\n");
            // iftDrawPoint(skel, v, YCbCr1, C, 255);

            skel->val[q] = 0;
            iftInsertFIFO(F, q);
          }
        }
      }
    }
  }
  iftDestroyFIFO(&F);
  iftDestroyAdjRel(&A);
}

void deleteAnomalousMinuciae(iftImage *skel, int root)
{
  skel->val[root] = 255;
}

void fixAnomalousBranches(iftImage *skel, iftImage *term, iftImage *bpts)
{
  // iftColor RGB1, YCbCr1;
  // RGB1.val[0] = 255;
  // RGB1.val[1] = 0;
  // RGB1.val[2] = 0;
  // YCbCr1      = iftRGBtoYCbCr(RGB1,255);
  // iftAdjRel *C = iftCircular(1.0);
  iftLabeledSet *seeds = NULL;

  iftAdjRel *A = iftCircular(sqrt(2.0));
  for (int i = 0; i < skel->n; i++)
  {
    if (bpts->val[i] != 0)
    {
      iftUnionLabeledSetElem(&seeds, i, 2);
    }
  }

  for (int i = 0; i < skel->n; i++)
  {
    if (term->val[i] != 0)
    {
      iftUnionLabeledSetElem(&seeds, i, 1);
    }
  }

  while (seeds != NULL)
  {

    int label;
    int r = iftRemoveLabeledSet(&seeds, &label);
    // if (label == 2) continue; // we only care about terminal points
    iftImage *distance = iftCreateImage(skel->xsize, skel->ysize, skel->zsize);
    iftImage *root = iftCreateImage(skel->xsize, skel->ysize, skel->zsize);
    iftFIFO *F = iftCreateFIFO(skel->n);

    distance->val[r] = 0;
    root->val[r] = r;
    iftInsertFIFO(F, r);

    // BFS
    while (!iftEmptyFIFO(F))
    {
      int p = iftRemoveFIFO(F);

      iftVoxel u = iftGetVoxelCoord(skel, p);

      if (distance->val[p] == MIN_MNT_PROXIMITY*RESCALE_FACTOR)
        continue; // propagate until a safe distance

      for (int i = 1; i < A->n; i++)
      {
        iftVoxel v = iftGetAdjacentVoxel(A, u, i);
        if (iftValidVoxel(skel, v))
        {
          int q = iftGetVoxelIndex(skel, v);
          if (F->color[q] == IFT_WHITE)
          {
            if (skel->val[q])
            {
              if ((label == 1) && bpts->val[q])
              { // terminal point finding close-by bifurcation point
                // printf("Deletando um branch anômalo...\n");
                // iftDrawPoint(skel, v, YCbCr1, C, 255);
                deleteAnomalousBranch(skel, distance, root->val[p]);
                term->val[p] = 0;
              }
              else if ((label == 1) && term->val[q])
              { // terminal point finding a close-by terminal point
                deleteAnomalousMinuciae(skel, root->val[p]);
                term->val[p] = 0;
              }
              else if ((label == 2) && bpts->val[q])
              { // bifurcation point finding a close-by bifurcation point
                deleteAnomalousMinuciae(skel, root->val[p]);
                bpts->val[p] = 0;
              }
              else if ((label == 2) && term->val[q])
              { // bifucation point finding a close-by terminal point
                deleteAnomalousMinuciae(skel, root->val[p]);
                bpts->val[p] = 0;
              }
              else
              { // propagate normally
                distance->val[q] = distance->val[p] + 1;
                root->val[q] = root->val[p];
                iftInsertFIFO(F, q);
              }
            }
          }
        }
      }
    }
    iftDestroyImage(&distance);
    iftDestroyImage(&root);
    iftDestroyFIFO(&F);
  }

  iftDestroyAdjRel(&A);
  iftDestroyLabeledSet(&seeds);
}

double euclidianDistance(iftVoxel u, iftVoxel v)
{
  double euc_dist = sqrt((u.x - v.x) * (u.x - v.x) + (u.y - v.y) * (u.y - v.y));
  return euc_dist;
}

iftVoxel triangleMidVoxel(iftVoxel u, iftVoxel v, iftVoxel l)
{
  iftVoxel m;
  m.x = (u.x + v.x + l.x) / 3;
  m.y = (u.y + v.y + l.y) / 3;
  return m;
}

void connectPoint(iftImage *skel, iftVoxel u, iftVoxel t)
{
  int r = iftGetVoxelIndex(skel, u);
  skel->val[r] = 128;

  float v_x = t.x;
  float v_y = t.y;

  float dx = u.x - v_x;
  float dy = u.y - v_y;

  float step = abs(dx) >= abs(dy) ? abs(dx) : abs(dy);

  dx = step == 0 ? dx : dx / step;
  dy = step == 0 ? dy : dy / step;

  int k = 1;

  float x = v_x, y = v_y;

  while (k <= step)
  {
    u.x = x;
    u.y = y;

    if (iftValidVoxel(skel, u))
    {
      int p = iftGetVoxelIndex(skel, u);
      skel->val[p] = 255;
    }
    x += dx;
    y += dy;
    k++;
  }
}

void connectCloseTerminations(iftImage *skel, iftImage *term, iftImage *bpts)
{
  // iftColor RGB1, YCbCr1;
  // RGB1.val[0] = 255;
  // RGB1.val[1] = 0;
  // RGB1.val[2] = 255;
  // YCbCr1      = iftRGBtoYCbCr(RGB1,255);
  // iftAdjRel *C = iftCircular(1.0);
  int n_triangles = 0;
  for (int i = 0; i < term->n; i++)
  {
    if (term->val[i])
    {
      for (int j = 0; j < term->n; j++)
      {
        if ((term->val[j]) && (j != i))
        {
          iftVoxel u = iftGetVoxelCoord(term, i);
          iftVoxel v = iftGetVoxelCoord(term, j);
          if (euclidianDistance(u, v) < TERM_DISTANCE_THRESHOLD)
          { // valid edge
            for (int k = 0; k < term->n; k++)
            {
              if ((term->val[k]) && (k != i) && (k != j))
              {
                iftVoxel l = iftGetVoxelCoord(term, k);
                if ((euclidianDistance(v, l) < TERM_DISTANCE_THRESHOLD) && (euclidianDistance(l, u)) < TERM_DISTANCE_THRESHOLD)
                { // its a triangle!
                  n_triangles++;
                  iftVoxel m = triangleMidVoxel(u, v, l);
                  int p = iftGetVoxelIndex(skel, m);
                  // iftDrawPoint(skel, u, YCbCr1, C, 255);
                  // iftDrawPoint(skel, v, YCbCr1, C, 255);
                  // iftDrawPoint(skel, l, YCbCr1, C, 255);

                  bpts->val[p] = 255;
                  term->val[i] = 0;
                  term->val[j] = 0;
                  term->val[k] = 0;
                  connectPoint(skel, m, u);
                  connectPoint(skel, m, v);
                  connectPoint(skel, m, l);
                }
              }
            }
          }
        }
      }
    }
  }
  n_triangles = n_triangles / 6;
  // printf("Number of triangles: %d\n", n_triangles);
}

// Defines an 8-Neighboorhood adjacency relation
// around pixel p in clockwise-order (p = n0):
// n2  n3  n4
// n1  p   n5
// n8  n7  n6
iftAdjRel *EightNeighborhood()
{
  iftAdjRel *A;

  A = iftCreateAdjRel(9);

  int dx[] = {0, -1, -1, 0, 1, 1, 1, 0, -1};
  int dy[] = {0, 0, 1, 1, 1, 0, -1, -1, -1};

  for (int i = 0; i < 9; i++)
  {
    A->dx[i] = dx[i];
    A->dy[i] = dy[i];
    A->dz[i] = 0;
  }

  return (A);
}

// calculates crossing number of pixel p in skel image
int crossingNumber(iftImage *skel, int p)
{
  iftAdjRel *A = EightNeighborhood();

  float cn = 0;
  if (skel->val[p])
  {
    iftVoxel w = iftGetVoxelCoord(skel, p);
    int sum = 0;

    for (int i = 1; i <= 8; i++)
    {
      iftVoxel u = iftGetAdjacentVoxel(A, w, i);
      iftVoxel v = iftGetAdjacentVoxel(A, w, i % 8 + 1);
      if (iftValidVoxel(skel, u) && iftValidVoxel(skel, v))
      {
        int u_i = iftGetVoxelIndex(skel, u);
        int v_i = iftGetVoxelIndex(skel, v);

        int pixel_u_i, pixel_v_i;
        pixel_u_i = pixel_v_i = 0;
        
        if (skel->val[u_i] > 0)
          pixel_u_i = 1;
        if (skel->val[v_i] > 0)
          pixel_v_i = 1;
        sum += abs(pixel_u_i - pixel_v_i );
      }
    }

    cn = sum / 2;
  }

  iftDestroyAdjRel(&A);
  return iftRound(cn);
}

iftImage *crossingNumberImg(iftImage *skel)
{
  int p;

  iftImage *cn_pts = iftCreateImage(skel->xsize, skel->ysize, skel->zsize);

  if (iftIs3DImage(skel))
    iftError("It works for 2D images only", "iftTerminalPoints2D");

  for (p = 0; p < skel->n; p++)
  {
    if (skel->val[p] != 0)
    {
      int cn = crossingNumber(skel, p);

      // if (cn == 1)
      // {
      //   term->val[p] = 1;
      // }
      // else if (cn >= 3)
      // {
      //   bpts->val[p] = 1;
      // } else {
      //   term->val[p] = 0;
      //   bpts->val[p] = 0;
      // }

      cn_pts->val[p] = cn;
    } else {
      cn_pts->val[p] = 0;
      // term->val[p] = 0;
      // bpts->val[p] = 0;
    }
  }

  return (cn_pts);
}

// Perform Breadth-First traversal from a given source pixel p to find branches
void traverse_term(int p, iftImage *skel, iftImage *cn_img)
{
  // Directions for 8-connected neighbors

  iftFIFO *queue = iftCreateFIFO(skel->n);
  iftAdjRel *A = iftCircular(sqrt(2));

  // Initialize for the source pixel
  iftInsertFIFO(queue, p);

  iftImage* distance = iftCreateImage(skel->xsize, skel->ysize, skel->zsize);
  for (int i = 0; i < distance->n; i++) {
    distance->val[i] = IFT_INFINITY_INT;
  }

  distance->val[p] = 0;

  int branch_len = 0;


  iftFIFO *to_be_deleted = iftCreateFIFO(skel->n);

  while (!iftEmptyFIFO(queue))
  {
    int curr = iftRemoveFIFO(queue);

    branch_len = distance->val[curr];

    if ((cn_img->val[curr] == 1) && branch_len>0)
    {
      iftInsertFIFO(to_be_deleted, curr);
      break;
    } else if (cn_img->val[curr] >= 3 && branch_len>0) {
      break;
    }

    iftInsertFIFO(to_be_deleted, curr);

    // Explore neighbors
    iftVoxel u = iftGetVoxelCoord(skel, curr);
    for (int i = 1; i < A->n; i++)
    {
      iftVoxel v = iftGetAdjacentVoxel(A, u, i);
      if (iftValidVoxel(skel, v))
      {
        int q = iftGetVoxelIndex(skel, v);

        // Check if neighbor is not visited
        if ((queue->color[q] == IFT_WHITE) && (skel->val[q] != 0))
        {
          iftInsertFIFO(queue, q);
          distance->val[q] = distance->val[curr] + 1;
        }
      }
    }
  }

  if (branch_len < MIN_MNT_PROXIMITY*RESCALE_FACTOR)
  {
    while (!iftEmptyFIFO(to_be_deleted))
    {
      int x = iftRemoveFIFO(to_be_deleted);
      skel->val[x] = 0;
    }
  }

  iftResetFIFO(to_be_deleted);
  iftResetFIFO(queue);

  iftDestroyFIFO(&to_be_deleted);
  iftDestroyFIFO(&queue);
  iftDestroyAdjRel(&A);

  iftDestroyImage(&distance);
}


void traverse_bifurc(int p, iftImage *skel, iftImage *cn_img)
{
  // Directions for 8-connected neighbors

  iftFIFO *queue = iftCreateFIFO(skel->n);
  iftAdjRel *A = iftCircular(sqrt(2));

  // Initialize for the source pixel
  iftInsertFIFO(queue, p);

  iftImage* distance = iftCreateImage(skel->xsize, skel->ysize, skel->zsize);
  for (int i = 0; i < distance->n; i++) {
    distance->val[i] = IFT_INFINITY_INT;
  }

  distance->val[p] = 0;

  int branch_len = 0;


  iftFIFO *to_be_deleted = iftCreateFIFO(skel->n);

  int last = p;

  while (!iftEmptyFIFO(queue))
  {
    int curr = iftRemoveFIFO(queue);

    branch_len = distance->val[curr];

    if (((cn_img->val[curr] >= 3) && branch_len>0))
    {
      last = curr;
      // iftInsertFIFO(to_be_deleted, curr);
      break;
    } 

    if (branch_len > 0){
      iftInsertFIFO(to_be_deleted, curr);
    }

    // Explore neighbors
    iftVoxel u = iftGetVoxelCoord(skel, curr);
    for (int i = 1; i < A->n; i++)
    {
      iftVoxel v = iftGetAdjacentVoxel(A, u, i);
      if (iftValidVoxel(skel, v))
      {
        int q = iftGetVoxelIndex(skel, v);

        // Check if neighbor is not visited
        if ((queue->color[q] == IFT_WHITE) && (skel->val[q] != 0))
        {
          distance->val[q] = distance->val[curr] + 1;
          iftInsertFIFO(queue, q);
        }
      }
    }
  }

  if (branch_len < MIN_MNT_PROXIMITY*RESCALE_FACTOR)
  {
    while (!iftEmptyFIFO(to_be_deleted))
    {
      int x = iftRemoveFIFO(to_be_deleted);
      skel->val[x] = 0;
    }
    skel->val[p] = 255;
    skel->val[last] = 255;
  }

  iftResetFIFO(to_be_deleted);
  iftResetFIFO(queue);

  iftDestroyFIFO(&to_be_deleted);
  iftDestroyFIFO(&queue);
  iftDestroyAdjRel(&A);

  iftDestroyImage(&distance);
}


void removeBugPixels(iftImage *skel)
{

  iftAdjRel *A = iftCircular(sqrt(1.0));

  for (int p = 0; p < skel->n; p++)
  {
    iftVoxel u = iftGetVoxelCoord(skel, p);

    if (skel->val[p] == 0)
      continue;


    int neighbors_count = 0;
    for (int i = 0; i < A->n; i++) {
      iftGetAdjacentVoxel(A, u, i);

      iftVoxel v = iftGetAdjacentVoxel(A, u, i);
      if (iftValidVoxel(skel, v))
      {
        int q = iftGetVoxelIndex(skel, v);
        
        // count number of 4-connected-neighbors
        if ((q != p) && skel->val[q] != 0) {
          neighbors_count++;
        }
      }
    }

    // p is bug pixel
    if (neighbors_count > 2) {
      skel->val[p] = 0;
    } 
  }

  iftDestroyAdjRel(&A);
  // iftDestroyLabeledSet(&seeds);
}

void filterSpikes(iftImage *skel, iftImage *cn_img)
{
  // iftColor RGB1, YCbCr1;
  // RGB1.val[0] = 255;
  // RGB1.val[1] = 0;
  // RGB1.val[2] = 0;
  // YCbCr1      = iftRGBtoYCbCr(RGB1,255);
  // iftAdjRel *C = iftCircular(1.0);
  // iftLabeledSet *seeds = NULL;

  iftAdjRel *A = iftCircular(sqrt(2.0));

  for (int p = 0; p < skel->n; p++)
  {
    // if (p % (skel->n/10) == 0)
      // printf("%d%%\n", (int)(p*100/skel->n));
    // Breadth-fisrt traversal on every ridge-ending point
    if (skel->val[p] != 0 && cn_img->val[p] == 1)
    {
      traverse_term(p, skel, cn_img);
    }
  }

  // for (int p = 0; p < skel->n; p++)
  // {
  //   // if (p % (skel->n/10) == 0)
  //     // printf("%d%%\n", (int)(p*100/skel->n));
  //   // Breadth-fisrt traversal on every ridge-ending point
  //   if (skel->val[p] != 0 && cn_img->val[p] == 1)
  //   {
  //     traverse_bifurc(p, skel, cn_img);
  //   }
  // }

  iftDestroyAdjRel(&A);
  // iftDestroyLabeledSet(&seeds);
}

void find_connected_pairs(int p, iftImage *skel, iftImage *cn_img)
{
  // ########### PYTHON CODE:
  // Q = [(row, col)]
  // parent = {}
  // distance = np.zeros_like(img) + np.inf
  // color = np.zeros_like(img)
  // bfs_img = np.zeros_like(img)

  // pairs = []
  // distance[row][col] = 0


  // is_term = False
  // if crossing_number(img, row, col) == 1:
  //     is_term = True
      
  // while len(Q) > 0:
  //     curr = Q.pop(0)
  //     # print(curr)

  //     color[curr[0]][curr[1]] = 2
  //     bfs_img[curr[0]][curr[1]] = 1

  //     if distance[curr[0]][curr[1]] > 4:
  //         break

  //     if is_term:
  //         if (crossing_number(img, curr[0], curr[1]) == 1) and (distance[curr[0]][curr[1]] > 1):
  //             pairs.append((curr[0], curr[1]))
  //     else:
  //         if (crossing_number(img, curr[0], curr[1]) >= 3) and (distance[curr[0]][curr[1]] > 1):
  //             pairs.append((curr[0], curr[1]))

  //     neighbors_coords = neighbors(img, curr[0], curr[1])
      
  //     for neighbor_coord in neighbors_coords:
  //         q_row = neighbor_coord[0]
  //         q_col = neighbor_coord[1]

  //         if ((img[q_row][q_col] != 0) or is_term) and (color[q_row][q_col] == 0):
  //             Q.append(neighbor_coord)
  //             parent[neighbor_coord] = curr
  //             distance[neighbor_coord[0]][neighbor_coord[1]] = distance[curr[0]][curr[1]] + 1
  //             color[neighbor_coord[0]][neighbor_coord[1]] = 1
      
  // return pairs
}

// M[i][0] = p and M[i][1] = q means p and q form minutiae pair of same type
iftMatrix *getMntPairMatrix(iftImage* skel, iftImage *cn_img) {

  iftList *pair_0 = iftCreateList();
  iftList *pair_1 = iftCreateList();

  int n_mnt_pairs = 0;
  
  printf("Getting minutiae pair matrix...\n");
  for (int p = 0; p < skel->n; p++) {
    if ((cn_img->val[p] == 1) || (cn_img->val[p] >= 3)) {
      iftFIFO *queue = iftCreateFIFO(skel->n);
      iftAdjRel *A = iftCircular(sqrt(2));

      bool is_term = false;
      if (cn_img->val[p] == 1)
        is_term = true;

      // Initialize for the source pixel
      iftInsertFIFO(queue, p);

      iftImage* distance = iftCreateImage(skel->xsize, skel->ysize, skel->zsize);
      for (int i = 0; i < distance->n; i++) {
        distance->val[i] = IFT_INFINITY_INT;
      }

      distance->val[p] = 0;

      while (!iftEmptyFIFO(queue))
      {
        int curr = iftRemoveFIFO(queue);

        if (distance->val[curr] > MIN_MNT_PROXIMITY*RESCALE_FACTOR)
        {
          break;
        }


        if(is_term) {
          if (cn_img->val[curr] == 1 && distance->val[curr] > 1) {
            // pairs.append(p, curr)

            iftInsertListIntoTail(pair_0, p);
            iftInsertListIntoTail(pair_1, curr);

            n_mnt_pairs++;
          }
        } else {
          if (cn_img->val[curr] <= 3 && distance->val[curr] > 1) {
            // pairs.append(p, curr)

            iftInsertListIntoTail(pair_0, p);
            iftInsertListIntoTail(pair_1, curr);

            n_mnt_pairs++;
          }
        }

        // Explore neighbors
        iftVoxel u = iftGetVoxelCoord(skel, curr);
        for (int i = 1; i < A->n; i++)
        {
          iftVoxel v = iftGetAdjacentVoxel(A, u, i);
          if (iftValidVoxel(skel, v))
          {
            int q = iftGetVoxelIndex(skel, v);

            // Check if neighbor is not visited
            if ((queue->color[q] == IFT_WHITE) && (skel->val[q] != 0 || is_term))
            {
              iftInsertFIFO(queue, q);
              distance->val[q] = distance->val[curr] + 1;
            }
          }
        }
      }


      iftResetFIFO(queue);

      iftDestroyFIFO(&queue);
      iftDestroyAdjRel(&A);

      iftDestroyImage(&distance);
    }

  }

  // creatue minutae pair matrix
  iftMatrix *M = iftCreateMatrix(2, n_mnt_pairs);

  for (int i = 0; i < n_mnt_pairs; i++) {
    iftMatrixElem(M, 0, i) = iftRemoveListTail(pair_0);
    iftMatrixElem(M, 1, i) = iftRemoveListTail(pair_1);
  }

  iftDestroyList(&pair_0);
  iftDestroyList(&pair_1);

  return M;

  
}

// Function to calculate the orientation of the ordered triplet (p, q, r)
// 0 -> p, q and r are collinear
// 1 -> Clockwise
// 2 -> Counterclockwise
int orientation(int px, int py, int qx, int qy, int rx, int ry) {
  int val = (qy - py) * (rx - qx) - (qx - px) * (ry - qy);
  if (val == 0) return 0; // collinear
  return (val > 0) ? 1 : 2; // clockwise or counterclockwise
}

// Function to check if point (px, py) lies on line segment (x1, y1) - (x2, y2)
bool onSegment(int x1, int y1, int x2, int y2, int px, int py) {
  if (px <= (x1 > x2 ? x1 : x2) && px >= (x1 < x2 ? x1 : x2) &&
    py <= (y1 > y2 ? y1 : y2) && py >= (y1 < y2 ? y1 : y2)) {
    return true;
  }
  return false;
}

// Function to check if two segments (p1, q1) and (p2, q2) intersect
bool doIntersect(iftImage *img, int p1, int p2, int p3, int p4) {

    int x1 = iftGetVoxelCoord(img, p1).x;
    int y1 = iftGetVoxelCoord(img, p1).y;

    int x2 = iftGetVoxelCoord(img, p2).x;
    int y2 = iftGetVoxelCoord(img, p2).y;

    int x3 = iftGetVoxelCoord(img, p3).x;
    int y3 = iftGetVoxelCoord(img, p3).y;

    int x4 = iftGetVoxelCoord(img, p4).x;
    int y4 = iftGetVoxelCoord(img, p4).y;

    // Find the four orientations needed for the general and special cases
    int o1 = orientation(x1, y1, x2, y2, x3, y3);
    int o2 = orientation(x1, y1, x2, y2, x4, y4);
    int o3 = orientation(x3, y3, x4, y4, x1, y1);
    int o4 = orientation(x3, y3, x4, y4, x2, y2);

    // General case
    if (o1 != o2 && o3 != o4) {
      return true;
    }

    // Special Cases
    // x1, y1, x2, y2, x3, y3 are collinear and x3, y3 lies on segment x1, y1 - x2, y2
    if (o1 == 0 && onSegment(x1, y1, x2, y2, x3, y3)) return true;

    // x1, y1, x2, y2, x4, y4 are collinear and x4, y4 lies on segment x1, y1 - x2, y2
    if (o2 == 0 && onSegment(x1, y1, x2, y2, x4, y4)) return true;

    // x3, y3, x4, y4, x1, y1 are collinear and x1, y1 lies on segment x3, y3 - x4, y4
    if (o3 == 0 && onSegment(x3, y3, x4, y4, x1, y1)) return true;

    // x3, y3, x4, y4, x2, y2 are collinear and x2, y2 lies on segment x3, y3 - x4, y4
    if (o4 == 0 && onSegment(x3, y3, x4, y4, x2, y2)) return true;

    return false;
}

void colorShortestPath(iftImage* img, int p1, int p2, int value) {
   // Directions for 8-connected neighbors

  iftFIFO *queue = iftCreateFIFO(img->n);
  iftAdjRel *A = iftCircular(sqrt(2));

  // Initialize for the source pixel
  iftInsertFIFO(queue, p1);

  iftImage* distance = iftCreateImage(img->xsize, img->ysize, img->zsize);
  iftImage* parent = iftCreateImage(img->xsize, img->ysize, img->zsize);
  iftImage* drawn = iftCreateImage(img->xsize, img->ysize, img->zsize);

  for (int i = 0; i < distance->n; i++) {
    distance->val[i] = IFT_INFINITY_INT;
    parent->val[i] = -1;
    drawn->val[i] = 0;
  }

  distance->val[p1] = 0;

  while (!iftEmptyFIFO(queue))
  {
    int curr = iftRemoveFIFO(queue);

    if (curr == p2)
    {
      break;
    }

    // Explore neighbors
    iftVoxel u = iftGetVoxelCoord(img, curr);
    for (int i = 1; i < A->n; i++)
    {
      iftVoxel v = iftGetAdjacentVoxel(A, u, i);
      if (iftValidVoxel(img, v))
      {
        int q = iftGetVoxelIndex(img, v);

        // Check if neighbor is not visited
        if ((queue->color[q] == IFT_WHITE))
        {
          iftInsertFIFO(queue, q);
          distance->val[q] = distance->val[curr] + 1;
          parent->val[q] = curr;
        }
      }
    }
  }

  int p = parent->val[p2];
  while (p != p1) {
    img->val[p] = value;
    drawn->val[p] = 255;
    p = parent->val[p];
  }


  iftResetFIFO(queue);
  iftDestroyFIFO(&queue);
  iftDestroyAdjRel(&A);

  iftDestroyImage(&distance);
  iftDestroyImage(&parent);

  iftWriteImageByExt(drawn, "drawn.png");
  iftDestroyImage(&drawn);
}

float getBifurcAngleDual(iftImage *skel, iftImage*skel_dual, int p) {

  iftFIFO *queue = iftCreateFIFO(skel->n);
  iftAdjRel *A = iftCircular(sqrt(2));

  // Initialize for the source pixel
  iftInsertFIFO(queue, p);

  iftImage* distance = iftCreateImage(skel->xsize, skel->ysize, skel->zsize);
  for (int i = 0; i < distance->n; i++) {
    distance->val[i] = IFT_INFINITY_INT;
  }

  distance->val[p] = 0;

  int closest_term = -1;

  while (!iftEmptyFIFO(queue))
  {
    int curr = iftRemoveFIFO(queue);

    if (crossingNumber(skel_dual, curr) == 1)
    {
      closest_term = curr;
      break;
    }

    // Explore neighbors
    iftVoxel u = iftGetVoxelCoord(skel, curr);
    for (int i = 1; i < A->n; i++)
    {
      iftVoxel v = iftGetAdjacentVoxel(A, u, i);
      if (iftValidVoxel(skel, v))
      {
        int q = iftGetVoxelIndex(skel, v);

        // Check if neighbor is not visited
        if (queue->color[q] == IFT_WHITE)
        {
          iftInsertFIFO(queue, q);
          distance->val[q] = distance->val[curr] + 1;
        }
      }
    }
  }

  iftVoxel v1 = iftGetVoxelCoord(skel, p);
  iftVoxel v2 = iftGetVoxelCoord(skel, closest_term);

  float theta = atan2(v2.y - v1.y, v2.x - v1.x);

  iftResetFIFO(queue);

  iftDestroyFIFO(&queue);
  iftDestroyAdjRel(&A);

  iftDestroyImage(&distance);

  return theta;
}

float getTermAngle(iftImage *skel, int p) {

  iftFIFO *queue = iftCreateFIFO(skel->n);
  iftAdjRel *A = iftCircular(sqrt(2));

  // Initialize for the source pixel
  iftInsertFIFO(queue, p);

  iftImage* distance = iftCreateImage(skel->xsize, skel->ysize, skel->zsize);
  for (int i = 0; i < distance->n; i++) {
    distance->val[i] = IFT_INFINITY_INT;
  }

  distance->val[p] = 0;

  int closest_term = -1;

  while (!iftEmptyFIFO(queue))
  {
    int curr = iftRemoveFIFO(queue);

    if (distance->val[curr] >= 10)
    {
      closest_term = curr;
      break;
    }

    // Explore neighbors
    iftVoxel u = iftGetVoxelCoord(skel, curr);
    for (int i = 1; i < A->n; i++)
    {
      iftVoxel v = iftGetAdjacentVoxel(A, u, i);
      if (iftValidVoxel(skel, v))
      {
        int q = iftGetVoxelIndex(skel, v);

        // Check if neighbor is not visited
        if (queue->color[q] == IFT_WHITE && skel->val[q] > 0)
        {
          iftInsertFIFO(queue, q);
          distance->val[q] = distance->val[curr] + 1;
        }
      }
    }
  }

  iftVoxel v1 = iftGetVoxelCoord(skel, p);
  iftVoxel v2 = iftGetVoxelCoord(skel, closest_term);

  float theta = atan2(v2.y - v1.y, v2.x - v1.x);

  iftResetFIFO(queue);

  iftDestroyFIFO(&queue);
  iftDestroyAdjRel(&A);

  iftDestroyImage(&distance);

  return theta;
}

float AbsSmallestAngleDiff(float theta1, float theta2) {
  float a = atan2(sin(theta1-theta2), cos(theta1-theta2));
  return abs(a);
}


double AngleBetweenPoints(iftImage* img, int p1, int p2) {
    iftVoxel v1 = iftGetVoxelCoord(img, p1);
    iftVoxel v2 = iftGetVoxelCoord(img, p2);

    // Calculate the difference in x and y coordinates
    double deltaX = v2.x - v1.x;
    double deltaY = v2.y - v1.y;

    // Calculate the angle using atan2
    double angle = atan2(deltaY, deltaX); // Angle in radians

    return angle;
}

void RemoveShortBreaksSkel(iftImage* skel) {

  iftAdjRel *A = iftCircular(15*RESCALE_FACTOR);
  for (int p = 0; p < skel->n; p++) {
    if (p % (skel->n/10) == 0)
      printf("%d%%\n", (int)(p*100/skel->n));
    if (crossingNumber(skel, p) == 1) {
      // is termination point

      float theta1 = getTermAngle(skel, p);

      // find neighboring termination points in radius
      iftVoxel u = iftGetVoxelCoord(skel, p);
      for (int i = 1; i < A->n; i++)
      {
        iftVoxel v = iftGetAdjacentVoxel(A, u, i);
        if (iftValidVoxel(skel, v))
        {
          int q = iftGetVoxelIndex(skel, v);

          // condition a) p and q are terminations within distance 16*RESCLAE_FACTROE
          if (crossingNumber(skel, q) == 1) {
            // bool ok = true;

            float theta2 = getTermAngle(skel , q);
            float theta3 = AngleBetweenPoints(skel,p,q);

            // condition b) theta1 and theta2 almost equal
            if (abs(theta1 - (theta2 + PI)) < PI/10 || abs(theta2 - (theta1 + PI)) < PI/10) {
            // if (abs(theta1 - (theta2 + PI)) < PI/20 || abs(theta2 - (theta1 + PI)) < PI/20) {
              // condition c) angle between point close to either theta1 or theta2
              if (AbsSmallestAngleDiff(theta3, theta1) < PI/10  
                  || AbsSmallestAngleDiff(theta3, theta2) < PI/10) {
                  skel->val[p] = 255;
                  skel->val[q] = 255;
                  // ok = false;
              }
            }

            // printf("termination pair: %d %d %d | %d\n", iftRound(180*theta1/PI), iftRound(180*(theta2)/PI), iftRound(180*(theta3)/PI),(int)ok);
  
          }
        }
      }
    }
  }

  iftDestroyAdjRel(&A);
}


iftSet *h_point_removal(iftImage* skel, iftImage* skel_dual, iftImage* cn_img, iftImage* cn_img_dual) {
  printf("Removing H-points...\n");

  iftMatrix *skel_mnt_pairs = getMntPairMatrix(skel, cn_img); 

  // iftWriteMatrixCSV(skel_mnt_pairs, "skel_mnt_pairs.csv");

  iftMatrix *comp_skel_mnt_pairs = getMntPairMatrix(skel_dual, cn_img_dual);

  // iftSet* false_mnts = NULL;

  printf("Detecting intersections between %d dual pairs...\n", skel_mnt_pairs->nrows);

  iftSet *false_mnts = NULL;
  
  for (int i = 0; i < skel_mnt_pairs->nrows; i++) {
    if (i% (skel_mnt_pairs->nrows/10) == 0)
      printf("%d%%\n", (int)(i*100/skel_mnt_pairs->nrows));

    int p1 = iftMatrixElem(skel_mnt_pairs, 0, i);
    int p2 = iftMatrixElem(skel_mnt_pairs, 1, i);

    // termination mnt pairs on original skel
    if (cn_img->val[p1] == 1) {

      for (int j = 0; j < comp_skel_mnt_pairs->nrows; j++) {
        
        if(iftSetHasElement(false_mnts, p1) || iftSetHasElement(false_mnts, p1))
          break;

        int p3 = iftMatrixElem(comp_skel_mnt_pairs, 0, j);
        int p4 = iftMatrixElem(comp_skel_mnt_pairs, 1, j);

        // iftVoxel v1 = iftGetVoxelCoord(skel, p1);
        // iftVoxel v2 = iftGetVoxelCoord(skel, p2);
        // iftVoxel v3 = iftGetVoxelCoord(skel, p3);
        // iftVoxel v4 = iftGetVoxelCoord(skel, p4);

        // bifurcation mnt pairs on dual skel
        if (cn_img_dual->val[p3] >= 3) {

          if (doIntersect(skel, p1, p2, p3, p4)) {
            iftUnionSetElem(&false_mnts, p1);
            iftUnionSetElem(&false_mnts, p2);
            cn_img->val[p1] = 2;
            cn_img->val[p2] = 2;
            skel->val[p1] = 255;
            skel->val[p2] = 255;

            // colorShortestPath(skel, p1, p2, 255);

            cn_img_dual->val[p3] = 2;
            cn_img_dual->val[p4] = 2;
            // printf("intersect: \n (%d, %d) <=> (%d, %d)\n (%d, %d) <=> (%d, %d)\n", 
            // v1.x, v1.y, v2.x, v2.y, 
            // v3.x, v3.y, v4.x, v4.y);
          }

          // iftUnionSetElem(&false_mnts, p1);

        }
      }
    }
  } 

  iftDestroyMatrix(&skel_mnt_pairs);
  iftDestroyMatrix(&comp_skel_mnt_pairs);

  return false_mnts;
  
}

void mergeDualSkels(iftImage* skel, iftImage* skel_dual) {
  for (int p = 0; p < skel->n; p++) {
    // iftVoxel v = iftGetVoxelCoord(skel, p);
    if(skel_dual->val[p] > 0) {
      skel->val[p] = 80;
    }
  }
}
int SkelToOrigIndex(iftImage* skel, iftImage* orig, int p) {
  float scale_factor = skel->xsize/orig->xsize;
  iftVoxel v = iftGetVoxelCoord(skel, p);
  iftVoxel u = {iftRound(v.x / scale_factor), iftRound(v.y / scale_factor), 0};
  int q = iftGetVoxelIndex(orig, u);

  return q;
}

// Function to compute the smallest angular distance between two angles
double angular_distance(double angle1, double angle2) {
    double diff = fabs(angle1 - angle2);
    return fmin(diff, 2*PI - diff); // Choose the smallest of direct and wrapped distances
}

// iftMatrix *mntAnglesFromDual(iftImage* skel, iftImage* skel_dual, iftImage* bin, iftImage* of_map, iftSet* mnts) {
//   int n = iftSetSize(mnts);
//   iftMatrix *M = iftCreateMatrix(2, n);

//   for (int i = 0; i < n; i++)
//   {
//     iftMatrixElem(M, 0, i) = -1;
//     iftMatrixElem(M, 1, i) = -1;
//   }

//   int c = 0;
//   while(iftSetSize(mnts) > 0)
//   {
//     int p = iftRemoveSet(&mnts);

//     float theta = -100;
//     int cn = crossingNumber(skel, p);

//     if (cn >= 3) {
//       theta = getBifurcAngle(skel, skel_dual, p);
//     } else if (cn == 1) {
//       theta = getTermAngle(skel, p);
//     }


//     int q = SkelToOrigIndex(skel, bin, p);


//     float theta_of = ((float)of_map->val[q] / 255.0) * PI;
//     // printf("theta_of = %f\n", theta_of);

//     float theta_180 = fmod(theta + PI, PI*2);
//     if (theta_180 < 0) {
//         theta_180 += PI*2; // Adjust for negative values
//     }
//     // printf("theta_of_180 = %f\n", theta_of_180);

//     // Compute distances
//     float dist1 = angular_distance(theta, theta_of);
//     // printf("dist1 = %f\n", dist1);
//     float dist2 = angular_distance(theta_180, theta_of);
//     // printf("dist2 = %f\n", dist2);

//     // float theta_of_180 = fmod(theta_of + PI, PI*2);
//     // if (theta_of_180 < 0) {
//     //     theta_of_180 += PI*2; // Adjust for negative values
//     // }
//     // // printf("theta_of_180 = %f\n", theta_of_180);

//     // // Compute distances
//     // float dist1 = angular_distance(theta_of, theta);
//     // // printf("dist1 = %f\n", dist1);
//     // float dist2 = angular_distance(theta_of_180, theta);

//     // Return the closer angle
//     float final_theta = (dist1 <= dist2) ? theta : theta_180;


//     iftMatrixElem(M, 0, c) = q;
//     iftMatrixElem(M, 1, c) = final_theta;

//     c++;

//   }

//   return M;
// }



iftImage* Skeleton(iftImage *gabor_bin)
{
  iftAdjRel *A = iftCircular(1.5);
  iftImage *skel = NULL;

  /* Generate skeleton of the interpolated image with value 255*/

  iftImage *bin      = InterpolateBin(gabor_bin, RESCALE_FACTOR, RESCALE_FACTOR);
  
  iftFImage *msskel = iftMSSkel2D(bin, A, IFT_INTERIOR, NULL, NULL);

  skel = iftFThreshold(msskel, SKEL_THRES, 100.0, 255);

  iftDestroyFImage(&msskel);
  iftDestroyImage(&bin);

  iftDestroyAdjRel(&A);

  return skel;
}

iftImage *fixSkeletonAndFindMnts(iftImage* skel, iftImage *gabor_bin)
{
  /* Generate skeleton of the interpolated image with value 255 and
     set its minuciae points to 128 */

  // iftImage *bin_dual = iftComplement(gabor_bin);
  // iftImage *skel_dual = Skeleton(bin_dual);

  // iftImage *cn_img_dual = crossingNumberImg(skel_dual);

  removeBugPixels(skel);
  // removeBugPixels(skel_dual);

  iftImage *cn_img = crossingNumberImg(skel);

  // printf("Filtering Spikes...\n");
  filterSpikes(skel, cn_img);

  // filterBridges(skel, cn_img);

  iftImage *cn_img_new = crossingNumberImg(skel);

  iftDestroyImage(&cn_img);

  // mergeDualSkels(skel, skel_dual);


  int mnts_count = 0;
  for (int p = 0; p < skel->n; p++)
  {
    if ((cn_img_new->val[p] >= 3) || (cn_img_new->val[p] == 1))
    {
      // skel->val[p] = 170;
      skel->val[p] = 128;
      mnts_count++;
    }
  }

  // printf("Removing short breaks...\n");
  // RemoveShortBreaks(skel);

  // iftSet* false_mnts = h_point_removal(skel, skel_dual, cn_img_new, cn_img_dual);

  // iftSet* mnts = mntAnglesFromDual(skel, skel_dual, mnts_count - iftSetSize(false_mnts));


  iftDestroyImage(&cn_img_new);
  // iftDestroyImage(&cn_img_dual);
  // iftDestroyImage(&cn_filtered_img);

  // iftDestroySet(&false_mnts);

  return (skel);
}

iftImage **GaborImageArray(iftImage **img, int n)
{
  iftMakeDir("tmp_gabor");
  char filename[200];

  for (int i = 0; i < n; i++)
  {
    sprintf(filename, "tmp/img_%03d.png", i);
    iftWriteImageByExt(img[i], filename);
  }
  system("python fingerprint_enhancement.py tmp tmp_gabor");
  system("rm -rf tmp");
  iftImage **gabor_img = CreateImageArray(n);
  for (int i = 0; i < n; i++)
  {
    sprintf(filename, "tmp_gabor/enh/img_%03d.png", i);
    gabor_img[i] = iftReadImageByExt(filename);
    iftImage *correct = iftInterp2D(gabor_img[i], (float)img[i]->xsize / gabor_img[i]->xsize, (float)img[i]->ysize / gabor_img[i]->ysize);
    iftDestroyImage(&gabor_img[i]);
    gabor_img[i] = correct;
  }
  system("rm -rf tmp_gabor");
  return (gabor_img);
}

iftImage **SkeletonArray(iftImage **gabor_bin, int n)
{
  iftAdjRel *A = iftCircular(1.5);
  iftImage **skels = CreateImageArray(n);

  /* Generate skeleton of the interpolated image with value 255 and
     set its minuciae points to 128 */

  for (int i = 0; i < n; i++)
  {
    iftImage *bin = InterpolateBin(gabor_bin[i], 2.0, 2.0);
    iftFImage *msskel = iftMSSkel2D(bin, A, IFT_INTERIOR, NULL, NULL);
    iftDestroyImage(&bin);
    skels[i] = iftFThreshold(msskel, SKEL_THRES, 100.0, 255);
    iftDestroyFImage(&msskel);
    iftImage *term = iftTerminalPoints2D(skels[i]);
    iftImage *bpts = iftBranchPoints2D(skels[i]);
    for (int p = 0; p < skels[i]->n; p++)
    {
      if ((term->val[p] != 0) || (bpts->val[p] != 0))
      {
        skels[i]->val[p] = 128;
      }
    }
    iftDestroyImage(&term);
    iftDestroyImage(&bpts);
  }

  iftDestroyAdjRel(&A);
  return (skels);
}

iftLabeledSet **GetSkeletonPointArray(iftImage **tskels, int n)
{
  iftLabeledSet **S = (iftLabeledSet **)calloc(n, sizeof(iftLabeledSet *));

  for (int i = 0; i < n; i++)
  {
    S[i] = NULL;
    for (int p = 0; p < tskels[i]->n; p++)
    {
      if (tskels[i]->val[p] == 128)
      {
        iftInsertLabeledSet(&S[i], p, 1);
      }
    }
  }

  return (S);
}

iftVoxel InvTransformPoint(iftImage *src, iftVoxel v, iftMatrix *InvM)
{
  int diag = iftDiagonalSize(src);
  iftPoint P = {v.x - diag / 2.0, v.y - diag / 2.0, 0.0};
  iftPoint Q = iftTransformPoint(InvM, P);
  iftVoxel u = {iftRound(Q.x + src->xsize / 2.0), iftRound(Q.y + src->ysize / 2.0), 0.0};
  return (u);
}

iftVoxel TransformPoint(iftImage *src, iftVoxel u, iftMatrix *M)
{
  int diag = iftDiagonalSize(src);
  iftPoint P = {u.x - src->xsize / 2.0, u.y - src->ysize / 2.0, 0.0};
  iftPoint Q = iftTransformPoint(M, P);
  iftVoxel v = {iftRound(Q.x + diag / 2.0), iftRound(Q.y + diag / 2.0), 0.0};
  return (v);
}

iftDataSet *PatchesFromSeeds(iftImage *orig, iftImage **timgs, iftMatrix **M, int n, iftLabeledSet *seeds, iftDict *param)
{
  int patch_size_x = iftGetLongValFromDict("patch_size_x", param);
  int patch_size_y = iftGetLongValFromDict("patch_size_y", param);
  iftAdjRel *A = iftRectangular(patch_size_x, patch_size_y);
  int nfeats = A->n;
  int nseeds = iftLabeledSetSize(seeds);
  int nsamples = nseeds * (n + 1);

  iftDataSet *Z = iftCreateDataSet(nsamples, nfeats);
  Z->nclasses = nseeds;

  int s = 0, c = 0;
  iftLabeledSet *S = seeds;
  while (S != NULL)
  {
    int p1 = S->elem;
    Z->sample[s].id = p1;
    Z->sample[s].truelabel = c + 1;
    iftVoxel u1 = iftGetVoxelCoord(orig, p1);
    for (int i = 0; i < A->n; i++)
    {
      iftVoxel v1 = iftGetAdjacentVoxel(A, u1, i);
      if (iftValidVoxel(orig, v1))
      {
        int q1 = iftGetVoxelIndex(orig, v1);
        Z->sample[s].feat[i] = orig->val[q1] / 255.0;
      }
      else
      {
        Z->sample[s].feat[i] = 0;
      }
    }
    s++;
    for (int t = 0; t < n; t++)
    {
      Z->sample[s].id = p1;
      Z->sample[s].truelabel = c + 1;
      iftVoxel v1 = TransformPoint(orig, u1, M[t]);
      for (int i = 0; i < A->n; i++)
      {
        iftVoxel v2 = iftGetAdjacentVoxel(A, v1, i);
        if (iftValidVoxel(timgs[t], v2))
        {
          int q2 = iftGetVoxelIndex(timgs[t], v2);
          Z->sample[s].feat[i] = timgs[t]->val[q2] / 255.0;
        }
        else
        {
          Z->sample[s].feat[i] = 0;
        }
      }
      s++;
    }
    c++;
    S = S->next;
  }

  iftSetStatus(Z, IFT_TRAIN);
  iftAddStatus(Z, IFT_SUPERVISED);

  return (Z);
}

iftDataSet *PatchesFromMntSet(iftImage *img, iftMatrix **M, int n, iftLabeledSet *S, iftDict *param)
{
  int patch_size_x = iftGetLongValFromDict("patch_size_x", param);
  int patch_size_y = iftGetLongValFromDict("patch_size_y", param);
  iftAdjRel *A = iftRectangular(patch_size_x, patch_size_y);
  int nfeats = A->n;
  int nmnt = iftLabeledSetSize(S);
  int nsamples = nmnt * (n + 1);
  iftImage **timgs = GetTransformedImages(img, M, n);

  iftDataSet *Z = iftCreateDataSet(nsamples, nfeats);
  Z->nclasses = nmnt;

  int s = 0, c = 0;
  iftLabeledSet *mnt = S;
  while (mnt != NULL)
  {
    int p1 = mnt->elem;
    Z->sample[s].id = p1;
    Z->sample[s].truelabel = c + 1;
    iftVoxel u1 = iftGetVoxelCoord(img, p1);
    for (int i = 0; i < A->n; i++)
    {
      iftVoxel v1 = iftGetAdjacentVoxel(A, u1, i);
      if (iftValidVoxel(img, v1))
      {
        int q1 = iftGetVoxelIndex(img, v1);
        Z->sample[s].feat[i] = img->val[q1] / 255.0;
      }
      else
      {
        Z->sample[s].feat[i] = 0;
      }
    }
    s++;
    for (int t = 0; t < n; t++)
    {
      Z->sample[s].id = p1;
      Z->sample[s].truelabel = c + 1;
      iftVoxel v1 = TransformPoint(img, u1, M[t]);
      for (int i = 0; i < A->n; i++)
      {
        iftVoxel v2 = iftGetAdjacentVoxel(A, v1, i);
        if (iftValidVoxel(timgs[t], v2))
        {
          int q2 = iftGetVoxelIndex(timgs[t], v2);
          Z->sample[s].feat[i] = timgs[t]->val[q2] / 255.0;
        }
        else
        {
          Z->sample[s].feat[i] = 0;
        }
      }
      s++;
    }
    c++;
    S = S->next;
  }

  iftSetStatus(Z, IFT_TRAIN);
  iftAddStatus(Z, IFT_SUPERVISED);
  DestroyImageArray(&timgs, n);

  return (Z);
}

float *MeanValues(iftDataSet *Z)
{
  float *mean = iftAllocFloatArray(Z->nfeats);

  for (int s = 0; s < Z->nsamples; s++)
  {
    for (int f = 0; f < Z->nfeats; f++)
    {
      mean[f] += Z->sample[s].feat[f];
    }
  }

  for (int f = 0; f < Z->nfeats; f++)
  {
    mean[f] /= Z->nsamples;
  }

  return (mean);
}

float *StdevValues(iftDataSet *Z, float *mean, iftDict *param)
{
  float *stdev = iftAllocFloatArray(Z->nfeats);

  for (int s = 0; s < Z->nsamples; s++)
  {
    for (int f = 0; f < Z->nfeats; f++)
    {
      stdev[f] += (Z->sample[s].feat[f] - mean[f]) * (Z->sample[s].feat[f] - mean[f]);
    }
  }

  float stdev_factor = iftGetDblValFromDict("stdev_factor", param);

  for (int f = 0; f < Z->nfeats; f++)
  {
    stdev[f] = sqrtf(stdev[f] / Z->nsamples) + stdev_factor;
  }

  return (stdev);
}

void CompWriteModel(iftDataSet *Z, int n, iftDict *param,
                    char *dirname, char *basename)
{
  float *mean = MeanValues(Z);
  float *stdev = StdevValues(Z, mean, param);

  /* iftWriteDataSet(Z,"patches.zip"); */

  iftMatrix *kernels = iftCreateMatrix(Z->nsamples, Z->nfeats);

  for (int s = 0, col = 0; s < Z->nsamples; s++)
  {
    iftUnitNorm(Z->sample[s].feat, Z->nfeats);
    for (int row = 0; row < Z->nfeats; row++)
    {
      iftMatrixElem(kernels, col, row) = Z->sample[s].feat[row];
    }
    col++;
  }

  int *seedpt = iftAllocIntArray(kernels->ncols);
  float *bias = iftAllocFloatArray(kernels->ncols);

  for (int col = 0; col < kernels->ncols; col++)
  {
    seedpt[col] = Z->sample[col].id;
    for (int row = 0; row < kernels->nrows; row++)
    {
      iftMatrixElem(kernels, col, row) =
          iftMatrixElem(kernels, col, row) / stdev[row];
      bias[col] -= (mean[row] * iftMatrixElem(kernels, col, row));
    }
  }
  WriteModel(kernels, bias, seedpt, dirname, basename);

  iftFree(bias);
  iftDestroyMatrix(&kernels);
  iftFree(seedpt);

  iftFree(mean);
  iftFree(stdev);
}

void SavePointMatchImages(iftImage *skel, iftImage *src, iftImage **dst, iftImage **tskels, iftMatrix **M, int n, char *dirname)
{
  iftColor RGB1, YCbCr1;
  RGB1.val[0] = 0;
  RGB1.val[1] = 255;
  RGB1.val[2] = 255;
  YCbCr1 = iftRGBtoYCbCr(RGB1, 255);
  char filename[200];
  for (int t = 0; t < n; t++)
  {
    iftSetCbCr(dst[t], 128);
  }
  iftAdjRel *C = iftCircular(3.0);

  for (int p2 = 0; p2 < skel->n; p2++)
  {
    if (skel->val[p2] == 128)
    {
      iftVoxel u2 = iftGetVoxelCoord(skel, p2);
      iftVoxel u1 = {iftRound(u2.x / 2.0), iftRound(u2.y / 2.0), 0};
      for (int t = 0; t < n; t++)
      {
        iftVoxel v1 = TransformPoint(src, u1, M[t]);
        if (iftValidVoxel(dst[t], v1))
        {
          iftDrawPoint(dst[t], v1, YCbCr1, C, 255);
        }
      }
    }
  }

  RGB1.val[0] = 255;
  RGB1.val[1] = 255;
  RGB1.val[2] = 0;
  YCbCr1 = iftRGBtoYCbCr(RGB1, 255);
  iftMakeDir(dirname);
  iftAdjRel *A = iftCircular(0.0);

  for (int t = 0; t < n; t++)
  {
    sprintf(filename, "%s/img_%03d.png", dirname, t);
    for (int p = 0; p < tskels[t]->n; p++)
    {
      if (tskels[t]->val[p] == 128)
      {
        iftVoxel u = iftGetVoxelCoord(tskels[t], p);
        u.x = iftRound(u.x / 2.0);
        u.y = iftRound(u.y / 2.0);
        if (iftValidVoxel(dst[t], u))
        {
          iftColor YCbCr2;
          for (int i = 0; i < C->n; i++)
          {
            iftVoxel v = iftGetAdjacentVoxel(C, u, i);
            if (iftValidVoxel(dst[t], v))
            {
              int q = iftGetVoxelIndex(dst[t], v);
              if ((dst[t]->Cb[q] != 128) || (dst[t]->Cr[q] != 128))
              {
                YCbCr2.val[1] = (dst[t]->Cb[q] + YCbCr1.val[1]) / 2;
                YCbCr2.val[2] = (dst[t]->Cr[q] + YCbCr1.val[2]) / 2;
              }
              else
              {
                YCbCr2 = YCbCr1;
              }
              iftDrawPoint(dst[t], v, YCbCr2, A, 255);
            }
          }
        }
      }
    }
    iftWriteImageByExt(dst[t], filename);
  }

  iftDestroyAdjRel(&A);
  iftDestroyAdjRel(&C);
}

iftLabeledSet *GetStableSkelPoints(iftImage *skel, iftImage *orig, iftImage *mask,
                                   iftImage **tskels, iftImage **timgs,
                                   iftMatrix **M, int n, iftDict *param)
{
  iftLabeledSet *seeds = NULL;
  bool stable_pt;
  iftImage *emask = ErodedInterpolatedMask(mask, 2.0, 2.0, param);
  float homologous_radius = iftGetDblValFromDict("homologous_radius", param);
  iftAdjRel *A = iftCircular(homologous_radius);

  for (int p2 = 0; p2 < skel->n; p2++)
  {
    if ((skel->val[p2] == 128) && (emask->val[p2] != 0))
    {
      stable_pt = true;
      iftVoxel u2 = iftGetVoxelCoord(skel, p2);
      iftVoxel u1 = {iftRound(u2.x / 2.0), iftRound(u2.y / 2.0), 0};
      for (int t = 0; t < n; t++)
      {
        iftVoxel v1 = TransformPoint(orig, u1, M[t]);
        if (iftValidVoxel(timgs[t], v1))
        {
          iftVoxel v2 = {iftRound(v1.x * 2.0), iftRound(v1.y * 2.0), 0};
          int qmin = IFT_NIL; /* search the closest skeleton point
               since A is sorted by distance to
               the central point */
          if (iftValidVoxel(tskels[t], v2))
          {
            for (int i = 0; i < A->n; i++)
            {
              iftVoxel v3 = iftGetAdjacentVoxel(A, v2, i);
              if (iftValidVoxel(tskels[t], v3))
              {
                int q3 = iftGetVoxelIndex(tskels[t], v3);
                if (tskels[t]->val[q3] == 128)
                {
                  qmin = q3;
                  break;
                }
              }
            }
          }
          if (qmin == IFT_NIL)
          {
            stable_pt = false;
            break;
          }
        }
      }
      if (stable_pt)
      {
        int p1 = iftGetVoxelIndex(orig, u1);
        iftUnionLabeledSetElem(&seeds, p1, 1);
      }
    }
  }

  iftDestroyAdjRel(&A);
  iftDestroyImage(&emask);

  return (seeds);
}

iftSet *GetSkelPoints(iftImage *skel, iftImage *orig, iftImage* skel_mask)
{
  iftSet *seeds = NULL;
  // iftImage *emask = ErodedInterpolatedMask(mask,2.0,2.0,param);

  float scale_factor = skel->xsize / orig->xsize;

  iftAdjRel *A = iftCircular(MIN_BORDER_DISTANCE*scale_factor);

  for (int p2 = 0; p2 < skel->n; p2++)
  {
    int cn = crossingNumber(skel, p2);
    if ((cn == 1) || (cn >= 3))
    {
      bool is_border = false;
      iftVoxel u2 = iftGetVoxelCoord(skel, p2);

      // iftVoxel u1 = {iftRound(u2.x / scale_factor), iftRound(u2.y / scale_factor), 0};

      // ignore minutiae close to mask border

      for (int i = 0; i < A->n; i++) {
        iftVoxel v = iftGetAdjacentVoxel(A, u2, i);
        if (iftValidVoxel(skel_mask, v)) {
          int p3 = iftGetVoxelIndex(skel, v);

          if (skel_mask != NULL) {
            if (skel_mask->val[p3] == 0) {
              is_border = true;
              break;
            }
          }
        } else {
          is_border = true;
          break;
        }


        if (is_border) {
          // skel->val[p2] = 255;
          continue;
        }
      }

      if (!is_border)
        iftUnionSetElem(&seeds, p2);
    }
  }

  iftDestroyAdjRel(&A);

  return (seeds);
}

iftMatrix *ImageToFeatureMatrix(iftImage *img, iftDict *param)
{
  int patch_size_x, patch_size_y;

  patch_size_x = iftGetLongValFromDict("patch_size_x", param);
  patch_size_y = iftGetLongValFromDict("patch_size_y", param);
  iftAdjRel *A = iftRectangular(patch_size_x, patch_size_y);

  iftMatrix *matrix = iftCreateMatrix(A->n, img->n);

#pragma omp parallel for
  for (int p = 0; p < img->n; p++)
  {
    iftVoxel u = iftGetVoxelCoord(img, p);
    for (int i = 0; i < A->n; i++)
    {
      iftVoxel v = iftGetAdjacentVoxel(A, u, i);
      if (iftValidVoxel(img, v))
      {
        int q = iftGetVoxelIndex(img, v);
        iftMatrixElem(matrix, i, p) = img->val[q] / 255.0;
      }
    }
  }

  iftDestroyAdjRel(&A);

  return matrix;
}
