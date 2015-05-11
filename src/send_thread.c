#include "send_thread.h"

static void *send_thread(void *);

int send_func(uint8_t *IP, int Port, SendBuffer_t *sendBuf)
{
	int err = 0;
	pthread_t tid;
	int 			   sendfd;
	struct sockaddr_in servaddr;

	memset(&servaddr, '\0', sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(Port);

	if(inet_pton(AF_INET, IP, &servaddr.sin_addr) <= 0)
	{
		fprintf(stderr, "\tsend_func: inet_pton error for %s\n", IP);
		return -2;
	}

	if((sendfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("\tsend_func: UDP socket error");
		return -2;
	}

	DoubleArg_t arg = {sendBuf, &sendfd};
	err = pthread_create(&tid, NULL, send_thread, &arg);
	if(err)
	{
		fprintf(stderr, "\tcan't create send_thread: %s\n", strerror(err));
		return -2;
	}

	usleep(1000*200);
	return 0;
}

static void *send_thread(void *arg)
{
	SendBuffer_t *sendBuf = (SendBuffer_t *)((DoubleArg_t *)arg)->arg1;
	int sendfd = *(int *)((DoubleArg_t *)arg)->arg2;

	uint8_t *sendPac = (uint8_t *)malloc(1024);
	FrameHeader_t *frameHeader = (FrameHeader_t *)malloc(sizeof(FrameHeader_t));

	int T = 128;
	int sliceNum = 0;
	int nowLen = 0;
	int nowFrameNo = 0;
	int nextFrameFlag = 0;

	uint8_t *tempSlice = (uint8_t *)malloc(1024);
	uint8_t *nextFrame = (uint8_t *)malloc(1024);
	while(1)
	{
		sliceNum = 0;
		nowLen = 0;
		if(nextFrameFlag == 1){
        	memcpy(frameHeader, nextFrame, sizeof(FrameHeader_t));
        	nowFrameNo = ntohl(frameHeader->frame_no);
        	memcpy(sendPac, nextFrame, sizeof(FrameHeader_t)+T);

            sliceNum++;
		    nowLen = T + sizeof(FrameHeader_t) + sizeof(int);
		    nextFrameFlag = 0;
        }

		while(nowLen+T<=1024)
		{
			sem_wait(&sendBuf->sem_full);
			memcpy(tempSlice, sendBuf->buffer+sendBuf->sig_get*(T+sizeof(FrameHeader_t)), sizeof(FrameHeader_t)+T);
			sem_post(&sendBuf->sem_empty);
			sendBuf->sig_get = (sendBuf->sig_get + 1) % 20;

			memcpy(frameHeader, tempSlice, sizeof(FrameHeader_t));

			if(nowLen == 0)
			{
				memcpy(sendPac, tempSlice, sizeof(FrameHeader_t)+T);
				sliceNum++;
				nowLen = T + sizeof(FrameHeader_t) + sizeof(int);
			}
			else
			{
				if(nowFrameNo == ntohl(frameHeader->frame_no))
				{
	        		memcpy(sendPac+nowLen, tempSlice+sizeof(FrameHeader_t), T);
	        		sliceNum++;
	        	    nowLen += T;
	        	}
	        	else
	        	{
	        		nextFrameFlag = 1;
	        		memcpy(nextFrame, tempSlice, sizeof(FrameHeader_t)+T);
	        		break;
	        	}
			}
		}

		sliceNum = htonl(sliceNum-1);
		memcpy(sendPac+T+sizeof(FrameHeader_t), &sliceNum, sizeof(sliceNum));
		send(sendfd, sendPac, nowLen, 0);
	}

	pthread_exit((void *)0);
}
