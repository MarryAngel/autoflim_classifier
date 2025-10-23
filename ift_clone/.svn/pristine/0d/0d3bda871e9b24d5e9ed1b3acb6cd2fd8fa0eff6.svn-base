/*
Unicamp - 07-06-2023
Author: André Igor Nóbrega da Silva
email:  a203758@dac.unicamp.br

This module solves the graph matching assignement problem using the Hungarian method to match a set of n vertices in partition A with m vertices
in partition B. This match is made based on the hungarian matrix that contains the value (or cost) of each pair (Ai, Bj).

Input: input matrix, constraints, optimization type
Output: assignement matrix
*/

#include "ift.h"
#include <math.h>
#include <stdbool.h>

#define MAX(a, b) a > b ? a : b

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


int main(int argc, char *argv[])
{
    timer *tstart=NULL;
    char   filename[100];

    /*--------------------------------------------------------*/

    void *trash = malloc(1);                 
    struct mallinfo info;   
    int MemDinInicial, MemDinFinal;
    free(trash); 
    info = mallinfo();
    MemDinInicial = info.uordblks;

    /*--------------------------------------------------------*/


    if (argc != 4) {
    printf("usage iftHungarianMatching <P1> <P2> <P3>\n");
    printf("P1: input matrix (.npy)\n");
    printf("P2: constraints matrix (.npy) \n");
    printf("P3: optimization type (0: minimization, 1: maximization)\n");
    exit(0);
    }

    tstart = iftTic();

    iftMatrix *input_matrix       = iftReadMatrix(argv[1]);
    iftMatrix *constraints_matrix = iftReadMatrix(argv[2]);
    int optimization_type         = atoi(argv[3]);

    iftMatrix *assignement = hungarianSolver(input_matrix, constraints_matrix, optimization_type);

    // float *matches = (float *)calloc(input_matrix->nrows, sizeof(float));
    // int *idxs      = (int *)calloc(input_matrix->nrows, sizeof(int));

    // for (int i = 0; i < input_matrix->nrows; i++) idxs[i] = i;

    // for (int r = 0; r < input_matrix->nrows; r++){
    //     for (int c = 0; c < input_matrix->ncols; c++){
    //         if(iftMatrixElem(assignement, c, r) != 0){
    //             matches[r] = iftMatrixElem(input_matrix, c, r);
    //             break;
    //         }
    //     }
    // }

    float *pair_matches = (float *)calloc(input_matrix->nrows, sizeof(float));
    int *pair_idxs      = (int *)calloc(input_matrix->nrows, sizeof(int));
    MntPair *mnts_pairs = (MntPair *)calloc(input_matrix->nrows, sizeof(MntPair));

    for (int i = 0; i < input_matrix->nrows; i++) pair_idxs[i] = i;


    for (int r = 0; r < input_matrix->nrows; r++){
        for (int c = 0; c < input_matrix->ncols; c++){
            if(iftMatrixElem(assignement, c, r) != 0){
                mnts_pairs[r].latent_id = r;
                mnts_pairs[r].reference_id = c;
                mnts_pairs[r].score = iftMatrixElem(input_matrix, c, r);
                pair_matches[r]     = iftMatrixElem(input_matrix, c, r);
                break;
            }
        }
    }

    iftWriteMatrix(assignement, "diff_assignement.npy");

    iftFQuickSort(pair_matches, pair_idxs, 0, input_matrix->nrows - 1, IFT_DECREASING);


    for (int m = 0; m < input_matrix->nrows; m++){
        printf("%d %d %f\n", mnts_pairs[pair_idxs[m]].latent_id, mnts_pairs[pair_idxs[m]].reference_id, mnts_pairs[pair_idxs[m]].score);
    }

    /* Calculating score */
    float pair_score = 0.0;
    for (int m = 0; m < 10; m++) pair_score += pair_matches[m];
    pair_score /= 10;

    printf("pair score: %f\n", pair_score);

   
    iftDestroyMatrix(&input_matrix);
    iftDestroyMatrix(&constraints_matrix);
    iftDestroyMatrix(&assignement);



    

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








