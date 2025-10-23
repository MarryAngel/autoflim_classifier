#include "ift.h"
#include <math.h>

#define PATCH_SIZE 127



int main(int argc, char *argv[])
{
  timer *tstart=NULL;
  char   filename[200];
  
  /*--------------------------------------------------------*/

  void *trash = malloc(1);                 
  struct mallinfo info;   
  int MemDinInicial, MemDinFinal;
  free(trash); 
  info = mallinfo();
  MemDinInicial = info.uordblks;

  /*--------------------------------------------------------*/

  
  if (argc != 4) {
    printf("usage iftExtractAlignedPatches <P1> <P2> <P3>\n");
    printf("P1: folder with fingerprint images (.png)\n");
    printf("P2: folder with minuciae files (.mnt)\n");
    printf("P3: output folder with aligned patches (.mimg)\n");
    exit(0);
  }

  tstart = iftTic();

  iftFileSet *fs    = iftLoadFileSetFromDirBySuffix(argv[1],".png", 1);

  int nimages      = fs->n;
  char *out_dir    = argv[3];
  
  iftMakeDir(out_dir);

  int dilated_patch_size = (int)ceil(PATCH_SIZE + PATCH_SIZE * sqrt(2)/2);
  iftAdjRel *A = iftRectangularWithDilationForConv(PATCH_SIZE, PATCH_SIZE, 1, 1);
  iftAdjRel *C = iftRectangularWithDilationForConv(dilated_patch_size, dilated_patch_size, 1, 1);

  for (int i=0; i < nimages; i++) {
    printf("Progress: %d/%d\n", i + 1, nimages);
    char *basename     = iftFilename(fs->files[i]->path,".png");
    iftImage *orig     = iftReadImageByExt(fs->files[i]->path);
    
    sprintf(filename, "%s/%s.mnt", argv[2], basename);
    FILE *fp = fopen(filename, "r");

    char name_of_file[100];
    int npts, xsize, ysize;
    
    fscanf(fp,"%s",name_of_file);
    fscanf(fp,"%d %d %d",&npts,&xsize,&ysize);
    if ((xsize != orig->xsize)||(ysize!=orig->ysize)) iftError("incompatible minuciae file","main");

    // Iterating over each minuciae and extracting patches
    
    iftMImage *patches = iftCreateMImage(PATCH_SIZE, PATCH_SIZE, 1, npts);
    iftVoxel u; u.z = 0;
    float ori, score;

    for (int pt = 0; pt < npts; pt++) {
        iftImage *patch = iftCreateImage(dilated_patch_size, dilated_patch_size, 1);
        iftImage *rotated_patch;
        fscanf(fp,"%d %d %f %f",&u.x,&u.y,&ori,&score);
        for (int i = 0; i < C->n; i++){
          iftVoxel v = iftGetAdjacentVoxel(C, u, i);
          if (iftValidVoxel(orig, v)){
            int q      = iftGetVoxelIndex(orig, v);
            patch->val[i] = orig->val[q];
          }
        }

        rotated_patch = iftRotateImage2D(patch, ori * 180.0 / IFT_PI + 90);

        iftVoxel w;
        w.x = 0.5 * rotated_patch->xsize;
        w.y = 0.5 * rotated_patch->ysize;
        w.z = 0;
        for (int i = 0; i < A->n; i++){
          iftVoxel v = iftGetAdjacentVoxel(A, w, i);
          if (iftValidVoxel(rotated_patch, v)){
            int q      = iftGetVoxelIndex(rotated_patch, v);
            patches->val[i][pt] = rotated_patch->val[q];
          }
        }

    iftDestroyImage(&patch);
    iftDestroyImage(&rotated_patch);

    }

    sprintf(filename, "%s/%s.mimg", out_dir, basename);
    iftWriteMImage(patches, filename);
    iftDestroyImage(&orig);

    iftDestroyMImage(&patches);
    fclose(fp);
  }

  iftDestroyFileSet(&fs);
  iftDestroyAdjRel(&A);
  iftDestroyAdjRel(&C);


  
  puts("\nDone...");
  puts(iftFormattedTime(iftCompTime(tstart, iftToc())));
  
  /* ---------------------------------------------------------- */

  info = mallinfo();
  MemDinFinal = info.uordblks;
  if (MemDinInicial!=MemDinFinal)
    printf("\n\nDinamic memory was not completely deallocated (%d, %d)\n",
	   MemDinInicial,MemDinFinal);   

  return 0;
}








