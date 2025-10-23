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
  
  if (argc != 4) {
    printf("iftLearnFPModel <P1> <P2> <P3>\n");
    printf("P1: input folder with the original FP images\n");
    printf("P2: input FPModel parameters \n");
    printf("P3: output folder with the model\n");
    exit(0);
  }

  tstart = iftTic();

  /* Read input parameters */
  
  char *orig_dir    = argv[1];
  iftFileSet *fs    = iftLoadFileSetFromDirBySuffix(orig_dir,".png", 1);  
  int nimages       = fs->n;
  iftDict *param    = iftReadJson(argv[2]); 
  int num_of_transformations = (int)
      iftGetLongValFromDict("num_of_transformations",param);
  char *model_dir   = argv[3];
  iftMakeDir(model_dir);

  /* Model learning for each image */
  
  for (int i=0; i < nimages; i++) {
    char *basename        = iftFilename(fs->files[i]->path,".png");
    iftImage *orig        = iftReadImageByExt(fs->files[i]->path);
    iftImage *gabor_img   = Gabor(orig);
    iftImage *gabor_bin   =
      iftThreshold(gabor_img,
		   iftGetLongValFromDict("gabor_threshold",param),
		   255,255);
    iftImage *mask        = GetMask(gabor_bin, param);
    Masking(gabor_img, mask);
    Masking(gabor_bin, mask);    
    FilterGaborBin(&gabor_bin, param);
    iftImage *skel        = Skeleton(gabor_bin);
    
    /* WriteImage(gabor_img,model_dir,basename,"gabor_img"); */
    /* WriteImage(gabor_bin,model_dir,basename,"gabor_bin"); */
    /* WriteImage(mask,model_dir,basename,"mask"); */
    /* WriteImage(skel,model_dir,basename,"skel"); */
    
    iftMatrix **M          = GetRandomTransformations(gabor_img, param);
    iftImage **tgabor_img  = GetTransformedImages(gabor_img,M,num_of_transformations);
    iftImage **tgabor_bin  =
      ThresholdImageArray(tgabor_img, num_of_transformations,
			  iftGetLongValFromDict("gabor_threshold",param),
			  255, 255);
    iftImage **tmasks      = GetMaskArray(tgabor_bin,num_of_transformations, param);
    MaskingArray(tgabor_img, num_of_transformations, tmasks);
    MaskingArray(tgabor_bin, num_of_transformations, tmasks);    
    FilterGaborBinArray(tgabor_bin,num_of_transformations,param); 
    iftImage **tskels      = SkeletonArray(tgabor_bin,num_of_transformations);
    iftLabeledSet **skpts  = GetSkeletonPointArray(tskels,num_of_transformations);
    /* iftLabeledSet *seeds = GetSkelPoints(skel,orig,mask,param); */
    iftLabeledSet *seeds   = GetStableSkelPoints(skel, gabor_img, mask, tskels,
    						tgabor_img, M, num_of_transformations, param);

    /* WriteTransformations(M,num_of_transformations,model_dir, */
    /*    basename,"trfs"); */
    /* WriteImageArray(tgabor_img,num_of_transformations,model_dir,basename,"tgabor_img"); */
    /* WriteImageArray(tgabor_bin,num_of_transformations, model_dir,basename,"tgabor_bin"); */
    /* WriteImageArray(tmasks,num_of_transformations, model_dir,basename,"tmasks"); */
    /* WriteImageArray(tskels,num_of_transformations,model_dir,basename,"tskels"); */
    /* WritePointArray(skpts,num_of_transformations, model_dir,basename,"skpts",tskels); */
    WritePoints(seeds,model_dir,basename,"seeds",orig);
    
    iftDataSet *Z = PatchesFromSeeds(gabor_img, tgabor_img, M, num_of_transformations, seeds, param);

    CompWriteModel(Z, num_of_transformations, param, model_dir, basename);

    /* SavePointMatchImages(skel,gabor_bin,tgabor_bin,tskels, */
    /* 			 M,num_of_transformations,"PointMatchImages"); */
    
    iftDestroyLabeledSet(&seeds);
    iftDestroyImage(&orig);
    iftDestroyImage(&gabor_bin);    
    iftDestroyImage(&gabor_img);    
    iftDestroyImage(&mask);    
    iftDestroyImage(&skel);
    iftDestroyDataSet(&Z);
    iftFree(basename);
    for (int t=0; t < num_of_transformations; t++){
       iftDestroyMatrix(&M[t]);
       iftDestroyLabeledSet(&skpts[t]);
    }
    iftFree(skpts);
    iftFree(M);
    DestroyImageArray(&tgabor_img, num_of_transformations);
    DestroyImageArray(&tgabor_bin, num_of_transformations);
    DestroyImageArray(&tskels, num_of_transformations);
    DestroyImageArray(&tmasks, num_of_transformations);
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








