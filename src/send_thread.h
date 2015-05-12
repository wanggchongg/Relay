#ifndef SEND_THREAD_H
#define SEND_THREAD_H

#include "global_var.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

void *send_malloc(int buf_num, int buf_len);
int send_func(char *, int, SendBuffer_t *);

#endif //SEND_THREAD_H
