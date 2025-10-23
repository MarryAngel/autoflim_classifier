#include "ift.h"
#include "iftFPModel.h"

void SaveActivationsMatrix(iftMatrix *activ_matrix, char *basename){
  char filename[200];
  sprintf(filename, "%s_activations.txt", basename);
  FILE *activ_file = fopen(filename,"w");
  printf("filename: %s\n", filename);

  for (int i = 0; i < activ_matrix->nrows; i++){
    fprintf(activ_file, "%d", i); // latent minutia index
    for (int j = 0; j < activ_matrix->ncols; j++){
	    int k = i * activ_matrix->ncols + j;
      fprintf(activ_file, " %lf", activ_matrix->val[k]);
    }
    fprintf(activ_file, "\n");
  }

  fclose(activ_file);
}


int main(int argc, char *argv[]) 
{
  size_t           mem_start, mem_end;
  timer           *t1, *t2;

  mem_start = iftMemoryUsed();

  if (argc != 6){
    printf("Usage: iftDetectHomologous <P1> <P2> <P3> <P4> <P5>\n");
    printf("P1: input folder with the test images\n");
    printf("P2: input folder with the reference images\n");
    printf("P3: input folder with the reference FP models\n");    
    printf("P4: input FP model parameters \n");
    printf("P5: output folder with homologous points\n");
    exit(-1);
  }

  t1 = iftTic();

  /* Read input parameters */

  iftFileSet *fs       = iftLoadFileSetFromDirBySuffix(argv[1], ".png", 1);
  iftFileSet *fs_ref   = iftLoadFileSetFromDirBySuffix(argv[2], ".png", 1);
  char    *model_dir   = argv[3];
  iftDict *param       = iftReadJson(argv[4]);
  char    *out_dir     = argv[5];
  iftMakeDir(out_dir);
  char filename[200];

  /* Detect homologous points on test images with respect to FP models
     of reference (training) images. */
  
  for (int i=0; i < fs->n; i++) {

    /* read test image and compute its skeleton with value 128 on the
       minucia points */
    
    iftImage *img   = iftReadImageByExt(fs->files[i]->path);
    char *basename  = iftFilename(fs->files[i]->path, ".png");

    printf("Test %s\n",basename);
    
    iftImage *gabor_img = Gabor(img);
    iftImage *gabor_bin =
      iftThreshold(gabor_img,
		   iftGetLongValFromDict("gabor_threshold",param),
		   255,255);    
    iftImage *mask  = GetMask(gabor_bin,param);    
    iftMatrix *img_matrix = ImageToFeatureMatrix(gabor_img, param);
    Masking(gabor_img,mask);
    Masking(gabor_bin,mask);
    FilterGaborBin(&gabor_bin, param);
    iftImage *skel   = Skeleton(gabor_bin);
    iftLabeledSet *S = GetSkelPoints(skel,gabor_img,mask,param);
    iftImage *skelpt = iftCreateImageFromImage(mask);
    iftLabeledSetToImage(S,skelpt,false);      
    sprintf(filename,"%s/%s-seeds.txt",argv[1],basename);
    iftWriteSeeds(S,gabor_img,filename);
    iftDestroyLabeledSet(&S);
    iftDestroyImage(&gabor_img);
    iftDestroyImage(&gabor_bin);
    iftDestroyImage(&skel);      
    
    /* For each reference image, apply its corresponding model and
       verify if there exist homologous points, saving them in the
       output folder. */
    
    for (int j=0; j < fs_ref->n; j++) {
      iftImage *ref      = iftReadImageByExt(fs_ref->files[j]->path); 
      char *basename_ref = iftFilename(fs_ref->files[j]->path, ".png");

      printf("Model %s\n",basename_ref);
      
      /* read FP model */
      
      iftMatrix *kernels = ReadFPModelKernels(model_dir, basename_ref);
      float     *bias    = ReadFPModelBias(model_dir, basename_ref);
      int       *seedpt  = ReadSeedPoints(model_dir,basename_ref);

      /* compute convolution + bias and the maximum activation of each
	 channel */

      printf("convolution \n");
      
      iftMatrix *conv  = iftMultMatrices(img_matrix,kernels);
      iftMImage *activ = iftMatrixToMImage(conv, img->xsize, img->ysize, img->zsize, kernels->ncols, 'c');
      iftDestroyMatrix(&conv);
      float *max_activ = iftAllocFloatArray(activ->m);
      
      for (int p=0; p < activ->n; p++) {	
	if (mask->val[p]!=0){	  
	  for (int b=0; b < activ->m; b++) {
	    activ->val[p][b] += bias[b];
	    if (activ->val[p][b]<0)
	      activ->val[p][b]=0;	
	    if (max_activ[b] < activ->val[p][b])
	      max_activ[b] = activ->val[p][b];
	  }
	}else{
	  for (int b=0; b < activ->m; b++) {
	    activ->val[p][b] =0;
	  }
	}
      }
      
      /* Detect candidates and count votes */

      printf("voting \n");

      iftMImage *votes
	= iftCreateMImage(activ->xsize,activ->ysize,activ->zsize,activ->m);
      float homologous_radius =
	iftGetDblValFromDict("homologous_radius",param); 
      iftAdjRel *B = iftCircular(homologous_radius);

      for (int p=0; p < activ->n; p++) {
	for (int b=0; b < activ->m; b++) {
	  if (activ->val[p][b] >= 0.95*max_activ[b]){
	    iftVoxel u = iftMGetVoxelCoord(activ,p);
	    for (int k=0; k < B->n; k++) {
	      iftVoxel v = iftGetAdjacentVoxel(B,u,k);
	      if (iftValidVoxel(skelpt,v)){
		int q = iftGetVoxelIndex(skelpt,v);
		if (skelpt->val[q]!=0) {
		  votes->val[p][b] += 1.0;
		  break;
		}
	      }
	    }
	  }
	}
      }
      
      iftDestroyAdjRel(&B);
      iftFree(max_activ);

      /* Select the most voted points for homologous detection */

      printf("drawing points \n");

      iftImage *cref = iftCopyImage(ref);
      iftImage *cimg = iftCopyImage(img);
      iftSetCbCr(cref,128);
      iftSetCbCr(cimg,128);      
      iftColorTable *ctb = iftCreateRandomColorTable(ref->n+1);
      iftAdjRel *C = iftCircular(3.0);

      for (int p=0; p < votes->n; p++){
	int bmax = 0;
	for (int b=1; b < votes->m; b++){
	  if (votes->val[p][b]>votes->val[p][bmax]){
	    bmax = b;
	  }
	}
	if (votes->val[p][bmax]>0){ 
	  int q      = seedpt[bmax];
	  iftVoxel u = iftGetVoxelCoord(cref,q);
	  iftVoxel v = iftGetVoxelCoord(cimg,p);
	  iftDrawPoint(cref, u, ctb->color[q], C, 255);
	  iftDrawPoint(cimg, v, ctb->color[q], C, 255);
	}
      }
      iftDestroyMImage(&votes);

      char suffix[50];
      sprintf(suffix,"test_%s",basename);
      WriteImage(cimg,out_dir,basename_ref,suffix);
      sprintf(suffix,"train_%s",basename);
      WriteImage(cref,out_dir,basename_ref,suffix);
      iftDestroyColorTable(&ctb);
      iftDestroyMImage(&activ);
      
      iftDestroyImage(&cref);
      iftDestroyImage(&cimg);
      iftDestroyImage(&ref);
      iftFree(bias);
      iftFree(seedpt);
      iftDestroyMatrix(&kernels);
      iftFree(basename_ref);
    }
    
    iftDestroyImage(&img);
    iftDestroyImage(&mask);
    iftDestroyImage(&skelpt);      
    iftFree(basename);
    iftDestroyMatrix(&img_matrix);
  }

  iftDestroyFileSet(&fs);
  iftDestroyFileSet(&fs_ref);
  
  mem_end = iftMemoryUsed();
  iftVerifyMemory(mem_start,mem_end);
    
  t2 = iftToc();
  puts(iftFormattedTime(iftCompTime(t1,t2)));

  return(0);
}
