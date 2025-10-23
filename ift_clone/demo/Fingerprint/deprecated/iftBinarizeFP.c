#include "ift.h"

/* It binarizes the enhanced fingerprint image and closes the pores */

int main(int argc, char *argv[])
{
  timer *tstart=NULL;
  char   filename[200];
  iftAdjRel *A = iftCircular(5.0);
  
  /*--------------------------------------------------------*/

  void *trash = malloc(1);                 
  struct mallinfo info;   
  int MemDinInicial, MemDinFinal;
  free(trash); 
  info = mallinfo();
  MemDinInicial = info.uordblks;

  /*--------------------------------------------------------*/

  
  if (argc != 3) {
    printf("iftBinarizeFP <P1> <P2>\n");
    printf("P1: folder with enhanced images\n");
    printf("P2: folder with binary images\n");
    exit(0);
  }

  tstart = iftTic();

  iftFileSet *fs   = iftLoadFileSetFromDirBySuffix(argv[1],".png", 1);  
  int nimages      = fs->n;
  char *out_dir    = argv[2];
  iftMakeDir(out_dir);
  
  for (int i=0; i < nimages; i++) {
    char *basename  = iftFilename(fs->files[i]->path,".png");
    iftImage *enha  = iftReadImageByExt(fs->files[i]->path);
    iftImage *fgp   = iftBelowAdaptiveThreshold(enha, NULL, A, 0.99, 1, 255);
    /* iftImage *bin   = iftFastAreaOpen(fgp,30); */
    /* iftDestroyImage(&fgp); */
    /* fgp             = iftFastAreaClose(bin,30); */
    /* iftDestroyImage(&bin); */

    sprintf(filename,"%s/%s.png",out_dir,basename);
    iftWriteImageByExt(fgp,filename);
    
    iftDestroyImage(&enha);
    iftDestroyImage(&fgp);
    iftFree(basename);
  }

  iftDestroyFileSet(&fs);
  iftDestroyAdjRel(&A);
  
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








