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
  
  if (argc != 3) {
    printf("iftGabor <P1> <P2>\n");
    printf("P1: input folder with the original FP images\n");
    printf("P2: output folder with Gabor-filtered images\n");
    exit(0);
  }

  tstart = iftTic();

  /* Read input parameters */
  
  char *orig_dir    = argv[1];
  iftFileSet *fs    = iftLoadFileSetFromDirBySuffix(orig_dir,".png", 1);  
  int nimages       = fs->n;
  char *gabor_dir   = argv[2];
  iftMakeDir(gabor_dir);
  char filename[200];
  
  /* Apply Gabor filtering and save result */
  
  for (int i=0; i < nimages; i++) {
    char *basename        = iftFilename(fs->files[i]->path,".png");
    iftImage *orig        = iftReadImageByExt(fs->files[i]->path);

    iftImage *gabor_img   = Gabor(orig);
    sprintf(filename,"%s/%s.png",gabor_dir,basename);
    iftWriteImageByExt(gabor_img,filename);
    iftDestroyImage(&orig);
    iftDestroyImage(&gabor_img);
    iftFree(basename);
  }

  iftDestroyFileSet(&fs);

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








