#ifndef GLOBAL_VAR_H
#define GLOBAL_VAR_H

#include "Raptor/def.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <limits.h>
#include <time.h>

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
extern char IP_default[];
extern int Port_default;
extern int Lost_default;
extern int K_default;
extern int T_default;

extern int sendfd;
extern int node_id;
extern int mode;
extern double rtt;
extern double time_dec;
extern double rate_loss;
extern double rate_fail;
extern double time_dec_temp;
extern double time_enc_temp;
/*********************************************************************/

#endif //GLOBAL_VAR_H

