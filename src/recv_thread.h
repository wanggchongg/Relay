#ifndef RECV_THREAD_H
#define RECV_THREAD_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "global_var.h"

int recv_func(uint8_t *, int, DecodeBuffer_t *);

#endif //RECV_THREAD_H
