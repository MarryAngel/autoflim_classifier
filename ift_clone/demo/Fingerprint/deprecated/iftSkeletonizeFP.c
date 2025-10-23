#include "ift.h"

void EliminateShortSegments(iftImage *skel, int thres)
{
  iftAdjRel *A   = iftCircular(1.5);
  iftImage  *len = iftComponentArea(skel,A);

  for (int p=0; p < skel->n; p++){
    if ((skel->val[p] != 0)&&(len->val[p]<=thres)){
      skel->val[p] = 0;
    }
  }
  iftDestroyImage(&len);
  iftDestroyAdjRel(&A);
}

iftImage *iftExtractMinuciae(iftImage *term, iftImage *bpts)
{
  iftImage *minu    = iftCopyImage(term);
  
  for (int p=0; p < term->n; p++){
    if (bpts->val[p] != 0){
      minu->val[p] = bpts->val[p];
    }
  }
  
  return(minu);
}

iftFImage *TermPointOrient(iftImage *term, iftImage *skel, float radius)
{
  iftFImage  *ori = iftCreateFImage(term->xsize,term->ysize,term->zsize);
  iftAdjRel *A    = iftCircularEdges(radius);
  
  for (int p=0; p < term->n; p++){
    if (term->val[p] != 0){
      iftVoxel u = iftGetVoxelCoord(term,p);
      for (int i=1; i < A->n; i++){
	iftVoxel v = iftGetAdjacentVoxel(A,u,i);
	if (iftValidVoxel(skel,v)){
	  int q = iftGetVoxelIndex(skel,v);
	  if (skel->val[q]!=0){
	    ori->val[p] = (0.5*atan2(A->dy[i],A->dx[i])*180.0/IFT_PI + 90);
	    break;
	  }
	}
      }
    }
  }

  iftDestroyAdjRel(&A);
  return(ori);
}

int main(int argc, char *argv[])
{
  timer *tstart=NULL;
  char   filename[200];
  iftAdjRel *A = iftCircular(1.5), *B = iftCircular(3.0), *C = iftCircular(1.0);
  
  /*--------------------------------------------------------*/

  void *trash = malloc(1);                 
  struct mallinfo info;   
  int MemDinInicial, MemDinFinal;
  free(trash); 
  info = mallinfo();
  MemDinInicial = info.uordblks;

  /*--------------------------------------------------------*/

  
  if (argc != 3) {
    printf("iftSkeletonizeFP <P1> <P2>\n");
    printf("P1: folder with binary images\n");
    printf("P2: folder with skeleton images\n");
    exit(0);
  }

  tstart = iftTic();

  iftFileSet *fs   = iftLoadFileSetFromDirBySuffix(argv[1],".png", 1);  
  int nimages      = fs->n;
  char *out_dir    = argv[2];
  iftMakeDir(out_dir);
  
  iftColor RGB1, YCbCr1;
  iftColor RGB2, YCbCr2;
  RGB1.val[0] = 255;
  RGB1.val[1] = 0;
  RGB1.val[2] = 255;
  YCbCr1      = iftRGBtoYCbCr(RGB1,255);
  RGB2.val[0] = 0;
  RGB2.val[1] = 255;
  RGB2.val[2] = 0;
  YCbCr2      = iftRGBtoYCbCr(RGB2,255);

  for (int i=0; i < nimages; i++) {
    char *basename    = iftFilename(fs->files[i]->path,".png");
    iftImage *img     = iftReadImageByExt(fs->files[i]->path);
    iftImage *bin     = iftThreshold(img,1,255,255);
    iftDestroyImage(&img);
    iftFImage *msskel = iftMSSkel2D(bin,A,IFT_INTERIOR, NULL, NULL);
    iftImage  *skel   = iftFThreshold(msskel,0.5,100.0,255);
    iftImage  *term   = iftTerminalPoints2D(skel);
    iftImage  *bpts   = iftBranchPoints2D(skel);
    iftImage *minu    = iftExtractMinuciae(term,bpts);

    
    iftSetCbCr(bin,128);
    for (int p = 0; p < bin->n; p++) {
      if (skel->val[p]) {
    	iftVoxel u = iftGetVoxelCoord(bin,p);
    	iftDrawPoint(bin, u, YCbCr1, C, 255);
	if (minu->val[p]){
	  iftDrawPoint(bin,u,YCbCr2, B, 255);
	}
      }
    }
    /* replace skel by bin */
    
    sprintf(filename,"%s/%s.png",out_dir,basename);
    iftWriteImageByExt(bin,filename);
    
    iftDestroyImage(&bin);
    iftDestroyImage(&term);
    iftDestroyImage(&bpts);
    iftDestroyImage(&skel);
    iftDestroyImage(&minu);
    iftDestroyFImage(&msskel);
    iftFree(basename);
  }

  iftDestroyFileSet(&fs);
  iftDestroyAdjRel(&A);
  iftDestroyAdjRel(&B);
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








