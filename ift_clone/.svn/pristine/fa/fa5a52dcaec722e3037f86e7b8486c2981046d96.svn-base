#include "ift.h"
#include <math.h>

#define MAX_CANDIDATES 4
#define SIMILARITY_THRESHOLD 0.56

int main(int argc, char *argv[])
{
    timer *tstart=NULL;
    char   filename[300];

    /*--------------------------------------------------------*/

    void *trash = malloc(1);                 
    struct mallinfo info;   
    int MemDinInicial, MemDinFinal;
    free(trash); 
    info = mallinfo();
    MemDinInicial = info.uordblks;

    /*--------------------------------------------------------*/


    if (argc != 4) {
    printf("usage iftFPVerification <P1> <P2> <P3>\n");
    printf("P1: folder with patch features for each image(.npy)\n");
    printf("P2: image verification comparisons file (.txt)\n");
    printf("P3: output folder with verification scores (.txt) and match candidates\n");
    exit(0);
    }

    tstart = iftTic();

    iftFileSet *fs    = iftLoadFileSetFromDirBySuffix(argv[1],".npy", 1);
    FILE *fp          = fopen(argv[2], "r");
    int n_comparisons;
    fscanf(fp, "%d", &n_comparisons);

    char *out_dir     = argv[3];
    iftMakeDir(out_dir);

    sprintf(filename, "%s/matches", out_dir);
    iftMakeDir(filename);

    sprintf(filename, "%s/score.txt", out_dir);
    FILE *score = fopen(filename, "w");
  
    for (int i = 0; i < n_comparisons; i++){
        char latent[100], reference[100];
        iftMatrix *latent_feature_vector, *reference_feature_vector, *similarity_score;


        /* Reading comparison */
        fscanf(fp, "%s %s", latent, reference);

        sprintf(filename, "%s/matches/%s_%s.txt", out_dir, latent, reference);
        FILE *match_file = fopen(filename, "w");
        /* Reading latent feature vector */
        sprintf(filename, "%s/%s.npy", argv[1], latent);
        latent_feature_vector = iftReadMatrix(filename);

        /* Reading reference feature vector */
        sprintf(filename, "%s/%s.npy", argv[1], reference);
        reference_feature_vector = iftReadMatrix(filename);


        iftMatrix *aux = iftTransposeMatrix(reference_feature_vector);
        iftDestroyMatrix(&reference_feature_vector);

        reference_feature_vector = aux;

        similarity_score = iftMultMatrices(latent_feature_vector, reference_feature_vector);
        iftDestroyMatrix(&latent_feature_vector);
        iftDestroyMatrix(&reference_feature_vector);

        
        /*Sorting each line of the matrix*/
        int *idxs = iftAllocIntArray(similarity_score->n);
        for (int k = 0; k < similarity_score->n; k++){
          idxs[k] = k % similarity_score->ncols;
        }

        for (int l = 0; l < similarity_score->nrows ; l++){
          iftFQuickSort(similarity_score->val, idxs, l * similarity_score->ncols, (l + 1) * similarity_score->ncols - 1, IFT_DECREASING);
        }

        /*Checking number of candidates per each minutiae*/
        int *n_candidates_per_mnt = (int *)calloc(similarity_score->nrows, sizeof(int));
        int counter = 0;

        for (int j = 0; j < similarity_score->n; j+=similarity_score->ncols){
          for (int c = 0; c < MAX_CANDIDATES; c++){
            if (similarity_score->val[j + c] > SIMILARITY_THRESHOLD){
              n_candidates_per_mnt[counter]++;
            }
          }
          counter++;
        }

        /*Calculating match score and formatting output*/
        float match_score     = 0;
        int line_breaker_flag = 0;
        counter               = 0;
        for (int l = 0; l < similarity_score->nrows; l++){
          int line_index = l * similarity_score->ncols;
          if (n_candidates_per_mnt[l]){
            line_breaker_flag = 1;
            match_score++;
            fprintf(match_file, "%d", l);
            for (int c = 0; c < n_candidates_per_mnt[l]; c++){
              fprintf(match_file, " %d", idxs[line_index + c]);
            }
          }
          if (line_breaker_flag) fprintf(match_file, "\n");
          line_breaker_flag = 0;
        }

        match_score = match_score / similarity_score->nrows;

        fprintf(score, "%s %s %f\n", latent, reference, match_score);


        iftDestroyMatrix(&similarity_score);
        free(idxs);
        free(n_candidates_per_mnt);
        fclose(match_file);
        if (i == 1) break;
    }

  fclose(fp);
  fclose(score);
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








