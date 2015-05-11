#ifndef MATRIX_H
#define MATRIX_H

#include "def.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define LEN_INT 4;
typedef struct {
	uint32 row;
	uint32 colum;
	uint8 **rowpoint;
}Matrix_t;

int matrix_init(uint32 rownum, uint32 columnum, Matrix_t *mat);
int matrix_reset(uint32 rownum, uint32 columnum, Matrix_t *mymat);
int matrix_inverse(Matrix_t *A, Matrix_t *A_1);
int matrix_equations(Matrix_t *A, uint8* D, uint32 size);

#endif
