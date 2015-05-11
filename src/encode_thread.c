#include "encode_thread.h"

static void *encode_thread(void *);

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

	uint32 T = 128, K = 0, R = 0;

	RParamEnc_t *para;
	para = (RParamEnc_t *)malloc(sizeof(RParamEnc_t));
	raptor_init(500, 0, para, 0);

	while(1)
	{
		if(!frameBuf)
			frameBuf = (uint8 *)malloc(20000);
		sem_wait(&encodeBuf->sem_full);
		frameSize = encodeBuf->frame[encodeBuf->sig_get]->size;
		frameNo = encodeBuf->frame[encodeBuf->sig_get]->frameNo;
		cameraNo = encodeBuf->frame[encodeBuf->sig_get]->cameraNo;
		memcpy(frameBuf, encodeBuf->frame[encodeBuf->sig_get]->buffer, frameSize);
		sem_post(&encodeBuf->sem_empty);
		encodeBuf->sig_get = (encodeBuf->sig_get + 1) % MAXBUFSIZE;

		K = (uint32)ceil(frameSize/T);
		if(K<5) K = 5;
		R = (uint32)ceil(K/2);

		raptor_reset(K, para, 0);

		if(!inputBuf)
			inputBuf = (uint8 *)malloc(50000);
		memset(inputBuf, 0, para->L*T);
		memcpy(inputBuf+(para->S+para->H)*T, frameBuf, frameSize);

		free(frameBuf);
		frameBuf = NULL;

		if(!intermediate)
			intermediate = (uint8*)malloc(50100);
		if(!outputBuf)
			outputBuf = (uint8 *)malloc(50000);

		if(raptor_encode(para, R, inputBuf, intermediate, outputBuf, T) == 0)
		{
			printf("encode error!\n");
			continue;
		}

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
			memcpy(sendBuf->buffer+(T+sizeof(FrameHeader_t))*sendBuf->sig_put, frameHeader, sizeof(frameHeader));
			memcpy(sendBuf->buffer+(T+sizeof(FrameHeader_t))*sendBuf->sig_put+sizeof(FrameHeader_t), outputBuf+i*T, T);
			sendBuf->sig_put = (sendBuf->sig_put+1)%20;
			sem_post(&sendBuf->sem_full);
		}//for
		free(outputBuf);
		outputBuf = NULL;
	}//while

	pthread_exit((void *)0);
}
