#ifndef ENCODE_THREAD_H
#define ENCODE_THREAD_H

#include "Raptor/raptorcode.h"
#include "global_var.h"
#include <arpa/inet.h>

void *encode_malloc(int buf_num, int buf_len);
int encode_func(EncodeBuffer_t *, SendBuffer_t *);

#endif //ENCODE_THREAD_H
