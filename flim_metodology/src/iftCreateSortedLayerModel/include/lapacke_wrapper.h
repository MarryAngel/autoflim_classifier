#ifndef LAPACKE_WRAPPER_H
#define LAPACKE_WRAPPER_H

#include <stdlib.h>
#include <string.h>

#define LAPACK_ROW_MAJOR 101

// Fortran LAPACK declarations
extern void spotrf_(char *uplo, int *n, float *a, int *lda, int *info);
extern void spotri_(char *uplo, int *n, float *a, int *lda, int *info);
extern void sgetrf_(int *m, int *n, float *a, int *lda, int *ipiv, int *info);
extern void sgetri_(int *n, float *a, int *lda, int *ipiv, float *work, int *lwork, int *info);

// C wrappers mimicking LAPACKE interface
static inline int LAPACKE_spotrf(int matrix_layout, char uplo, int n, float *a, int lda) {
    int info;
    char uplo_f = uplo;
    spotrf_(&uplo_f, &n, a, &lda, &info);
    return info;
}

static inline int LAPACKE_spotri(int matrix_layout, char uplo, int n, float *a, int lda) {
    int info;
    char uplo_f = uplo;
    spotri_(&uplo_f, &n, a, &lda, &info);
    return info;
}

static inline int LAPACKE_sgetrf(int matrix_layout, int m, int n, float *a, int lda, int *ipiv) {
    int info;
    sgetrf_(&m, &n, a, &lda, ipiv, &info);
    return info;
}

static inline int LAPACKE_sgetri(int matrix_layout, int n, float *a, int lda, int *ipiv) {
    int info;
    int lwork = n * 64;
    float *work = malloc(lwork * sizeof(float));
    sgetri_(&n, a, &lda, ipiv, work, &lwork, &info);
    free(work);
    return info;
}

#endif