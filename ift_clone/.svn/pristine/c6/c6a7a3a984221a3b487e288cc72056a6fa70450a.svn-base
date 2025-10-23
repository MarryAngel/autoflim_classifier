#include "ift.h"
#include "iftFPModel.h"
#include <omp.h>


int main(int argc, char *argv[]) 
{
  size_t           mem_start, mem_end;
  timer           *t1, *t2;

  mem_start = iftMemoryUsed();

  if (argc != 3){
    printf("Usage: iftSkeleton <P1> <P2>\n");
    printf("P1: input folder with binary-filtered-Gabor images\n");
    printf("P2: output folder with skeletons\n");
    exit(-1);
  }
  t1 = iftTic();

  /* Read input parameters */

  iftFileSet *fs       = iftLoadFileSetFromDirBySuffix(argv[1], ".png", 1);
  char    *skel_dir    = argv[2];
  iftMakeDir(skel_dir);

  int total = fs->n;
  int progress = 0;

  /* Compute skeletons */
  #pragma omp parallel for schedule(static)
  for (int i=0; i < fs->n; i++) { 
    char filename[200];
    char *basename      = iftFilename(fs->files[i]->path, ".png");
    iftImage *gabor_bin = iftReadImageByExt(fs->files[i]->path);
    iftImage *skel      = Skeleton(gabor_bin);
    // printf("Filtering spikes and finding minutiae\n");
    fixSkeletonAndFindMnts(skel, gabor_bin);
    sprintf(filename,"%s/%s.png",skel_dir,basename);
    iftWriteImageByExt(skel,filename);
    iftDestroyImage(&gabor_bin);
    iftDestroyImage(&skel);
    iftFree(basename);

    /* Progress tracking */
    #pragma omp atomic
    progress++;

    if (omp_get_thread_num() == 0 || progress % 10 == 0) { // reduce IO contention
      printf("\rProgress: %d/%d (%.1f%%)", progress, total, 100.0 * progress / total);
      fflush(stdout);
    } 
  }

  iftDestroyFileSet(&fs);
  
  mem_end = iftMemoryUsed();
  iftVerifyMemory(mem_start,mem_end);
    
  t2 = iftToc();
  puts(iftFormattedTime(iftCompTime(t1,t2)));

  return(0);
}
