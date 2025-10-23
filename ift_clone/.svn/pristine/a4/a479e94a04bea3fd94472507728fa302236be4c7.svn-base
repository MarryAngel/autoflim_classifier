#include "ift.h"
#include <string.h>  // For strcmp

#define MNT_ANGLE_RADIUS 10.0
#define DRAW_ORIENTATION 1

int main(int argc, char *argv[])
{
  timer *tstart = NULL;
  char filename[200];
  iftAdjRel *C = iftCircular(3.0);
  iftAdjRel *draw_radius = iftCircular(3.0);
  
  // Add the neuroMntFormat variable
  int neuroMntFormat = 0;

  /*--------------------------------------------------------*/

  void *trash = malloc(1);
  struct mallinfo info;
  int MemDinInicial, MemDinFinal;
  free(trash);
  info = mallinfo();
  MemDinInicial = info.uordblks;

  /*--------------------------------------------------------*/

  // Parse the command line arguments for the --neuro flag
  for (int i = 1; i < argc; i++)
  {
    if (strcmp(argv[i], "--neuro") == 0)
    {
      neuroMntFormat = 1;  // Set neuroMntFormat to true
      break;  // No need to check further if we've found the flag
    }
  }

  if (argc != 5 && !neuroMntFormat)
  {
    printf("iftDrawMinuciae <P1> <P2> <P3> <P4>\n");
    printf("P1: folder with original images\n");
    printf("P2: folder with minuciae points\n");
    printf("P3: folder with the output images\n");
    printf("P4: format of the points (0: .mnt, 1: -seeds.txt)\n");
    exit(0);
  }

  tstart = iftTic();

  iftFileSet *fs = iftLoadFileSetFromDirBySuffix(argv[1], ".png", 1);
  int nimages = fs->n;
  char *out_dir = argv[3];
  char pt_format = atoi(argv[4]);

  iftMakeDir(out_dir);
  iftColor RGB1, YCbCr1;
  RGB1.val[0] = 255;
  RGB1.val[1] = 0;
  RGB1.val[2] = 0;
  YCbCr1 = iftRGBtoYCbCr(RGB1, 255);

  for (int i = 0; i < nimages; i++)
  {
    char *basename = iftFilename(fs->files[i]->path, ".png");
    iftImage *orig = iftReadImageByExt(fs->files[i]->path);
    iftSetCbCr(orig, 128);

    if (pt_format == 1)
    {
      sprintf(filename, "%s/%s-seeds.txt", argv[2], basename);
      iftLabeledSet *S = iftReadSeeds(orig, filename);
      while (S != NULL)
      {
        int label;
        int p = iftRemoveLabeledSet(&S, &label);
        iftVoxel u = iftGetVoxelCoord(orig, p);
        iftDrawPoint(orig, u, YCbCr1, draw_radius, 255);
      }
    }
    else
    {
      sprintf(filename, "%s/%s.mnt", argv[2], basename);
      FILE *fp = fopen(filename, "r");

      if (fp == NULL) {
        printf("Error: File %s does not exist or cannot be opened.\n", filename);
        continue; // Skip this iteration and move to the next image
    }

      char name_of_file[100];
      int npts, xsize, ysize;
      float ori, score;
      score = 1;
      iftVoxel u;
      u.z = 0;

      if (!neuroMntFormat) {
        fscanf(fp, "%s", name_of_file);
      }
      
      fscanf(fp, "%d %d %d", &npts, &xsize, &ysize);

      printf("npts: %d, xsize, ysize: %d %d\n", npts, xsize, ysize);

      for (int pt = 0; pt < npts; pt++)
      {
        if (!neuroMntFormat) {
          fscanf(fp, "%d %d %f %f", &u.x, &u.y, &ori, &score);
        } else {
          fscanf(fp, "%d %d %f", &u.x, &u.y, &ori);
          ori = IFT_PI/180 * (2 * ori * 360 + 256) / (2 * 256);
        }
        iftDrawPoint(orig, u, YCbCr1, C, 255);

        if (!DRAW_ORIENTATION) continue;

        // Drawing orientation
        float v_x = u.x + MNT_ANGLE_RADIUS * cos(ori);
        float v_y = u.y + MNT_ANGLE_RADIUS * sin(ori);

        float dx = u.x - v_x;
        float dy = u.y - v_y;

        float step = abs(dx) >= abs(dy) ? abs(dx) : abs(dy);

        dx = step == 0 ? dx : dx / step;
        dy = step == 0 ? dy : dy / step;

        int k = 1;

        float x = v_x, y = v_y;
        iftAdjRel *A = iftCircular(1.0);

        while (k <= step)
        {
          u.x = x;
          u.y = y;

          if (iftValidVoxel(orig, u))
          {
            iftDrawPoint(orig, u, YCbCr1, A, 255);
          }
          x += dx;
          y += dy;
          k++;
        }

        iftDestroyAdjRel(&A);
      }
    }

    sprintf(filename, "%s/%s.png", out_dir, basename);
    iftWriteImageByExt(orig, filename);
    iftDestroyImage(&orig);
    iftFree(basename);
  }

  iftDestroyFileSet(&fs);
  iftDestroyAdjRel(&C);
  iftDestroyAdjRel(&draw_radius);

  puts("\nDone...");
  puts(iftFormattedTime(iftCompTime(tstart, iftToc())));

  /* ---------------------------------------------------------- */

  info = mallinfo();
  MemDinFinal = info.uordblks;
  if (MemDinInicial != MemDinFinal)
    printf("\n\nDinamic memory was not completely deallocated (%d, %d)\n",
           MemDinInicial, MemDinFinal);

  return 0;
}
