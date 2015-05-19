#ifndef DECODE_THREAD_H
#define DECODE_THREAD_H

#include "Raptor/raptorcode.h"
#include "global_var.h"
#include <arpa/inet.h>

void *decode_malloc(int buf_num, int buf_len);
int decode_func(DecodeBuffer_t *, EncodeBuffer_t *);

#endif // DECODE_THREAD_H
