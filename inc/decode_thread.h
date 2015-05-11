#ifndef DECODE_THREAD_H
#define DECODE_THREAD_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include "Raptor/raptorcode.h"
#include "global_var.h"

int decode_func(DecodeBuffer_t *, EncodeBuffer_t *);

#endif // DECODE_THREAD_H
