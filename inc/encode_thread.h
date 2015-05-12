#ifndef ENCODE_THREAD_H
#define ENCODE_THREAD_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include "Raptor/raptorcode.h"
#include "global_var.h"

void *encode_malloc(int buf_num, int buf_len);
int encode_func(EncodeBuffer_t *, SendBuffer_t *);

#endif //ENCODE_THREAD_H
