#ifndef GLOBAL_VAR_H
#define GLOBAL_VAR_H

#include "Raptor/def.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

typedef struct FrameHeader_t
{
	int frame_no;
	long slice_no;
	int frame_type;
	long F;
	int T;
	int K;
	int R;
	int esi;
	int camera_no;
}FrameHeader_t;

#define T_MAX 1024
#define MAXLINE (sizeof(FrameHeader_t)+T_MAX)
#define MAXBUFSIZE 2

typedef struct
{
	void *arg1;
	void *arg2;
}DoubleArg_t;

/*********************************************************************/
typedef struct
{
	sem_t sem_empty;
	sem_t sem_full;
	int sig_put;
	int sig_get;
	uint8 **buffer;
}DecodeBuffer_t;

typedef struct
{
	uint8 *buffer;
	int frameNo;
	int size;
	int cameraNo;
}Frame_t;

typedef struct
{
	sem_t sem_empty;
	sem_t sem_full;
	int sig_put;
	int sig_get;
	Frame_t **frame;
}EncodeBuffer_t;

typedef struct
{
	sem_t sem_empty;
	sem_t sem_full;
	int sig_put;
	int sig_get;
	uint8 *buffer;
}SendBuffer_t;
/*********************************************************************/

#endif //GLOBAL_VAR_H
