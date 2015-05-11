#ifndef RAPTORCODE_H
#define RAPTORCODE_H

#include "def.h"
#include "matrix.h"

typedef struct {
	uint32 a;
	uint32 b;
	uint8 d;
}Triple_t;

typedef struct {
	uint32 K;
	uint32 S;
	uint32 L;
	uint32 H;
	Triple_t *trp;
	Matrix_t *Amat;
	Matrix_t *A_1mat;
}RParamEnc_t;

typedef struct RaptorParam_dec{
    uint32 K;
    uint32 S;
    uint32 L;
    uint32 H;
    uint32 N;
    Matrix_t *Amat;
    uint16 *list;
}RParamDec_t;

void *raptor_init(uint32 k, uint32 n, void *arg, int opt);
int raptor_reset(uint32 k, void *para, int opt);
int raptor_decode(RParamDec_t *para, uint8 *input, uint8 *output_return, uint32 T);
int raptor_encode(RParamEnc_t *para, uint32 R, uint8 *input, uint8 *intermediate, uint8 *output, uint32 size);
void raptor_free(void *arg, int opt);

#endif
