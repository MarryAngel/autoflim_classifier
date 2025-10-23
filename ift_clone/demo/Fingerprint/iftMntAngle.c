#include "ift.h"
#include <math.h>

#define BRANCH_RADIUS 15
#define MAX_N_BRANCHES 3

float calculateTerminalAngle(int root, int *term_pt, iftImage *orig){
  // Calculate angle of a terminal point using simple arc tan function
  iftVoxel u  = iftGetVoxelCoord(orig, root);
  iftVoxel v  = iftGetVoxelCoord(orig, *term_pt);
  return atan2((float)(v.y - u.y), (float)(v.x - u.x));
}

float angleBetweenVectors(iftVoxel v1, iftVoxel v2){
  // Calculate angle between vectors using the dot product. 
  float dotProduct   = (v1.x * v2.x + v1.y * v2.y);
  float magnitudes   = sqrt((float)(v1.x)* (v1.x) + (v1.y) * (v1.y)) * sqrt((float)(v2.x)* (v2.x) + (v2.y) * (v2.y));
  return acos(dotProduct/magnitudes);
}

float calculateBranchAngle(int root, int *term_pt, iftImage *orig){
  // Calculate angle of a bifurcation minutia.

  iftVoxel u, p1, p2, p3;
  u  = iftGetVoxelCoord(orig, root);
  p1 = iftGetVoxelCoord(orig, term_pt[0]);
  p2 = iftGetVoxelCoord(orig, term_pt[1]); 
  p3 = iftGetVoxelCoord(orig, term_pt[2]);

  iftVoxel v1, v2, v3; // vectors from root point to terminal points

  v1.x = p1.x - u.x, v1.y = p1.y - u.y;
  v2.x = p2.x - u.x, v2.y = p2.y - u.y; 
  v3.x = p3.x - u.x, v3.y = p3.y - u.y; 

  float theta12, theta13, theta23; // angle between each of the two vectors

  theta12 = angleBetweenVectors(v1, v2);
  theta13 = angleBetweenVectors(v1, v3);
  theta23 = angleBetweenVectors(v2, v3);


  // finding the smallest angle
  float min_theta = theta12 <= theta13? theta12 <= theta23? theta12 : theta23 : theta13 <= theta23? theta13 : theta23;

  iftVoxel resultVector;

  if (min_theta == theta12){
    resultVector.x =  (v1.x + v2.x) + (-1) * v3.x;
    resultVector.y =  (v1.y + v2.y) + (-1) * v3.y;
  } 
  else if (min_theta == theta13){
    resultVector.x =  (v1.x + v3.x) + (-1) * v2.x;
    resultVector.y =  (v1.y + v3.y) + (-1) * v2.y;
  }
  else {
    resultVector.x =  (v2.x + v3.x) + (-1) * v1.x;
    resultVector.y =  (v2.y + v3.y) + (-1) * v1.y;
  }

  return atan2((float)(resultVector.y), (float)(resultVector.x)) + 2 * IFT_PI;
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
    printf("iftDetectSkelMntAngle <P1> <P2> <P3>\n");
    printf("P1: folder with seeds points\n");
    printf("P2: folder with skel images\n");
    printf("P3: folder with the output mnts with angles\n");
    exit(0);
  }

  tstart = iftTic();

  iftFileSet *fs_imgs    = iftLoadFileSetFromDirBySuffix(argv[2],".png", 1);

  int nimages      = fs_imgs->n;
  char *out_dir    = argv[3];
  
  iftMakeDir(out_dir);

  iftAdjRel *A = iftCircular(sqrtf(2.0));
  iftAdjRel *C = iftCircular(1.0);

  for (int i=0; i < nimages; i++) {
    printf("Progress: %d/%d\n", i + 1, nimages);

    char *basename     = iftFilename(fs_imgs->files[i]->path,".png");
    iftImage *orig     = iftReadImageByExt(fs_imgs->files[i]->path);
    iftImage *root     = iftReadImageByExt(fs_imgs->files[i]->path); // predecessores
    iftImage *distance = iftCreateImage(orig->xsize, orig->ysize, orig->zsize); // distancia até origem

    sprintf(filename, "%s/%s.mnt", out_dir, basename);
    FILE *fp_mnt = fopen(filename, "w");

    // Fixing Seeds file for skel image (dimensions are multiplied by 2)
    sprintf(filename,"%s/%s-seeds.txt",argv[1],basename);
    FILE *fp = fopen(filename,"r");
    
    sprintf(filename,"%s/%s-seeds.txt",out_dir,basename);
    FILE *fp_corrected = fopen(filename, "w");


    // reading file header
    int npts, xsize, ysize;
    fscanf(fp,"%d %d %d",&npts,&xsize,&ysize);
    fprintf(fp_corrected, "%d %d %d\n", npts, xsize * 2, ysize * 2);
    fprintf(fp_mnt, "%s\n", basename);
    
    
    // fixing mnts locations
    int x, y, a, b;
    for (int pt = 0; pt < npts; pt++) {
	    fscanf(fp,"%d %d %d %d",&x, &y, &a, &b);
        fprintf(fp_corrected, "%d %d %d %d\n", x * 2,y * 2,a,b);    
    }

    // Vetores das minúcias - para salvar depois
    iftVoxel *mnts_loc = (iftVoxel *)calloc(npts, sizeof(iftVoxel));
    float    *mnts_ori = (float *)calloc(npts, sizeof(float));

    int correct_mnt_counter = 0;

    fclose(fp);
    fclose(fp_corrected);

    // Propagating minutia points to find branches
    iftLabeledSet *S = iftReadSeeds(orig, filename);
    iftColorTable *ctb = iftCreateRandomColorTable(orig->n + 1, 255);

    while(S != NULL) {
        iftFIFO *F = iftCreateFIFO(orig->n);
        int label;
        int r = iftRemoveLabeledSet(&S, &label);
        iftVoxel u = iftGetVoxelCoord(orig,r);
        // iftDrawPoint(img, u, ctb->color[r], C, 255);

        distance->val[r] = 0;
        root->val[r] = r;
        iftInsertFIFO(F, r);

        int *end_pts   = (int *)calloc(MAX_N_BRANCHES, sizeof(int));
        int k = 0; // number of branch points that have geodesic distance at least equal to BRANCH_RADIUS

        // Busca em largura
        while(!iftEmptyFIFO(F)){
          int p = iftRemoveFIFO(F);
          iftVoxel u = iftGetVoxelCoord(orig, p);

          if (distance->val[p] == BRANCH_RADIUS){
            // iftDrawPoint(img, u, ctb->color[root->val[p]], C, 255);
            end_pts[k] = p;
            k++;
          }
          else{
            for (int i = 1; i < A->n; i++){
              iftVoxel v = iftGetAdjacentVoxel(A, u, i);
              if (iftValidVoxel(orig, v)){
                int q = iftGetVoxelIndex(orig, v);
                if (orig->val[q]){
                  if (F->color[q] == IFT_WHITE){
                    root->val[q] = root->val[p];
                    distance->val[q] = distance->val[p] + 1;
                    iftInsertFIFO(F, q);
                  }
                }
              }
            }
          }
        }

        // Calculando o angulo da minúcia
        float theta;
        if (k == 1){
           theta = calculateTerminalAngle(r, end_pts, orig);
           mnts_loc[correct_mnt_counter] = u;
           mnts_ori[correct_mnt_counter] = theta;
           correct_mnt_counter++;
        }
        
        else if (k == 3){
          theta = calculateBranchAngle(r, end_pts, orig);
          mnts_loc[correct_mnt_counter] = u;
          mnts_ori[correct_mnt_counter] = theta;
          correct_mnt_counter++;
        }
    free(end_pts);
    iftDestroyFIFO(&F);
    }

    // Saving mnts to result file
    fprintf(fp_mnt, "%d %d %d\n", correct_mnt_counter, xsize  , ysize );

    for (int i = 0; i < correct_mnt_counter; i++){
      fprintf(fp_mnt, "%d %d %f 1.0\n", mnts_loc[i].x / 2 , mnts_loc[i].y /2 , mnts_ori[i]);
    }

    remove(filename);


    fclose(fp_mnt);

    free(mnts_loc);
    free(mnts_ori);

    iftDestroyImage(&root);
    iftDestroyImage(&distance);
    iftDestroyImage(&orig);



    iftFree(basename);
    iftDestroyLabeledSet(&S);
    iftDestroyColorTable(&ctb);
  }

  iftDestroyFileSet(&fs_imgs);
  iftDestroyAdjRel(&A);
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








