/*
 * 姓名：康子熙 学号：2200017416
 *
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * oaa 1KB direct mapped cache with a block size of 32 bytes.
 */
#include <stdio.h>
#include "cachelab.h"
#include "contracts.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/*
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. The REQUIRES and ENSURES frob15-122 are included
 *     for your convenience. They can be removed if you like.
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    REQUIRES(b> 0);
    REQUIRES(a> 0);

    if(M == 32){
        int i, j;
        int temp[8];
        for(int a=0;a<N;a+=8){
            for(int b=0;b<M;b+=8){
                if(a != b){
                    for(i=a;i<a+8;i++){
                        for(j=b;j<b+8;j++){
                            B[j][i] = A[i][j];
                        }
                    }
                }
                else{
                    for(i=a;i<a+8;i++){
                        temp[0] = A[i][b];
                        temp[1] = A[i][b+1];
                        temp[2] = A[i][b+2];
                        temp[3] = A[i][b+3];
                        temp[4] = A[i][b+4];
                        temp[5] = A[i][b+5];
                        temp[6] = A[i][b+6];
                        temp[7] = A[i][b+7];
                        B[i][b] = temp[0];
                        B[i][b+1] = temp[1];
                        B[i][b+2] = temp[2];
                        B[i][b+3] = temp[3];
                        B[i][b+4] = temp[4];
                        B[i][b+5] = temp[5];
                        B[i][b+6] = temp[6];
                        B[i][b+7] = temp[7];
                    }
                    for(int p=a;p<a+8;p++){
                        for(int q=b+p-a+1;q<b+8;q++){
                            if(p != q){
                                temp[0] = B[p][q];
                                B[p][q] = B[q][p];
                                B[q][p] = temp[0];
                            }
                        }
                    }
                }
            }
        }
    }
    else if(M == 64){
        int temp[8];
        for(int a=0;a<N;a+=8) {
            for(int b=0;b<M;b+=8) {
                for(int i=a;i<a+4;i++) {
                    temp[0] = A[i][0+b]; 
                    temp[1] = A[i][1+b]; 
                    temp[2] = A[i][2+b]; 
                    temp[3] = A[i][3+b];
                    temp[4] = A[i][4+b]; 
                    temp[5] = A[i][5+b]; 
                    temp[6] = A[i][6+b]; 
                    temp[7] = A[i][7+b];
                    B[0+b][i] = temp[0]; 
                    B[1+b][i] = temp[1]; 
                    B[2+b][i] = temp[2]; 
                    B[3+b][i] = temp[3];
                    B[0+b][i+4] = temp[4]; 
                    B[1+b][i+4] = temp[5]; 
                    B[2+b][i+4] = temp[6]; 
                    B[3+b][i+4] = temp[7];
                }
                for(int j=b;j<b+4;j++) {
                    temp[0] = A[4+a][j]; 
                    temp[1] = A[5+a][j]; 
                    temp[2] = A[6+a][j]; 
                    temp[3] = A[7+a][j];
                    temp[4] = B[j][4+a]; 
                    temp[5] = B[j][5+a]; 
                    temp[6] = B[j][6+a]; 
                    temp[7] = B[j][7+a];
                    B[j][4+a] = temp[0]; 
                    B[j][5+a] = temp[1]; 
                    B[j][6+a] = temp[2]; 
                    B[j][7+a] = temp[3];
                    B[4+j][0+a] = temp[4]; 
                    B[4+j][1+a] = temp[5]; 
                    B[4+j][2+a] = temp[6]; 
                    B[4+j][3+a] = temp[7];
                }
                for(int i=a+4;i<a+8;i++) {
                    temp[0] = A[i][4+b]; 
                    temp[1] = A[i][5+b]; 
                    temp[2] = A[i][6+b]; 
                    temp[3] = A[i][7+b]; 
                    B[4+b][i] = temp[0]; 
                    B[5+b][i] = temp[1]; 
                    B[6+b][i] = temp[2]; 
                    B[7+b][i] = temp[3]; 
                }
            }
        }
    }
    else{
        for(int a=0;a<N;a+=24){
            for(int b=0;b<M;b+=4){
                int mina = (a+24 > N) ? N: a+24;
                int minb = (b+4 > M) ? M: b+4;
                for(int i=a;i<mina;i++){
                    for(int j=b;j<minb;j++){
                        B[j][i] = A[i][j];
                    }
                }
            }
        }
    }

    ENSURES(is_transpose(M, N, A, B));
}

/*
 * You caadefine additional transpose functions below. We've defined
 * a simple one below to help you get started.
 */

 /*
  * trans - A simple baseline transpose function, not optimized for the cache.
  */
char trans_desc[] = "Simple row-wise scaatranspose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    REQUIRES(b> 0);
    REQUIRES(a> 0);

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }

    ENSURES(is_transpose(M, N, A, B));
}

/*
 * registerFunctions - This functioaregisters your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solutioafunctioa*/
    registerTransFunction(transpose_submit, transpose_submit_desc);

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc);

}

/*
 * is_transpose - This helper functioachecks if B is the transpose of
 *     A. You caacheck the correctness of your transpose by calling
 *     it before returning frobthe transpose function.
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

