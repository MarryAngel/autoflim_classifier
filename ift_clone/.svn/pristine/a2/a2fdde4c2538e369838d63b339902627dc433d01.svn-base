#include "ift.h"
#include "iftFPModel.h"


int main(int argc, char *argv[]) 
{
  size_t           mem_start, mem_end;
  timer           *t1, *t2;

  mem_start = iftMemoryUsed();

  if (argc != 5){
    printf("Usage: iftGaborBin <P1> <P2> <P3> <P4>\n");
    printf("P1: input folder with Gabor-filtered images\n");
    printf("P2: input folder with Gabor masks \n");
    printf("P3: input FPModel parameters\n");
    printf("P4: output folder with binary-filtered-Gabor images\n");
    exit(-1);
  }

  t1 = iftTic();

  /* Read input parameters */

  iftFileSet *fs       = iftLoadFileSetFromDirBySuffix(argv[1], ".png", 1);
  char *mask_dir       = argv[2];
  iftDict *param       = iftReadJson(argv[3]);
  char    *gabor_bin_dir = argv[4];
  iftMakeDir(gabor_bin_dir);
  char filename[200];

  /* Compute skeletons */
  
  for (int i=0; i < fs->n; i++) {    
    char *basename      = iftFilename(fs->files[i]->path, ".png");
    iftImage *gabor_img = iftReadImageByExt(fs->files[i]->path);
    iftImage *gabor_bin =
      iftThreshold(gabor_img,iftGetLongValFromDict("gabor_threshold",param),
		   255,255);
    // iftAdjRel *A = iftCircular(8.0);
    // iftImage *gabor_bin = iftBelowAdaptiveThreshold(gabor_img, NULL, A, 0.9, 2, 255);
    sprintf(filename,"%s/%s.png",mask_dir,basename);
    iftImage *mask      = iftReadImageByExt(filename);
    Masking(gabor_img,mask);
    Masking(gabor_bin,mask);
    // FilterGaborBin(&gabor_bin, param);
    sprintf(filename,"%s/%s.png",gabor_bin_dir,basename);
    iftWriteImageByExt(gabor_bin,filename);
    iftDestroyImage(&gabor_img);
    iftDestroyImage(&gabor_bin);
    iftDestroyImage(&mask);
    iftFree(basename);
  }

  iftDestroyFileSet(&fs);
  iftDestroyDict(&param);
  
  mem_end = iftMemoryUsed();
  iftVerifyMemory(mem_start,mem_end);
    
  t2 = iftToc();
  puts(iftFormattedTime(iftCompTime(t1,t2)));

  return(0);
}
