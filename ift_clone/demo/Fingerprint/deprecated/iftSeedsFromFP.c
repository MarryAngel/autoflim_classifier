#include "ift.h"

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

  
  if (argc != 3) {
    iftError("iftSeedsFromFP <folder with binary fingerprints> <folder with seeds>", "main");
  }

  tstart = iftTic();

  iftFileSet *fs_input     = iftLoadFileSetFromDirBySuffix(argv[1],".png", 1);
  int nimages              = fs_input->n;     
  char *outputdir          = argv[2];
  iftMakeDir(outputdir);

  for (int i=0; i < nimages; i++) {
    char *basename   = iftFilename(fs_input->files[i]->path,".png");
    iftImage *bin    = iftReadImageByExt(fs_input->files[i]->path);
    iftLabeledSet *S = iftLabeledSetFromSeedImage(bin,false);
    sprintf(filename,"%s/%s-seeds.txt",outputdir,basename);
    iftWriteSeeds(S, bin, filename);

    iftDestroyImage(&bin);
    iftDestroyLabeledSet(&S);
    iftFree(basename);
  }
  
  iftDestroyFileSet(&fs_input);

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








