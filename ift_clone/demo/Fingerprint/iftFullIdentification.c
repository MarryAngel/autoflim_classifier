/*
Unicamp - 07-06-2023
Author: André Igor Nóbrega da Silva
email:  a203758@dac.unicamp.br

Solving the FP Identification problem using the Hungarian algorithm as match strategy and spatial coherence
*/

#include "ift.h"
#include <math.h>
#include <stdbool.h>

#define SIMILARITY_THRESHOLD 0.556
#define SPATIAL_COHERENCE_THRESHOLD 0.7
#define K_MATCHES 10
#define MAX(a, b) a > b ? a : b

#define MAXIMIZATION_PROBLEM 1
#define MINIMIZATION_PROBLEM 0

#define KL 5
#define KR 5
#define NVIZ 1


void minimizeHungarianMatrix(iftMatrix *hungarian_matrix){
    float max = IFT_INFINITY_FLT_NEG;

    // Find maximum value in matrix
    for (int i = 0; i < hungarian_matrix->n; i++){
        if (max < hungarian_matrix->val[i]) max = hungarian_matrix->val[i];
    } 

    // Aij = Aij - max --->> This transforms the maximization problem into a minimization problem
    for (int i = 0; i < hungarian_matrix->n; i++){
        hungarian_matrix->val[i] = max - hungarian_matrix->val[i];
    }
}

bool isSquareMatrix(iftMatrix *input_matrix){
    return (input_matrix->nrows == input_matrix->ncols);

}

iftMatrix *balanceHungarianMatrix(iftMatrix *hungarianMatrix){
    int max_dimension          = MAX(hungarianMatrix->nrows, hungarianMatrix->ncols);
    iftMatrix *balanced_matrix = iftCreateMatrix(max_dimension, max_dimension);

    // Copy every element from input matrix to output balanced matrix
    for (int r = 0; r < hungarianMatrix->nrows; r++){
        for (int c = 0; c < hungarianMatrix->ncols; c++){
            iftMatrixElem(balanced_matrix, c, r) = iftMatrixElem(hungarianMatrix, c, r);
        }
    }
    return balanced_matrix;
}

void applyConstraintsToHungarianMatrix(iftMatrix *hungarian_matrix, iftMatrix *constraints){
    for (int r = 0; r < constraints->nrows; r++){
        for (int c = 0; c < constraints->ncols; c++){
            if (iftMatrixElem(constraints, c, r))
                iftMatrixElem(hungarian_matrix, c, r) = IFT_INFINITY_FLT;
        }
    }
}

iftMatrix *createHungarianMatrix(iftMatrix *input_matrix, iftMatrix *constraints, int optimization_type){
    /*
    Creates a Hungarian minimization matrix from the input matrix and the optimization type.
    If the optimization type is 'max', we must transform the input matrix in its correspondent minimization matrix using the following equation:
    A = max(A) - A
    */

    iftMatrix *hungarian_matrix = iftCopyMatrix(input_matrix);
    if (optimization_type == 1) // we need to adapt input matrix to maximize
        minimizeHungarianMatrix(hungarian_matrix);

    if (!isSquareMatrix(hungarian_matrix)){
        iftMatrix *aux = balanceHungarianMatrix(hungarian_matrix);
        iftDestroyMatrix(&hungarian_matrix);
        hungarian_matrix = aux;
    }

    applyConstraintsToHungarianMatrix(hungarian_matrix, constraints);

    return hungarian_matrix;
}

void subtractRowMinimum(iftMatrix *hungarian_matrix){
    // Subtract row minimum from each row
    for (int r = 0; r < hungarian_matrix->nrows; r++){
        float row_min = IFT_INFINITY_FLT;

        // Find row minimum
        for (int c = 0; c < hungarian_matrix->ncols; c++){
            float matrix_elem = iftMatrixElem(hungarian_matrix, c, r);
            if (row_min > matrix_elem) row_min = matrix_elem;
        }
        // Subtract by row min
        for (int c = 0; c < hungarian_matrix->ncols; c++)
            iftMatrixElem(hungarian_matrix, c, r) -= row_min;
    }
}

void subtractColMinimum(iftMatrix *hungarian_matrix){
    // Subtract row minimum from each row
    for (int c = 0; c < hungarian_matrix->ncols; c++){
        float col_min = IFT_INFINITY_FLT;

        // Find col minimum
        for (int r = 0; r < hungarian_matrix->nrows; r++){
            float matrix_elem = iftMatrixElem(hungarian_matrix, c, r);
            if (col_min > matrix_elem) col_min = matrix_elem;
        }
        // Subtract by col min
        for (int r = 0; r < hungarian_matrix->nrows; r++)
            iftMatrixElem(hungarian_matrix, c, r) -= col_min;
    }
}

void subtractRowAndColMinimum(iftMatrix *hungarian_matrix){
    subtractRowMinimum(hungarian_matrix);
    subtractColMinimum(hungarian_matrix);
}

bool almostZero(float f){
    return ((f <= 1E-4) && (f >= -1E-4));
}

bool isThereZerosLeft(iftMatrix *aux){
    for (int p = 0; p < aux->n; p++){
        if (almostZero(aux->val[p])) return true;
    }
    return false;
}



bool isUniqueZero(iftMatrix *aux, int c, int r){
    if (!(almostZero(iftMatrixElem(aux, c, r))))
        return false; // matrix elem is not zero, so its not a unique zero
    
    // checking in the row
    int row_flag = 1;
    for (int j = 0; j < aux->ncols; j++){
        if (j != c){
            if (almostZero(iftMatrixElem(aux, j, r))){
                row_flag = 0;
                break;
            }
        }
    }
    if (row_flag)
        return true;
    
    // checking in the col
    int col_flag = 1;
    for (int i = 0; i < aux->nrows; i++){
        if (i != r){
            if (almostZero(iftMatrixElem(aux, c, i))){
                col_flag = 0;
                break;
            }
                
        }
    }
    if (col_flag)
        return true;
    
    return false;
}

void markAllUniqueZeros(iftMatrix *aux, iftMatrix *assignement){
    /* Function to mark all unique zeros until its impossible to mark any more */

    int n_marked = 1; // flag to check whether a unique zero is marked in current iteration
    while(n_marked){
        n_marked = 0;
        for (int r = 0; r < aux->nrows; r++){
            for (int c = 0; c < aux->ncols; c++){
                if (isUniqueZero(aux, c, r)){
                    iftMatrixElem(assignement, c, r) = 1;
                    // iftMatrixElem(aux, c, r)         = 1;
                    
                    /* Block the row and the column of the aux matrix */
                    for (int j = 0; j < aux->ncols; j++) iftMatrixElem(aux, j, r) = -1;
                    for (int i = 0; i < aux->nrows; i++) iftMatrixElem(aux, c, i) = -1;
                    
                    n_marked = 1;

                    /* no need to keep checking the given row so we break*/
                    break;
                }
            }
        }
    }
     
}

void markFirstNonUniqueZero(iftMatrix *aux, iftMatrix *assignement){
    for (int r = 0; r < aux->nrows; r++){
        for (int c = 0; c < aux->ncols; c++){
            if (almostZero(iftMatrixElem(aux, c, r))){ /* We found the first non unique zero */
                /* Mark the assignement */
                iftMatrixElem(assignement, c, r) = 1;

                /* Block the row and the column of the aux matrix*/
                for (int j = 0; j < aux->ncols; j++) iftMatrixElem(aux, j, r) = -1;
                for (int i = 0; i < aux->nrows; i++) iftMatrixElem(aux, c, i) = -1;
                return;  
            }
        }
    }
}

iftMatrix *attemptAssignement(iftMatrix *hungarian_matrix){
    /*
    Attempts to find an assignment in a hungarian matrix. The attempted assignment is return as a boolean matrix with same shape as input where the 1s represent
    a given assignment.

    This algorithm iteratively goes through the matrix trying to find unique zeros to chose. It stops when there are no more available zeros in the matrix
    */
   iftMatrix *aux         = iftCopyMatrix(hungarian_matrix);
   iftMatrix *assignement = iftCreateMatrix(aux->ncols, aux->nrows);
   while(isThereZerosLeft(aux)){
        markAllUniqueZeros(aux, assignement);

        /* if the problem isnt solved yet, we need to mark the first safe non unique zero*/
        if(isThereZerosLeft(aux))
            markFirstNonUniqueZero(aux, assignement);
   }

   iftDestroyMatrix(&aux);
   return assignement;

}

void tickNoAssignementRows(iftMatrix *assignement, int *ticked_rows){
    for (int r = 0; r < assignement->nrows; r++){
        int tick_flag = 1;
        for (int c = 0; c < assignement->ncols; c++){
            if (iftMatrixElem(assignement, c, r) != 0){
                tick_flag = 0;
                break;
            }
        }
        if (tick_flag)
            ticked_rows[r] = 1;
    }
}

bool tickColumnsWithZerosInTickedRows(iftMatrix *hungarian_matrix, int *ticked_rows, int *ticked_cols){
    bool colTickFlag = false;

    for (int r = 0; r < hungarian_matrix->nrows; r++){
        if (ticked_rows[r]){
            /* Check the columns that have a zero in the given ticked row*/
            for (int c = 0; c < hungarian_matrix->ncols; c++){
                if (ticked_cols[c] == 0){ /* we are only ticking if hasnt been ticked yet */
                    if (almostZero(iftMatrixElem(hungarian_matrix, c, r))){
                        ticked_cols[c] = 1;
                        colTickFlag    = true;
                    }
                }
            }
        }
    }
    return colTickFlag;
}

bool tickRowsWithAssignementInTickedCols(iftMatrix *assignement, int *ticked_rows, int *ticked_cols){
    bool rowTickFlag = false;

    for (int c = 0; c < assignement->ncols; c++){
        if (ticked_cols[c]){
            /* Check the rows that have an assignement in the given ticked col */
            for (int r = 0; r < assignement->nrows; r++){
                if (ticked_rows[r] == 0){ /* we are only ticking if hasnt been ticked yet */
                    if (iftMatrixElem(assignement, c, r) != 0){
                        ticked_rows[r] = 1;
                        rowTickFlag    = true;
                    }
                }
            }
        }
    }
    return rowTickFlag;
} 

void tickUntilNoMoreTicks(iftMatrix *hungarian_matrix, iftMatrix *assignement, int *ticked_rows, int *ticked_cols){
    bool ticks = true;
    while(ticks){
        bool colTickFlag = tickColumnsWithZerosInTickedRows(hungarian_matrix, ticked_rows, ticked_cols);
        bool rowTickFlag = tickRowsWithAssignementInTickedCols(assignement, ticked_rows, ticked_cols);
        ticks = (colTickFlag || rowTickFlag);
    }
}

int minimumLineCoverage(int *ticked_rows, int *ticked_cols, iftMatrix *covered_lines){
    int colSum = 0, rowSum = 0;
    /* Draw lines through ticked columns */
    for (int c = 0; c < covered_lines->ncols; c++){
        if (ticked_cols[c]){
            colSum++;
            for (int r = 0; r < covered_lines->nrows; r++) iftMatrixElem(covered_lines, c, r) = 1;
        }
    }

    /* Draw lines through UNticked rows */
    for (int r = 0; r < covered_lines->nrows; r++){
        if (!ticked_rows[r]){
            rowSum++;
            for (int c = 0; c < covered_lines->ncols; c++) iftMatrixElem(covered_lines, c, r) = 1;
        }
    }

    return rowSum + colSum;
}

void modifyHungarianMatrix(iftMatrix *hungarian_matrix, iftMatrix *covered_lines, int *ticked_rows, int *ticked_cols){
    float min_uncovered_value = IFT_INFINITY_FLT;
    for (int i = 0; i < hungarian_matrix->n; i++){
        if ((!covered_lines->val[i]) && (min_uncovered_value > hungarian_matrix->val[i]))
            min_uncovered_value = hungarian_matrix->val[i];
    }

    for (int r = 0; r < hungarian_matrix->nrows; r++){
        if (ticked_rows[r]){ /* Subtract every element of column by min value*/
            for (int c = 0; c < hungarian_matrix->ncols; c++){
                iftMatrixElem(hungarian_matrix, c, r) -= min_uncovered_value;
            }
        }
    }

    for (int c = 0; c < hungarian_matrix->ncols; c++){
        if (ticked_cols[c]){ /* Add every element of row by min value*/
            for (int r = 0; r < hungarian_matrix->nrows; r++){
                iftMatrixElem(hungarian_matrix, c, r) += min_uncovered_value;
            }
        }
    }

}

iftMatrix *hungarianSolver(iftMatrix *input_matrix, iftMatrix *constraints, int optimization_type){
    iftMatrix *hungarian_matrix = createHungarianMatrix(input_matrix, constraints, optimization_type);
    iftMatrix *assignement      = NULL;

    subtractRowAndColMinimum(hungarian_matrix);

    int line_coverage = 0;
    int matrix_dim    = hungarian_matrix->nrows;

    /* start hungarian iterations */

    while(line_coverage != matrix_dim){
        assignement = attemptAssignement(hungarian_matrix);

        /* Find minimum line coverage from given assignement */
        int *ticked_rows = (int *)calloc(matrix_dim, sizeof(int));
        int *ticked_cols = (int *)calloc(matrix_dim, sizeof(int));
        
        tickNoAssignementRows(assignement, ticked_rows);
        tickUntilNoMoreTicks(hungarian_matrix, assignement, ticked_rows, ticked_cols);

        iftMatrix *covered_lines = iftCreateMatrix(matrix_dim, matrix_dim);
        line_coverage            = minimumLineCoverage(ticked_rows, ticked_cols, covered_lines);

        if (line_coverage != matrix_dim){
            modifyHungarianMatrix(hungarian_matrix, covered_lines, ticked_rows, ticked_cols);
            iftDestroyMatrix(&assignement);
        }

        free(ticked_rows);
        free(ticked_cols);
        iftDestroyMatrix(&covered_lines);
        
    }

    iftDestroyMatrix(&hungarian_matrix);

    return assignement;


}

typedef struct _mnt_pair{
    int latent_id, reference_id;
    float score;
} MntPair;


iftMatrix *calculateSimilarityMatrix(iftMatrix *latent_feature_vector, iftMatrix *reference_feature_vector){
    iftMatrix *aux               = iftTransposeMatrix(reference_feature_vector);
    iftMatrix *similarity_matrix = iftMultMatrices(latent_feature_vector, aux);
    iftDestroyMatrix(&aux);

    return similarity_matrix;
}

iftMatrix *defineProblemConstraints(iftMatrix *similarity_matrix, float threshold){
    iftMatrix *constraints = iftCreateMatrix(similarity_matrix->ncols, similarity_matrix->nrows);


    /* Defining constraints */
    for (int v = 0; v < constraints->n; v++){
        if (similarity_matrix->val[v] < threshold) constraints->val[v] = 1;
        else constraints->val[v] = 0;
    }

    return constraints;

}

void initializeScoresArray(int n, float **scores, int **idxs){
    *scores = (float *)calloc(n, sizeof(float));
    *idxs   = (int *)calloc(n, sizeof(int));
    for (int k = 0; k < n; k++) (*idxs)[k] = k;
}

void initializeDoubleScoresArray(int n, double **scores, int **idxs){
    *scores = (double *)calloc(n, sizeof(double));
    *idxs   = (int *)calloc(n, sizeof(int));
    for (int k = 0; k < n; k++) (*idxs)[k] = k;
}

MntPair *decodeHungarianSolution(iftMatrix *similarity_matrix, iftMatrix *assignement){
    /* Function to get the ordered pairs of minutias, along with theirs scores from the similarity matrix and from the assignement */

    float *pair_matches;
    int *pair_idxs;
    initializeScoresArray(similarity_matrix->nrows, &pair_matches, &pair_idxs);

    /* Getting latent and reference id, along with score */
    int *latent_id    = (int *)calloc(similarity_matrix->nrows, sizeof(int));
    int *reference_id = (int *)calloc(similarity_matrix->nrows, sizeof(int));
    for (int r = 0; r < similarity_matrix->nrows; r++){
        for (int c = 0; c < similarity_matrix->ncols; c++){
            if(iftMatrixElem(assignement, c, r) != 0){
                latent_id[r]        = r;
                reference_id[r]     = c;
                pair_matches[r]     = iftMatrixElem(similarity_matrix, c, r);
                break;
            }
        }
    }

    iftFQuickSort(pair_matches, pair_idxs, 0, similarity_matrix->nrows - 1, IFT_DECREASING);

    /* Saving ordered scores */
    MntPair *mnts_pairs = (MntPair *)calloc(similarity_matrix->nrows, sizeof(MntPair));
    for (int i = 0; i < similarity_matrix->nrows; i++){
        mnts_pairs[i].latent_id    = latent_id[pair_idxs[i]];
        mnts_pairs[i].reference_id = reference_id[pair_idxs[i]];
        mnts_pairs[i].score        = pair_matches[i];
    }

    iftFree(latent_id);
    iftFree(reference_id);
    iftFree(pair_matches);
    iftFree(pair_idxs);

    return mnts_pairs;

}

float calculateMatchScore(int n_latent_mnts, MntPair *mnts_pairs){
    int k_matches_to_consider = (n_latent_mnts < K_MATCHES) ? n_latent_mnts : K_MATCHES;


    /* Calculating score */
    float pair_score = 0.0;
    for (int m = 0; m < k_matches_to_consider; m++) pair_score += mnts_pairs[m].score;
    // for (int m = 0; m < k_matches_to_consider; m++){
    //     float tmp = 0.0;
    //     for (int i = 0; i <= m; i++){
    //         tmp += mnts_pairs[i].score;
    //     }
    //     tmp /= (m + 1);
    //     pair_score += tmp;
    // } 
    pair_score /= k_matches_to_consider;

    return pair_score;
}

typedef struct _mnt{
    int id;
    int x, y;
    double theta;
} MinutiaPoint;

// typedef struct _segment{
//     int p1;
//     int p2;
// }MinutiaSegment;


MinutiaPoint *readMntFile(char *filename, int *n){
    int xsize, ysize;
    char mnt_filename[50];

    FILE *fp = fopen(filename, "r");

    /* Reading mnt file header */
    fscanf(fp, "%s\n", mnt_filename); // minutia filename
    fscanf(fp, "%d %d %d\n", n, &xsize, &ysize);

    /* Reading minutiae points */
    MinutiaPoint *mnts = (MinutiaPoint *)calloc(*n, sizeof(MinutiaPoint));
    float score;
    for (int i = 0; i < *n; i++){
        mnts[i].id = i;
        fscanf(fp, "%d %d %lf %f\n", &mnts[i].x, &mnts[i].y, &mnts[i].theta, &score);
    }

    fclose(fp);

    return mnts;
}

void printMinutiaPoints(int n, MinutiaPoint *mnts){
    for (int i = 0; i < n; i++){
        printf("%d %d %d %lf\n",  mnts[i].id, mnts[i].x, mnts[i].y, mnts[i].theta);
    }
}

MinutiaPoint *getKNearestMinutia(int mnt_id, int n_mnts, MinutiaPoint *mnt_set, int K){
    MinutiaPoint center_minutia = mnt_set[mnt_id];

    double *distances;
    int *idxs;
    initializeDoubleScoresArray(n_mnts, &distances, &idxs);

    for (int i = 0; i < n_mnts; i++){
        distances[i] = sqrt((center_minutia.x - mnt_set[i].x) * (center_minutia.x - mnt_set[i].x) + (center_minutia.y - mnt_set[i].y) * (center_minutia.y - mnt_set[i].y));
    }

    iftDQuickSort(distances, idxs, 0, n_mnts - 1, IFT_INCREASING);

    /* Returning the K nearest minutiae */
    MinutiaPoint *k_nearest_mnt = (MinutiaPoint *)calloc(K, sizeof(MinutiaPoint));

    for (int i = 1; i < K + 1; i++){
      k_nearest_mnt[i - 1].id = mnt_set[idxs[i]].id;
      k_nearest_mnt[i - 1].x = mnt_set[idxs[i]].x;
      k_nearest_mnt[i - 1].y = mnt_set[idxs[i]].y;
      k_nearest_mnt[i - 1].theta = mnt_set[idxs[i]].theta;
    }

    iftFree(distances);
    iftFree(idxs);
    

    return k_nearest_mnt;

}

double lengthSegment(MinutiaPoint segment[2]){
    return sqrt((segment[0].x - segment[1].x) * (segment[0].x - segment[1].x) + (segment[0].y - segment[1].y) * (segment[0].y - segment[1].y));
}

double segmentLengthSimilarity(MinutiaPoint latent_segment[2], MinutiaPoint reference_segment[2]){
    double lat_len = lengthSegment(latent_segment);
    double ref_len = lengthSegment(reference_segment);

    // double length_score = iftMin(lat_len, ref_len)/(iftMax(lat_len, ref_len));

    double length_score = iftMin(lat_len, ref_len)/(iftMax(lat_len, ref_len));
    // length_score = 1.0/(1.0 + exp(-1000 * (length_score - 0.90)));
    length_score = length_score < 0.7 ? 0.0 : 1.0;


    return length_score;
}



double segmentAngle(MinutiaPoint segment[2]){
    return atan2((segment[0].y - segment[1].y), (segment[0].x - segment[1].x));
}

double segmentAngleSimilarity(MinutiaPoint latent_segment[2], MinutiaPoint reference_segment[2]){
    double alpha_1       = latent_segment[0].theta;
    double alpha_1_prime = reference_segment[0].theta;
    double alpha_2       = latent_segment[1].theta;
    double alpha_2_prime = reference_segment[1].theta;

    double beta       = segmentAngle(latent_segment);
    double beta_prime = segmentAngle(reference_segment);

    double first_term  = cos(fabs((alpha_1 - beta) - (alpha_1_prime - beta_prime)));
    double second_term = cos(fabs((alpha_2 - beta) - (alpha_2_prime - beta_prime)));

    double angle_score = (first_term) * (second_term);
    // double angle_score = pow((first_term) * (second_term), 5.0);

    if ((first_term < 0) || (second_term < 0)) angle_score = 0;

    // if ((first_term < 0.965) || (second_term < 0.965)) angle_score = 0;
    return angle_score;
}

double segmentDeepFeaturesSimilarity(MinutiaPoint latent_segment[2], MinutiaPoint reference_segment[2], iftMatrix *similarity_matrix){
    double deep_feature_score = iftMatrixElem(similarity_matrix, reference_segment[1].id, latent_segment[1].id);
    return deep_feature_score;
}

double segmentSimilarityScore(MinutiaPoint latent_segment[2], MinutiaPoint reference_segment[2], MinutiaPoint *latent_mnts, MinutiaPoint *reference_mnts, iftMatrix *similarity_matrix){
    double score = 1.0;

    score *= segmentLengthSimilarity(latent_segment, reference_segment);
    score *= segmentAngleSimilarity(latent_segment, reference_segment);
    score *= segmentDeepFeaturesSimilarity(latent_segment, reference_segment, similarity_matrix);


    return score;
}

double getFinalSpatialSimilarityScore(double *scores){
    double score = 1.0;
    for (int i = 0; i < NVIZ; i++) score *= scores[i];

    return score;
}

double spatialSimilarityScore(int latent_mnt_id, int reference_mnt_id, int n_latent_mnts, int n_reference_mnts, MinutiaPoint *latent_mnts, MinutiaPoint *reference_mnts, iftMatrix *similarity_matrix){
    /*
    Calculates the spatial domain similarity between a latent and a reference minutiae.
    The spatial similarity is calculated upon three criteria:
        * segment legth similarity
        * segment angle similarity
        * segment minutiae similarity
    Inputs:
        - latent_mnt_id, reference_mnt_id: the latent and reference minutiae for which we want to calculate the similarity (int - index)
        - latent_mnt_set, reference_mnt_set: the latent and referece minutiae set for the given images (list of minutiae)
        - similarity matrix: matrix containing the similarity between every minutiae in the latent and reference (np matrix)
        - Kl: Kl nearest neighbours in latent to construct segments (int)
        - Kr: Kr nearest neighbours in reference to construct segments (int)
        - Nviz: number of best segment matches to consider (int)
    
    Output: spatial similarity matrix
    */

   if (NVIZ > KL) iftError("Amount of segments to consider must be less or equal to the amount of segments", "spatialSimilarityScore");


    /* Gettint the nearest neighbours from latent minutiaes */
    MinutiaPoint *latent_nearest_mnts    = getKNearestMinutia(latent_mnt_id, n_latent_mnts, latent_mnts, KL);
    MinutiaPoint *reference_nearest_mnts = getKNearestMinutia(reference_mnt_id, n_reference_mnts, reference_mnts, KR);

    /* Array containing the score of all segment pairs */
    double *scores;
    int *idxs;
    initializeDoubleScoresArray(KL * KR, &scores, &idxs);


    /* Iterating through every segment pair */
    int k = 0;
    for (int i = 0; i < KL; i++){
        MinutiaPoint latent_segment[2] = { latent_mnts[latent_mnt_id], latent_nearest_mnts[i] };
        for (int j = 0; j < KR; j++){
            MinutiaPoint reference_segment[2] = { reference_mnts[reference_mnt_id], reference_nearest_mnts[j] };
            scores[k] = segmentSimilarityScore(latent_segment, reference_segment, latent_mnts, reference_mnts, similarity_matrix);
            k++;
        }
    }

    /* Getting the Nviz best segment scores */
    iftDQuickSort(scores, idxs, 0, KR * KL - 1, IFT_DECREASING);
    double final_score = getFinalSpatialSimilarityScore(scores);


    free(latent_nearest_mnts);
    free(reference_nearest_mnts);
    free(scores);
    free(idxs);
    return final_score;
}

iftMatrix *calculateSpatialCoherentSimilarityMatrix(iftMatrix *similarity_matrix, int n_latent_mnts, int n_reference_mnts, MinutiaPoint *latent_mnts, MinutiaPoint *reference_mnts){
    iftMatrix *spatial_coherent_similarity_matrix = iftCopyMatrix(similarity_matrix);
    for (int i = 0; i < similarity_matrix->nrows; i++){
        for (int j = 0; j < similarity_matrix->ncols; j++){
            double spatial_similarity_score = spatialSimilarityScore(i, j, n_latent_mnts, n_reference_mnts, latent_mnts, reference_mnts, similarity_matrix);
            iftMatrixElem(spatial_coherent_similarity_matrix, j, i) += spatial_similarity_score;
            iftMatrixElem(spatial_coherent_similarity_matrix, j, i) /= 2.0;

        }
    }

    return spatial_coherent_similarity_matrix;
}

int main(int argc, char *argv[])
{
    timer *tstart=NULL;
    char   filename[400];

    /*--------------------------------------------------------*/

    void *trash = malloc(1);                 
    struct mallinfo info;   
    int MemDinInicial, MemDinFinal;
    free(trash); 
    info = mallinfo();
    MemDinInicial = info.uordblks;

    /*--------------------------------------------------------*/


    if (argc != 6) {
    printf("usage iftHungarianIdentification <P1> <P2> <P3>\n");
    printf("P1: folder with query database features\n");
    printf("P2: folder with query database minutiaes\n");
    printf("P3: folder with reference database features\n");
    printf("P4: folder with reference database minutiaes\n");
    printf("P5: output folder with identification results\n");
    exit(0);
    }


    tstart = iftTic();

    iftFileSet *query_fs    = iftLoadFileSetFromDirBySuffix(argv[1],".npy", 1);
    iftFileSet *ref_fs      = iftLoadFileSetFromDirBySuffix(argv[3],".npy", 1);

    char *out_dir     = argv[5];
    iftMakeDir(out_dir);

    for (int i = 0; i < query_fs->n; i++){
        printf("Progress: %d/%ld\n", i + 1, query_fs->n);
        char *latent      = iftFilename(query_fs->files[i]->path, ".npy");

        /* Read latent information feature vector */
        sprintf(filename, "%s/%s.npy", argv[1], latent);
        iftMatrix *latent_feature_vector = iftReadMatrix(filename);

        /* Read latent minutiae */
        sprintf(filename, "%s/%s.mnt", argv[2], latent);
        int n_latent_mnts;
        MinutiaPoint *latent_mnts        = readMntFile(filename, &n_latent_mnts);

        /* Create matches Folder */
        sprintf(filename, "%s/%s/matches", out_dir, latent);
        iftMakeDir(filename);

        /* Create scores file */
        sprintf(filename, "%s/%s/score.txt", out_dir, latent);
        FILE *score   = fopen(filename, "w");

        /* Array of scores - one for each reference, and reference idx for sorting */
        float *reference_scores;
        int   *reference_scores_idxs;
        initializeScoresArray(ref_fs->n, &reference_scores, &reference_scores_idxs);

        for (int j = 0; j < ref_fs->n; j++){

            /* Opening match file */
            char *reference    = iftFilename(ref_fs->files[j]->path, ".npy");
            sprintf(filename, "%s/%s/matches/%s_%s.txt", out_dir, latent, latent, reference);
            FILE *match_file   = fopen(filename, "w");

            /* Reading reference feature vector */
            sprintf(filename, "%s/%s.npy", argv[3], reference);
            iftMatrix *reference_feature_vector = iftReadMatrix(filename);

            /* Reading reference mnts */
            sprintf(filename, "%s/%s.mnt", argv[4], reference);

            // printf("filename: %s\n", filename);
            int n_reference_mnts;
            MinutiaPoint *reference_mnts        = readMntFile(filename, &n_reference_mnts);


            // printf("Latent:\n");
            // for(int x = 0; x < n_latent_mnts; x++){
            //     printf("%d %d %lf\n", latent_mnts[x].x,latent_mnts[x].y, latent_mnts[x].theta);
            // }

            // printf("#################################\n");

            // printf("Reference:\n");
            // for(int x = 0; x < n_reference_mnts; x++){
            //     printf("%d %d %lf\n", reference_mnts[x].x,reference_mnts[x].y, reference_mnts[x].theta);
            // }

            /* Solve assignement problem */
            iftMatrix *similarity_matrix                  = calculateSimilarityMatrix(latent_feature_vector, reference_feature_vector);
            iftMatrix *constraints                        = defineProblemConstraints(similarity_matrix, SIMILARITY_THRESHOLD);
            iftMatrix *spatial_coherent_similarity_matrix = calculateSpatialCoherentSimilarityMatrix(similarity_matrix, n_latent_mnts, n_reference_mnts, latent_mnts, reference_mnts);
            iftMatrix *assignement                        = hungarianSolver(spatial_coherent_similarity_matrix, constraints, MAXIMIZATION_PROBLEM);
            MntPair *decoded_pairs                        = decodeHungarianSolution(spatial_coherent_similarity_matrix, assignement);
            reference_scores[j]                           = calculateMatchScore(similarity_matrix->nrows, decoded_pairs);


            sprintf(filename, "similarity_matrices/%s_%s.npy", iftFilename(query_fs->files[i]->path, ".npy"), iftFilename(ref_fs->files[j]->path, ".npy"));
            iftWriteMatrix(similarity_matrix, filename);
            
            /* Saving the best K matches */
            for (int m = 0; m < similarity_matrix->nrows; m++){
                fprintf(match_file, "%d %d %f\n", decoded_pairs[m].latent_id, decoded_pairs[m].reference_id, decoded_pairs[m].score);
            }

            free(decoded_pairs);
            free(reference);
            free(reference_mnts);
            fclose(match_file);    
            iftDestroyMatrix(&similarity_matrix);
            iftDestroyMatrix(&spatial_coherent_similarity_matrix);
            iftDestroyMatrix(&assignement);
            iftDestroyMatrix(&constraints);
            iftDestroyMatrix(&reference_feature_vector);

        }
        /* Sorting score file by scores */
        iftFQuickSort(reference_scores, reference_scores_idxs, 0, ref_fs->n - 1, IFT_DECREASING);

        /* Write scores file in a ordered manner */
        for (int j = 0; j < ref_fs->n; j++){
            char *reference    = iftFilename(ref_fs->files[reference_scores_idxs[j]]->path, ".npy");
            fprintf(score, "%s %s %f\n", latent, reference, reference_scores[j]);
            free(reference);
        }


        fclose(score);
        free(latent);
        free(latent_mnts);
        iftFree(reference_scores);
        iftFree(reference_scores_idxs);
        iftDestroyMatrix(&latent_feature_vector);

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








