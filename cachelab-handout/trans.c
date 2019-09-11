/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    int out_i = 0;
    int out_j = 0;
    int in_i = 0;
    int in_j = 0;
    int temp[8];
    temp[0] = -1;
    
    if(N == 32){
        for(out_i=0;out_i<N;out_i+=8){
          for(out_j=0;out_j<M;out_j+=8){
              for(in_i=0;in_i<8;++in_i){
                  for(in_j=0;in_j<8;++in_j){
	              temp[in_j] = A[out_i+in_i][out_j+in_j];
                  }
		  for(in_j=0;in_j<8;++in_j){
	              B[out_j+in_j][out_i+in_i] = temp[in_j];
                  }
              }
          }
        }
    }
    else if(N == 64){
        for(out_i=0;out_i<N;out_i+=4){
          for(out_j=0;out_j<M;out_j+=4){
              for(in_i=0;in_i<4;++in_i){
                  for(in_j=0;in_j<4;++in_j){
	              temp[in_j] = A[out_i+in_i][out_j+in_j];
                  }
		  for(in_j=0;in_j<4;++in_j){
	              B[out_j+in_j][out_i+in_i] = temp[in_j];
                  }

              }
          }
        }
    }
    	
}

char transpose_test_desc[] = "Transpose test";
void transpose_test(int M, int N, int A[N][M], int B[M][N])
{
    int Block = 32;
    int out_i = 0;
    int out_j = 0;
    int in_i = 0;
    int in_j = 0;
    int y = 0;
    int x = -1;
    for(out_i=0;out_i<N;out_i+=Block){
        for(out_j=0;out_j<M;out_j+=Block){
            for(in_i=0;in_i<Block;++in_i){
                for(in_j=0;in_j<Block;++in_j){
		    if(out_i + in_i == out_j + in_j){
		    	x = out_i + in_i;
			y = A[x][x];
			continue;
		    }
                    B[out_j+in_j][out_i+in_i] = A[out_i+in_i][out_j+in_j];
                }
		if(x >= 0){
		    B[x][x] = y;
		    x = -1;
	        }
            }
        }
    }
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
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
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 
    registerTransFunction(transpose_test, transpose_test_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
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

