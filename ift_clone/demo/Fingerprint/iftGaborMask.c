#include "ift.h"
#include "iftFPModel.h"
#include <omp.h>

int main(int argc, char *argv[])
{
  timer *tstart=NULL;
  
  /*--------------------------------------------------------*/

  void *trash = malloc(1);                 
  struct mallinfo info;   
  int MemDinInicial, MemDinFinal;
  free(trash); 
  info = mallinfo();
  MemDinInicial = info.uordblks;

  /*--------------------------------------------------------*/
  
  if (argc != 5) {
    printf("iftLearnFPModel <P1> <P2> <P3>\n");
    printf("P1: input folder with Gabor-filtered images\n");
    printf("P2: input FPModel parameters \n");
    printf("P3: output folder with the FP masks\n");
    printf("P4: output folder with the bin imgs\n");
    exit(0);
  }

  tstart = iftTic();

  /* Read input parameters */
  
  char *gabor_dir   = argv[1];
  iftFileSet *fs    = iftLoadFileSetFromDirBySuffix(gabor_dir,".png", 1);  
  int nimages       = fs->n;
  iftDict *param    = iftReadJson(argv[2]); 
  char *mask_dir    = argv[3];
  char *bin_dir    = argv[4];
  iftMakeDir(mask_dir);
  
  /* Binarize gabor images and compute gabor masks */
  int count = 0;
  
  #pragma omp parallel for
  for (int i=0; i < nimages; i++) {
    char filename[200];
    // printf("Processing image %d of %ld\n", i+1, fs->n); 
    char *basename        = iftFilename(fs->files[i]->path,".png");
    iftImage *gabor_img   = iftReadImageByExt(fs->files[i]->path);
    iftImage *gabor_bin   =
      iftThreshold(gabor_img,0,
      127,255);
    iftImage *mask        = GetMask(gabor_bin, param);
    sprintf(filename,"%s/%s.png",mask_dir,basename);
    iftWriteImageByExt(mask,filename);

    sprintf(filename,"%s/%s.png",bin_dir,basename);
    iftWriteImageByExt(gabor_bin,filename);

    iftDestroyImage(&gabor_img);
    iftDestroyImage(&gabor_bin);
    iftDestroyImage(&mask);
    count += 1;

    #pragma omp critical
    printf("Finished processing image %d/%ld : %s\n", count, fs->n, basename); 

    iftFree(basename);
  }

  iftDestroyFileSet(&fs);
  iftDestroyDict(&param);

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








