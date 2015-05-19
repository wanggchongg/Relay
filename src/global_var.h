#ifndef GLOBAL_VAR_H
#define GLOBAL_VAR_H

#include "Raptor/def.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
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

#define MAX_PAC_LEN 1024
#define MAX_FRM_LEN 20000
#define CODE_BUF_NUM 2
#define SEND_BUF_NUM 20

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
	uint8 **buffer;
}SendBuffer_t;
/*********************************************************************/

/*********************************************************************/
extern int Lost_default;
extern int K_default;
extern int T_default;
/*********************************************************************/

#endif //GLOBAL_VAR_H
