#include "main.h"

void *decode_malloc(int buf_num, int buf_len)
{
	DecodeBuffer_t *buf = NULL;
	if((buf = (DecodeBuffer_t *)malloc(sizeof(DecodeBuffer_t))) == NULL)
		return NULL;
	if((buf->buffer = (uint8 **)malloc(buf_num*sizeof(uint8 *))) == NULL)
		return NULL;

	int i = 0;
	for(i=0; i<buf_num; i++)
	{
		if((buf->buffer[i] = (uint8 *)malloc(buf_len)) == NULL)
			return NULL;
		memset(buf->buffer[i], 0, buf_len);
	}

	sem_init(&buf->sem_empty, 0, buf_num);
	sem_init(&buf->sem_full, 0, 0);
	buf->sig_put = 0;
	buf->sig_get = 0;

	return (void *)buf;
}

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

void *send_malloc(int buf_num, int buf_len)
{
	SendBuffer_t *buf = NULL;
	if((buf = (SendBuffer_t *)malloc(sizeof(SendBuffer_t))) == NULL)
		return NULL;
	if((buf->buffer = (uint8 *)malloc(buf_len)) == NULL)
		return NULL;

	sem_init(&buf->sem_empty, 0, buf_num);
	sem_init(&buf->sem_full, 0, 0);
	buf->sig_put = 0;
	buf->sig_get = 0;

	return (void *)buf;
}

int main(int argc, char *argv[])
{
	DecodeBuffer_t *decodeBuf = NULL;
	EncodeBuffer_t *encodeBuf = NULL;
	SendBuffer_t *sendBuf = NULL;

	decodeBuf = (DecodeBuffer_t *)decode_malloc(MAXBUFSIZE, MAXLINE);
	encodeBuf = (EncodeBuffer_t *)encode_malloc(MAXBUFSIZE, 20000);
	sendBuf = (SendBuffer_t *)encode_malloc(MAXBUFSIZE, 20000);

	int recvPort = 8888;

	uint8 *sendIP = "10.103.254.241";
	int sendPort = 8889;


	if(recv_func(NULL, recvPort, decodeBuf) < 0) return -1;
	if(decode_func(decodeBuf, encodeBuf) < 0) return -1;
	if(encode_func(encodeBuf, sendBuf) < 0) return -1;
	if(send_func(sendIP, sendPort, sendBuf) < 0) return -1;

	while(1)
	{
		sleep(10);
	}
	return 0;
}

