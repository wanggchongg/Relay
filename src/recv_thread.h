#ifndef RECV_THREAD_H
#define RECV_THREAD_H

#include "global_var.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int recv_func(char *, int, DecodeBuffer_t *);

#endif //RECV_THREAD_H
