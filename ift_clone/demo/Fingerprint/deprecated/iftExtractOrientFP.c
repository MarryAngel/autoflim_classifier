#include "ift.h"

#define EXT ".png"

/* It segments the region with fingerprint in an image */

iftImage *GradOrientation(iftMImage *grad_x, iftMImage *grad_y, iftImage *skel)
{
  iftImage *ori = iftCreateImage(grad_x->xsize,grad_x->ysize,grad_x->zsize); 
  
  for (int p=0; p < ori->n; p++) {
    if (skel->val[p] != 0) {
      if (!iftAlmostZero(grad_x->val[p][0])){      
	ori->val[p] = (int)(0.5*atan2(grad_y->val[p][0],grad_x->val[p][0])*180.0/IFT_PI + 90);
      }
    }
  }
  
  return(ori);
}


iftMImage *StandardizeImage(iftImage *orig, float desired_mean, float desired_stdev)
{
  iftMImage *mimg;
  
  if (iftIsColorImage(orig)){
    mimg = iftImageToMImage(orig,RGB_CSPACE);
  } else {
    mimg = iftImageToMImage(orig,GRAY_CSPACE);
  }

  float *mean, *stdev;

  mean  = iftAllocFloatArray(mimg->m);
  stdev = iftAllocFloatArray(mimg->m);
  
  for (int p=0; p < mimg->n; p++) {
    for (int b=0; b < mimg->m; b++) {
      mean[b] += mimg->val[p][b];
    }
  }
  for (int b=0; b < mimg->m; b++) {
    mean[b] /= mimg->n;
  }
  for (int p=0; p < mimg->n; p++) {
    for (int b=0; b < mimg->m; b++) {
      stdev[b] += (mimg->val[p][b]-mean[b])*(mimg->val[p][b]-mean[b]);
    }
  }
  for (int b=0; b < mimg->m; b++) {
    stdev[b] = sqrt(stdev[b]/mimg->n);
  }

  for (int p=0; p < mimg->n; p++) {
    for (int b=0; b < mimg->m; b++) {
      if (mimg->val[p][b] > mean[b]){
	mimg->val[p][b] = desired_mean + sqrt(((mimg->val[p][b] - mean[b])*
					       (mimg->val[p][b] - mean[b]))*
					      desired_stdev/stdev[b]);
      } else {
	mimg->val[p][b] = desired_mean - sqrt(((mimg->val[p][b] - mean[b])*
					       (mimg->val[p][b] - mean[b]))*
					      desired_stdev/stdev[b]);       
      }
    }
  }
  
  iftFree(mean);
  iftFree(stdev);

  return(mimg);
}





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
    printf("iftExtractOrientFP <P1> <P2> <P3>\n");
    printf("P1: input folder with enhanced images\n");
    printf("P2: input folder with skeletons\n");
    printf("P3: output folder with orientations\n");
    exit(0);
  }

  tstart = iftTic();

  iftFileSet *fs   = iftLoadFileSetFromDirBySuffix(argv[1],EXT, 1);  
  int nimages      = fs->n;
  char *out_dir    = argv[3];
  iftMakeDir(out_dir);
  
  for (int i=0; i < nimages; i++) {
    char *basename    = iftFilename(fs->files[i]->path,EXT);
    iftImage  *orig   = iftReadImageByExt(fs->files[i]->path);
    sprintf(filename,"%s/%s%s",argv[2],basename,EXT);
    /* not needed, just to fix temporarily that bug */
    iftImage  *aux   = iftReadImageByExt(filename);
    iftImage  *skel  = iftRemFrame(aux,1);
    iftDestroyImage(&aux);
    
    iftMImage *simg   = iftImageToMImage(orig,GRAYNorm_CSPACE);
    /* iftMImage *simg   = StandardizeImage(orig, 255, 256/2); */
    iftKernel *Kx     = iftSobelXKernel2D();
    iftKernel *Ky     = iftSobelYKernel2D();
    iftMKernel *Sx    = iftCreateMKernel(Kx->A, 1);
    iftMKernel *Sy    = iftCreateMKernel(Ky->A, 1);
    for (int i=0; i < Kx->A->n; i++){
      Sx->weight[0].val[i] = Kx->weight[i];
      Sy->weight[0].val[i] = Ky->weight[i];
    }
    iftDestroyKernel(&Kx);
    iftDestroyKernel(&Ky);
    iftMImage *grad_x   = iftMLinearFilter(simg, Sx);
    iftMImage *grad_y   = iftMLinearFilter(simg, Sy);
    /* There is a bug here, because iftMLinearFilter is not doing
       padding. FIX IT */
    
    iftDestroyMKernel(&Sx);
    iftDestroyMKernel(&Sy);
    iftDestroyMImage(&simg);
    iftImage  *ori     = GradOrientation(grad_x, grad_y, skel);

    sprintf(filename,"%s/%s%s",out_dir,basename,EXT);
    iftWriteImageByExt(ori,filename);
    
    iftDestroyImage(&orig);
    iftDestroyImage(&skel);
    iftDestroyMImage(&grad_x);
    iftDestroyMImage(&grad_y);
    iftDestroyImage(&ori);
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








