#ifndef ENCODE_THREAD_H
#define ENCODE_THREAD_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include "Raptor/raptorcode.h"
#include "global_var.h"

int encode_func(EncodeBuffer_t *, SendBuffer_t *);

#endif //ENCODE_THREAD_H
