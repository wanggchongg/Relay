#include "encode_thread.h"

static void *encode_thread(void *);

void *encode_malloc(int buf_num, int buf_len)
{
	EncodeBuffer_t *buf = NULL;
	if((buf = (EncodeBuffer_t *)malloc(sizeof(EncodeBuffer_t))) == NULL)
		return NULL;
	if((buf->frame = (Frame_t **)malloc(buf_num*sizeof(Frame_t *))) == NULL)
		return NULL;

	int i = 0;
	for(i=0; i<buf_num; i++)
	{
		if((buf->frame[i] = (Frame_t *)malloc(sizeof(Frame_t))) == NULL)
			return NULL;
		if((buf->frame[i]->buffer = (uint8 *)malloc(buf_len)) == NULL)
			return NULL;
		buf->frame[i]->frameNo = 0;
		buf->frame[i]->size = 0;
		memset(buf->frame[i]->buffer, 0, buf_len);
	}

	sem_init(&buf->sem_empty, 0, buf_num);
	sem_init(&buf->sem_full, 0, 0);
	buf->sig_put = 0;
	buf->sig_get = 0;

	return (void *)buf;
}

int encode_func(EncodeBuffer_t *encodeBuf, SendBuffer_t *sendBuf)
{
	int err;
	pthread_t tid;

	DoubleArg_t arg = {encodeBuf, sendBuf};
	err = pthread_create(&tid, NULL, encode_thread, &arg);
	if(err)
	{
		fprintf(stderr, "\tcan't create encode_thread: %s\n", strerror(err));
		return -2;
	}
	usleep(1000 * 200);
	return 0;
}

static void *encode_thread(void *arg)
{
	EncodeBuffer_t *encodeBuf = (EncodeBuffer_t *)((DoubleArg_t *)arg)->arg1;
	SendBuffer_t *sendBuf = (SendBuffer_t *)((DoubleArg_t *)arg)->arg2;
	pthread_detach(pthread_self());

	uint8 *frameBuf = NULL;
	int frameSize = 0;
	int frameNo = 0;
	int cameraNo = 0;

	FrameHeader_t *frameHeader = NULL;
	uint8 *inputBuf = NULL, *intermediate = NULL, *outputBuf = NULL;

	uint32 T = T_default, K = 0, R = 0;

	///////////////////////////////////////////////
	struct timeval t_start, t_end;
	long cost_t_sec, cost_t_usec;
	///////////////////////////////////////////////

	RParamEnc_t *para;
	para = (RParamEnc_t *)malloc(sizeof(RParamEnc_t));
	raptor_init(K_default, 0, para, 0);

	printf("encode_thread started...\n");

	while(1)
	{
		if(!frameBuf)
			frameBuf = (uint8 *)malloc(MAX_FRM_LEN);
		sem_wait(&encodeBuf->sem_full);
		frameSize = encodeBuf->frame[encodeBuf->sig_get]->size;
		frameNo = encodeBuf->frame[encodeBuf->sig_get]->frameNo;
		cameraNo = encodeBuf->frame[encodeBuf->sig_get]->cameraNo;
		memcpy(frameBuf, encodeBuf->frame[encodeBuf->sig_get]->buffer, frameSize);
		sem_post(&encodeBuf->sem_empty);
		encodeBuf->sig_get = (encodeBuf->sig_get + 1) % CODE_BUF_NUM;
		//printf("encodeBuf->sig_get=%d\n", encodeBuf->sig_get);
	
		///////////////////////////////////////////////////////
		gettimeofday(&t_start, NULL);
		///////////////////////////////////////////////////////

		K = (uint32)ceil(frameSize/T);
		if(K<5) K = 5;
		R = (uint32)ceil(K/2);

		raptor_reset(K, para, 0);

		if(!inputBuf)
			inputBuf = (uint8 *)malloc(MAX_FRM_LEN * 2);
		memset(inputBuf, 0, para->L*T);
		memcpy(inputBuf+(para->S+para->H)*T, frameBuf, frameSize);

		free(frameBuf);
		frameBuf = NULL;

		if(!intermediate)
			intermediate = (uint8*)malloc(MAX_FRM_LEN * 2);
		if(!outputBuf)
			outputBuf = (uint8 *)malloc(MAX_FRM_LEN * 2);

		if(raptor_encode(para, R, inputBuf, intermediate, outputBuf, T) == 0)
		{
			printf("encode error!\n");
			continue;
		}
	
		//////////////////////////////////////////////////////////////
		gettimeofday(&t_end, NULL);
		cost_t_sec = t_end.tv_sec - t_start.tv_sec;
		cost_t_usec = t_end.tv_usec - t_start.tv_usec;
		if (cost_t_usec < 0) {
			cost_t_usec += 1000000;
			cost_t_sec -= 1;
		}
		time_enc_temp = cost_t_sec + 0.000001 * cost_t_usec;
		time_dec = 0.875 * time_dec + 0.125 * (time_enc_temp + time_dec_temp);
		//////////////////////////////////////////////////////////////


		free(intermediate);
		intermediate = NULL;

		if(!frameHeader)
			frameHeader = (FrameHeader_t *)malloc(sizeof(FrameHeader_t));

		frameHeader->frame_no = htonl(frameNo);
		frameHeader->slice_no = htonl(frameNo);
		frameHeader->frame_type = htonl(1);
		frameHeader->F = htonl(frameSize);
		frameHeader->T = htonl(T);
		frameHeader->K = htonl(K);
		frameHeader->R = htonl(R);
		frameHeader->camera_no = htonl(cameraNo);

		int i = 0;
		for(i=0; i<K+R; i++)
		{
			frameHeader->esi = htonl(i);

			sem_wait(&sendBuf->sem_empty);
			memcpy(sendBuf->buffer[sendBuf->sig_put], frameHeader, sizeof(FrameHeader_t));
			memcpy(sendBuf->buffer[sendBuf->sig_put]+sizeof(FrameHeader_t), outputBuf+i*T, T);
			sem_post(&sendBuf->sem_full);
			sendBuf->sig_put = (sendBuf->sig_put+1) % SEND_BUF_NUM;
			//printf("sendBuf->sig_put=%d\n", sendBuf->sig_put);
		}//for
		free(outputBuf);
		outputBuf = NULL;
	}//while

	pthread_exit((void *)0);
}
