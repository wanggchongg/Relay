#ifndef SEND_THREAD_H
#define SEND_THREAD_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "global_var.h"

int send_func(uint8_t *, int, SendBuffer_t *);

#endif //SEND_THREAD_H
