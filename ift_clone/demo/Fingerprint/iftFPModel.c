#include "ift.h"
#include "iftFPModel.h"

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
  
  if (argc != 6) {
    printf("iftFPModel <P1> <P2> <P3> <P4> <P5>\n");
    printf("P1: input folder with original images\n");
    printf("P2: input folder with filtered-gabor images\n");
    printf("P3: input folder with minuciae points\n");
    printf("P4: input FPModel parameters \n");
    printf("P5: output folder with FP models\n");
    exit(0);
  }

  tstart = iftTic();

  /* Read input parameters */
  
  char *orig_dir    = argv[1];
  iftFileSet *fs    = iftLoadFileSetFromDirBySuffix(orig_dir,".png", 1);  
  int nimages       = fs->n;
  char *gabor_dir   = argv[2];
  char *mnt_dir     = argv[3];
  iftDict *param    = iftReadJson(argv[4]); 
  int num_of_transformations = (int)
      iftGetLongValFromDict("num_of_transformations",param);
  char *model_dir   = argv[5];
  iftMakeDir(model_dir);
  char filename[200];
  
  /* Model learning for each image */
  
  for (int i=0; i < nimages; i++) {
    char *basename        = iftFilename(fs->files[i]->path,".png");
    iftImage *orig        = iftReadImageByExt(fs->files[i]->path);
    sprintf(filename,"%s/%s.png",gabor_dir,basename);    
    iftImage *gabor_img   = iftReadImageByExt(filename);
    iftMatrix **M         = GetRandomTransformations(gabor_img, param);
    sprintf(filename,"%s/%s-seeds.txt",mnt_dir,basename);    
    iftLabeledSet *S      = iftReadSeeds(gabor_img,filename);    
    iftDataSet *Z         = PatchesFromMntSet(gabor_img, M, num_of_transformations, S, param);
    CompWriteModel(Z, num_of_transformations, param, model_dir, basename);
    
    iftDestroyLabeledSet(&S);
    iftDestroyImage(&orig);
    iftDestroyImage(&gabor_img);    
    iftDestroyDataSet(&Z);
    iftFree(basename);
    for (int t=0; t < num_of_transformations; t++){
       iftDestroyMatrix(&M[t]);
    }
    iftFree(M);
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








