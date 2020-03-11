/*
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */
#include <math.h>
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/*
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded.
 * 利用分块技术，教程https://blog.csdn.net/u013471946/article/details/43957423
 * 局部变量个数超过了12个，把bszie、x等暂存量替换掉可用省去不少，但是代码冗余，这里就免了
 * 答案解析：https://blog.csdn.net/xbb224007/article/details/81103995
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N]) {
  int i, j, k, kk;
  int bsize = 8;
  int x1,x2,x3,x4,x5,x6,x7,x8;
  if (N == 32) {
    bsize = 8;
  } else if (N == 64) {
    bsize = 4;
  } else {
    bsize = 16;
  }

  int x, y;
  if (N == 64) {
    for (i = 0; i < N; i += 8) {
      for (j = 0; j < M; j += 8) {
        for (x = i; x < i + 4; x++) {
          x1 = A[x][j];
          x2 = A[x][j + 1];
          x3 = A[x][j + 2];
          x4 = A[x][j + 3];
          x5 = A[x][j + 4];
          x6 = A[x][j + 5];
          x7 = A[x][j + 6];
          x8 = A[x][j + 7];

          B[j][x] = x1;
          B[j + 1][x] = x2;
          B[j + 2][x] = x3;
          B[j + 3][x] = x4;
          B[j][x + 4] = x5;
          B[j + 1][x + 4] = x6;
          B[j + 2][x + 4] = x7;
          B[j + 3][x + 4] = x8;
        }
        for (y = j; y < j + 4; y++) {
          x1 = A[i + 4][y];
          x2 = A[i + 5][y];
          x3 = A[i + 6][y];
          x4 = A[i + 7][y];
          x5 = B[y][i + 4];
          x6 = B[y][i + 5];
          x7 = B[y][i + 6];
          x8 = B[y][i + 7];

          B[y][i + 4] = x1;
          B[y][i + 5] = x2;
          B[y][i + 6] = x3;
          B[y][i + 7] = x4;
          B[y + 4][i] = x5;
          B[y + 4][i + 1] = x6;
          B[y + 4][i + 2] = x7;
          B[y + 4][i + 3] = x8;
        }
        for (x = i + 4; x < i + 8; x++) {
          x1 = A[x][j + 4];
          x2 = A[x][j + 5];
          x3 = A[x][j + 6];
          x4 = A[x][j + 7];
          B[j + 4][x] = x1;
          B[j + 5][x] = x2;
          B[j + 6][x] = x3;
          B[j + 7][x] = x4;
        }
      }
    }
    return;
  }
  // 32 * 32, 61 * 67
  for (kk = 0; kk < N; kk += bsize) {
    for (i = 0; i < M; i += bsize) {
      for (j = kk; j < N && j < kk + bsize; j++) {
        if (bsize == 8 && kk == i) {
          x1 = A[j][i];
          x2 = A[j][i + 1];
          x3 = A[j][i + 2];
          x4 = A[j][i + 3];
          x5 = A[j][i + 4];
          x6 = A[j][i + 5];
          x7 = A[j][i + 6];
          x8 = A[j][i + 7];
          B[i][j] = x1;
          B[i + 1][j] = x2;
          B[i + 2][j] = x3;
          B[i + 3][j] = x4;
          B[i + 4][j] = x5;
          B[i + 5][j] = x6;
          B[i + 6][j] = x7;
          B[i + 7][j] = x8;
          continue;
        }    
        for (k = i; k < M && k < i + bsize; k++) {
          B[k][j] = A[j][k];
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
void trans(int M, int N, int A[N][M], int B[M][N]) {
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
void registerFunctions() {
  /* Register your solution function */
  registerTransFunction(transpose_submit, transpose_submit_desc);

  /* Register any additional transpose functions */
  registerTransFunction(trans, trans_desc);
}

/*
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N]) {
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
