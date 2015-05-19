#include "send_thread.h"

static void *send_thread(void *);

void *send_malloc(int buf_num, int buf_len)
{
	SendBuffer_t *buf = NULL;
	if((buf = (SendBuffer_t *)malloc(sizeof(SendBuffer_t))) == NULL)
		return NULL;
	if((buf->buffer = (uint8 **)malloc(buf_num*sizeof(uint8 *))) == NULL)
		return NULL;

	int i = 0;
	for(i=0; i<buf_num; i++)
	{
		if((buf->buffer[i] = (uint8 *)malloc(buf_len)) == NULL)
			return NULL;
	}

	sem_init(&buf->sem_empty, 0, buf_num);
	sem_init(&buf->sem_full, 0, 0);
	buf->sig_put = 0;
	buf->sig_get = 0;

	return (void *)buf;
}


int send_func(char *IP, int Port, SendBuffer_t *sendBuf)
{
	int err = 0;
	pthread_t tid;
	int 		    sendfd;
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

	if(connect(sendfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
	{
		perror("\tsend_func: udp connect error");
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

	uint8_t *sendPac = (uint8_t *)malloc(MAX_PAC_LEN);
	FrameHeader_t *frameHeader = (FrameHeader_t *)malloc(sizeof(FrameHeader_t));

	int T = T_default;
	int sliceNum = 0;
	int nowLen = 0;
	int nowFrameNo = 0;
	int nextFrameFlag = 0;

	uint8_t *tempSlice = (uint8_t *)malloc(MAX_PAC_LEN);
	uint8_t *nextFrame = (uint8_t *)malloc(MAX_PAC_LEN);

	printf("send_thread started...\n");
	while(1)
	{
		//printf("send_thread started...\n");
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

		while(nowLen+T<=MAX_PAC_LEN)
		{
			sem_wait(&sendBuf->sem_full);
			memcpy(tempSlice, sendBuf->buffer[sendBuf->sig_get], sizeof(FrameHeader_t)+T);
			sem_post(&sendBuf->sem_empty);
			sendBuf->sig_get = (sendBuf->sig_get + 1) % SEND_BUF_NUM;
			//printf("sendBuf->sig_get=%d\n", sendBuf->sig_get);

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
					memcpy(sendPac+nowLen, tempSlice + sizeof(FrameHeader_t), T);
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
