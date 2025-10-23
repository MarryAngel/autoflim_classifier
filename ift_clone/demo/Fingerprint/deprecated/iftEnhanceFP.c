#include "ift.h"

#define cropsize 450

/* It enhances and crops the region of interest */

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
    printf("iftEnhanceFP <P1> <P2> <P3>\n");
    printf("P1: folder with original images\n");
    printf("P2: folder with enhanced images\n");
    printf("P3: 0 or 1 to either not crop or crop fingerprint\n");
    exit(0);
  }

  tstart = iftTic();

  iftFileSet *fs   = iftLoadFileSetFromDirBySuffix(argv[1],".png", 1);  
  int nimages      = fs->n;
  char *out_dir    = argv[2];
  iftMakeDir(out_dir);
  iftAdjRel *A   = iftCircular(3.5);
  
  for (int i=0; i < nimages; i++) {
    char *basename = iftFilename(fs->files[i]->path,".png");
    iftImage *orig = iftReadImageByExt(fs->files[i]->path);
    iftImage *norm = iftNormalize(orig,0,255);

    if (atoi(argv[3])!=0){
      iftImage *fgp  = iftBelowAdaptiveThreshold(norm, NULL, A, 0.98, 2, 255);
      iftImage *aux  = iftAsfCOBin(fgp,15.0);      
      iftDestroyImage(&fgp);
      fgp            = iftSelectLargestComp(aux,NULL);            
      iftVoxel  pos;
      iftBoundingBox bb = iftMinBoundingBox(fgp, &pos);
      pos.x      = (bb.begin.x+bb.end.x)/2; 
      pos.y      = (bb.begin.y+bb.end.y)/2;
      int sz     = iftMax(iftMax(iftMax(cropsize/2-pos.x,cropsize/2-pos.y),
				 ((pos.x + cropsize/2)-(fgp->xsize-1))),
			  ((pos.y + cropsize/2)-(fgp->ysize-1)));
      if (sz > 0){
	iftImage *aux = iftAddFrame(norm,sz,255);
	iftDestroyImage(&norm);
	norm = aux;
	pos.x += sz;
	pos.y += sz;
      }
	
      bb.begin.x = pos.x - cropsize/2;
      bb.begin.y = pos.y - cropsize/2;
      bb.end.x   = pos.x + cropsize/2 - 1;
      bb.end.y   = pos.y + cropsize/2 - 1;
      iftDestroyImage(&fgp);
      fgp               = iftExtractROI(norm,bb);
      
      sprintf(filename,"%s/%s.png",out_dir,basename);
      iftWriteImageByExt(fgp,filename);
      iftDestroyImage(&fgp);
    } else {
      sprintf(filename,"%s/%s.png",out_dir,basename);
      iftWriteImageByExt(norm,filename);
    }
    iftDestroyImage(&orig);
    iftDestroyImage(&norm);
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








