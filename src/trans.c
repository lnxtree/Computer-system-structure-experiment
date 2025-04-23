/* 
 * trans.c - 矩阵转置B=A^T
 *每个转置函数都必须具有以下形式的原型：
 *void trans（int M，int N，int a[N][M]，int B[M][N]）；
 *通过计算，块大小为32字节的1KB直接映射缓存上的未命中数来计算转置函数。
 */ 
#include <stdio.h>
#include "cachelab.h"
int is_transpose(int M, int N, int A[N][M], int B[M][N]);
char transpose_submit_desc[] = "Transpose submission";  //请不要修改“Transpose_submission”


void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{

//                          请在此处添加代码
//*************************************Begin********************************************************
    int block_size, i, j, k, l, tmp, diagonal;

    if (M == 32 && N == 32) {
        block_size = 8;
        for (i = 0; i < N; i += block_size) {
            for (j = 0; j < M; j += block_size) {
                for (k = i; k < i + block_size; k++) {
                    for (l = j; l < j + block_size; l++) {
                        if (k != l) {
                            B[l][k] = A[k][l];
                        } else {
                            tmp = A[k][l];
                            diagonal = k;
                        }
                    }
                    if (i == j) {
                        B[diagonal][diagonal] = tmp;
                    }
                }
            }
        }
    } else if (M == 64 && N == 64) {
        block_size = 4;
        for (i = 0; i < N; i += block_size) {
            for (j = 0; j < M; j += block_size) {
                for (k = i; k < i + block_size; k++) {
                    for (l = j; l < j + block_size; l++) {
                        if (k != l) {
                            B[l][k] = A[k][l];
                        } else {
                            tmp = A[k][l];
                            diagonal = k;
                        }
                    }
                    if (i == j) {
                        B[diagonal][diagonal] = tmp;
                    }
                }
            }
        }
    } else {
        block_size = 16;
        for (i = 0; i < N; i += block_size) {
            for (j = 0; j < M; j += block_size) {
                for (k = i; k < i + block_size && k < N; k++) {
                    for (l = j; l < j + block_size && l < M; l++) {
                        if (k != l) {
                            B[l][k] = A[k][l];
                        } else {
                            tmp = A[k][l];
                            diagonal = k;
                        }
                    }
                    if (i == j && k < M && k < N) {
                        B[diagonal][diagonal] = tmp;
                    }
                }
            }
        }
    }



//**************************************End**********************************************************
}

/* 
 * 我们在下面定义了一个简单的方法来帮助您开始，您可以根据下面的例子把上面值置补充完整。
 */ 

/* 
 * 简单的基线转置功能，未针对缓存进行优化。
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions-此函数向驱动程序注册转置函数。
 *在运行时，驱动程序将评估每个注册的函数并总结它们的性能。这是一种试验不同转置策略的简便方法。
 */
void registerFunctions()
{
    /* 注册解决方案函数  */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* 注册任何附加转置函数 */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - 函数检查B是否是A的转置。在从转置函数返回之前，可以通过调用它来检查转置的正确性。
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

