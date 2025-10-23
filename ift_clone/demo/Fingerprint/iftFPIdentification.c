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
    printf("usage iftFPIdentification <P1> <P2> <P3>\n");
    printf("P1: folder with query database features (.npy)\n");
    printf("P2: folder with reference database features \n");
    printf("P3: output folder with identification results\n");
    exit(0);
    }

    tstart = iftTic();

    iftFileSet *query_fs    = iftLoadFileSetFromDirBySuffix(argv[1],".npy", 1);
    iftFileSet *ref_fs      = iftLoadFileSetFromDirBySuffix(argv[2],".npy", 1);

    char *out_dir     = argv[3];
    iftMakeDir(out_dir);
  
    for (int i = 0; i < query_fs->n; i++){
        printf("Progress: %d/%ld\n", i + 1, query_fs->n);

        char *latent      = iftFilename(query_fs->files[i]->path, ".npy");
        sprintf(filename, "%s/%s/matches", out_dir, latent);
        iftMakeDir(filename);

        sprintf(filename, "%s/%s/score.txt", out_dir, latent);
        FILE *score = fopen(filename, "w");

        iftMatrix *latent_feature_vector, *reference_feature_vector, *similarity_score;
        sprintf(filename, "%s/%s.npy", argv[1], latent);
        latent_feature_vector = iftReadMatrix(filename);

        /* Storing reference names, scores and indexes for sorting */

        float *scores = (float *)calloc(ref_fs->n, sizeof(float));
        int *idxs     = (int *)calloc(ref_fs->n, sizeof(int));
        // char **names  = (char **)calloc(ref_fs->n, sizeof(char *));

        for (int k = 0; k < ref_fs->n; k++){
          idxs[k] = k;
        }

        for (int j = 0; j < ref_fs->n; j++){
            char *reference    = iftFilename(ref_fs->files[j]->path, ".npy");
            sprintf(filename, "%s/%s/matches/%s_%s.txt", out_dir, latent, latent, reference);
            FILE *match_file = fopen(filename, "w");


            sprintf(filename, "%s/%s.npy", argv[2], reference);
            reference_feature_vector = iftReadMatrix(filename);

            iftMatrix *aux = iftTransposeMatrix(reference_feature_vector);
            iftDestroyMatrix(&reference_feature_vector);

            reference_feature_vector = aux;

            similarity_score = iftMultMatrices(latent_feature_vector, reference_feature_vector);
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
            scores[j] = match_score;

            // fprintf(score, "%s %s %f\n", latent, reference, match_score);


            iftDestroyMatrix(&similarity_score);
            free(idxs);
            free(n_candidates_per_mnt);
            fclose(match_file);
            iftFree(reference);
        }

    /* Sorting score file by scores*/
    iftFQuickSort(scores, idxs, 0, ref_fs->n, IFT_DECREASING);
    for (int j = 0; j < ref_fs->n; j++){
        fprintf(score, "%s %s %f\n", latent, iftFilename(ref_fs->files[idxs[j]]->path, ".npy"), scores[j]);
    }

    iftDestroyMatrix(&latent_feature_vector);
    fclose(score);
    iftFree(scores);
    iftFree(idxs);
    }

  iftDestroyFileSet(&query_fs);
  iftDestroyFileSet(&ref_fs);



  
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








