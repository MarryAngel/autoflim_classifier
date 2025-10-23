#include "ift.h"
#include "iftFPModel.h"

#define STRIDE 1
#define PATCH_SIZE 17



float localOrientationRidge(iftImage *img, iftImage *gradX, iftImage *gradY, iftVoxel u, iftAdjRel *A)
{
  float Gxy = 0;
  float Gxx = 0;
  float Gyy = 0;
  for (int i = 0; i < A->n; i++)
  {
    iftVoxel v = iftGetAdjacentVoxel(A, u, i);
    if (!iftValidVoxel(img, v))
    {
      return 0;
    }
    int p = iftGetVoxelIndex(img, v);
    Gxx += gradX->val[p] * gradX->val[p];
    Gyy += gradY->val[p] * gradY->val[p];
    Gxy += gradX->val[p] * gradY->val[p];
  }

  float theta = IFT_PI / 2 + atan2(2 * Gxy, Gxx - Gyy) / 2;

  return theta;
}

iftFImage *orientationField(iftImage *image)
{
  int w = image->xsize;
  int h = image->ysize;

  iftFImage *result = iftCreateFImage(image->xsize, image->ysize, 1);
  iftAdjRel *A = iftRectangular(PATCH_SIZE, PATCH_SIZE);

  iftKernel *K_sobel_x = iftSobelXKernel();
  iftKernel *K_sobel_y = iftSobelYKernel();
  iftImage *gradX = iftLinearFilter(image, K_sobel_x);
  iftImage *gradY = iftLinearFilter(image, K_sobel_y);

  for (int x = 0; x < w; x += STRIDE)
  {
    for (int y = 0; y < h; y += STRIDE)
    {
      iftVoxel u = {x, y, 0, 0};
      int p = iftGetVoxelIndex(image, u);
      result->val[p] = localOrientationRidge(image, gradX, gradY, u, A);
    }
  }
  return result;
}

int main(int argc, char *argv[])
{
  size_t mem_start, mem_end;
  timer *t1, *t2;

  mem_start = iftMemoryUsed();

  if (argc != 5)
  {
    printf("Usage: iftDetectMntsBySkel <P1> <P2> <P3>\n");
    printf("P1: input folder with the fingerprint images\n");
    printf("P2: input FPModel parameters\n");
    printf("P3: input folder with gabor images\n");
    printf("P4: output folder with the seeds\n");
    exit(-1);
  }

  t1 = iftTic();

  /* Read input parameters */
  iftFileSet *fs = iftLoadFileSetFromDirBySuffix(argv[1], ".png", 1);
  iftDict *param = iftReadJson(argv[2]);
  char *out_dir = argv[4];
  iftMakeDir(out_dir);
  char filename[200];

  sprintf(filename, "%s/skel",out_dir);
  iftMakeDir(filename);

  sprintf(filename, "%s/seeds",out_dir);
  iftMakeDir(filename);


  /* Detect skeleton minutia points */

  for (int i = 0; i < fs->n; i++)
  {
    printf("Progress: %d/%ld\n", i+1, fs->n);

    /* read test image and compute its skeleton with value 128 on the
       minucia points */

    iftImage *img = iftReadImageByExt(fs->files[i]->path);
    char *basename = iftFilename(fs->files[i]->path, ".png");

    sprintf(filename, "%s/enh/%s.png", argv[3], basename);
    // iftImage *gabor_img = Gabor(img);
    iftImage *gabor_img = GaborFromDisk(img, filename);
    iftImage *gabor_bin =
            iftThreshold(gabor_img, iftGetLongValFromDict("gabor_threshold", param),
                         255, 255);
    iftImage *mask = GetMask(gabor_bin, param);
    Masking(gabor_img, mask);
    Masking(gabor_bin, mask);
    FilterGaborBin(&gabor_bin, param);
    iftImage *skel = Skeleton(gabor_bin);

    // sprintf(filename, "%s/%s_gabor_bin.png", out_dir, basename);
    // iftWriteImageByExt(gabor_bin, filename);

    sprintf(filename, "%s/skel/%s.png", out_dir, basename);
    iftWriteImageByExt(skel, filename);

    iftLabeledSet *S = GetSkelPoints(skel, gabor_img, mask, param);
    sprintf(filename, "%s/seeds/%s-seeds.txt", out_dir, basename);
    iftWriteSeeds(S, gabor_img, filename);

    iftDestroyLabeledSet(&S);
    iftDestroyImage(&img);
    iftDestroyImage(&gabor_img);
    iftDestroyImage(&gabor_bin);
    iftDestroyImage(&mask);
    iftDestroyImage(&skel);
    iftFree(basename);
  }

  iftDestroyFileSet(&fs);

  mem_end = iftMemoryUsed();
  iftVerifyMemory(mem_start, mem_end);

  t2 = iftToc();
  puts(iftFormattedTime(iftCompTime(t1, t2)));

  return (0);
}
